//
// Created by deamon on 18.05.17.
//

#include <iostream>
#include "../algorithms/mathHelper.h"
#include "firstPersonCamera.h"
#include "math.h"

void FirstPersonCamera::addForwardDiff(float val) {
    this->depthDiff = this->depthDiff + val;
}

void FirstPersonCamera::addHorizontalViewDir(float val) {
    delta_x += val;
//    this->ah = ah + val;
}
void FirstPersonCamera::addVerticalViewDir(float val) {
    delta_y += val;
}

void FirstPersonCamera::startMovingForward(){
    this->MDDepthPlus = 1;
}
void FirstPersonCamera::stopMovingForward(){
    this->MDDepthPlus = 0;
}
void FirstPersonCamera::startMovingBackwards(){
    this->MDDepthMinus = 1;
}
void FirstPersonCamera::stopMovingBackwards(){
    this->MDDepthMinus = 0;
}

void FirstPersonCamera::startStrafingLeft(){
    this->MDHorizontalMinus = 1;
}
void FirstPersonCamera::stopStrafingLeft(){
    this->MDHorizontalMinus = 0;
}
void FirstPersonCamera::startStrafingRight(){
    this->MDHorizontalPlus = 1;
}
void FirstPersonCamera::stopStrafingRight(){
    this->MDHorizontalPlus = 0;
}

void FirstPersonCamera::startMovingUp(){
    this->MDVerticalPlus = 1;
}
void FirstPersonCamera::stopMovingUp(){
    this->MDVerticalPlus = 0;
}
void FirstPersonCamera::startMovingDown(){
    this->MDVerticalMinus = 1;
}
void FirstPersonCamera::stopMovingDown(){
    this->MDVerticalMinus = 0;
}

void FirstPersonCamera::setMovementSpeed(float value) {
    this->m_moveSpeed = value;
};

float springiness = 300; // tweak to taste.

void FirstPersonCamera::tick (animTime_t timeDelta) {
    double d = 1.0f-exp(log(0.5f)*springiness*timeDelta);

    ah += delta_x*d;
    av += delta_y*d;

    delta_x = 0;
    delta_y = 0;


    if (av < -89.99999f) {
        av = -89.99999f;
    } else if (av > 89.99999f) {
        av = 89.99999f;
    }


    mathfu::vec3 dir = {1, 0, 0};
    mathfu::vec3 up = {0, 0, 1};
    float moveSpeed = m_moveSpeed * 1.0f / 30.0f;
    mathfu::vec3 camera = this->camera.xyz();

    double dTime = timeDelta;

    float horizontalDiff = (float) (dTime * moveSpeed * (this->MDHorizontalPlus - this->MDHorizontalMinus));
    float depthDiff      = (float) (dTime * moveSpeed * (this->MDDepthPlus - this->MDDepthMinus) + this->depthDiff);
    float verticalDiff   = (float) (dTime * moveSpeed * (this->MDVerticalPlus - this->MDVerticalMinus));

    this->depthDiff = 0;

    /* Calc look at position */

    dir = mathfu::mat3::RotationY((float) (this->av * M_PI / 180.0f)) * dir;
    dir = mathfu::mat3::RotationZ((float) (-this->ah * M_PI / 180.0f)) * dir;

    dir = mathfu::normalize(dir);


    mathfu::vec3 right_move = mathfu::mat3::RotationZ((float) (-90.0f * M_PI / 180.0f)) * dir;
    right_move[2] = 0;
    right_move = mathfu::normalize(right_move);

    up = mathfu::normalize(mathfu::vec3::CrossProduct(right_move,dir));

    /* Calc camera position */
    if (horizontalDiff != 0) {
//        right = right * horizontalDiff;

        camera = camera + right_move*horizontalDiff;
    }

    if (depthDiff != 0) {
        mathfu::vec3 movDir = mathfu::vec3(dir);

        movDir = movDir * depthDiff;
        camera = camera + movDir;
    }
    if (verticalDiff != 0) {
        camera[2] = camera[2] + verticalDiff;
    }

    this->lookAt = camera + dir;

//    cameraRotationMat = cameraRotationMat * MathHelper::RotationX(90*M_PI/180);
    lookAtMat = mathfu::mat4(
        right_move.x, up.x, -dir.x, 0.0f,
        right_move.y, up.y, -dir.y, 0.0f,
        right_move.z, up.z, -dir.z, 0.0f,
        0,0,0,1.0f //translation
    );
    lookAtMat *= mathfu::mat4::FromTranslationVector(-camera) ;

    this->camera = mathfu::vec4(camera, 1.0);//(lookAtMat.Inverse() * mathfu::vec4(0,0,0,1.0)).xyz();

    //std::cout<<"camera " << camera[0] <<" "<<camera[1] << " " << camera[2] << " " << std::endl;

    mathfu::mat4 invTranspViewMat = lookAtMat.Transpose().Inverse();

    mathfu::vec4 interiorSunDir = mathfu::vec4(-0.30822f, -0.30822f, -0.89999998f, 0);
    interiorSunDir = invTranspViewMat * interiorSunDir;
    interiorSunDir = mathfu::vec4(interiorSunDir.xyz() * (1.0f / interiorSunDir.xyz().Length()), 0.0f);

    this->interiorDirectLightDir = interiorSunDir;

    mathfu::vec4 upVector ( 0.0, 0.0 , 1.0 , 0.0);
    this->upVector = (invTranspViewMat * upVector.xyz()).Normalized();
    updatedAtLeastOnce = true;
}
void FirstPersonCamera :: setCameraPos (float x, float y, float z) {
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

void FirstPersonCamera::zoomInFromTouch(float val) {
    addForwardDiff(val);
}

void FirstPersonCamera::zoomInFromMouseScroll(float val) {

}

void FirstPersonCamera::addCameraViewOffset(float x, float y) {

}

HCameraMatrices FirstPersonCamera::getCameraMatrices(float fov,
                                                     float canvasAspect,
                                                     float nearPlane,
                                                     float farPlane) {
    if (!updatedAtLeastOnce)
        tick(0.0f);

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
