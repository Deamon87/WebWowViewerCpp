//
// Created by deamon on 18.05.17.
//

#include <iostream>
#include "firstPersonOrthoStaticCamera.h"

void FirstPersonOrthoStaticCamera::addForwardDiff(float val) {}
void FirstPersonOrthoStaticCamera::addHorizontalViewDir(float val) {}
void FirstPersonOrthoStaticCamera::addVerticalViewDir(float val) {}
void FirstPersonOrthoStaticCamera::startMovingForward(){}
void FirstPersonOrthoStaticCamera::stopMovingForward(){}
void FirstPersonOrthoStaticCamera::startMovingBackwards(){}
void FirstPersonOrthoStaticCamera::stopMovingBackwards(){}
void FirstPersonOrthoStaticCamera::startStrafingLeft(){}
void FirstPersonOrthoStaticCamera::stopStrafingLeft(){}
void FirstPersonOrthoStaticCamera::startStrafingRight(){}
void FirstPersonOrthoStaticCamera::stopStrafingRight(){}
void FirstPersonOrthoStaticCamera::startMovingUp(){}
void FirstPersonOrthoStaticCamera::stopMovingUp(){}
void FirstPersonOrthoStaticCamera::startMovingDown(){}
void FirstPersonOrthoStaticCamera::stopMovingDown(){}

void FirstPersonOrthoStaticCamera::tick (animTime_t timeDelta) {
//    cameraRotationMat = cameraRotationMat * MathHelper::RotationX(90*M_PI/180);
    lookAtMat =
        mathfu::mat4::LookAt(
            lookAt,
            camera.xyz(),
            mathfu::vec3(0,0,1), 1.0);


    mathfu::vec4 interiorSunDir = mathfu::vec4(-0.30822f, -0.30822f, -0.89999998f, 0);
    interiorSunDir = lookAtMat.Transpose().Inverse() * interiorSunDir;
    interiorSunDir = mathfu::vec4(interiorSunDir.xyz() * (1.0f / interiorSunDir.xyz().Length()), 0.0f);

    this->interiorDirectLightDir = interiorSunDir;

    mathfu::vec4 upVector ( 0.0, 0.0 , 1.0 , 0.0);
    mathfu::mat3 lookAtRotation = mathfu::mat4::ToRotationMatrix(lookAtMat);
    this->upVector = (lookAtRotation * upVector.xyz());
}
void FirstPersonOrthoStaticCamera::setCameraPos (float x, float y, float z) {
    //Reset camera
    this->camera = mathfu::vec4(x, y, z, 0);
}
void FirstPersonOrthoStaticCamera::setCameraLookAt(float x, float y, float z) {
    this->lookAt = mathfu::vec3(x, y, z);
}


HCameraMatrices FirstPersonOrthoStaticCamera::getCameraMatrices(float fov,
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

