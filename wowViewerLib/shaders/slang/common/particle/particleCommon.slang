#ifndef PARICLE_PARTICLE_COMMON_SLANG
#define PARICLE_PARTICLE_COMMON_SLANG

#include "../../common/animation.slang"
#include "./rndSeed.slang"

bool feq(const float a, const float b)
{
    const float ROUNDING_ERROR_f32 = 0.001f;
    const float tolerance = ROUNDING_ERROR_f32;

    return (a + tolerance >= b) && (a - tolerance <= b);
}

struct ParticleM2Data {
    int flags;

    float colorMult;
    float alphaMult;

    float followMult;
    float followBase;

    vec3 position;
    int bone;
    int emitterType;

    int multiTextureParamX[2];
    int textureTileRotation;
    int textureDimensions_rows;
    int textureDimensions_columns;

    float lifespanVary;
    float emissionRateVary;
    vec2 scaleVary;
    float tailLength;
    float TwinkleSpeed;
    float TwinklePercent;
    vec2 twinkleScale; //min, max
    float BurstMultiplier;
    float drag;
    float baseSpin;
    float baseSpinVary;
    float Spin;
    float spinVary;
    vec3 WindVector;
    float WindTime;
    float followSpeed1;
    float followScale1;
    float followSpeed2;
    float followScale2;

    vec2 multiTextureParam0[2];
    vec2 multiTextureParam1[2];

    //These tracks depend on particle's life
    M2AnimTrack colorTrack; //vec3
    M2AnimTrack alphaTrack; //float
    M2AnimTrack scaleTrack; //float
    M2AnimTrack headCellTrack; //int
    M2AnimTrack tailCellTrack; //int

};

struct GeneratorAniProp {
    float emissionSpeed;
    float speedVariation;
    float lifespan;
    float emissionRate;
    vec3 gravity;
    float zSource;
    float emissionAreaX;
    float emissionAreaY;
    //Latitude in 4.1.0
    float verticalRange;
    //Longitude in 4.1.0
    float horizontalRange;
    int isEnabled;
};

//Intermediate data for each particle instance
struct ParticlePreRenderData {
    vec3 m_particleCenter;
    struct
    {
        vec3 m_timedColor;
        float m_alpha;
        vec2 m_particleScale;
        int timedHeadCell;
        int timedTailCell;
        float alphaCutoff;
    } m_ageDependentValues;
};

struct CParticle2 {
    vec3 position;
    float age;
    vec3 velocity;
    int state;
    int seed;
    int isDead;
    vec2 texPos[2];
    vec2 texVel[2];
};

struct ParticleEmitterGPUState {
    float emission; 
    CRndSeed m_seed;
    vec3 m_prevPosition;
    float burstTime;
    vec3 burstVec;
    vec3 m_fullPosDeltaSinceLastUpdate;
    float m_inheritedScale;

    vec3 m_deltaPosition;
    mat4 m_emitterModelMatrix;
    
    uint numberOf10msStepUpdates;
    float endDeltaTime;

    int particlesCount; //current amount of particles
};

struct ParticleEmitterCPUState {
    GeneratorAniProp aniProp;
    uint maxAmount;                 // max amount of particles
    uint particleM2DataIndex;
}

#endif //PARICLE_PARTICLE_COMMON_SLANG