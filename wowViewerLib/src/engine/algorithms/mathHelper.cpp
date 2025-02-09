//
// Created by deamon on 22.06.17.
//

#include "mathHelper.h"
#include "grahamScan.h"
#include <cmath>
#include <array>

#ifdef _WIN32
#define WIN32_MEAN_AND_LEAN
#define NOMINMAX
#endif

#if (__AVX__ && __SSE2__)
#include <emmintrin.h>
#include <immintrin.h>
#endif


float MathHelper::fp69ToFloat(uint16_t x) {
    float result = (((x & 0x1FF) * 0.001953125f) + (float)(x >> 9));
    if ((x & 0x8000) != 0) {
        result *= -1.0f;
    }
    return result;

//    const float divider = float(1 << 9);
//    float int_part = (x >> 9);
//    float float_part = (x & (1<<10) - 1) / divider;
//    return int_part + float_part;
}

mathfu::vec2 MathHelper::convertV69ToV2(vector_2fp_6_9 &fp69) {
    return mathfu::vec2(MathHelper::fp69ToFloat(fp69.x), MathHelper::fp69ToFloat(fp69.y));
}

CAaBox MathHelper::transformAABBWithMat4(const mathfu::mat4 &mat4, const mathfu::vec4 &min, const mathfu::vec4 &max) {
    //Adapted from http://dev.theomader.com/transform-bounding-boxes/

    mathfu::vec4 xa = mathfu::vec4(mat4[0], mat4[1], mat4[2], mat4[3])* min.x;
    mathfu::vec4 xb = mathfu::vec4(mat4[0], mat4[1], mat4[2], mat4[3])* max.x;

    mathfu::vec4 ya = mathfu::vec4(mat4[4], mat4[5], mat4[6], mat4[7]) * min.y;
    mathfu::vec4 yb = mathfu::vec4(mat4[4], mat4[5], mat4[6], mat4[7]) * max.y;

    mathfu::vec4 za = mathfu::vec4(mat4[8], mat4[9], mat4[10], mat4[11]) * min.z;
    mathfu::vec4 zb = mathfu::vec4(mat4[8], mat4[9], mat4[10], mat4[11]) * max.z;

    mathfu::vec4 vecx_min = mathfu::MinHelper(xa, xb);
    mathfu::vec4 vecy_min = mathfu::MinHelper(ya, yb);
    mathfu::vec4 vecz_min = mathfu::MinHelper(za, zb);
    mathfu::vec4 vecx_max = mathfu::MaxHelper(xa, xb);
    mathfu::vec4 vecy_max = mathfu::MaxHelper(ya, yb);
    mathfu::vec4 vecz_max = mathfu::MaxHelper(za, zb);

    mathfu::vec4 translation = mathfu::vec4(mat4[12], mat4[13], mat4[14], 0);

    mathfu::vec4 bb_min = vecx_min + vecy_min + vecz_min + translation;
    mathfu::vec4 bb_max = vecx_max + vecy_max + vecz_max + translation;

    mathfu::vec3_packed bb_min_packed;
    mathfu::vec3_packed bb_max_packed;

    mathfu::vec3(bb_min[0],bb_min[1],bb_min[2]).Pack(&bb_min_packed);
    mathfu::vec3(bb_max[0],bb_max[1],bb_max[2]).Pack(&bb_max_packed);

    return CAaBox(bb_min_packed, bb_max_packed);
}

framebased::vector<mathfu::vec4> MathHelper::transformPlanesWithMat(const framebased::vector<mathfu::vec4> &planes, const mathfu::mat4 &mat) {
    auto newPlanes = framebased::vector<mathfu::vec4>(planes.size());
    for (int i = 0; i < planes.size(); i++) {
        newPlanes[i] = (mat) * planes[i];
    }

    return newPlanes;
}

framebased::vector<mathfu::vec4> MathHelper::getFrustumClipsFromMatrix(const mathfu::mat4 &mat) {

#define el(x, y) (((y)-1) + 4*((x) - 1))

    //The order of planes is changed to make it easier to get intersections in getIntersectionPointsFromPlanes
    //And to be in line of how planes are created for portal verticies in portal culling
    framebased::vector<mathfu::vec4> planes(6);
    // Right clipping plane.
    planes[0] = mathfu::vec4(mat[el(1,4)] + mat[el(1,1)],
                             mat[el(2,4)] + mat[el(2,1)],
                             mat[el(3,4)] + mat[el(3,1)],
                             mat[el(4,4)] + mat[el(4,1)]);
    // Top clipping plane.
    planes[1] = mathfu::vec4(mat[el(1,4)] - mat[el(1,2)],
                             mat[el(2,4)] - mat[el(2,2)],
                             mat[el(3,4)] - mat[el(3,2)],
                             mat[el(4,4)] - mat[el(4,2)]);
    // Left clipping plane.
    planes[2] = mathfu::vec4(mat[el(1,4)] - mat[el(1,1)],
                             mat[el(2,4)] - mat[el(2,1)],
                             mat[el(3,4)] - mat[el(3,1)],
                             mat[el(4,4)] - mat[el(4,1)]);
    // Bottom clipping plane.
    planes[3] = mathfu::vec4(mat[el(1,4)] + mat[el(1,2)],
                             mat[el(2,4)] + mat[el(2,2)],
                             mat[el(3,4)] + mat[el(3,2)],
                             mat[el(4,4)] + mat[el(4,2)]);
    // Near clipping plane.
    planes[4] = mathfu::vec4(mat[el(1,4)] + mat[el(1,3)],
                             mat[el(2,4)] + mat[el(2,3)],
                             mat[el(3,4)] + mat[el(3,3)],
                             mat[el(4,4)] + mat[el(4,3)]);
    // Far clipping plane.
    planes[5] = mathfu::vec4(mat[el(1,4)] - mat[el(1,3)],
                             mat[el(2,4)] - mat[el(2,3)],
                             mat[el(3,4)] - mat[el(3,3)],
                             mat[el(4,4)] - mat[el(4,3)]);

    for (int i = 0; i < 6; i++) {
        //Hand made normalize
        float invVecLength = 1.0f / (planes[i].xyz().Length());
        planes[i] = planes[i] * invVecLength;
    }

    return planes;
}

