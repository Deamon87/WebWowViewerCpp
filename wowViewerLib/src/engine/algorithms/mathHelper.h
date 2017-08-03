//
// Created by deamon on 22.06.17.
//

#ifndef WOWVIEWERLIB_MATHHELPER_H
#define WOWVIEWERLIB_MATHHELPER_H


#include "../persistance/header/commonFileStructs.h"

class MathHelper {
public:
    static CAaBox transformAABBWithMat4(mathfu::mat4 matrix, mathfu::vec4 min, mathfu::vec4 max);
    static std::vector<mathfu::vec4> getFrustumClipsFromMatrix(mathfu::mat4 &mat);
    static void fixNearPlane(std::vector<mathfu::vec4> planes, mathfu::vec4 camera) {
        mathfu::vec4 &nearPlane = planes[5];
        mathfu::vec4 cameraVec4 = mathfu::vec4(camera[0], camera[1], camera[2], 1);
        float dist = mathfu::dot(nearPlane, cameraVec4);
        nearPlane[3] -= dist;
    }
    static std::vector<mathfu::vec3> calculateFrustumPointsFromMat(mathfu::mat4 &perspectiveViewMat);
};


#endif //WOWVIEWERLIB_MATHHELPER_H
