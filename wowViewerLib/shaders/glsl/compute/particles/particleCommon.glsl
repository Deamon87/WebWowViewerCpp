bool feq(const float a, const float b)
{
    const float ROUNDING_ERROR_f32 = 0.001f;

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