framebased::vector<mathfu::vec3> MathHelper::calculateFrustumPointsFromMat(mathfu::mat4 &perspectiveViewMat) {
        mathfu::mat4 perspectiveViewMatInv = perspectiveViewMat.Inverse();

        static mathfu::vec4 vertices[] = {
                {-1, -1, -1, 1}, //0
                {-1,  1, -1, 1}, //1
                {1,   1, -1, 1}, //2
                {1,  -1, -1, 1}, //3
                {-1, -1,  1, 1}, //4
                {-1,  1,  1, 1}, //5
                {1,   1,  1, 1}, //6
                {1,  -1,  1, 1}, //7
        };

        framebased::vector<mathfu::vec3> points(8);
        
        for (int i = 0; i < 8; i++) {
            mathfu::vec4 &vert = vertices[i];

            mathfu::vec4 resVec4 = perspectiveViewMatInv * vert;
            resVec4 = resVec4 * (1.0f/resVec4.w);
            //vec4.transformMat4(resVec4, vert, perspectiveViewMat);

            points[i] = resVec4.xyz();
        }

        return points;
}

bool hullSort(mathfu::vec3 &a, mathfu::vec3 &b, mathfu::vec2 &center) {
    if (a.x - center.x >= 0 && b.x - center.x < 0)
        return true;
    if (a.x - center.x < 0 && b.x - center.x >= 0)
        return false;
    if (a.x - center.x == 0 && b.x - center.x == 0) {
        if (a.y - center.y >= 0 || b.y - center.y >= 0)
            return a.y > b.y;
        return b.y > a.y;
    }

    // compute the cross product of vectors (center -> a) x (center -> b)
    float det = (a.x - center.x) * (b.y - center.y) - (b.x - center.x) * (a.y - center.y);
    if (det < 0)
        return true;
    if (det > 0)
        return false;

    // points a and b are on the same line from the center
    // check which point is closer to the center
    float d1 = (a.x - center.x) * (a.x - center.x) + (a.y - center.y) * (a.y - center.y);
    float d2 = (b.x - center.x) * (b.x - center.x) + (b.y - center.y) * (b.y - center.y);
    return d1 > d2;
}

framebased::vector<mathfu::vec3> MathHelper::getHullPoints(framebased::vector<mathfu::vec3> &points){
    if (points.empty())
        return {};
    std::stack<Point> hullPoints = grahamScan(points);
    if (hullPoints.size() <= 2) {
        return framebased::vector<mathfu::vec3>(0);
    }

//    mathfu::vec3* end   = &hullPoints.top() + 1;
//    mathfu::vec3* begin = end - hullPoints.size();
    framebased::vector<mathfu::vec3> hullPointsArr;
    hullPointsArr.reserve(hullPoints.size());
    while(!hullPoints.empty()) {
        hullPointsArr.push_back(hullPoints.top());
        hullPoints.pop();
    }

    mathfu::vec2 centerPoint = mathfu::vec2(0,0);
    for (int i = 0; i< hullPointsArr.size(); i++) {
        centerPoint += hullPointsArr[i].xy();
    }
    centerPoint = centerPoint * (1.0f / hullPointsArr.size());

    std::sort(hullPointsArr.begin(),
              hullPointsArr.end(),
              [&](const mathfu::vec3 &a, const mathfu::vec3 &b) -> bool {
                  if (a.x - centerPoint.x >= 0 && b.x - centerPoint.x < 0)
                      return true;
                  if (a.x - centerPoint.x < 0 && b.x - centerPoint.x >= 0)
                      return false;
                  if (a.x - centerPoint.x == 0 && b.x - centerPoint.x == 0) {
                      if (a.y - centerPoint.y >= 0 || b.y - centerPoint.y >= 0)
                          return a.y > b.y;
                      return b.y > a.y;
                  }

                  // compute the cross product of vectors (center -> a) x (center -> b)
                  float det = (a.x - centerPoint.x) * (b.y - centerPoint.y) - (b.x - centerPoint.x) * (a.y - centerPoint.y);
                  if (det < 0)
                      return true;
                  if (det > 0)
                      return false;

                  // points a and b are on the same line from the center
                  // check which point is closer to the center
                  float d1 = (a.x - centerPoint.x) * (a.x - centerPoint.x) + (a.y - centerPoint.y) * (a.y - centerPoint.y);
                  float d2 = (b.x - centerPoint.x) * (b.x - centerPoint.x) + (b.y - centerPoint.y) * (b.y - centerPoint.y);
                  return d1 > d2;
              });

    return hullPointsArr;
}

