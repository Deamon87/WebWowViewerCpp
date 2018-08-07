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
#include "../../../gapi/GDevice.h"

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

    void Update(animTime_t delta, mathfu::mat4 &boneMatrix, mathfu::vec3 invMatTransl);
    void prepearBuffers(mathfu::mat4 &viewMatrix);
    CParticleGenerator * getGenerator(){
        return generator;
    }
    void collectMeshes(std::vector<HGMesh> &meshes);

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
    mathfu::vec3 m_invMatTransl;
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

    HGIndexBuffer m_indexVBO;
    HGVertexBuffer m_bufferVBO;

    HGVertexBufferBindings m_bindings;
    HGParticleMesh m_mesh;

    void createMesh();
};



#endif //WEBWOWVIEWERCPP_PARTICLEEMITTER_H
