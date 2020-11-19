//
// Created by deamon on 18.05.17.
//

#include <iostream>
#include "FirstPersonOrthoStaticTopDownCamera.h"
#include "m2TiedCamera.h"

void FirstPersonOrthoStaticTopDownCamera::addForwardDiff(float val) {}
void FirstPersonOrthoStaticTopDownCamera::addHorizontalViewDir(float val) {}
void FirstPersonOrthoStaticTopDownCamera::addVerticalViewDir(float val) {}
void FirstPersonOrthoStaticTopDownCamera::startMovingForward(){}
void FirstPersonOrthoStaticTopDownCamera::stopMovingForward(){}
void FirstPersonOrthoStaticTopDownCamera::startMovingBackwards(){}
void FirstPersonOrthoStaticTopDownCamera::stopMovingBackwards(){}
void FirstPersonOrthoStaticTopDownCamera::startStrafingLeft(){}
void FirstPersonOrthoStaticTopDownCamera::stopStrafingLeft(){}
void FirstPersonOrthoStaticTopDownCamera::startStrafingRight(){}
void FirstPersonOrthoStaticTopDownCamera::stopStrafingRight(){}
void FirstPersonOrthoStaticTopDownCamera::startMovingUp(){}
void FirstPersonOrthoStaticTopDownCamera::stopMovingUp(){}
void FirstPersonOrthoStaticTopDownCamera::startMovingDown(){}
void FirstPersonOrthoStaticTopDownCamera::stopMovingDown(){}

void FirstPersonOrthoStaticTopDownCamera::tick (animTime_t timeDelta) {
//    cameraRotationMat = cameraRotationMat * MathHelper::RotationX(90*M_PI/180);
    lookAtMat =
        mathfu::mat4(
            0,1,0,0,
            -1,0,0,0,
            0,0,1,0,
            0,0,0,1
         ) *  mathfu::mat4::FromTranslationVector(-this->camera.xyz());

    mathfu::vec4 interiorSunDir = mathfu::vec4(-0.30822f, -0.30822f, -0.89999998f, 0);
    interiorSunDir = lookAtMat.Transpose().Inverse() * interiorSunDir;
    interiorSunDir = mathfu::vec4(interiorSunDir.xyz() * (1.0f / interiorSunDir.xyz().Length()), 0.0f);

    this->interiorDirectLightDir = interiorSunDir;

    mathfu::vec4 upVector ( 0.0, 0.0 , 1.0 , 0.0);
    mathfu::mat3 lookAtRotation = mathfu::mat4::ToRotationMatrix(lookAtMat);
    this->upVector = (lookAtRotation * upVector.xyz());
}
void FirstPersonOrthoStaticTopDownCamera::setCameraPos (float x, float y, float z) {
    //Reset camera
    this->camera = mathfu::vec4(x, y, z, 0);
}
void FirstPersonOrthoStaticTopDownCamera::setCameraLookAt(float x, float y, float z) {
    this->lookAt = mathfu::vec3(x, y, z);
}


HCameraMatrices FirstPersonOrthoStaticTopDownCamera::getCameraMatrices(float fov,
                                                                float canvasAspect,
                                                                float nearPlane,
                                                                float farPlane) {
    HCameraMatrices cameraMatrices = std::make_shared<CameraMatrices>();
    cameraMatrices->perspectiveMat = mathfu::mat4::Perspective(
        fov,
        canvasAspect,
        nearPlane,
        farPlane);
    cameraMatrices->lookAtMat = lookAtMat;

    cameraMatrices->cameraPos = camera;
    cameraMatrices->viewUp = mathfu::vec4(upVector, 0);
    cameraMatrices->interiorDirectLightDir = this->interiorDirectLightDir;

    return cameraMatrices;
}