framebased::vector<mathfu::vec3> MathHelper::getHullLines(framebased::vector<Point> &points){
    framebased::vector<mathfu::vec3> hullPointsArr = MathHelper::getHullPoints(points);

    framebased::vector<mathfu::vec3> hullLines;
    hullLines.reserve(hullPointsArr.size());
    
    if (hullPointsArr.size() > 2) {
        for (int i = 0; i < hullPointsArr.size(); i++) {
            int index1 = (i+1) % (hullPointsArr.size());
            int index2 = i;

            mathfu::vec3 line (
                hullPointsArr[index2].y - hullPointsArr[index1].y,
                -(hullPointsArr[index2].x - hullPointsArr[index1].x),
                -hullPointsArr[index1].x * (hullPointsArr[index2].y - hullPointsArr[index1].y) +
                hullPointsArr[index1].y*( hullPointsArr[index2].x - hullPointsArr[index1].x)
            );
            float normalLength = sqrt(line[0]*line[0] + line[1]*line[1]);
            line =  line * (1.0f/normalLength);

            hullLines.push_back(line);
        }
    } else {
        //"invalid number of hullPoints"
    }

    return hullLines;
}

mathfu::vec3 MathHelper::getIntersection( mathfu::vec3 &tail1, mathfu::vec3 &head1, mathfu::vec3 &tail2, mathfu::vec3 &head2) {

//Edge vectors
    mathfu::vec3 dir1 = head1 - tail1;
    mathfu::vec3 dir2 = head2 - tail2;

    mathfu::vec3 diff = tail1 - tail2;

    double a = mathfu::vec3::DotProduct(dir1, dir1);//always >= 0
    double b = mathfu::vec3::DotProduct(dir1, dir2);
    double c = mathfu::vec3::DotProduct(dir2, dir2);//always >= 0
    double d = mathfu::vec3::DotProduct(dir1, diff);
    double e = mathfu::vec3::DotProduct(dir2, diff);
    double f = a * c - b * b;//always >= 0

    double sc;//sc = sN / sD, default sD = D >= 0
    double tc;//tc = tN / tD, default tD = D >= 0

//compute the line parameters of the two closest points

//M_EQUAL здесь проверяет, лежит ли f в некоторой окрестности 0.0
//Т. е. 0.0 - epsilon < f < 0.0 + epsilon
//epsilon задано где-то в header'ах мат. библиотеки...
    if(f > -0.001f && f < 0.001f)//the lines are almost parallel
    {
        sc = 0.0;
        tc = (b > c ? d / b : e / c);//use the largest denominator
    }
    else
    {
        sc = (b * e - c * d) / f;
        tc = (a * e - b * d) / f;
    }

    mathfu::vec3 npoints[2];//Nearest points' coordinates

    npoints[0] = tail1 + dir1 * (float)sc;
    npoints[1] = tail2 + dir2 * (float)tc;

    if((npoints[1] - npoints[0]).Length() < 0.1)
    {
        return (npoints[0] + npoints[1]) / 2.0f;
    }
    else return mathfu::vec3(0,0,0);
}

bool MathHelper::checkFrustum(const MathHelper::FrustumCullingData &frustumData, const CAaBox &box) {
    return MathHelper::checkFrustum(frustumData.frustums, box);
}





