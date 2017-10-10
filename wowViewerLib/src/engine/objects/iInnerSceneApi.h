//
// Created by deamon on 08.09.17.
//

#ifndef WEBWOWVIEWERCPP_IINNERSCENEAPI_H
#define WEBWOWVIEWERCPP_IINNERSCENEAPI_H

#include <mathfu/glsl_mappings.h>

class iInnerSceneApi {
public:
    virtual void checkCulling(mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos) = 0;
    virtual void draw() = 0;

    virtual void update(double deltaTime, mathfu::vec3 cameraVec3,  mathfu::mat4 &frustumMat, mathfu::mat4 lookAtMat) = 0;
};
#endif //WEBWOWVIEWERCPP_IINNERSCENEAPI_H
