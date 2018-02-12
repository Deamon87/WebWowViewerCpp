//
// Created by deamon on 14.12.17.
//

#ifndef WEBWOWVIEWERCPP_PARTICLEEMITTER_H
#define WEBWOWVIEWERCPP_PARTICLEEMITTER_H

#include <cstdlib>
#include <vector>
#include <x86intrin.h>
#include "../../persistance/header/M2FileHeader.h"
#include "generators/CParticleGenerator.h"
#include "generators/CSphereGenerator.h"
#include "generators/CPlaneGenerator.h"
#include "../../wowInnerApi.h"
#include "../../algorithms/mathHelper.h"

struct ParticleForces {
    mathfu::vec3 drift; // 0
    mathfu::vec3 velocity; // 3
    mathfu::vec3 position; // 6
    float drag; // 9
};

struct vectorMultiTex {
    float pos[3];
    float col[4];
    float texcoord[3][2];
};

class ParticleEmitter {
public:
    ParticleEmitter(IWoWInnerApi *api, M2Particle *particle, M2Data *data) : m_seed(0), m_api(api), m_m2Data(data) {

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
                std::cout << "Found unimplemented generator " << m_data->old.emitterType << std::flush;
                break;
        }

//        std::cout << "particleId = " << m_data->old.particleId
//                  << "Mentioned models :" << std::endl
//                  << "geometry_model_filename = " << m_data->old.geometry_model_filename.toString() << std::endl
//                  << "recursion_model_filename = " << m_data->old.recursion_model_filename.toString()
//                  << std::endl << std::endl << std::flush;

        const float followDen = m_data->old.followSpeed2 - m_data->old.followSpeed1;
        if (!feq(followDen, 0)) {
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
        this->texScaleX = 1.0f / cols;
        this->texScaleY = 1.0f / rows;

        if (m_data->old.flags & 0x10100000) {
            const bool isMultitex = (0 != (1 & (m_data->old.flags >> 0x1c)));
            if (isMultitex) {
                this->particleType = 2;
            }
            else {
                this->particleType = 3;
            }
        }
    }


    void Update(animTime_t delta, mathfu::mat4 transform);
    void prepearBuffers(mathfu::mat4 &viewMatrix);
    CParticleGenerator * getGenerator(){
        return generator;
    }
    void Render();

    int flags = 2;
    bool emittingLastFrame = false;
    bool isEnabled = false;
private:
    IWoWInnerApi *m_api;

    M2Particle *m_data;
    M2Data *m_m2Data;
    CRndSeed m_seed;

    mathfu::mat4 transform;
    mathfu::mat4 particleToView;
    mathfu::mat4 viewMatrix;

    CParticleGenerator *generator;

    std::vector<CParticle2> particles;

    mathfu::vec3 deltaPosition;

    int particleType = 0;

    float followMult;
    float followBase;

    animTime_t burstTime = 0;
    mathfu::vec3 burstVec;//(0,0,0);
    float inheritedScale = 1;
    float emission = 0;


    int32_t textureIndexMask;
    int32_t textureStartIndex;
    int32_t textureColBits;
    int32_t textureColMask;

    float texScaleX;
    float texScaleY;

    std::vector<uint8_t> szVertexBuf;
    std::vector<uint16_t> szIndexBuff;


private:

    CParticle2 &BirthParticle();

    void KillParticle(int index);

    bool UpdateParticle(CParticle2 &p, animTime_t delta, ParticleForces &forces);

    void CalculateForces(ParticleForces &forces, animTime_t delta);

    void CreateParticle(animTime_t delta);

    void EmitNewParticles(animTime_t delta);

    void Simulate(animTime_t delta);

    void resizeParticleBuffer();

    int RenderParticle(CParticle2 &p, std::vector<uint8_t> &szVertexBuf);

    void
    BuildQuadT3(
            std::vector<uint8_t> &szVertexBuf,
            mathfu::vec3 &m0, mathfu::vec3 &m1, mathfu::vec3 &viewPos, mathfu::vec3 &color, float alpha, float texStartX,
                float texStartY, mathfu::vec2 *texPos);

    void
    BuildQuad(
        std::vector<uint8_t> &szVertexBuf,
        mathfu::vec3 &m0, mathfu::vec3 &m1,
        mathfu::vec3 &viewPos, mathfu::vec3 &color, float alpha,
        float texStartX, float texStartY);


};



#endif //WEBWOWVIEWERCPP_PARTICLEEMITTER_H
