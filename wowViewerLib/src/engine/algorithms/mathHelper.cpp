//
// Created by deamon on 22.06.17.
//

#include "mathHelper.h"
#include "grahamScan.h"
#include <cmath>


float MathHelper::fp69ToFloat(uint16_t x) {
    const float divider = float(1 << 9);
    float int_part = (x >> 9);
    float float_part = (x & (1<<10) - 1) / divider;
    return int_part + float_part;
}

mathfu::vec2 MathHelper::convertV69ToV2(vector_2fp_6_9 &fp69) {
    return mathfu::vec2(MathHelper::fp69ToFloat(fp69.x), MathHelper::fp69ToFloat(fp69.y));
}

CAaBox MathHelper::transformAABBWithMat4(mathfu::mat4 mat4, mathfu::vec4 min, mathfu::vec4 max) {
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

std::vector<mathfu::vec4> MathHelper::getFrustumClipsFromMatrix(mathfu::mat4 &mat) {
    std::vector<mathfu::vec4> planes(6);
    // Right clipping plane.

    planes[0] = mathfu::vec4(mat[3] - mat[0],
                                mat[7] - mat[4],
                                mat[11] - mat[8],
                                mat[15] - mat[12]);
    // Left clipping plane.
    planes[1] = mathfu::vec4(mat[3] + mat[0],
                                mat[7] + mat[4],
                                mat[11] + mat[8],
                                mat[15] + mat[12]);
    // Bottom clipping plane.
    planes[2] = mathfu::vec4(mat[3] + mat[1],
                                mat[7] + mat[5],
                                mat[11] + mat[9],
                                mat[15] + mat[13]);
    // Top clipping plane.
    planes[3] = mathfu::vec4(mat[3] - mat[1],
                                mat[7] - mat[5],
                                mat[11] - mat[9],
                                mat[15] - mat[13]);
    // Far clipping plane.
    planes[4] = mathfu::vec4(mat[3] - mat[2],
                                mat[7] - mat[6],
                                mat[11] - mat[10],
                                mat[15] - mat[14]);
    // Near clipping plane.
    planes[5] = mathfu::vec4(mat[3] + mat[2],
                                mat[7] + mat[6],
                                mat[11] + mat[10],
                                mat[15] + mat[14]);

    for (int i = 0; i < 6; i++) {
        //Hand made normalize
        float invVecLength = 1 / (planes[i].xyz().Length());
        planes[i] = planes[i] * invVecLength;
    }

    return planes;
}

std::vector<mathfu::vec3> MathHelper::calculateFrustumPointsFromMat(mathfu::mat4 &perspectiveViewMat) {
        mathfu::mat4 perspectiveViewMatInv = perspectiveViewMat.Inverse();

        static mathfu::vec4 vertices[] = {
                {-1, -1, -1, 1}, //0
                {1, -1, -1, 1},  //1
                {1, -1, 1, 1},   //2
                {-1, -1, 1, 1},  //3
                {-1, 1, 1, 1},   //4
                {1, 1, 1, 1},    //5
                {1, 1, -1, 1},   //6
                {-1, 1, -1, 1},  //7
        };

        std::vector<mathfu::vec3> points;
        for (int i = 0; i < 8; i++) {
            mathfu::vec4 &vert = vertices[i];

            mathfu::vec4 resVec4 = perspectiveViewMatInv * vert;
            resVec4 = resVec4 * (1/resVec4[3]);
            //vec4.transformMat4(resVec4, vert, perspectiveViewMat);

            points.push_back(resVec4.xyz());
        }

        return points;
}

bool hullSort(mathfu::vec3 a, mathfu::vec3 b, mathfu::vec2 center) {
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

std::vector<mathfu::vec3> MathHelper::getHullPoints(std::vector<mathfu::vec3> &points){
    std::stack<Point> hullPoints = grahamScan(points);
    if (hullPoints.size() <= 2) {
        return std::vector<mathfu::vec3>(0);
    }

    mathfu::vec3* end   = &hullPoints.top() + 1;
    mathfu::vec3* begin = end - hullPoints.size();
    std::vector<mathfu::vec3> hullPointsArr(begin, end);

    mathfu::vec2 centerPoint = mathfu::vec2(0,0);
    for (int i = 0; i< hullPoints.size(); i++) {
        centerPoint += hullPointsArr[i].xy();
    }
    centerPoint = centerPoint * (1.0f / hullPoints.size());

    std::sort(hullPointsArr.begin(),
              hullPointsArr.end(),
              [&](mathfu::vec3 a, mathfu::vec3 b) -> bool {
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

std::vector<mathfu::vec3> MathHelper::getHullLines(std::vector<Point> &points){
    std::vector<mathfu::vec3> hullPointsArr = MathHelper::getHullPoints(points);

    std::vector<mathfu::vec3> hullLines;
    
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


bool MathHelper::checkFrustum(const std::vector<mathfu::vec4> &planes, const CAaBox &box, const std::vector<mathfu::vec3> &points) {
    // check box outside/inside of frustum
    mathfu::vec4 boxMin = mathfu::vec4(box.min.x, box.min.y, box.min.z, 1.0);
    mathfu::vec4 boxMax = mathfu::vec4(box.max.x, box.max.y, box.max.z, 1.0);

    int num_planes = planes.size();
    for (int i = 0; i < num_planes; i++) {
        int out = 0;



        out += ((mathfu::vec4::DotProduct(planes[i], mathfu::vec4(box.min.x, box.min.y, box.min.z, 1.0)) < 0.0 ) ? 1 : 0);
        out += ((mathfu::vec4::DotProduct(planes[i], mathfu::vec4(box.max.x, box.min.y, box.min.z, 1.0)) < 0.0 ) ? 1 : 0);
        out += ((mathfu::vec4::DotProduct(planes[i], mathfu::vec4(box.min.x, box.max.y, box.min.z, 1.0)) < 0.0 ) ? 1 : 0);
        out += ((mathfu::vec4::DotProduct(planes[i], mathfu::vec4(box.max.x, box.max.y, box.min.z, 1.0)) < 0.0 ) ? 1 : 0);
        out += ((mathfu::vec4::DotProduct(planes[i], mathfu::vec4(box.min.x, box.min.y, box.max.z, 1.0)) < 0.0 ) ? 1 : 0);
        out += ((mathfu::vec4::DotProduct(planes[i], mathfu::vec4(box.max.x, box.min.y, box.max.z, 1.0)) < 0.0 ) ? 1 : 0);
        out += ((mathfu::vec4::DotProduct(planes[i], mathfu::vec4(box.min.x, box.max.y, box.max.z, 1.0)) < 0.0 ) ? 1 : 0);
        out += ((mathfu::vec4::DotProduct(planes[i], mathfu::vec4(box.max.x, box.max.y, box.max.z, 1.0)) < 0.0 ) ? 1 : 0);

        if (out == 8) return false;
    }

    // check frustum outside/inside box
//    if (points.size() > 0) {
//        int out = 0; for (int i = 0; i < 8; i++) out += ((points[i].x > box.max.x) ? 1 : 0); if (out == 8) return false;
//            out = 0; for (int i = 0; i < 8; i++) out += ((points[i].x < box.min.x) ? 1 : 0); if (out == 8) return false;
//            out = 0; for (int i = 0; i < 8; i++) out += ((points[i].y > box.max.y) ? 1 : 0); if (out == 8) return false;
//            out = 0; for (int i = 0; i < 8; i++) out += ((points[i].y < box.min.y) ? 1 : 0); if (out == 8) return false;
//            out = 0; for (int i = 0; i < 8; i++) out += ((points[i].z > box.max.z) ? 1 : 0); if (out == 8) return false;
//            out = 0; for (int i = 0; i < 8; i++) out += ((points[i].z < box.min.z) ? 1 : 0); if (out == 8) return false;
//    }

    return true;
}

bool MathHelper::checkFrustum2D(std::vector<mathfu::vec3> &planes, CAaBox &box) {

    //var maxLines = window.lines != null ? window.lines : 1;
    //for (var i = 0; i < Math.min(num_planes, maxLines); i++) {
    for (int i = 0; i < planes.size(); i++) {
        int out = 0;
        out += (((planes[i][0]*box.min.x+ planes[i][1]*box.min.y+ planes[i][2]) > 0.0 ) ? 1 : 0);
        out += (((planes[i][0]*box.max.x+ planes[i][1]*box.min.y+ planes[i][2]) > 0.0 ) ? 1 : 0);
        out += (((planes[i][0]*box.min.x+ planes[i][1]*box.max.y+ planes[i][2]) > 0.0 ) ? 1 : 0);
        out += (((planes[i][0]*box.max.x+ planes[i][1]*box.max.y+ planes[i][2]) > 0.0 ) ? 1 : 0);
        if (out == 4) return false;
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
bool MathHelper::planeCull(std::vector<mathfu::vec3> &points, std::vector<mathfu::vec4> &planes) {
    // check box outside/inside of frustum
    std::vector<mathfu::vec4> vec4Points(points.size());

    const float epsilon = 0.001;

    for (int j = 0; j < points.size(); j++) {
        vec4Points[j] = mathfu::vec4(points[j].x, points[j].y, points[j].z, 1.0);
    }


    for (int i = 0; i < planes.size(); i++) {
        int out = 0;
        float epsilon = 0;

        for (int j = 0; j < vec4Points.size(); j++) {
            out += ((mathfu::vec4::DotProduct(planes[i], vec4Points[j]) + epsilon < 0.0 ) ? 1 : 0);
        }

        if (out == vec4Points.size()) {
            return false;
        }

        //---------------------------------
        // Cull by points by current plane
        //---------------------------------

        std::vector<mathfu::vec4> resultPoints;

//        mathfu::vec3 pointO;
//        if (planes[i][2] != 0) {
//            pointO = mathfu::vec3(0, 0, -planes[i][3] / planes[i][2]);
//        } else if (planes[i][1] != 0) {
//            pointO = mathfu::vec3(0, -planes[i][3] / planes[i][1], 0);
//        } else if (planes[i][0] != 0) {
//            pointO = mathfu::vec3(-planes[i][3] / planes[i][0], 0, 0);
//        } else {
//            continue;
//        }

        for (int j = 0; j < vec4Points.size(); j++) {
            mathfu::vec4 p1 = vec4Points[j];
            mathfu::vec4 p2 = vec4Points[(j + 1) % vec4Points.size()];

            mathfu::vec3 p1_xyz = p1.xyz();
            mathfu::vec3 p2_xyz = p2.xyz();

            // InFront = plane.Distance( point ) > 0.0f
            // Behind  = plane.Distance( point ) < 0.0f

            float t1 = mathfu::vec4::DotProduct(p1, planes[i]);
            float t2 = mathfu::vec4::DotProduct(p2, planes[i]);

            if (t1 > 0 && t2 > 0) { //p1 InFront and p2 InFront
                resultPoints.push_back(p2);
            } else if (t1 > 0 && t2 < -epsilon) { //p1 InFront and p2 Behind
//                float k = std::fabs(t1) / (std::fabs(t1) + std::fabs(t2));
                resultPoints.push_back(MathHelper::planeLineIntersection( planes[i], p1, p2));
            } else if (t1 < -epsilon && t2 > 0) { //p1 Behind and p2 InFront
//                float k = std::fabs(t1) / (std::fabs(t1) + std::fabs(t2));
                resultPoints.push_back(MathHelper::planeLineIntersection( planes[i], p1, p2));
                resultPoints.push_back(p2);
            } else if (t2 < epsilon && t2 > -epsilon) { //P2 Inside
                resultPoints.push_back(p2);
            }
        }

        vec4Points = std::vector<mathfu::vec4>(resultPoints.begin(), resultPoints.end());
    }

    points = std::vector<mathfu::vec3>(vec4Points.size());
    for (int j = 0; j < vec4Points.size(); j++) {
        points[j] = vec4Points[j].xyz();
    }

    return vec4Points.size() > 2;

}

void MathHelper::sortVec3ArrayAgainstPlane(std::vector<mathfu::vec3> &thisPortalVertices,
                                                  const mathfu::vec4 &plane) {
    mathfu::vec3 center(0, 0, 0);
    for (int j = 0; j < thisPortalVertices.size(); j++) {
        center += thisPortalVertices[j];
    }
    center *= 1.0f / (float)thisPortalVertices.size();

    std::sort(thisPortalVertices.begin(), thisPortalVertices.end(),
        [&](mathfu::vec3 &a, mathfu::vec3 &b) -> bool {
            mathfu::vec3 ac = a - center;

            mathfu::vec3 bc = b - center;

            mathfu::vec3 cross = mathfu::vec3::CrossProduct(ac, bc).Normalized();
            float dotResult = mathfu::vec3::DotProduct(cross, plane.xyz());

            return dotResult < 0;
    });
}

bool MathHelper::isPointInsideAABB(const CAaBox &aabb, mathfu::vec3 &p) {
    bool result = p[0] > aabb.min.x && p[0] < aabb.max.x &&
             p[1] > aabb.min.y && p[1] < aabb.max.y &&
             p[2] > aabb.min.z && p[2] < aabb.max.z;
    return result;
}

bool MathHelper::isPointInsideAABB(const mathfu::vec2 aabb[2], mathfu::vec2 &p) {
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

    if (MathHelper::isPointInsideAABB(aabb, p))
        return std::min(dx, dy);    // or 0 in case of distance from the area
    else
        return dx * dx + dy * dy;
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
    if ((denom < 0.0001) && (denom > -0.0001)) {
        return mathfu::vec3(-1, -1, -1);
    };

    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0 - v - w;
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