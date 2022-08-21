//
// Created by Deamon on 2/14/2021.
//

#include "C3Spline_Bezier3.h"

mathfu::mat4 s_bezierCoeffs = {
    -1.0,3.0,-3.0,1.0,
    3.0,-6.0,3.0,0.0,
    -3.0,3.0,0.0,0.0,
    1.0,0,0,0
};

mathfu::mat4 s_bezierDer1Coeffs = {
    -3.0, 6.0, -3.0, 0,
    9.0, -12.0, 3.0, 0,
    -9.0, 6.0, 0.0, 0,
    3.0, 0.0, 0.0, 0
};

C3Spline_Bezier3::C3Spline_Bezier3(M2Array<C3Vector> &splinePoints) : C3Spline(splinePoints) {

    //Calculate segment lengths
    calcSegmentLength();
}

void C3Spline_Bezier3::posArclength(float angle, mathfu::vec3 &pos) {
    unsigned int segment;

    C3Spline_Bezier3::arclengthSegT(angle, &segment, &angle);
    C3Spline_Bezier3::evaluate(segment, angle, pos);
}

void C3Spline_Bezier3::velArclength(float t, mathfu::vec3 &vel) {
    unsigned int segment;

    C3Spline_Bezier3::arclengthSegT(t, &segment, &t);
    C3Spline_Bezier3::evaluateDer1(segment, t, vel);
}

void C3Spline_Bezier3::arclengthSegT(float s, unsigned int *seg, float *t) {
    C3Spline::arclengthSegT(s, &s_bezierCoeffs, this->m_splinePoints.size() / 3, seg, t);
}

void C3Spline_Bezier3::evaluate(unsigned int segment, float t, mathfu::vec3 &pos) {
    C3Spline::evaluate(3 * segment, t, s_bezierCoeffs, pos);
}

void C3Spline_Bezier3::evaluateDer1(unsigned int segment, float t, mathfu::vec3 &der) {
    C3Spline::evaluateDer1(3 * segment, t, s_bezierDer1Coeffs, der);
}

float C3Spline_Bezier3::segLength(unsigned int segment) {
    return C3Spline::segLength(3 *segment, s_bezierCoeffs);
}

void C3Spline_Bezier3::calcSegmentLength() {
    int segCount = m_splinePoints.size() / 3;
    segmentLengths.reserve(segCount);

    for (int i = 0; i < segCount; i++) {
        float segLength = C3Spline_Bezier3::segLength(i);
        segmentLengths.push_back(segLength);
    }

    for (float segmentLength : segmentLengths) {
        splineLength += segmentLength;
    }
}



