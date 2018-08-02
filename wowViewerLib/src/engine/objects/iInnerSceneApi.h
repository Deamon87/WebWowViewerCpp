//
// Created by deamon on 08.09.17.
//

#ifndef WEBWOWVIEWERCPP_IINNERSCENEAPI_H
#define WEBWOWVIEWERCPP_IINNERSCENEAPI_H

#include "mathfu/glsl_mappings.h"
#include "../persistance/header/M2FileHeader.h"

class iInnerSceneApi {
public:
    virtual void checkCulling(mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos) = 0;
    virtual void draw() = 0;

    virtual void doPostLoad() = 0;
    virtual void copyToCurrentFrame() = 0;
    virtual void update(double deltaTime, mathfu::vec3 &cameraVec3,  mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat) = 0;
    virtual mathfu::vec4 getAmbientColor() = 0;
    virtual void setAmbientColorOverride(mathfu::vec4 &ambientColor, bool override) = 0;
    virtual bool getCameraSettings(M2CameraResult &cameraResult) = 0;
};
#endif //WEBWOWVIEWERCPP_IINNERSCENEAPI_H
