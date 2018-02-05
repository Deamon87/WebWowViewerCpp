//
// Created by deamon on 14.12.17.
//

#include "particleEmitter.h"
#include "../../algorithms/mathHelper.h"
#include "../../algorithms/animate.h"

void ParticleEmitter::resizeParticleBuffer() {
    int newCount = this->generator->GetMaxEmissionRate() * this->generator->GetMaxLifeSpan() * 1.15;
    if (newCount > this->particles.size()) {
        this->particles.reserve(newCount);
    }
}

void ParticleEmitter::Update(animTime_t delta, mathfu::mat4 boneModelMat) {
    this->transform = boneModelMat;
    this->resizeParticleBuffer();
    mathfu::vec3 lastPos;
    mathfu::vec3 currPos;
    mathfu::vec3 dPos;

    //this->inheritedScale = Mat34.GetUniformScale(transform);
    dPos = lastPos - currPos;
    if (this->m_data->old.flags & 0x4000 > 0) {
        float x = this->followMult * (dPos.Length() / delta) + this->followBase;
        if (x < 0)
            x = 0;
        if (x > 1)
            x = 1;
        this->deltaPosition =  dPos * x;
    }
    if (this->m_data->old.flags & 0x40) {
        this->burstTime += delta;
        animTime_t frameTime = 30.0 / 1000.0;
        if (this->burstTime > frameTime) {
            if (this->particles.size() == 0) {
                animTime_t frameAmount = frameTime / this->burstTime;
                this->burstTime = 0;
                this->burstVec = dPos * frameAmount * this->m_data->old.BurstMultiplier;
            }
            else {
                this->burstVec = mathfu::vec3(0, 0, 0);
            }
        }
    }
    if (this->particles.size() > 0 && 0 == (this->flags & 16)) {
        delta += 5;
        this->flags |= 16;
    }
    if (delta > 0.1) {
        animTime_t clamped = delta;
        if (delta > 5) {
            clamped = 5;
        }
        for (float i = 0; i < clamped; i += 0.05) {
            animTime_t d = 0.05;
            if (clamped - i < 0.05) {
                d = clamped - i;
            }
            this->Simulate(d);
        }
    }
    else {
        this->Simulate(delta);
    }
}

