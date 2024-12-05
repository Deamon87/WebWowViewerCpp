#include "particleCommon.slang"
#include "rndSeed.slang"

float GetEmissionRate(in GeneratorAniProp aniProp, in ParticleM2Data m_particleData, inout CRndSeed seed) {
    float rate = aniProp.emissionRate;
    return rate + seed.Uniform() * m_particleData.emissionRateVary;
}

float GetMaxEmissionRate(in GeneratorAniProp aniProp, in ParticleM2Data m_particleData) {
    return aniProp.emissionRate + m_particleData.emissionRateVary;
}

float GetMaxLifeSpan(in GeneratorAniProp aniProp, in ParticleM2Data m_particleData) {
    return aniProp.lifespan + m_particleData.lifespanVary;
}

float GetLifeSpan(in GeneratorAniProp aniProp, in ParticleM2Data m_particleData, int state) {
    return aniProp.lifespan + (float(state) * 0.000030518509f) * m_particleData.lifespanVary;
}

float CalcVelocity(in GeneratorAniProp aniProp, in ParticleM2Data m_particleData, inout CRndSeed seed) {
    float velocity = aniProp.emissionSpeed;
    velocity *= 1.0f + aniProp.speedVariation * seed.Uniform();
    return velocity;
}

vec3 GetGravity(in GeneratorAniProp aniProp, in ParticleM2Data m_particleData) {
    return vec3(aniProp.gravity);
}



void PlaneGenerator__CreateParticle(
    //Generator state
    in GeneratorAniProp aniProp,
    in ParticleM2Data m_particleData,
    inout CRndSeed seed,

    //Actual parameters
    inout CParticle2 p,
    float delta
) {
    float dvary = (delta * seed.UniformPos());
    float life = seed.Uniform();
    int state;
    if ( life < 1.0 )
    {
        if ( life > -1.0 )
            state = int(trunc((life * 32767.0f) + 0.5f));
        else
            state = -32767;
    }
    else
    {
        state = 32767;
    }
    p.state = state;

    float lifespan = GetLifeSpan(aniProp, m_particleData, state);
    if (lifespan < 0.001) {
        lifespan = 0.001;
    }
//    p.age = 0.0;
    p.age = fmod(dvary, lifespan);

    p.seed = (0xffff & seed.uint32t());
    p.position = vec3(
            seed.Uniform() * aniProp.emissionAreaX * 0.5f,
            seed.Uniform() * aniProp.emissionAreaY * 0.5f,
            0
    );
    float velocity = CalcVelocity(aniProp, m_particleData, seed);
    float zSource = aniProp.zSource;

    if (zSource < 0.001) {
        float polar = aniProp.verticalRange * seed.Uniform();
        float azimuth = aniProp.horizontalRange * seed.Uniform();
        float sinPolar = sin(polar);
        float sinAzimuth = sin(azimuth);
        float cosPolar = cos(polar);
        float cosAzimuth = cos(azimuth);
        p.velocity = vec3(
            cosAzimuth * sinPolar * velocity,
            sinAzimuth * sinPolar * velocity,
            cosPolar * velocity
        );
    }
    else {
        vec3 r0 = p.position - vec3(0, 0, zSource);
        if (length(r0) > 0.0001) {
            r0 = normalize(r0);
            p.velocity = r0 * velocity;
        }
    }

//    mathfu::mat4 rotateMat = MathHelper::RotationY(toRadian(-90));

//    p.position = mathfu::vec3(p.position.y, p.position.x, p.position.z);
//    p.velocity = rotateMat.Transpose() * p.velocity;
}

void CSphereGenerator__CreateParticle(
    //Generator state
    in GeneratorAniProp aniProp,
    in ParticleM2Data m_particleData,
    inout CRndSeed seed,

    //Actual parameters
    inout CParticle2 p,
    float delta
) {
    bool particlesGoUp = (m_particleData.flags & 0x100) > 0;

    float dvary = delta * seed.UniformPos();
    float life = seed.Uniform();

    int state;
    if ( life < 1.0 )
    {
        if ( life > -1.0 )
            state = int(trunc((life * 32767.0f) + 0.5f));
        else
            state = -32767;
    }
    else
    {
        state = 32767;
    }
    p.state = state;

    float lifespan = GetLifeSpan(aniProp, m_particleData, state);
    if (lifespan < 0.001) {
        lifespan = 0.001;
    }
//    p.age = 0.0;
    p.age = fmod(dvary, lifespan);
    p.seed = 0xffff & seed.uint32t();
    float emissionArea = aniProp.emissionAreaY - aniProp.emissionAreaX;
    float radius = aniProp.emissionAreaX + emissionArea * seed.UniformPos();
    const float polar = aniProp.verticalRange * seed.Uniform();
    const float azimuth = aniProp.horizontalRange * seed.Uniform();
    float cosPolar = cos(polar);
    vec3 emissionDirection = vec3(
            cosPolar * cos(azimuth),
            cosPolar * sin(azimuth),
            sin(polar)
    );

    p.position = emissionDirection * radius;
    float velocity = CalcVelocity(aniProp, m_particleData, seed);
    float zSource = aniProp.zSource;

    vec3 r0 = vec3(0.5,0.5,0.5);
    if (feq(zSource, 0)) {
        if (particlesGoUp) {
            r0 = vec3(0,0,1);
        } else {
            r0 = vec3(cosPolar * cos(azimuth), cosPolar * sin(azimuth), sin(polar));
        }
    }
    else {
        r0 = vec3(0, 0, zSource);
        r0 = p.position - r0;
        if (length(r0) > 0.0001) {
            r0 = normalize(r0);
        }
    }
    vec3 velocityCandidate = r0 * velocity;
    p.velocity = velocityCandidate;


//    mathfu::mat4 rotateMat = MathHelper::RotationY(toRadian(-90));
//
//    p.position = rotateMat * p.position;
//    p.velocity = rotateMat.Transpose().Inverse() * p.velocity;

}
