//
// Created by deamon on 20.12.17.
//

#include "CSplineGenerator.h"
#include "../../../algorithms/mathHelper.h"

CSplineGenerator::CSplineGenerator(CRndSeed &seed, M2Particle *particle, bool particlesGoUp) :
    CParticleGenerator(seed, particle), particlesGoUp(particlesGoUp), splineBezier3(particle->old.splinePoints) {

}


void CSplineGenerator::CreateParticle(CParticle2 &p, animTime_t delta) {
    float dvary = delta * this->seed.UniformPos();
    float life = this->seed.Uniform();


    int16_t state;
    if ( life < 1.0 )
    {
        if ( life > -1.0 )
            state = trunc((life * 32767.0f) + 0.5f);
        else
            state = -32767;
    }
    else
    {
        state = 32767;
    }
    p.state = state;

    float lifespan = this->GetLifeSpan(state);
    if (lifespan < 0.001) {
        lifespan = 0.001;
    }
//    p.age = 0.0;
    p.age = fmod(dvary, lifespan);
    p.seed = 0xffff & this->seed.uint32t();

    auto areaX = this->aniProp.emissionAreaX;
    if (areaX >= 0.0) {
        areaX = std::min(areaX, 1.0f);
    } else {
        areaX = 0.0;
    }

    auto areaY = this->aniProp.emissionAreaY;
    if (areaY >= 0.0) {
        areaY = std::min(areaY, 1.0f);
    } else {
        areaY = 0.0;
    }

    if (fabs(this->aniProp.emissionAreaY - this->m_areaY) >= 0.00000023841858f) {
        this->m_areaY = areaY;
    } else {
        areaY = (areaY - areaX) * this->seed.UniformPos() + areaX;
    }

    //Get particle position from spline using areaY as angle and C3Spline::Pos
    if (areaY > 0.0) {
        if (areaY < 1.0f ) {
            splineBezier3.posArclength(areaY, p.position);
        } else {
            p.position = splineBezier3.getLastPoint();
        }
    } else {
        p.position = splineBezier3.getFirstPoint();
    }

    float velocity = this->CalcVelocity();
    float zSource = this->aniProp.zSource;
    mathfu::vec3 resVelocityVector = mathfu::vec3(0,0,velocity);
    if (zSource > 0.001) {
        auto pos = p.position;
        float zdiff = pos.z - zSource;
        velocity = velocity / sqrt(pos.y*pos.y + pos.x * pos.x + zdiff*zdiff);
        resVelocityVector.x = pos.x * velocity;
        resVelocityVector.y = pos.y * velocity;
        resVelocityVector.z = zdiff * velocity;
    } else if (!feq(this->aniProp.verticalRange,0.0)){
        //Get vector from spline using areaY and C3Spline::Vel
        float t = areaY;
        t = std::max(t, 0.0f);
        t = std::min(t, 1.0f);
        splineBezier3.velArclength(t, p.position);

        mathfu::vec3 vec;
        vec = vec.Normalized();

        auto rotMat = mathfu::quat::FromAngleAxis(this->seed.Uniform() * aniProp.verticalRange, vec).ToMatrix();
        auto zVec = rotMat.GetColumn(2);
        if ( !feq(this->aniProp.horizontalRange,0.0) ) {
            p.position += (this->aniProp.horizontalRange * this->seed.UniformPos());
        }

        resVelocityVector = zVec * velocity;
    }
    p.velocity = resVelocityVector;

}

