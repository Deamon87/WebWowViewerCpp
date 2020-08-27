//
// Created by deamon on 18.05.17.
//

#include <iostream>
#include "firstPersonOrthoCamera.h"

void FirstPersonOrthoCamera::addForwardDiff(float val) {
    this->depthDiff = this->depthDiff + val;
}

void FirstPersonOrthoCamera::addHorizontalViewDir(float val) {

}
void FirstPersonOrthoCamera::addVerticalViewDir(float val) {

}

void FirstPersonOrthoCamera::startMovingForward(){
    this->MDDepthPlus = 1;
}
void FirstPersonOrthoCamera::stopMovingForward(){
    this->MDDepthPlus = 0;
}
void FirstPersonOrthoCamera::startMovingBackwards(){
    this->MDDepthMinus = 1;
}
void FirstPersonOrthoCamera::stopMovingBackwards(){
    this->MDDepthMinus = 0;
}

void FirstPersonOrthoCamera::startStrafingLeft(){
    this->MDHorizontalMinus = 1;
}
void FirstPersonOrthoCamera::stopStrafingLeft(){
    this->MDHorizontalMinus = 0;
}
void FirstPersonOrthoCamera::startStrafingRight(){
    this->MDHorizontalPlus = 1;
}
void FirstPersonOrthoCamera::stopStrafingRight(){
    this->MDHorizontalPlus = 0;
}

void FirstPersonOrthoCamera::startMovingUp(){
    this->MDVerticalPlus = 1;
}
void FirstPersonOrthoCamera::stopMovingUp(){
    this->MDVerticalPlus = 0;
}
void FirstPersonOrthoCamera::startMovingDown(){
    this->MDVerticalMinus = 1;
}
void FirstPersonOrthoCamera::stopMovingDown(){
    this->MDVerticalMinus = 0;
}

void FirstPersonOrthoCamera::tick (animTime_t timeDelta) {
    mathfu::vec3 dir = {1, 0, 0};
    float moveSpeed = 1.0f / 10.0f;
    mathfu::vec3 camera = this->camera.xyz();

    double dTime = timeDelta;

    float horizontalDiff = (float) (dTime * moveSpeed * (this->MDHorizontalPlus - this->MDHorizontalMinus));
    float depthDiff      = (float) (dTime * moveSpeed * (this->MDDepthPlus - this->MDDepthMinus) + this->depthDiff);
    float verticalDiff   = (float) (dTime * moveSpeed * (this->MDVerticalPlus - this->MDVerticalMinus));

    this->depthDiff = 0;

    /* Calc look at position */

    this->av = 89.9999f;

    dir = mathfu::mat3::RotationY((float) (this->av * M_PI / 180.0f)) * dir;
    dir = mathfu::mat3::RotationZ((float) (-this->ah * M_PI / 180.0f)) * dir;

    dir = mathfu::normalize(dir);


    mathfu::vec3 right_move = mathfu::mat3::RotationZ((float) (-90.0f * M_PI / 180.0f)) * dir;
    right_move[2] = 0;
    right_move = mathfu::normalize(right_move);

    mathfu::vec3 up = mathfu::normalize(mathfu::vec3::CrossProduct(right_move,dir));

    /* Calc camera position */
    if (horizontalDiff != 0) {
//        right = right * horizontalDiff;

        camera = camera + mathfu::vec3(0,1,0)*horizontalDiff;
    }

    if (depthDiff != 0) {
        mathfu::vec3 movDir = mathfu::vec3(dir);

        movDir = mathfu::vec3(1,0,0) * depthDiff;
        camera = camera + movDir;
    }
    if (verticalDiff != 0) {
        camera[2] = camera[2] + verticalDiff;
    }

    this->camera = mathfu::vec4(camera, 1.0);
    this->lookAt = camera + dir;

//    cameraRotationMat = cameraRotationMat * MathHelper::RotationX(90*M_PI/180);
    lookAtMat = mathfu::mat4(
        right_move.x, up.x, -dir.x, 0.0f,
        right_move.y, up.y, -dir.y, 0.0f,
        right_move.z, up.z, -dir.z, 0.0f,
        0,0,0,1.0f //translation
    );

    lookAtMat *= mathfu::mat4::FromTranslationVector(-camera) ;

    mathfu::vec4 interiorSunDir = mathfu::vec4(-0.30822f, -0.30822f, -0.89999998f, 0);
    interiorSunDir = lookAtMat.Transpose().Inverse() * interiorSunDir;
    interiorSunDir = mathfu::vec4(interiorSunDir.xyz() * (1.0f / interiorSunDir.xyz().Length()), 0.0f);

    this->interiorDirectLightDir = interiorSunDir;

    mathfu::vec4 upVector ( 0.0, 0.0 , 1.0 , 0.0);
    mathfu::mat3 lookAtRotation = mathfu::mat4::ToRotationMatrix(lookAtMat);
    this->upVector = (lookAtRotation * upVector.xyz());
}
void FirstPersonOrthoCamera :: setCameraPos (float x, float y, float z) {
    //Reset camera
    this->camera[0] = x;
    this->camera[1] = y;
    this->camera[2] = z;

    this->lookAt[0] = 0;
    this->lookAt[1] = 0;
    this->lookAt[2] = 0;

    this->av = 0;
    this->ah = 0;
}

HCameraMatrices FirstPersonOrthoCamera::getCameraMatrices(float fov,
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