bool MathHelper::checkFrustum(const std::vector<PlanesUndPoints> &frustums, const CAaBox &box) {
    // check box outside/inside of frustum

    ALIGNED_(16) const mathfu::vec4 checkedCorners[8] = {
        mathfu::vec4(box.min.x, box.min.y, box.min.z, 1.0),
        mathfu::vec4(box.max.x, box.min.y, box.min.z, 1.0),
        mathfu::vec4(box.min.x, box.max.y, box.min.z, 1.0),
        mathfu::vec4(box.max.x, box.max.y, box.min.z, 1.0),
        mathfu::vec4(box.min.x, box.min.y, box.max.z, 1.0),
        mathfu::vec4(box.max.x, box.min.y, box.max.z, 1.0),
        mathfu::vec4(box.min.x, box.max.y, box.max.z, 1.0),
        mathfu::vec4(box.max.x, box.max.y, box.max.z, 1.0)
    };
    for (auto &planeUndPoints : frustums) {
        auto const &planes = planeUndPoints.planes;
        auto const &points = planeUndPoints.points;
        ALIGNED_(16) uint64_t results[4];


        bool result = true;
        int num_planes = planes.size();
        for (int i = 0; i < num_planes; i++) {
            int out = 0;
#if (__AVX__ && __SSE2__)
            __m128 zeros = _mm_setzero_ps();
            __m128 plane = _mm_load_ps(planes[i].data_);

            for (int k = 0; k < 2; k++)
            {
                __m128 prod0 = _mm_mul_ps(plane, _mm_load_ps(checkedCorners[k*4 + 0].data_));
                __m128 prod1 = _mm_mul_ps(plane, _mm_load_ps(checkedCorners[k*4 + 1].data_));
                __m128 prod2 = _mm_mul_ps(plane, _mm_load_ps(checkedCorners[k*4 + 2].data_));
                __m128 prod3 = _mm_mul_ps(plane, _mm_load_ps(checkedCorners[k*4 + 3].data_));

                //Transpose
                //1   2  3  4
                //5   6  7  8
                //9  10 11 12
                //13 14 15 16

                __m128 row_per0 = _mm_shuffle_ps(prod0, prod1, _MM_SHUFFLE(1, 0, 1, 0)); // 1 2 5 6
                __m128 row_per1 = _mm_shuffle_ps(prod0, prod1, _MM_SHUFFLE(3, 2, 3, 2)); // 3 4 7 8
                __m128 row_per2 = _mm_shuffle_ps(prod2, prod3, _MM_SHUFFLE(1, 0, 1, 0)); // 9 10 13 14
                __m128 row_per3 = _mm_shuffle_ps(prod2, prod3, _MM_SHUFFLE(3, 2, 3, 2));  // 11 12 15 16

                __m128 row_1 = _mm_shuffle_ps(row_per0, row_per2, _MM_SHUFFLE(2, 0, 2, 0));// 1 5 9 13
                __m128 row_2 = _mm_shuffle_ps(row_per0, row_per2, _MM_SHUFFLE(3, 1, 3, 1));// 2 6 10 14
                __m128 row_3 = _mm_shuffle_ps(row_per1, row_per3, _MM_SHUFFLE(2, 0, 2, 0));// 3 7 11 15
                __m128 row_4 = _mm_shuffle_ps(row_per1, row_per3, _MM_SHUFFLE(3, 1, 3, 1));// 4 8 12 16

                __m128 dotResults = _mm_add_ps(_mm_add_ps(_mm_add_ps(row_1, row_2), row_3), row_4);

#if 0
                //Debug check
                _MM_ALIGN16 float dotResultsF[4];
                float dotResultsCalc[4];
                _mm_store_ps(dotResultsF, dotResults);
                for (int j = 0; j < 4; j++) {
                    dotResultsCalc[j] = mathfu::vec4::DotProduct(planes[i], checkedCorners[k*4 + j]) ;
                }

                for (int j = 0; j < 4; j++)
                    if (dotResultsCalc[j] != dotResultsF[j]) {
                        printf("error");
                    }
#endif

                _mm_store_si128((__m128i *)&results[k*2], (__m128i)_mm_cmpgt_ps(zeros, dotResults));
            }

            if (results[0] == 0xFFFFFFFFFFFFFFFF &&
                results[1] == 0xFFFFFFFFFFFFFFFF &&
                results[2] == 0xFFFFFFFFFFFFFFFF &&
                results[3] == 0xFFFFFFFFFFFFFFFF) {
                result = false;
            }

            if (!result)
                continue;

#else
            for (int j = 0; j < 8; j++) {
                out += ((mathfu::vec4::DotProduct(planes[i], checkedCorners[j]) < 0.0) ? 1 : 0);
            }

            if (out == 8) {
                result = false;
                break;
            }
#endif
        }

        //So that we can add additional checks down the line
        if (!result) continue;

//     check frustum outside/inside box
//        if (points.size() > 0) {
//            int out = 0; for (int i = 0; i < 8; i++) out += ((points[i].x > box.max.x) ? 1 : 0); if (out == 8) continue;
//                out = 0; for (int i = 0; i < 8; i++) out += ((points[i].x < box.min.x) ? 1 : 0); if (out == 8) continue;
//                out = 0; for (int i = 0; i < 8; i++) out += ((points[i].y > box.max.y) ? 1 : 0); if (out == 8) continue;
//                out = 0; for (int i = 0; i < 8; i++) out += ((points[i].y < box.min.y) ? 1 : 0); if (out == 8) continue;
//                out = 0; for (int i = 0; i < 8; i++) out += ((points[i].z > box.max.z) ? 1 : 0); if (out == 8) continue;
//                out = 0; for (int i = 0; i < 8; i++) out += ((points[i].z < box.min.z) ? 1 : 0); if (out == 8) continue;
//        }
        return true;
    }

    return false;
}

bool MathHelper::checkFrustum2D(const std::vector<PlanesUndPoints> &frustums, const CAaBox &box) {
    for (auto &planeUndPoints : frustums) {
        auto const &planes = planeUndPoints.hullLines;

        for (int i = 0; i < planes.size(); i++) {
            int out = 0;
            out += (((planes[i][0] * box.min.x + planes[i][1] * box.min.y + planes[i][2]) > 0.0) ? 1 : 0);
            out += (((planes[i][0] * box.max.x + planes[i][1] * box.min.y + planes[i][2]) > 0.0) ? 1 : 0);
            out += (((planes[i][0] * box.min.x + planes[i][1] * box.max.y + planes[i][2]) > 0.0) ? 1 : 0);
            out += (((planes[i][0] * box.max.x + planes[i][1] * box.max.y + planes[i][2]) > 0.0) ? 1 : 0);
            if (out == 4) return false;
        }
    }

    return true;
}


mathfu::vec4 intersection(mathfu::vec4 &p1, mathfu::vec4 &p2, float k) {
    mathfu::vec4 temp = mathfu::vec4(
            p1[0] + k * (p2[0] - p1[0]),
            p1[1] + k * (p2[1] - p1[1]),
            p1[2] + k * (p2[2] - p1[2]),
            1
    );
    return temp;
}

mathfu::vec4 MathHelper::planeLineIntersection(mathfu::vec4 &plane, mathfu::vec4 &p1, mathfu::vec4 &p2) {
    mathfu::vec3 lineVector = (p2.xyz() - p1.xyz()).Normalized();
    mathfu::vec3 planeNormal = plane.xyz();


    mathfu::vec3 intersectionPoint =
        p1.xyz() - (lineVector*((mathfu::vec3::DotProduct(p1.xyz(), planeNormal) + plane.w) /
          mathfu::vec3::DotProduct(lineVector, planeNormal)));

    return mathfu::vec4(intersectionPoint, 1.0);
}

