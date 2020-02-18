//
// Created by Deamon on 10/15/2018.
//

#ifndef AWEBWOWVIEWERCPP_PLANARCAMERA_H
#define AWEBWOWVIEWERCPP_PLANARCAMERA_H



#include <mathfu/vector.h>
#include "mathfu/glsl_mappings.h"
#include "CameraInterface.h"

class PlanarCamera: public ICamera {
public:
    PlanarCamera(){};

private:

    mathfu::vec3 camera = {0, 0, 0};
    mathfu::vec4 interiorDirectLightDir = {0, 0, 0, 0};
    mathfu::vec3 cameraOffset = {0, 0, 0};
    mathfu::vec3 lookAt = {0, 0, 0};
    mathfu::vec3 upVector = {0, 0, 0};
    mathfu::mat4 lookAtMat = {};

    mathfu::vec2 cameraViewOffset = {0, 0};

    float m_radius = 50;
    float MDDepthPlus = 0;
    float MDDepthMinus = 0;
    float MDHorizontalPlus = 0;
    float MDHorizontalMinus = 0;

    float MDVerticalPlus = 0;
    float MDVerticalMinus = 0;

    float depthDiff = 0;
    float m_moveSpeed = 1.0;

    bool staticCamera = false;

    float ah = 0;
    float av = 0;

    float delta_x = 0;
    float delta_y = 0;
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

    void addCameraViewOffset(float x, float y) override;

    void zoomInFromMouseScroll(float val) override;
    void zoomInFromTouch(float val) override;

    void getCameraPosition(float *position) override {
        position[0] = camera.x;
        position[1] = camera.y;
        position[2] = camera.z;
    }

    void setMovementSpeed(float value) override;

public:
    //Implemented ICamera
    HCameraMatrices getCameraMatrices(float fov,
                                      float canvasAspect,
                                      float nearPlane,
                                      float farPlane) override;

public:
    void tick(animTime_t timeDelta) override;
    void setCameraPos(float x, float y, float z) override;
    void setCameraOffset(float x, float y, float z);
};

#endif //AWEBWOWVIEWERCPP_PLANARCAMERA_H
