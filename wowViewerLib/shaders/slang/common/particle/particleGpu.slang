//
// GPU particle simulation: shared data structures.
// Mirrors engine/objects/m2/m2Helpers/M2GpuAnimData.h (C++) 1:1 — all
// buffers are ScalarDataLayout so offsets match the C++ structs exactly.
//
#ifndef PARTICLE_GPU_SLANG
#define PARTICLE_GPU_SLANG

import glsl;
#include "../m2TrackEval.slang"

// GpuM2ParticleStatic (C++), 368 bytes
struct GpuM2ParticleStatic {
    uint flags;
    int bone;
    float position[3];
    int emitterType;
    int blendingType;
    int particleColorIndex;

    float lifespanVary;
    float emissionRateVary;
    float scaleVary[2];

    float tailLength;
    float twinkleSpeed;
    float twinklePercent;
    float twinkleScaleMin;
    float twinkleScaleMax;
    float burstMultiplier;
    float drag;
    float baseSpin;
    float baseSpinVary;
    float spin;
    float spinVary;

    float windVector[3];
    float windTime;

    float followSpeed1;
    float followScale1;
    float followSpeed2;
    float followScale2;
    float followMult;
    float followBase;

    int textureColMask;
    int textureColBits;
    int textureIndexMask;
    int particleType;
    float texScaleX;
    float texScaleY;

    float multiTextureParam0_0[2];
    float multiTextureParam0_1[2];
    float multiTextureParam1_0[2];
    float multiTextureParam1_1[2];
    float multiTextureParamX0;
    float multiTextureParamX1;

    float zSourceExp2;
    int hasExp2;

    int emissionSpeedTrack;
    int speedVariationTrack;
    int verticalRangeTrack;
    int horizontalRangeTrack;
    int gravityTrack;
    int gravityIsCompressed;
    int lifespanTrack;
    int emissionRateTrack;
    int emissionAreaXTrack;
    int emissionAreaYTrack;
    int zSourceTrack;
    int enabledInTrack;

    GpuM2SeqSlice colorTrackSlice;
    GpuM2SeqSlice alphaTrackSlice;
    GpuM2SeqSlice scaleTrackSlice;
    GpuM2SeqSlice headCellTrackSlice;
    GpuM2SeqSlice tailCellTrackSlice;
    GpuM2SeqSlice alphaCutoffSlice;

    int pad[5];
};

// GpuM2ParticleFrameProps (C++), 64 bytes
struct GpuM2ParticleFrameProps {
    float emissionSpeed;
    float speedVariation;
    float verticalRange;
    float horizontalRange;
    float gravity[3];
    float lifespan;
    float emissionRate;
    float emissionAreaX;
    float emissionAreaY;
    float zSource;
    int isEnabled;
    int pad[3];
};

// GpuParticleState (C++), 320 bytes
struct GpuParticleState {
    float emitterModelMatrix[16];
    float s_particleToView[16];
    float4 p_quadToView0;
    float4 p_quadToView1;
    float4 p_quadToView2;
    float4 p_quadToViewZVector;

    float prevPosition[3];
    float burstTime;
    float burstVec[3];
    float inheritedScale;
    float fullPosDelta[3];
    int curBuf;
    float deltaPosition[3];
    int particleCount;

    float emission;
    uint emitterSeedValue;
    uint emitterSeedAccum;
    uint generatorSeedValue;
    uint generatorSeedAccum;

    int staticsIndex;
    int propsIndex;
    int particleOffset;  // pool-absolute first element in particlesA
    int capacity;
    int placementMatIndex;
    int boneOutOffset;
    int randomizedTextureIndexMask;
    int particleOffsetB; // pool-absolute first element in particlesB (may differ from A)
    int pad[3];
};

// GpuParticle (C++), 80 bytes
struct GpuParticle {
    float position[3];
    float age;
    float velocity[3];
    int state;
    float texPos0[2];
    float texPos1[2];
    float texVel0[2];
    float texVel1[2];
    uint seed;
    int isDead;
    int pad0;
    int pad1;
};

// Binding layout shared by the particle sim compute pass and the pull-model
// particle vertex shader (both include this file).
struct GpuParticleSimBindings {
    // set=3 on the vertex shader, set=0 on the compute pass — declared per shader.
};

// float[3] fields are used instead of vec3 so ScalarDataLayout matches the C++
// structs (slang aligns vec3 to 16 even in scalar layout).
vec3 arrToVec3(in float a[3]) { return vec3(a[0], a[1], a[2]); }
void vec3ToArr(in vec3 v, out float a[3]) { a[0] = v.x; a[1] = v.y; a[2] = v.z; }
vec2 arrToVec2(in float a[2]) { return vec2(a[0], a[1]); }
void vec2ToArr(in vec2 v, out float a[2]) { a[0] = v.x; a[1] = v.y; }

#endif // PARTICLE_GPU_SLANG
