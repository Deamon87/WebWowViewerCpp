//
// Created by deamon on 18.05.17.
//

#ifndef WOWMAPVIEWERREVIVED_FIRSTPERSONORTHOCAMERA_H
#define WOWMAPVIEWERREVIVED_FIRSTPERSONORTHOCAMERA_H

#include <mathfu/vector.h>
#include "mathfu/glsl_mappings.h"
#include "CameraInterface.h"


class FirstPersonOrthoStaticCamera: public ICamera {
public:
    FirstPersonOrthoStaticCamera(){};

private:
    mathfu::vec4 camera = {0, 0, 0, 0};
    mathfu::vec4 interiorDirectLightDir = {0, 0, 0, 0};
    mathfu::vec3 lookAt = {0, 0, 0};
    mathfu::vec3 upVector = {0, 0, 0};
    mathfu::mat4 lookAtMat = {};
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
    void zoomInFromMouseScroll(float val) override {};
    void zoomInFromTouch(float val)  override {};
    void addCameraViewOffset(float x, float y)  override {};
    void setMovementSpeed(float value) override {};

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
    void setCameraLookAt(float x, float y, float z);
};


#endif //WOWMAPVIEWERREVIVED_FIRSTPERSONORTHOCAMERA_H
