//
// Created by Deamon on 2/14/2021.
//

#ifndef AWEBWOWVIEWERCPP_C3SPLINE_BEZIER3_H
#define AWEBWOWVIEWERCPP_C3SPLINE_BEZIER3_H

#include "C3Spline.h"

class C3Spline_Bezier3 : public C3Spline {
public:
    C3Spline_Bezier3(M2Array<C3Vector> &splinePoints);

    void posArclength( float angle, mathfu::vec3 &pos);
    void velArclength(float t, mathfu::vec3 &vel);

    mathfu::vec3 getFirstPoint() {
        return mathfu::vec3(m_splinePoints[0]);
    };
    mathfu::vec3 getLastPoint() {
        return mathfu::vec3(m_splinePoints[m_splinePoints.size() - 1]);
    };
private:
    float segLength(unsigned int segment);

    void arclengthSegT(float s, unsigned int *seg, float *t);

    void evaluate(unsigned int segment, float t, mathfu::vec3 &pos);
    void evaluateDer1(unsigned int segment, float t, mathfu::vec3 &der);

    void calcSegmentLength() override;
};


#endif //AWEBWOWVIEWERCPP_C3SPLINE_BEZIER3_H
