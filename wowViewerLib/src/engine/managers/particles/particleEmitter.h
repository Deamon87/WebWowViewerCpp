//
// Created by deamon on 14.12.17.
//

#ifndef WEBWOWVIEWERCPP_PARTICLEEMITTER_H
#define WEBWOWVIEWERCPP_PARTICLEEMITTER_H

class ParticleEmitter;

#include <cstdlib>
#include <vector>
#include <random>
#include <array>
//#include <strings.h>
#include "../../persistance/header/M2FileHeader.h"
#include "../../algorithms/mathHelper.h"
#include "generators/CParticleGenerator.h"
#include "../../../gapi/interface/IDevice.h"
#include "../../ApiContainer.h"
#include "../../objects/m2/m2Object.h"

struct ParticleForces {
    mathfu::vec3 drift; // 0
    mathfu::vec3 velocity; // 3
    mathfu::vec3 position; // 6
    float drag; // 9
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

struct CParticleMaterialFlags {
    union {
        struct{
            uint8_t _0x01 : 1;
            uint8_t _0x02 : 1;
            uint8_t _0x04 : 1;
            uint8_t _0x08 : 1;
            uint8_t _0x10 : 1;
            uint8_t _0x20 : 1;
            uint8_t _0x40 : 1;
            uint8_t _0x80 : 1;
        } flags;
        uint8_t rawFlags;
    };
};

class ParticleEmitter {
public:
    ParticleEmitter(ApiContainer *api, M2Particle *particle, M2Object *m2Object, HM2Geom geom, int txac_val_raw);
    ~ParticleEmitter() {
        delete generator;
    }

    void selectShaderId();
    void Update(animTime_t delta, mathfu::mat4 &transformMat, mathfu::vec3 invMatTransl, mathfu::mat4 *frameOfReference, mathfu::mat4 &viewMatrix);
    void prepearBuffers(mathfu::mat4 &viewMatrix);
    CParticleGenerator * getGenerator(){
        return generator;
    }
    void collectMeshes(std::vector<HGMesh> &meshes, int renderOrder);
    void updateBuffers();

    int flags = 6;
    CParticleMaterialFlags materialFlags;
    bool emittingLastFrame = false;
    bool isEnabled = false;

    static float RandTable[128];
    static bool randTableInited;
private:
    ApiContainer *m_api;

    M2Particle *m_data;
    M2Object *m2Object;

    CRndSeed m_seed;

    mathfu::mat4 m_emitterModelMatrix;
    mathfu::mat4 viewMatrix;
    mathfu::mat4 inverseViewMatrix;

    mathfu::mat4 s_particleToView;
    C4Vector zUpVector;
    mathfu::mat3 p_quadToView;
    mathfu::vec3 p_quadToViewZVector;

    float m_inheritedScale = 1;

    mathfu::vec3 m_prevPosition;
    float m_currentBonePos;
    mathfu::vec3 m_deltaPosition;
    mathfu::vec3 m_deltaPosition1;

    CParticleGenerator *generator;

    std::vector<CParticle2> particles;

    int m_particleType = 0;
    int shaderId = 0;
    bool m_depthWrite = true;
    TXAC txac_particles_value;

    float followMult;
    float followBase;

    animTime_t burstTime = 0;
    mathfu::vec3 burstVec;//(0,0,0);

    mathfu::vec3 m_invMatTranslationVector;
    float emission = 0;


    uint32_t textureIndexMask;
    int32_t m_randomizedTextureIndexMask;
    int32_t textureColBits;
    int32_t textureColMask;

    float texScaleX;
    float texScaleY;

    ParticleBuffStructQuad *szVertexBuf = nullptr;
    int szVertexCnt = 0;
    std::vector<uint16_t> szIndexBuff;



private:

    struct ParticlePreRenderData
    {
        mathfu::vec3 m_particleCenter;
        struct
        {
            mathfu::vec3 m_timedColor;
            float m_alpha;
            mathfu::vec2 m_particleScale;
            uint32_t timedHeadCell;
            uint32_t timedTailCell;
            float exp2_unk3;
        } m_ageDependentValues;
        int field_24;
        int field_28;
        int field_2C;
        int field_30;
        float invertDiagonalLen;
    };

    CParticle2 &BirthParticle();


    void KillParticle(int index);

    bool UpdateParticle(CParticle2 &p, animTime_t delta, ParticleForces &forces);

    void calculateQuadToViewEtc(mathfu::mat4 *a1, mathfu::mat4 &a3);

    void CalculateForces(ParticleForces &forces, animTime_t delta);

    void CreateParticle(animTime_t delta);

    void EmitNewParticles(animTime_t delta);

    void StepUpdate(animTime_t delta);


    void fillTimedParticleData(CParticle2 &p, ParticlePreRenderData &particlePreRenderData, uint16_t seed);
    int CalculateParticlePreRenderData(CParticle2 &p, ParticlePreRenderData &ParticlePreRenderData);
    int buildVertex1(CParticle2 &p, ParticlePreRenderData &particlePreRenderData);
    int buildVertex2(CParticle2 &p, ParticlePreRenderData &particlePreRenderData);

    void InternalUpdate(animTime_t delta);
    void UpdateXform(const mathfu::mat4 &viewModelBoneMatrix, mathfu::vec3 &invMatTranslation, mathfu::mat4 *parentModelMatrix);

    void
    BuildQuadT3(
            mathfu::vec3 &m0, mathfu::vec3 &m1, mathfu::vec3 &viewPos, mathfu::vec3 &color, float alpha, float texStartX,
                float texStartY, mathfu::vec2 *texPos);

	struct particleFrame {
		HGVertexBufferDynamic m_bufferVBO = nullptr;

		HGVertexBufferBindings m_bindings = nullptr;
		HGParticleMesh m_mesh = nullptr;
		bool active = false;
	} ;
    std::array<particleFrame, 4> frame;

    void createMesh();

    static HGIndexBuffer m_indexVBO;

    void GetSpin(CParticle2 &p, float &baseSpin, float &deltaSpin) const;
};



#endif //WEBWOWVIEWERCPP_PARTICLEEMITTER_H
