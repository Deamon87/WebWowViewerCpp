//
// Created by Deamon on 22.07.2020.
//

#ifndef AWEBWOWVIEWERCPP_M2TIEDCAMERA_H
#define AWEBWOWVIEWERCPP_M2TIEDCAMERA_H


#include "CameraInterface.h"
#include "../objects/m2/m2Object.h"

class m2TiedCamera : public ICamera {
public:
    m2TiedCamera(std::shared_ptr<M2Object> m2Object, int cameraNum) {
        m_cameraNum = cameraNum;
        m_m2Object = m2Object;
    }
private:
    std::shared_ptr<M2Object> m_m2Object = nullptr;
    int m_cameraNum = -1;

    M2CameraResult m_lastCameraResult;
    mathfu::vec4 lastCameraPos = mathfu::vec4(0,0,0,1.0);
    mathfu::vec4 lastCameraDir = mathfu::vec4(0,0,0,1.0);
public:
    void startMovingForward() override {};
    void stopMovingForward() override {};
    void startMovingBackwards() override {};
    void stopMovingBackwards() override {};
    void startStrafingLeft() override {};
    void stopStrafingLeft() override {};
    void startStrafingRight() override {};
    void stopStrafingRight() override {};
    void startMovingUp() override {};
    void stopMovingUp() override {};
    void startMovingDown() override {};
    void stopMovingDown() override {};
    void stopAllMovement() override {};

    void addForwardDiff(float val) override {};

    void addHorizontalViewDir(float val) override {};
    void addVerticalViewDir(float val) override {};

    void zoomInFromMouseScroll(float val) override {};
    void zoomInFromTouch(float val) override {};
    void addCameraViewOffset(float x, float y) override {};

    void setCameraPos(float x, float y, float z) override {};
    void getCameraPosition(float *position)  override ;


    HCameraMatrices getCameraMatrices(float fov,
                                              float canvasAspect,
                                              float nearPlane,
                                              float farPlane) override;

    void tick(animTime_t timeDelta) override ;
    void setMovementSpeed(float value) override {};
    void setCameraOffset(float x, float y, float z) override {};

    bool isCompatibleWithInfiniteZ() override {
        return false;
    }

    void setCameraLookAt(float x, float y, float z) override {};

};


#endif //AWEBWOWVIEWERCPP_M2TIEDCAMERA_H
