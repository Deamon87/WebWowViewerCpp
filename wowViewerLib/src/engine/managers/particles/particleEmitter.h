//
// Created by deamon on 14.12.17.
//

#ifndef WEBWOWVIEWERCPP_PARTICLEEMITTER_H
#define WEBWOWVIEWERCPP_PARTICLEEMITTER_H

#include <cstdlib>
#include <vector>
#include <random>
//#include <strings.h>
#include "../../persistance/header/M2FileHeader.h"
#include "../../wowInnerApi.h"
#include "../../algorithms/mathHelper.h"
#include "generators/CParticleGenerator.h"
#include "../../../gapi/interface/IDevice.h"

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

class ParticleEmitter {
public:
    ParticleEmitter(IWoWInnerApi *api, M2Particle *particle, M2Object *m2Object);

    void Update(animTime_t delta, mathfu::mat4 &transformMat, mathfu::vec3 invMatTransl, mathfu::mat4 *frameOfReference, mathfu::mat4 &viewMatrix);
    void prepearBuffers(mathfu::mat4 &viewMatrix);
    CParticleGenerator * getGenerator(){
        return generator;
    }
    void collectMeshes(std::vector<HGMesh> &meshes, int renderOrder);
    void updateBuffers() const;

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

    mathfu::vec3 deltaPosition;

    int particleType = 0;
    bool m_depthWrite = true;

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

    std::vector<ParticleBuffStructQuad> szVertexBuf;
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

    void resizeParticleBuffer();

    int RenderParticle(CParticle2 &p, std::vector<ParticleBuffStructQuad> &szVertexBuf);

    void fillTimedParticleData(CParticle2 &p, ParticlePreRenderData &particlePreRenderData, uint16_t seed);
    int CalculateParticlePreRenderData(CParticle2 &p, ParticlePreRenderData &ParticlePreRenderData);
    int buildVertex1(CParticle2 &p, ParticlePreRenderData &particlePreRenderData);
    int buildVertex2(CParticle2 &p, ParticlePreRenderData &particlePreRenderData);

    void InternalUpdate(animTime_t delta);
    void UpdateXform(const mathfu::mat4 &viewModelBoneMatrix, mathfu::vec3 &invMatTranslation, mathfu::mat4 *parentModelMatrix);

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

    struct {
        HGIndexBuffer m_indexVBO;
        HGVertexBuffer m_bufferVBO;

        HGVertexBufferBindings m_bindings;
        HGParticleMesh m_mesh;
    } frame[4];

    void createMesh();


    void GetSpin(CParticle2 &p, float &baseSpin, float &deltaSpin) const;
};



#endif //WEBWOWVIEWERCPP_PARTICLEEMITTER_H
