//
// Created by deamon on 22.06.17.
//

#include "mathHelper.h"

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
