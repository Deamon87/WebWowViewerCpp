//
// Created by deamon on 15.06.17.
//

#ifndef WOWVIEWERLIB_CAMERAINTERFACE_H
#define WOWVIEWERLIB_CAMERAINTERFACE_H

class ICamera;
#include <mathfu/vector.h>
#include "../../include/controllable.h"
#include "../../include/iostuff.h"
#include "../CameraMatrices.h"


class ICamera : public IControllable {
public:

    virtual HCameraMatrices getCameraMatrices(float fov,
                                              float canvasAspect,
                                              float nearPlane,
                                              float farPlane) = 0;

    virtual void tick(animTime_t timeDelta) = 0;
    virtual float getMovementSpeed() = 0;
    virtual void setMovementSpeed(float value) = 0;
    virtual void setCameraOffset(float x, float y, float z) {};

    virtual bool isCompatibleWithInfiniteZ() {
        return true;
    }
};

#endif //WOWVIEWERLIB_CAMERAINTERFACE_H
