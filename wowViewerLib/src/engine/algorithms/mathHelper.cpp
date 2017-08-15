//
// Created by deamon on 22.06.17.
//

#include "mathHelper.h"
#include "grahamScan.h"

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
        planes[i] *= invVecLength;
    }

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

std::vector<mathfu::vec3> MathHelper::getHullLines(std::vector<Point> &points){
    std::stack<Point> hullPoints = grahamScan(points);
    mathfu::vec3* end   = &hullPoints.top() + 1;
    mathfu::vec3* begin = end - hullPoints.size();
    std::vector<mathfu::vec3> hullPointsArr(begin, end);

    mathfu::vec2 centerPoint = mathfu::vec2(0,0);
    for (int i = 0; i< hullPoints.size(); i++) {
        centerPoint += hullPointsArr[i].xy();
    }
    centerPoint = centerPoint * ((float)1/hullPoints.size());
    std::sort(hullPointsArr.begin(),
              hullPointsArr.end(),
              std::bind(hullSort, std::placeholders::_1, std::placeholders::_2, centerPoint));

    std::vector<mathfu::vec3> hullLines;
    
    if (hullPoints.size() > 2) {
        for (int i = 0; i < hullPointsArr.size() - 1; i++) {
            int index1 = i+1;
            int index2 = i;

            mathfu::vec3 line (
                hullPointsArr[index1].y - hullPointsArr[index2].y,
                hullPointsArr[index2].x - hullPointsArr[index1].x,
                -hullPointsArr[index1].y*(hullPointsArr[index2].x - hullPointsArr[index1].x) +
                hullPointsArr[index1].x*( hullPointsArr[index2].y - hullPointsArr[index1].y)
            );
            float normalLength = sqrt(line[0]*line[0] + line[1]+line[1]);
            line =  line * (1/normalLength);

            hullLines.push_back(line);
        }

        int index1 = 0;
        int index2 = hullPointsArr.size() - 1;
        mathfu::vec3 line (
            hullPointsArr[index1].y - hullPointsArr[index2].y,
            hullPointsArr[index2].x - hullPointsArr[index1].x,
            -hullPointsArr[index1].y*(hullPointsArr[index2].x - hullPointsArr[index1].x) +
            hullPointsArr[index1].x*( hullPointsArr[index2].y - hullPointsArr[index1].y)
        );
        float normalLength = sqrt(line[0]*line[0] + line[1]+line[1]);
        line *= 1/normalLength;

        hullLines.push_back(line);
    } else {
        //"invalid number of hullPoints"
    }

    return hullLines;
}

static bool MathHelper::checkFrustum(std::vector<mathfu::vec4> &planes, CAaBox &box, std::vector<mathfu::vec3> &points) {
    // check box outside/inside of frustum
    int num_planes = planes.size();
    for (int i = 0; i < num_planes; i++) {
        int out = 0;
        out += (((planes[i][0]*box.min.x+ planes[i][1]*box.min.y+ planes[i][2]*box.min.z+planes[i][3]) < 0.0 ) ? 1 : 0);
        out += (((planes[i][0]*box.max.x+ planes[i][1]*box.min.y+ planes[i][2]*box.min.z+planes[i][3]) < 0.0 ) ? 1 : 0);
        out += (((planes[i][0]*box.min.x+ planes[i][1]*box.max.y+ planes[i][2]*box.min.z+planes[i][3]) < 0.0 ) ? 1 : 0);
        out += (((planes[i][0]*box.max.x+ planes[i][1]*box.max.y+ planes[i][2]*box.min.z+planes[i][3]) < 0.0 ) ? 1 : 0);
        out += (((planes[i][0]*box.min.x+ planes[i][1]*box.min.y+ planes[i][2]*box.max.z+planes[i][3]) < 0.0 ) ? 1 : 0);
        out += (((planes[i][0]*box.max.x+ planes[i][1]*box.min.y+ planes[i][2]*box.max.z+planes[i][3]) < 0.0 ) ? 1 : 0);
        out += (((planes[i][0]*box.min.x+ planes[i][1]*box.max.y+ planes[i][2]*box.max.z+planes[i][3]) < 0.0 ) ? 1 : 0);
        out += (((planes[i][0]*box.max.x+ planes[i][1]*box.max.y+ planes[i][2]*box.max.z+planes[i][3]) < 0.0 ) ? 1 : 0);

        if (out == 8) return false;
    }

    // check frustum outside/inside box
    if (points.size() > 0) {
        int out = 0; for (int i = 0; i < 8; i++) out += ((points[i].x > box.max.x) ? 1 : 0); if (out == 8) return false;
            out = 0; for (int i = 0; i < 8; i++) out += ((points[i].x < box.min.x) ? 1 : 0); if (out == 8) return false;
            out = 0; for (int i = 0; i < 8; i++) out += ((points[i].y > box.max.y) ? 1 : 0); if (out == 8) return false;
            out = 0; for (int i = 0; i < 8; i++) out += ((points[i].y < box.min.y) ? 1 : 0); if (out == 8) return false;
            out = 0; for (int i = 0; i < 8; i++) out += ((points[i].z > box.max.z) ? 1 : 0); if (out == 8) return false;
            out = 0; for (int i = 0; i < 8; i++) out += ((points[i].z < box.min.z) ? 1 : 0); if (out == 8) return false;
    }

    return true;
}