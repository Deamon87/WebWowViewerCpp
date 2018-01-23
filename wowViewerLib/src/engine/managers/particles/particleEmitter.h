//
// Created by deamon on 14.12.17.
//

#ifndef WEBWOWVIEWERCPP_PARTICLEEMITTER_H
#define WEBWOWVIEWERCPP_PARTICLEEMITTER_H

#include <cstdlib>
#include "../../persistance/header/M2FileHeader.h"
#include "generators/CParticleGenerator.h"
#include "generators/CSphereGenerator.h"
#include "generators/CPlaneGenerator.h"

struct ParticleForces {
    mathfu::vec3 drift; // 0
    mathfu::vec3 velocity; // 3
    mathfu::vec3 position; // 6
    float drag; // 9
};

class ParticleEmitter {
public:
    ParticleEmitter(M2Particle *particle) : m_seed(0) {

        m_data = particle;

        switch (m_data->old.emitterType) {
            case 1:
                this->generator = new CPlaneGenerator(this->m_seed);
                break;
            case 2:
                this->generator = new CSphereGenerator(this->m_seed, 0 != (m_data->old.flags & 0x100));
                break;
            default:
                this->generator = nullptr;
                break;
        }

        const float followDen = m_data->old.followSpeed2 - m_data->old.followSpeed1;
        if (followDen != 0) {
            this->followMult = (m_data->old.followScale2 - m_data->old.followScale1) / followDen;
            this->followBase = m_data->old.followScale1 - m_data->old.followSpeed1 * this->followMult;
        }
        else {
            this->followMult = 0;
            this->followBase = 0;
        }

        uint16_t cols = m_data->old.textureDimensions_columns;
        if (cols <= 0) {
            cols = 1;
        }
        uint16_t rows = m_data->old.textureDimensions_rows;
        if (rows <= 0) {
            rows = 1;
        }
        this->textureIndexMask = cols * rows - 1;
        this->textureStartIndex = 0;
        this->textureColBits = _bit_scan_reverse(cols);
        this->textureColMask = cols - 1;
        if (m_data->old.flags & 0x200000) {
            this->textureStartIndex = this->m_seed.uint32t() & this->textureIndexMask;
        }
        this->texScaleX = 1 / cols;
        this->texScaleY = 1 / rows;
        if (m_data->old.flags & 0x10100000) {
            const bool isMultitex = 0 != (1 & (m_data->old.flags >> 0x1c));
            if (isMultitex) {
                this->particleType = 2;
            }
            else {
                this->particleType = 3;
            }
            this->particleSize = 17;
        }
    }

public:
    void resizeParticleBuffer() {
        int newCount = this->generator->GetMaxEmissionRate() * this->generator->GetMaxLifeSpan() * 1.15;
        if (newCount > this->particles.size()) {
            this->particles.reserve(newCount);
        }
    }
    void Update(animTime_t delta) {
//        this->resizeParticleBuffer();
//        const currPos = mem.Push(3);
//        let lastPos = mem.Push(3);
//        const dPos = mem.Push(3);
//        Mat34.ToT(transform, currPos);
//        if (this.transform == null) {
//            lastPos = currPos;
//        }
//        else {
//            Mat34.ToT(this.transform, lastPos);
//        }
//        Mat34.Copy(this.transform, transform);
//        this.inheritedScale = Mat34.GetUniformScale(transform);
//        Vec3.Sub(dPos, currPos, lastPos);
//        if (this.data.Flags & 0x4000) {
//            let x = this.followMult * (Vec3.Length(dPos) / delta) + this.followBase;
//            if (x < 0)
//                x = 0;
//            if (x > 1)
//                x = 1;
//            Vec3.Mul(this.deltaPosition, dPos, x);
//        }
//        if (this.data.Flags & 0x40) {
//            this.burstTime += delta;
//            const frameTime = 30 / 1000;
//            if (this.burstTime > frameTime) {
//                if (this->particles.size() == 0) {
//                    const frameAmount = frameTime / this.burstTime;
//                    this.burstTime = 0;
//                    Vec3.Mul(this.burstVec, dPos, frameAmount * this.data.BurstMultiplier);
//                }
//                else {
//                    Vec3.Set(this.burstVec, 0, 0, 0);
//                }
//            }
//        }
//        if (this->particles.size() > 0 && 0 == (this->flags & 16)) {
//            delta += 5;
//            this->flags |= 16;
//        }
//         if (delta > 0.1) {
//            animTime_t clamped = delta;
//            if (delta > 5) {
//                clamped = 5;
//            }
//            for (int i = 0; i < clamped; i += 0.05) {
//                animTime_t d = 0.05;
//                if (clamped - i < 0.05) {
//                    d = clamped - i;
//                }
//                this->Simulate(d);
//            }
//        }
//        else {
//            this->Simulate(delta);
//        }
    }