//Points should be sorted against center by this point
bool MathHelper::planeCull(framebased::vector<mathfu::vec3> &points, framebased::vector<mathfu::vec4> &planes) {
    // check box outside/inside of frustum
    framebased::vector<mathfu::vec4> vec4Points(points.size());

    const float epsilon = 0.0001;

    for (int j = 0; j < points.size(); j++) {
        vec4Points[j] = mathfu::vec4(points[j].x, points[j].y, points[j].z, 1.0);
    }

    for (int i = 0; i < planes.size(); i++) {
        int out = 0;

        for (int j = 0; j < vec4Points.size(); j++) {
            out += (((mathfu::vec4::DotProduct(planes[i], vec4Points[j]) + epsilon) < 0.0 ) ? 1 : 0);
        }

        if (out == vec4Points.size()) {
            return false;
        }

        //---------------------------------
        // Cull by points by current plane
        //---------------------------------

        framebased::vector<mathfu::vec4> resultPoints;
        resultPoints.reserve(vec4Points.size() + (vec4Points.size() >> 1));

        for (int j = 0; j < vec4Points.size(); j++) {
            mathfu::vec4 p1 = vec4Points[j];
            mathfu::vec4 p2 = vec4Points[(j + 1) % vec4Points.size()];

            // InFront = plane.Distance( point ) > 0.0f
            // Behind  = plane.Distance( point ) < 0.0f

            float t1 = mathfu::vec4::DotProduct(p1, planes[i]);
            float t2 = mathfu::vec4::DotProduct(p2, planes[i]);

            if ((t1 >= -epsilon) && (t2 >= -epsilon)) { //p1 InFront and p2 InFront
                resultPoints.push_back(p2);
            } else if ((t1 >= -epsilon) && (t2 < -epsilon)) { //p1 InFront and p2 Behind
//                float k = std::fabs(t1) / (std::fabs(t1) + std::fabs(t2));
                resultPoints.push_back(MathHelper::planeLineIntersection( planes[i], p1, p2));
            } else if ((t1 < -epsilon) && (t2 >= -epsilon)) { //p1 Behind and p2 InFront
//                float k = std::fabs(t1) / (std::fabs(t1) + std::fabs(t2));
                resultPoints.push_back(MathHelper::planeLineIntersection( planes[i], p1, p2));
                resultPoints.push_back(p2);
            } else if ((t2 < epsilon) && (t2 > -epsilon)) { //P2 Inside
                resultPoints.push_back(p2);
            }
        }

        vec4Points = framebased::vector<mathfu::vec4>(resultPoints.begin(), resultPoints.end());
    }

    points = framebased::vector<mathfu::vec3>(vec4Points.size());
    for (int j = 0; j < vec4Points.size(); j++) {
        points[j] = vec4Points[j].xyz();
    }

    return vec4Points.size() > 2;

}

void MathHelper::sortVec3ArrayAgainstPlane(framebased::vector<mathfu::vec3> &thisPortalVertices,
                                                  const mathfu::vec4 &plane) {
    mathfu::vec3 center(0, 0, 0);
    for (int j = 0; j < thisPortalVertices.size(); j++) {
        center += thisPortalVertices[j];
    }
    center *= 1.0f / (float)thisPortalVertices.size();

    std::sort(thisPortalVertices.begin(), thisPortalVertices.end(),
        [&](const mathfu::vec3 &a, const mathfu::vec3 &b) -> bool {
            mathfu::vec3 ac = a - center;

            mathfu::vec3 bc = b - center;

            mathfu::vec3 cross = mathfu::vec3::CrossProduct(ac, bc).Normalized();
            float dotResult = mathfu::vec3::DotProduct(cross, plane.xyz());

            return dotResult < 0;
    });
}

bool MathHelper::isPointInsideAABB(const CAaBox &aabb, const mathfu::vec3 &p) {
    bool result = p[0] > aabb.min.x && p[0] < aabb.max.x &&
             p[1] > aabb.min.y && p[1] < aabb.max.y &&
             p[2] > aabb.min.z && p[2] < aabb.max.z;
    return result;
}

bool MathHelper::isPointInsideAABB(const mathfu::vec2 aabb[2], const mathfu::vec2 &p) {
    bool result = p[0] > aabb[0].x && p[0] < aabb[1].x &&
             p[1] > aabb[0].y && p[1] < aabb[1].y;
    return result;
}

float MathHelper::distance_aux(float p, float lower, float upper) {
    if (p < lower) return lower - p;
    if (p > upper) return p - upper;
    return 0;
}

float MathHelper::distanceFromAABBToPoint(const CAaBox &aabb, mathfu::vec3 &p) {

    float dx = MathHelper::distance_aux(p.x, aabb.min.x, aabb.max.x);
    float dy = MathHelper::distance_aux(p.y, aabb.min.y, aabb.max.y);
    float dz = MathHelper::distance_aux(p.z, aabb.min.z, aabb.max.z);

    if (MathHelper::isPointInsideAABB(aabb, p))
        return std::min(std::min(dx, dy), dz);    // or 0 in case of distance from the area
    else
        return sqrt(dx * dx + dy * dy + dz * dz);
}

float MathHelper::distanceFromAABBToPoint2DSquared(const mathfu::vec2 aabb[2], mathfu::vec2 &p) {

    float dx = MathHelper::distance_aux(p.x, aabb[0].x, aabb[1].x);
    float dy = MathHelper::distance_aux(p.y, aabb[0].y, aabb[1].y);

    if (MathHelper::isPointInsideAABB(aabb, p)) {
        float dist = std::min(dx, dy);
        return dist*dist;    // or 0 in case of distance from the area
    } else {
        return dx * dx + dy * dy;
    }
}

