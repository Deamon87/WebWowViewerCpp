//
// Created by deamon on 20.12.17.
//

#ifndef WEBWOWVIEWERCPP_CSPLINEGENERATOR_H
#define WEBWOWVIEWERCPP_CSPLINEGENERATOR_H


#include "CParticleGenerator.h"
#include "../../../algorithms/C3Spline_Bezier3.h"

class CSplineGenerator : public CParticleGenerator {
public:
    CSplineGenerator(CRndSeed &seed, M2Particle *particle, bool particlesGoUp);
    ~CSplineGenerator() override {} ;

    void CreateParticle(CParticle2 &p, animTime_t delta) override;

private:
    C3Spline_Bezier3 splineBezier3;

    bool particlesGoUp = false;
    float m_areaY = 0;
};

#endif //WEBWOWVIEWERCPP_CSPLINEGENERATOR_H
