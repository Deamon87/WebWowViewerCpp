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
        return false;
    }

    static inline mathfu::mat4 persectiveInvertZ(float aspectRatio, float fovy, float n, float f)
    {
        mathfu::mat4 result;

        float s = aspectRatio;
        float g = 1.0f / std::tan(fovy * static_cast<float>(.5));
        float A = n / (f - n);
        float B = f*n / (f - n);

        result = mathfu::mat4(g/s , 0.0f, 0.0f, 0.0f,
                              0.0f,  g  , 0.0f, 0.0f,
                              0.0f, 0.0f,  A  ,  -1.0f  ,
                              0.0f, 0.0f, B, 0.0f);

        return result;
    }

};

#endif //WOWVIEWERLIB_CAMERAINTERFACE_H