mathfu::vec3 MathHelper::getBarycentric(mathfu::vec3 &p, mathfu::vec3 &a, mathfu::vec3 &b, mathfu::vec3 &c) {
    mathfu::vec3 v0 = b - a;
    mathfu::vec3 v1 = c - a;
    mathfu::vec3 v2 = p - a;

    float d00 = mathfu::vec3::DotProduct(v0, v0);
    float d01 = mathfu::vec3::DotProduct(v0, v1);
    float d11 = mathfu::vec3::DotProduct(v1, v1);

    float d20 = mathfu::vec3::DotProduct(v2, v0);
    float d21 = mathfu::vec3::DotProduct(v2, v1);
    float denom = d00 * d11 - d01 * d01;
//    float denom = v0.x * v1.y - v1.x * v0.y;
    if ((denom < 0.0001f) && (denom > -0.0001f)) {
        return mathfu::vec3(-1, -1, -1);
    };

    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
//    float v = (v2.x * v1.y - v1.x * v2.y) / denom;
//    float w = (v0.x * v2.y - v2.x * v0.y) / denom;
    float u = 1.0f - v - w;
    return mathfu::vec3(u, v, w);
}

mathfu::vec4 MathHelper::createPlaneFromEyeAndVertexes (
            const mathfu::vec3 &eye,
            const mathfu::vec3 &vertex1,
            const mathfu::vec3 &vertex2) {

    mathfu::vec3 edgeDir1 = vertex1 - eye;
    mathfu::vec3 edgeDir2 = vertex2 - eye;

    //planeNorm=cross(viewDir, edgeDir)
    mathfu::vec3 planeNorm = mathfu::vec3::CrossProduct(edgeDir2, edgeDir1).Normalized();

    //Plane fpl(planeNorm, dot(planeNorm, vertexA))
    float distToPlane = mathfu::vec3::DotProduct(planeNorm, eye);
    mathfu::vec4 plane(planeNorm, -distToPlane);

    return plane;
}

#define NO 0
#define YES 1
#define COLLINEAR 2

int areIntersecting(
    float v1x1, float v1y1, float v1x2, float v1y2,
    float v2x1, float v2y1, float v2x2, float v2y2
) {
    float d1, d2;
    float a1, a2, b1, b2, c1, c2;

    // Convert vector 1 to a line (line 1) of infinite length.
    // We want the line in linear equation standard form: A*x + B*y + C = 0
    // See: http://en.wikipedia.org/wiki/Linear_equation
    a1 = v1y2 - v1y1;
    b1 = v1x1 - v1x2;
    c1 = (v1x2 * v1y1) - (v1x1 * v1y2);

    // Every point (x,y), that solves the equation above, is on the line,
    // every point that does not solve it, is not. The equation will have a
    // positive result if it is on one side of the line and a negative one
    // if is on the other side of it. We insert (x1,y1) and (x2,y2) of vector
    // 2 into the equation above.
    d1 = (a1 * v2x1) + (b1 * v2y1) + c1;
    d2 = (a1 * v2x2) + (b1 * v2y2) + c1;

    // If d1 and d2 both have the same sign, they are both on the same side
    // of our line 1 and in that case no intersection is possible. Careful,
    // 0 is a special case, that's why we don't test ">=" and "<=",
    // but "<" and ">".
    if (d1 > 0 && d2 > 0) return NO;
    if (d1 < 0 && d2 < 0) return NO;

    // The fact that vector 2 intersected the infinite line 1 above doesn't
    // mean it also intersects the vector 1. Vector 1 is only a subset of that
    // infinite line 1, so it may have intersected that line before the vector
    // started or after it ended. To know for sure, we have to repeat
    // the same test the other way round. We start by calculating the
    // infinite line 2 in linear equation standard form.
    a2 = v2y2 - v2y1;
    b2 = v2x1 - v2x2;
    c2 = (v2x2 * v2y1) - (v2x1 * v2y2);

    // Calculate d1 and d2 again, this time using points of vector 1.
    d1 = (a2 * v1x1) + (b2 * v1y1) + c2;
    d2 = (a2 * v1x2) + (b2 * v1y2) + c2;

    // Again, if both have the same sign (and neither one is 0),
    // no intersection is possible.
    if (d1 > 0 && d2 > 0) return NO;
    if (d1 < 0 && d2 < 0) return NO;

    // If we get here, only two possibilities are left. Either the two
    // vectors intersect in exactly one point or they are collinear, which
    // means they intersect in any number of points from zero to infinite.
    if ((a1 * b2) - (a2 * b1) == 0.0f) return COLLINEAR;

    // If they are not collinear, they must intersect in exactly one point.
    return YES;
}

bool MathHelper::isPointInsideNonConvex(const mathfu::vec3 &p, const CAaBox &aabb, const std::vector<mathfu::vec2> &points) {
    if (!MathHelper::isPointInsideAABB(aabb, p)) return false;

    //Select point outside of convex;
    mathfu::vec2 outPnt = mathfu::vec2(aabb.max.x+1000, aabb.max.y+1000);

    int intersections = 0;
    for (int i = 0; i < points.size() - 1; i++) {
        // Test if current side intersects with ray.
        // If yes, intersections++;
        if (areIntersecting(
            outPnt.x, outPnt.y, p.x, p.y,
            points[i].x, points[i].y, points[i+1].x, points[i+1].y) == YES) {
            intersections++;
        }
    }
    if (areIntersecting(
        outPnt.x, outPnt.y, p.x, p.y,
        points[points.size() - 1].x, points[points.size() - 1].y, points[0].x, points[0].y) == YES) {
        intersections++;
    }


    if ((intersections & 1) == 1) {
        // Inside of polygon
        return true;
    }
//    else {
//        // Outside of polygon
//    }

    return false;
}

