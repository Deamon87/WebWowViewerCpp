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
    C3Vector position;   //0
    C4Vector color;      //12
    C2Vector textCoord0; //28
    C2Vector textCoord1; //36
    C2Vector textCoord2; //44
    float   alphaCutoff; //52
    float   padding[2];
};

static_assert(sizeof(ParticleBuffStruct) % 16 == 0, "ParticleBuffStruct must be aligned at 16 bytes");

struct ParticleBuffStructQuad {
    ParticleBuffStruct particle[4];
};

static_assert(sizeof(ParticleBuffStructQuad) == (sizeof(ParticleBuffStruct) * 4));
//static_assert(sizeof(ParticleBuffStruct) == 56);

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

typedef std::vector<CParticle2> ParticleBuffer;

class ParticleEmitter {
public:
    ParticleEmitter(const HApiContainer &api, const HMapSceneBufferCreate &sceneRenderer, M2Particle *particle, Exp2Record *exp2, M2Object *m2Object, HM2Geom geom, int txac_val_raw);
    ~ParticleEmitter() {
        delete generator;
    }

    void selectShaderId();
    void Update(animTime_t delta, const mathfu::mat4 &transformMat, mathfu::vec3 invMatTransl, mathfu::mat4 *frameOfReference, const mathfu::mat4 &viewMatrix);
    void prepearAndUpdateBuffers(const mathfu::mat4 &viewMatrix);
    CParticleGenerator * getGenerator(){
        return generator;
    }
    void collectMeshes(COpaqueMeshCollector &opaqueMeshCollector, transp_vec<HGSortableMesh> &transparentMeshes, int renderOrder);

    void fitBuffersToSize(const HMapSceneBufferCreate &sceneRenderer);

    int flags = 6;
    CParticleMaterialFlags materialFlags;
    bool emittingLastFrame = false;
    bool isEnabled = false;

    static float RandTable[128];
    static bool randTableInited;
private:
    HApiContainer m_api;

    M2Particle *m_data;
    Exp2Record *m_exp2Data = nullptr;
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
    mathfu::vec3 m_fullPosDeltaSinceLastUpdate;

    CParticleGenerator *generator;

    std::array<std::vector<CParticle2>, 2> particlesPingPong;

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

    int m_temp_maxFutureSize = 0;
private:

    struct ParticlePreRenderData
    {
        mathfu::vec3 m_particleCenter = mathfu::vec3(0,0,0);
        struct
        {
            mathfu::vec3 m_timedColor = mathfu::vec3(0,0,0);;
            float m_alpha = 0.0f;
            mathfu::vec2 m_particleScale = mathfu::vec2(0,0);
            uint32_t timedHeadCell = 0;
            uint32_t timedTailCell = 0;
            float alphaCutoff = 0.0;
        } m_ageDependentValues;
        int field_24;
        int field_28;
        int field_2C;
        int field_30;
        float invertDiagonalLen = 0.0;
    };

    bool UpdateParticle(CParticle2 &p, animTime_t delta, ParticleForces &forces);

    void calculateQuadToViewEtc(mathfu::mat4 *a1, const mathfu::mat4 &a3);

    void CalculateForces(ParticleForces &forces, animTime_t delta);

    void CreateParticle(ParticleBuffer &particlesCurr, animTime_t delta);

    void EmitNewParticles(ParticleBuffer &particlesCurr, animTime_t delta);

    void StepUpdate(ParticleBuffer &particlesCurr, ParticleBuffer &particlesLast, animTime_t delta);

    ParticleBuffer& GetLastPBuffer();
    ParticleBuffer& GetCurrentPBuffer();


    void fillTimedParticleData(CParticle2 &p, ParticlePreRenderData &particlePreRenderData, uint16_t seed);
    int CalculateParticlePreRenderData(CParticle2 &p, ParticlePreRenderData &ParticlePreRenderData);
    int buildVertex1(CParticle2 &p, ParticlePreRenderData &particlePreRenderData, ParticleBuffStruct *szVertexBuf, int &szVertexCnt);
    int buildVertex2(CParticle2 &p, ParticlePreRenderData &particlePreRenderData, ParticleBuffStruct *szVertexBuf, int &szVertexCnt);

    void InternalUpdate(ParticleBuffer &particlesCurr, ParticleBuffer &particlesLast, animTime_t delta);
    void UpdateXform(const mathfu::mat4 &viewModelBoneMatrix, mathfu::vec3 &invMatTranslation, mathfu::mat4 *parentModelMatrix);

    void
    BuildQuadT3(
            ParticleBuffStruct *szVertexBuf, int &szVertexCnt,
            mathfu::vec3 &m0, mathfu::vec3 &m1, mathfu::vec3 &viewPos, mathfu::vec3 &color, float alpha,
            float alphaCutoff,
            float texStartX,
            float texStartY, mathfu::vec2 *texPos);

	struct particleFrame {
		HGVertexBufferDynamic m_bufferVBO = nullptr;

		HGVertexBufferBindings m_bindings = nullptr;
		HGParticleMesh m_mesh = nullptr;
        int particlesUploaded = 0;
		bool active = false;
	} ;

    std::shared_ptr<IM2ParticleMaterial> m_material = nullptr;
    std::array<particleFrame, PARTICLES_BUFF_NUM> frame;


    void createMeshes(const HMapSceneBufferCreate &sceneRenderer);

    HGIndexBuffer m_indexVBO = nullptr;

    void GetSpin(CParticle2 &p, float &baseSpin, float &deltaSpin) const;
    void createMesh(const HMapSceneBufferCreate &sceneRenderer, particleFrame &frame, int frameIndex, int size);

};



#endif //WEBWOWVIEWERCPP_PARTICLEEMITTER_H