void ParticleEmitter::Simulate(animTime_t delta) {
    ParticleForces forces;

    this->CalculateForces(forces, delta);
    this->EmitNewParticles(delta);
    for (int i = 0; i < this->particles.size(); i++) {
        auto &p = this->particles[i];
        p.age = p.age + delta;
        auto life = p.lifespan;
        if (p.age > this->generator->GetLifeSpan(life)) {
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
void ParticleEmitter::EmitNewParticles(animTime_t delta) {
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
void ParticleEmitter::CreateParticle(animTime_t delta) {
    CParticle2 &p = this->BirthParticle();
//            if (p == 0)
//            return;

    mathfu::vec3 r0 = mathfu::vec3(0,0,0);
    this->generator->CreateParticle(p, delta);
    if (!(this->m_data->old.flags & 0x10)) {
        p.position = (this->transform * mathfu::vec4(p.position, 1.0f)).xyz();
        p.velocity = (this->transform * mathfu::vec4(p.velocity, 0.0f)).xyz();
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

            mathfu::vec2 v2 = MathHelper::convertV69ToV2(this->m_data->multiTextureParam1[i]) * this->m_seed.Uniform();
            p.texVel[i] = v2  + MathHelper::convertV69ToV2(this->m_data->multiTextureParam0[i]);
        }
    }
}


void ParticleEmitter::CalculateForces(ParticleForces &forces, animTime_t delta) {
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

bool ParticleEmitter::UpdateParticle(CParticle2 p, animTime_t delta, ParticleForces &forces) {

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
        p.position = p.position + this->deltaPosition;
    }

    mathfu::vec3 r0 = p.velocity * delta; // v*dt

    p.velocity = p.velocity + forces.velocity;
    p.velocity = p.velocity *  (1 - forces.drag);

    p.position = p.position + r0;
    p.position = p.position + forces.position;

    if (this->m_data->old.emitterType == 2 && (this->m_data->old.flags & 0x80)) {
        mathfu::vec3 r1 = p.position;
        if (!(this->m_data->old.flags & 0x10)) {

            this->transform.GetColumn(3) = mathfu::vec4(r1, 1.0);
            r1 = p.position - r1;
        }
        if (mathfu::vec3::DotProduct(r1, r0) > 0) {
            return false;
        }
    }

    return true;
}

void ParticleEmitter::KillParticle(int index) {
    particles[index] = particles[particles.size()-1];
    particles.resize(particles.size() - 1);
}
CParticle2& ParticleEmitter::BirthParticle() {
    particles.resize(particles.size()+1);

    auto &p = particles[particles.size()-1];
    return p;
}

void ParticleEmitter::prepearBuffers(mathfu::mat4 &viewMatrix) {
    if (particles.size() == 0 && this->generator != nullptr) {
        return;
    }

    // Load textures at top so we can bail out early
    auto textureCache = m_api->getTextureCache();
    BlpTexture* tex0 = textureCache->get(
            m_m2Data->textures.getElement(
                    *m_m2Data->texture_lookup_table.getElement(this->m_data->old.texture_0))->filename.toString());

    if (tex0 == nullptr) {
        return;
    }

    bool multitex = this->particleType >= 2;
    if (multitex) {
        BlpTexture* tex1 = textureCache->get(
                m_m2Data->textures.getElement(
                        *m_m2Data->texture_lookup_table[this->m_data->old.texture_1])->filename.toString());
        BlpTexture* tex2 = textureCache->get(
                m_m2Data->textures.getElement(
                        *m_m2Data->texture_lookup_table[this->m_data->old.texture_2])->filename.toString());
    }


    if (this->m_data->old.flags & 0x10) {
        // apply the model transform
        this->particleToView = viewMatrix * this->transform;
    }
    else {
        this->particleToView = viewMatrix;
    }
    // Mat34.Col(this.particleNormal, viewMatrix, 2);
    // Build vertices for each particle

    // Vertex format: {float3 pos; float3 norm; float4 col; float2 texcoord} = 12
    // Multitex format: {float3 pos; float4 col; float2 texcoord[3]} = 13

    std::vector<vectorMultiTex> szVertexBuf;
    std::vector<uint16_t> szIndexBuff;
    // TODO: z-sort
    for (int i = 0; i < particles.size(); i++) {
        CParticle2 &p = this->particles[i];
        if (this->RenderParticle(p, szVertexBuf)) {
//            for (int j = 0; j < nIndices; j += 6) {
                // 0 2
                // 1 3
                // 0, 1, 2; 3, 2, 1
                szIndexBuff.push_back(0);
                szIndexBuff.push_back(1);
                szIndexBuff.push_back(2);
                szIndexBuff.push_back(3);
                szIndexBuff.push_back(2);
                szIndexBuff.push_back(1);
//            }
        }
    }
}

bool ParticleEmitter::RenderParticle(CParticle2 &p, std::vector<vectorMultiTex> &szVertexBuf) {
    float twinkle = this->m_data->old.TwinklePercent;
    auto twinkleRange = this->m_data->old.twinkleScale;

    float twinkleMin = twinkleRange.min;
    float twinkleVary = twinkleRange.max - twinkleMin;
    uint16_t seed = p.seed;
    animTime_t age = p.age;
    mathfu::vec3 pos = p.position;
    mathfu::vec3 vel = p.velocity;
    int rndIdx = 0;
    if (twinkle < 1 || twinkleVary != 0) {
        rndIdx = 0x7f & ((int)(age * this->m_data->old.TwinkleSpeed) + seed);
    }
//    if (twinkle < ParticleEmitter.RandTable[rndIdx]) {
//        return false;
//    }

    mathfu::quat quadRot;
    mathfu::vec3 viewPos;
    mathfu::vec3 viewVel;
    mathfu::vec3 m0;
    mathfu::vec3 m1;
    mathfu::vec3 scaleViewX;
    mathfu::vec3 scaleViewY;
    mathfu::vec3 viewUp;
    mathfu::vec3 screenVel;
    mathfu::vec2 screenVelNorm;


    mathfu::vec3 defaultColor(1.0f, 1.0f, 1.0f);
    mathfu::vec2 defaultScale(1.0f, 1.0f);
    float defaultAlpha = 1.0f;
    uint16_t defaultCell = 0;

    mathfu::vec3 color = animatePartTrack<C3Vector, mathfu::vec3>(p.age, &m_data->old.colorTrack, defaultColor);
    mathfu::vec2 scale = animatePartTrack<C2Vector, mathfu::vec2>(p.age, &m_data->old.scaleTrack, defaultScale);
    float alpha = animatePartTrack<fixed16, float>(p.age, &m_data->old.alphaTrack, defaultAlpha);
    uint16_t headCell = animatePartTrack<uint16_t, uint16_t>(p.age, &m_data->old.headCellTrack, defaultCell);
    uint16_t tailCell = animatePartTrack<uint16_t, uint16_t>(p.age, &m_data->old.tailCellTrack, defaultCell);


    CRndSeed rand(seed);

    float baseSpin = this->m_data->old.baseSpin + rand.Uniform() * this->m_data->old.baseSpinVary;
    float deltaSpin = this->m_data->old.Spin + rand.Uniform() * this->m_data->old.spinVary;
//    float weight = twinkleVary * ParticleEmitter.RandTable[rndIdx] + twinkleMin;
    float weight = twinkleVary * rand.Uniform() + twinkleMin;
    scale = scale * weight;
    if (this->m_data->old.flags & 0x20) {
        //Vec2.Mul(scale, scale, this.inheritedScale);
    }
    viewPos = this->particleToView * pos;

    if (this->m_data->old.flags & 0x20000) {
        // head cell
        float texStartX = headCell & this->textureColMask;
        float texStartY = headCell >> this->textureColBits;
        texStartX *= this->texScaleX;
        texStartY *= this->texScaleY;
        // 4x4 affine matrix, viewPos is w column, and the z column is 0
        // this multiplies our quad coords
        // these are the column 0 and 1 vectors:
        m0 = mathfu::vec3(0, 0, 0);
        m1 = mathfu::vec3(0, 0, 0);
        float theta = 0;
        if (this->m_data->old.Spin != 0 || this->m_data->old.spinVary != 0) {
            theta = baseSpin + deltaSpin * age;
            if ((this->m_data->old.flags & 0x200) && (seed & 1)) {
                theta = -theta;
            }
        }
        if ((this->m_data->old.flags & 4) && mathfu::vec3::DotProduct(vel,vel) > 0.0001) {
            // scale and rotation align to particle velocity
            viewVel = (this->particleToView * mathfu::vec4(vel, 0)).xyz();
            screenVel = viewVel;
            float screenVelMag = screenVel.Length();
            screenVelNorm = screenVel.xy() * (1 / screenVelMag);
            float scaleMod = screenVelMag / viewVel.Length();
            float scaleModX = scale.x * scaleMod;
            float scaleModY = scale.y * scaleMod;
            m0 = mathfu::vec3(screenVelNorm.x * scaleModX, screenVelNorm.y * scaleModX, 0);
            m1 = mathfu::vec3(-screenVelNorm.y * scaleModY, screenVelNorm.x * scaleModY, 0);
        }
        else if (this->m_data->old.flags & 0x1000) {
            // quad aligns to world-space ground plane instead of view plane
            // the viewPos is still correct, but the (x, y) offsets need to
            // be transformed as world-space offsets (and scaled/rotated)
            // 1. transform the scale vector (which is in XY) to view space
            float transformInvScale = 1;
            if (this->m_data->old.flags & 0x10) {
                // particleToView has the model transform in it, we need to
                // scale that away
                transformInvScale = 1 / this->inheritedScale;
            }
            scaleViewX = this->particleToView.GetColumn(0).xyz();
            scaleViewY =  this->particleToView.GetColumn(1).xyz();
            scaleViewX = scaleViewX * transformInvScale * scale.x;
            scaleViewY = scaleViewY * transformInvScale * scale.y);
            // 2. rotate the transformed scale vectors around the up vector
            if (theta != 0) {
                viewUp = this->particleToView.GetColumn(2).xyz();
                quadRot = mathfu::quat::FromAngleAxis(viewUp, theta);

                m0 = quadRot.ToMatrix4() * scaleViewX;
                m1 = quadRot.ToMatrix4() * scaleViewY;
            }
            else {
                m0 = scaleViewX;
                m1 = scaleViewY;
            }
        }
        else {
            if (theta != 0) {
                float cosTheta = cos(theta);
                float sinTheta = sin(theta);
                m0 = mathfu::vec3(cosTheta * scale.x, sinTheta * scale.x, 0);
                m0 = mathfu::vec3(-sinTheta * scale.y, cosTheta * scale.y, 0);
            }
            else {
                m0 = mathfu::vec3(scale.x, scale.y, 0);
            }
        }
        // build vertices from coords:
        if (this->particleType >= 2) {
            this->BuildQuadT3(m0, m1, viewPos, color, alpha, texStartX, texStartY, p.texPos);
        }
        else {
            this->BuildQuad(m0, m1, viewPos, color, alpha, texStartX, texStartY);
        }
    }

    return true;
}

void
ParticleEmitter::BuildQuadT3(mathfu::vec3 &m0, mathfu::vec3 &m1, mathfu::vec3 &viewPos, mathfu::vec3 &color, float alpha,
                             float texStartX, float texStartY, mathfu::vec2 *texPos) {

}
