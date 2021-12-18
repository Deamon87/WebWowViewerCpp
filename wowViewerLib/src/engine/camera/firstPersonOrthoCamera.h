//
// Created by deamon on 18.05.17.
//

#ifndef WOWMAPVIEWERREVIVED_FIRSTPERSONORTHOCAMERA_H
#define WOWMAPVIEWERREVIVED_FIRSTPERSONORTHOCAMERA_H

#define _USE_MATH_DEFINES
#include <mathfu/vector.h>
#include "mathfu/glsl_mappings.h"
#include "CameraInterface.h"


class FirstPersonOrthoCamera: public ICamera {
public:
    FirstPersonOrthoCamera(){};

private:
    mathfu::vec4 camera = {0, 0, 0, 0};
    mathfu::vec4 interiorDirectLightDir = {0, 0, 0, 0};
    mathfu::vec3 lookAt = {0, 0, 0};
    mathfu::vec3 upVector = {0, 0, 0};
    mathfu::mat4 lookAtMat = {};

    float MDDepthPlus = 0;
    float MDDepthMinus = 0;
    float MDHorizontalPlus = 0;
    float MDHorizontalMinus = 0;

    float MDVerticalPlus = 0;
    float MDVerticalMinus = 0;

    float depthDiff = 0;

    bool staticCamera = false;

    float ah = 0;
    float av = 89;
public:
    //Implemented IControllable
    void addHorizontalViewDir(float val) override;
    void addVerticalViewDir(float val) override;
    void addForwardDiff(float val) override;
    void startMovingForward() override;
    void stopMovingForward() override;
    void startMovingBackwards() override;
    void stopMovingBackwards() override;
    void startStrafingLeft() override;
    void stopStrafingLeft() override;
    void startStrafingRight() override;
    void stopStrafingRight() override;
    void startMovingUp() override;
    void stopMovingUp() override;
    void startMovingDown() override;
    void stopMovingDown() override;

    void getCameraPosition(float *position) override {
        position[0] = camera.x;
        position[1] = camera.y;
        position[2] = camera.z;
    }


public:
    //Implemented ICamera
    HCameraMatrices getCameraMatrices(float fov,
                                      float canvasAspect,
                                      float nearPlane,
                                      float farPlane) override;

public:
    void tick(animTime_t timeDelta) override;
    void setCameraPos(float x, float y, float z) override;
};


#endif //WOWMAPVIEWERREVIVED_FIRSTPERSONORTHOCAMERA_H