inline float minimum_distance(const mathfu::vec2 &v, const mathfu::vec2 &w, const mathfu::vec2 &p) {
  // Return minimum distance between line segment vw and point p
  const float l2 = (w - v).LengthSquared();  // i.e. |w-v|^2 -  avoid a sqrt
  if (l2 == 0.0) return (p-v).Length();   // v == w case
  // Consider the line extending the segment, parameterized as v + t (w - v).
  // We find projection of point p onto the line.
  // It falls where t = [(p-v) . (w-v)] / |w-v|^2
  // We clamp t from [0,1] to handle points outside the segment vw.
  const float t = std::max<float>(0, std::min<float>(1, dot(p - v, w - v) / l2));
  const mathfu::vec2 projection = v + t * (w - v);  // Projection falls on the segment
  return (p - projection).Length();
}

float MathHelper::findLeastDistanceToBorder(const mathfu::vec3 &p, const std::vector<mathfu::vec2> &points, bool &isInsideConvex) {
    auto const calcDistance = [p](const mathfu::vec2 &a, const mathfu::vec2 &b) -> float {

        float distance = minimum_distance(a, b, p.xy());

        return distance;
    };

    auto const flipIsInside = [p](const mathfu::vec2 &v, const mathfu::vec2 &w) -> bool {
        return (w.x - v.x) * (p.y - v.y) / (w.y - v.y) + v.x < p.x;
    };

    float dist = 999999.f;
    isInsideConvex = false;
    for (int i = 0; i < points.size() - 1; i++) {
        dist = std::min(dist, calcDistance(points[i], points[i+1]));
        isInsideConvex = isInsideConvex ^ flipIsInside(points[i], points[i+1]);
    }
    dist = std::min(dist, calcDistance(points[points.size()-1], points[0]));
    isInsideConvex = isInsideConvex ^ flipIsInside(points[points.size()-1], points[0]);
    return dist;
}

bool MathHelper::isAabbIntersect2d(CAaBox a, CAaBox b) {

    bool result = (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
               (a.min.y <= b.max.y && a.max.y >= b.min.y);

    return result;
}


template<int tableSize>
constexpr float InterpTable(const std::array<std::array<float, 2>, tableSize> &table, float time) {

    if (time >= 0.0f) {
        time = std::min(time, 1.0f);
    }

    int i;
    int firstIndex = 0;
    int secondIndex = 0;
    for (i = 0; i < table.size(); i++) {
        if (time <= table[i][0]) {
            break;
        }
    }

    firstIndex = 0;
    if (i != table.size()) {
        firstIndex = i;
    }
    if (firstIndex) {
        secondIndex = firstIndex - 1;
    } else {
        secondIndex = table.size() - 1;
    }

    float tableTimeDiff = table[firstIndex][0] - table[secondIndex][0];
    if (fabs(tableTimeDiff) < 0.001f)
        return table[secondIndex][1];

    if (tableTimeDiff < 0.0f)
        tableTimeDiff = tableTimeDiff + 1.0f;

    float timeDiff = time - table[secondIndex][0];
    if (timeDiff < 0.0f)
        timeDiff = timeDiff + 1.0f;

    float alpha = timeDiff / tableTimeDiff;

    float firstValue = table[firstIndex][1];
    float secondValue = table[secondIndex][1];
    //if (firstValue < secondValue)
        //return table[secondIndex].y - alpha * (secondValue - firstValue);
    //else
    return secondValue + alpha * (firstValue - secondValue);

}

float doSomeConvert(float a) {
    int a1;
    if ( a <= 0.0f )
    {
        a1 = ~(int)(float)-a;
    }
    else
    {
        a1 = (int)a;
    }

    float res = (float)((float)((float)((float)(a - (float)a1) * 4.0f) + -6.0f)
                 * (float)((float)(a - (float)a1) * (float)(a - (float)a1)))
         + 1.0f;
    if ( (a1 & 1) != 0 )
    {
        res = -res;
    }

    return res;
}

namespace SkyConstantsAndFunctions {
        static constexpr std::array<std::array<float, 2>, 5> sunPhiTable = {
        {
            { 0.25f,        1.7453293f },
            { 0.49652779f,  0.08726646f},
            { 0.5f,         0.08726646f},
            { 0.50347221f,  0.08726646f},
            { 0.79166669f,  1.7453293f }
        }
    };

    static constexpr std::array<std::array<float, 2>, 3> sunThetaTable = {
        {
            { 0.25f,       0.78539819f},
            { 0.5f,        0.78539819f},
            { 0.79166669f, 0.78539819f}
        }
    };

    static constexpr std::array<std::array<float, 2>, 5> moonPhiTable = {
        {
            { 0,            0.61086524f },
            { 0.0034722222f,0.61086524 },
            { 0.16666667f,  1.7453293f },
            { 0.89583331f,  1.7453293f },
            { 0.99652779f,  0.61086524f }
        }
    };

    static constexpr std::array<std::array<float, 2>, 3> moonThetaTable = {
        {
            { 0.0f,        0.78539819f},
            { 0.16666667f, 0.78539819f},
            { 0.89583331f, 0.78539819f}
        }
    };

    static constexpr std::array<std::array<float, 2>, 4> directionalLightPhiTable = {
        {
            { 0,            2.2165682f },
            { 0.25f,        1.9198622f },
            { 0.5f,         2.2165682f },
            { 0.75f,        1.9198622f },
        }
    };

    static constexpr std::array<std::array<float, 2>, 4> directionalLightThetaTable = {
        {
            { 0,            3.9269907f },
            { 0.25f,        3.9269907f },
            { 0.5f,         3.9269907f },
            { 0.75f,        3.9269907f },
        }
    };

    enum class SkyDataType : int { SK_SUN, SK_MOON, SK_DIR_LIGHT };

    template <SkyDataType T>
    mathfu::vec3 getVector(int time) {
        float timeF = time / 2880.0f;

        float phi =   0.0f;
        float theta = 0.0f;

        if constexpr (T == SkyDataType::SK_DIR_LIGHT) {
            phi = InterpTable<4>(directionalLightPhiTable, timeF);
            theta = InterpTable<4>(directionalLightThetaTable, timeF);
        }
        if constexpr (T == SkyDataType::SK_SUN) {
            phi = InterpTable<5>(sunPhiTable, timeF);
            theta = InterpTable<3>(sunThetaTable, timeF);
        }
        if constexpr (T == SkyDataType::SK_MOON) {
            phi = InterpTable<5>(moonPhiTable, timeF);
            theta = InterpTable<3>(moonThetaTable, timeF);
        }

        constexpr float INV_PI = 1.0f / M_PI;

        float sinPhi = doSomeConvert(phi * INV_PI - 0.5f);
        float cosPhi = doSomeConvert(phi * INV_PI);

        float sinTheta = doSomeConvert(theta * INV_PI + -0.5f);
        float cosTheta = doSomeConvert(theta * INV_PI);

        mathfu::vec3 vec = mathfu::vec3(sinPhi * cosTheta, sinPhi * sinTheta, cosPhi);
        return vec;
    }
}

mathfu::vec3 MathHelper::calcExteriorColorDir(const mathfu::mat4 &lookAtMat, int time) {
    using namespace SkyConstantsAndFunctions;

    mathfu::vec4 sunDirWorld = mathfu::vec4(getVector<SkyDataType::SK_DIR_LIGHT>(time), 0.0f);
    sunDirWorld = mathfu::vec4(sunDirWorld.xyz().Normalized(), 0.0f);
    sunDirWorld = mathfu::vec4(0.000, -0.687, -0.688, 0.0f);

    return (lookAtMat.Inverse().Transpose() * sunDirWorld).xyz().Normalized();
}
mathfu::vec3 MathHelper::calcSunPlanetPos(const mathfu::mat4 &lookAtMat, int time) {
    using namespace SkyConstantsAndFunctions;

    mathfu::vec4 sunPlanetPos = mathfu::vec4(getVector<SkyDataType::SK_SUN>(time), 0.0f);
    return sunPlanetPos.xyz();
}

mathfu::vec3 MathHelper::hsv2rgb(const MathHelper::hsv &in) {
    double      hh, p, q, t, ff;
    long        i;
    mathfu::vec3    out;

    if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.x = in.v;
        out.y = in.v;
        out.z = in.v;
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
        case 0:
            out.x = in.v;
            out.y = t;
            out.z = p;
            break;
        case 1:
            out.x = q;
            out.y = in.v;
            out.z = p;
            break;
        case 2:
            out.x = p;
            out.y = in.v;
            out.z = t;
            break;

        case 3:
            out.x = p;
            out.y = q;
            out.z = in.v;
            break;
        case 4:
            out.x = t;
            out.y = p;
            out.z = in.v;
            break;
        case 5:
        default:
            out.x = in.v;
            out.y = p;
            out.z = q;
            break;
    }
    return out;
}

