//
// Created by deamon on 14.12.17.
//

#ifndef WEBWOWVIEWERCPP_PARTICLEEMITTER_H
#define WEBWOWVIEWERCPP_PARTICLEEMITTER_H

#include <cstdlib>
#include <vector>
#include <random>
#include <strings.h>
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


struct ParticleBuffStruct {
    C3Vector position; //0
    C4Vector color;    //12
    C2Vector textCoord0; //28
    C2Vector textCoord1; //36
    C2Vector textCoord2; //44
};

struct ParticleBuffStructQuad {
    ParticleBuffStruct particle[4];
};

class ParticleEmitter {
public:
    ParticleEmitter(IWoWInnerApi *api, M2Particle *particle, M2Object *m2Object) : m_seed(0), m_api(api), m2Object(m2Object) {

        if (!randTableInited) {
            for (int i = 0; i < 128; i++) {
                RandTable[i] = (float)std::rand() / (float)RAND_MAX;
            }
            randTableInited = true;
        }

        glGenBuffers(1, &indexVBO);
        glGenBuffers(1, &bufferVBO);


        m_data = particle;

        switch (m_data->old.emitterType) {
            case 1:
                this->generator = new CPlaneGenerator(this->m_seed, particle);
                break;
            case 2:
                this->generator = new CSphereGenerator(this->m_seed, particle, 0 != (m_data->old.flags & 0x100));
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
        this->textureColBits = __builtin_ffs(cols);
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

    static float RandTable[128];
    static bool randTableInited;
private:
    IWoWInnerApi *m_api;

    M2Particle *m_data;
    M2Object *m2Object;

    CRndSeed m_seed;

    mathfu::mat4 transform;
    mathfu::mat4 particleToView;
    mathfu::mat4 viewMatrix;
    mathfu::mat4 inverseViewMatrix;

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

    std::vector<ParticleBuffStructQuad> szVertexBuf;
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

    int RenderParticle(CParticle2 &p, std::vector<ParticleBuffStructQuad> &szVertexBuf);

    void
    BuildQuadT3(
            std::vector<ParticleBuffStructQuad> &szVertexBuf,
            mathfu::vec3 &m0, mathfu::vec3 &m1, mathfu::vec3 &viewPos, mathfu::vec3 &color, float alpha, float texStartX,
                float texStartY, mathfu::vec2 *texPos);

    void
    BuildQuad(
        std::vector<ParticleBuffStructQuad> &szVertexBuf,
        mathfu::vec3 &m0, mathfu::vec3 &m1,
        mathfu::vec3 &viewPos, mathfu::vec3 &color, float alpha,
        float texStartX, float texStartY);

    GLuint indexVBO;
    GLuint bufferVBO;
};



#endif //WEBWOWVIEWERCPP_PARTICLEEMITTER_H
