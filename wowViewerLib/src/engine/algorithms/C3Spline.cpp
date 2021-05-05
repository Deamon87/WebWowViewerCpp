//
// Created by Deamon on 2/14/2021.
//

#include "C3Spline.h"



C3Spline::C3Spline(M2Array<C3Vector> &splinePoints) {
    if (splinePoints.size > 0) {
        auto firstElem = splinePoints.getElement(0);
        m_splinePoints = std::vector<C3Vector>(firstElem, firstElem+splinePoints.size);
    }
}

float C3Spline::segLength(unsigned int segment, mathfu::mat4 &coeffs) {

    float t = 0.050000001;
    float length = 0.0;

    mathfu::vec3 curPos = {0, 0, 0};
    mathfu::vec3 nextPos = {0, 0, 0};

    this->evaluate(segment, 0.0, coeffs, curPos);
    unsigned int segmenta = 20;
    do
    {
        this->evaluate(segment, t, coeffs, nextPos);
        float coeffsa = (nextPos.z - curPos.z) * (nextPos.z - curPos.z)
                  + (nextPos.y - curPos.y) * (nextPos.y - curPos.y)
                  + (nextPos.x - curPos.x) * (nextPos.x - curPos.x);

        assert(coeffsa >= 0.0);

        curPos = nextPos;
        length = sqrt(coeffsa) + length;
        t = t + 0.050000001;
    }
    while ( segmenta-- != 1 );

    return length;
}

double EvaluatePolynomial(unsigned int count, float *coefficients, float t)
{
    double result = *coefficients;
    unsigned int i = 1;
    if ( count )
    {
        do
            result = result * t + coefficients[i++];
        while ( i <= count );
    }
    return result;
}

void C3Spline::evaluate(unsigned int segment, float t, mathfu::mat4 &coeffs, mathfu::vec3 &pos) {
    pos = {0, 0, 0};

    for (int i = 0; i < 4; i++) {
        assert(segment < m_splinePoints.size());

        auto column = coeffs.GetColumn(i);
        C3Vector &segmentVec = m_splinePoints[segment];
        float val = EvaluatePolynomial(3, column.data_, t);

        pos += mathfu::vec3(segmentVec) * val;

        ++segment;
    }
}


void C3Spline::evaluateDer1(unsigned int segment, float t, mathfu::mat4 &coeffs, mathfu::vec3 &der) {
    der = {0,0,0};

    for (int i = 0; i < 4; i++) {
        assert (segment < this->m_splinePoints.size());

        auto column = coeffs.GetColumn(i);

        float val = EvaluatePolynomial(2, column.data_, t);
        C3Vector &segmentVec = m_splinePoints[segment];
        der += mathfu::vec3(segmentVec) * val;

        segment++;
    }
}


void C3Spline::arclengthSegT(float s, mathfu::mat4 *coeffs, unsigned int segCount, unsigned int *seg, float *t) {
    if ( segCount == 1 )
    {
        *seg = 0;
        *t = s;
    }
    else
    {
        float tLength = this->splineLength * s;
        float totLength = 0.0;
        *seg = 0;
        if ( segCount != 1 )
        {
            do
            {
                assert(*seg < segmentLengths.size());

                if ( tLength < segmentLengths[*seg] + totLength )
                    break;

                totLength = segmentLengths[*seg] + totLength;
                *seg = *seg + 1;
            }
            while ( *seg < segCount - 1 );
        }

        assert(*seg < segmentLengths.size());
        assert(segmentLengths[*seg] > 0.0);

        *t = (tLength - totLength) / segmentLengths[*seg];
    }
}