MathHelper::hsv MathHelper::rgb2hsv(const mathfu::vec3 &in) {
    hsv         out;
    double      min, max, delta;

    min = in.x < in.y ? in.x : in.y;
    min = min  < in.z ? min  : in.z;

    max = in.x > in.y ? in.x : in.y;
    max = max  > in.z ? max  : in.z;

    out.v = max;                                // v
    delta = max - min;
    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = -1; //NAN;                            // its now undefined
        return out;
    }
    if( in.x >= max )                           // > is bogus, just keeps compilor happy
        out.h = ( in.y - in.z ) / delta;        // between yellow & magenta
    else
    if( in.y >= max )
        out.h = 2.0 + ( in.z - in.x ) / delta;  // between cyan & yellow
    else
        out.h = 4.0 + ( in.x - in.y ) / delta;  // between magenta & cyan

    out.h *= 60.0;                              // degrees

    if( out.h < 0.0 )
        out.h += 360.0;

    return out;
}

framebased::vector<mathfu::vec3>
MathHelper::getIntersectionPointsFromPlanes(const framebased::vector<mathfu::vec4> &planes) {
    framebased::vector<mathfu::vec3> points;
    //For far and near plane
    for (int x = planes.size()-2; x < planes.size(); x++) {
        //Look for intersection for of any two planes other planes with two planes above
        for (int y = 0; y < planes.size()-3; y++) {
//        for (int z = y+1; z < planes.size()-2; z++)
            int z = y + 1;
            {
                mathfu::mat3 mat = {
                    planes[x].x, planes[x].y, planes[x].z,
                    planes[y].x, planes[y].y, planes[y].z,
                    planes[z].x, planes[z].y, planes[z].z
                };
                mat = mat.Transpose();
                mathfu::mat3 inverseMat;
                if (mat.InverseWithDeterminantCheck(&inverseMat)) {
                    mathfu::vec3 point = inverseMat * mathfu::vec3(
                        -planes[x].w,
                        -planes[y].w,
                        -planes[z].w);

                    points.push_back(point);
                }
            }
        }
    }
    return points;
}
