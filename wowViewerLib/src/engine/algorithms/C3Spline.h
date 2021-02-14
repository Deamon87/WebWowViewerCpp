//
// Created by Deamon on 2/14/2021.
//

#ifndef AWEBWOWVIEWERCPP_C3SPLINE_H
#define AWEBWOWVIEWERCPP_C3SPLINE_H


#include "mathHelper.h"

class C3Spline {
public:
    C3Spline(M2Array<C3Vector> &splinePoints);

protected:
    std::vector<C3Vector> m_splinePoints = {};
    std::vector<float> segmentLengths = {};
    float splineLength = 0;

    float segLength(unsigned int segment, mathfu::mat4 &coeffs);
    void evaluate(unsigned int segment, float t, mathfu::mat4 &coeffs, mathfu::vec3 &pos);
    void evaluateDer1(unsigned int segment, float t, mathfu::mat4 &coeffs, mathfu::vec3 &der);

    void arclengthSegT(float s, mathfu::mat4 *coeffs, unsigned int segCount, unsigned int *seg, float *t);

    virtual void calcSegmentLength() = 0;
};


#endif //AWEBWOWVIEWERCPP_C3SPLINE_H