    void Simulate(animTime_t delta) {
        ParticleForces forces;

        this->CalculateForces(forces, delta);
        this->EmitNewParticles(delta);
        for (int i = 0; i < this->particles.size(); i++) {
            auto &p = this->particles[i];
            p.age = p.age + delta;
            auto life = p.lifespan;
            if (life > this->generator->GetLifeSpan(life)) {
                this->KillParticle(i);
                i--;
            }
            else {
                if (!this->UpdateParticle(p, delta, forces)) {
                    this->KillParticle(i);
                    i--;
                }
            }
        }
    }
    void EmitNewParticles(animTime_t delta) {
            float rate = this->generator->GetEmissionRate();
            if (6 == (this->flags & 6)) {
                int count = rate ;
                for (int i = 0; i < count; i++) {
                    this->CreateParticle(0);
                }
            }
            if (3 == (this->flags & 3)) {
                this->emission += delta * rate;
                while (this->emission > 1) {
                    this->CreateParticle(delta);
                    this->emission -= 1;
                }
            }
    }
    void CreateParticle(animTime_t delta) {
        CParticle2 &p = this->BirthParticle();
//            if (p == 0)
//            return;

        mathfu::vec3 r0 = mathfu::vec3(0,0,0);
        this->generator->CreateParticle(p, delta);
        if (!(this->m_data->old.flags & 0x10)) {
            p.position = this->transform * p.position;
            p.velocity = this->transform * p.velocity;
            if (this->m_data->old.flags & 0x2000) {
                // Snap to ground; set z to 0:
                p.position.z = 0.0;
            }
        }
        if (this->m_data->old.flags & 0x40) {
            float speedMul = 1 + this->generator->GetSpeedVariation() * this->m_seed.Uniform();
            //r0 = this->burstVec * speedMul;
            p.velocity += r0;
        }
        if (this->particleType >= 2) {
            for (int i = 0; i < 2; i++) {
                p.texPos[i].x = this->m_seed.UniformPos();
                p.texPos[i].y = this->m_seed.UniformPos();

                mathfu::vec2 v2 = convertV69ToV2(this->m_data->multiTextureParam1[i]) * this->m_seed.Uniform();
                p.texVel[i] = v2  + convertV69ToV2(this->m_data->multiTextureParam0[i]);
            }
        }
    }


    void CalculateForces(ParticleForces &forces, animTime_t delta) {
        if (false && (this->m_data->old.flags & 0x80000000)) {
            forces.drift = mathfu::vec3(0.707, 0.707, 0);
            forces.drift = forces.drift * delta;
        }
        else {
            forces.drift = mathfu::vec3(this->m_data->old.WindVector) * delta;
        }

        auto g = this->generator->GetGravity();
        forces.velocity = g * delta;
        forces.position = g * delta * delta * 0.5;
        forces.drag = this->m_data->old.drag * delta;
    }

    bool UpdateParticle(CParticle2 p, animTime_t delta, ParticleForces &forces) {

        if (this->particleType >= 2) {
            for (int i = 0; i < 2; i++) {
                // s = frac(s + v * dt)
                float val = (float) (p.texPos[i].x + delta * p.texVel[i].x);
                p.texPos[i].x = (float) (val - floor(val));

                val = (float) (p.texPos[i].y + delta * p.texVel[i].y);
                p.texPos[i].y = (float) (val - floor(val));
            }
        }

        p.velocity = p.velocity + forces.drift;
        if ((this->m_data->old.flags & 0x4000) && 2 * delta < p.age) {
            p.position = p.position + this->deltaPosition);
        }

        mathfu::vec3 r0 = p.velocity * delta; // v*dt

        p.velocity = p.velocity + forces.velocity;
        p.velocity = p.velocity *  (1 - forces.drag);

        p.position = p.position + r0;
        p.position = p.position + forces.position;

        if (this->m_data->old.emitterType == 2 && (this->m_data->old.flags & 0x80)) {
            mathfu::vec3 r1 = p.position;
            if (!(this->m_data->old.flags & 0x10)) {
                Mat34.ToT(this.transform, r1);
                r1 = pos - r1;
            }
            if (mathfu::vec3::DotProduct(r1, r0) > 0) {
                return false;
            }
        }

        return true;
    }

    void KillParticle(int index) {
        particles[index] = particles[particles.size()-1];
        particles.resize(particles.size() - 1);
    }
    CParticle2& BirthParticle() {
        particles.resize(particles.size()+1);

        auto &p = particles[particles.size()-1];
        return p;
    }

private:
    M2Particle *m_data;
    CRndSeed m_seed;

    CParticleGenerator *generator;

    std::vector<CParticle2> particles;

    mathfu::vec3 deltaPosition;

    int particleType;

    float followMult;
    float followBase;

    animTime_t burstTime = 0;
    float inheritedScale = 1;
    float emission = 0;
    bool emittingLastFrame = false;

    int32_t textureIndexMask;
    int32_t textureStartIndex;
    int32_t textureColBits;
    int32_t textureColMask;

    float texScaleX;
    float texScaleY;

    int particleSize;
    int flags;
};


#endif //WEBWOWVIEWERCPP_PARTICLEEMITTER_H
