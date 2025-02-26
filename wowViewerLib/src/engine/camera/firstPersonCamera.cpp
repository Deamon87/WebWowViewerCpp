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
void FirstPersonCamera::stopAllMovement() {
    MDDepthPlus = 0;
    MDDepthMinus = 0;
    MDHorizontalPlus = 0;
    MDHorizontalMinus = 0;

    MDVerticalPlus = 0;
    MDVerticalMinus = 0;
}

float FirstPersonCamera::getMovementSpeed() {
    return this->m_moveSpeed;
}
void FirstPersonCamera::setMovementSpeed(float value) {
    this->m_moveSpeed = value;
};

float springiness = 300; // tweak to taste.

void FirstPersonCamera::tick (animTime_t timeDelta) {
    double d = 1.0f-exp(log(0.5f)*springiness*timeDelta);

    ah += -delta_x*d;
    av += -delta_y*d;

    delta_x = 0;
    delta_y = 0;


    if (av < -89) {
        av = -89;
    } else if (av > 89) {
        av = 89;
    }


    mathfu::vec3 dir = {1, 0, 0};
    mathfu::vec3 up = {0, 0, 1};
    float moveSpeed = m_moveSpeed;
    mathfu::vec3 camera = this->camera.xyz();

    double dTime = timeDelta;

    float horizontalDiff = (float) (dTime * moveSpeed * (this->MDHorizontalPlus - this->MDHorizontalMinus));
    float depthDiff      = (float) (dTime * moveSpeed * (this->MDDepthPlus - this->MDDepthMinus) + this->depthDiff);
    float verticalDiff   = (float) (dTime * moveSpeed * (this->MDVerticalPlus - this->MDVerticalMinus));

    this->depthDiff = 0;

    /* Calc look at position */
    //Spherical to Cartesian conversion formula
    dir = mathfu::vec3(
        1 * sinf(toRadian(90 - av)) * cosf(toRadian(ah)),
        1 * sinf(toRadian(90 - av)) * sinf(toRadian(ah)),
        1 * cosf(toRadian(90 - av))
    );


    mathfu::vec3 right_move = mathfu::vec3(dir.y, -dir.x, 0);
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

    auto dirNorm = dir.Normalized();
    this->lookAt = camera + dirNorm;

    lookAtMat = mathfu::mat4::LookAt(this->lookAt, camera, mathfu::vec3(0,0,1), -1.0f);

    invTranspViewMat = lookAtMat.Inverse().Transpose();

    auto cameraForDebug = (lookAtMat.Inverse() * mathfu::vec4(0,0,0,1.0)).xyz();
    this->camera = mathfu::vec4(camera, 1.0);

    mathfu::vec4 interiorSunDir = mathfu::vec4(-0.30822f, -0.30822f, -0.89999998f, 0);
    interiorSunDir = invTranspViewMat * interiorSunDir;
    interiorSunDir = mathfu::vec4(interiorSunDir.xyz() * (1.0f / interiorSunDir.xyz().Length()), 0.0f);

    this->interiorDirectLightDir = interiorSunDir;

    mathfu::vec4 upVector ( 0.0, 0.0 , 1.0 , 0.0);
    this->upVector = ((invTranspViewMat * upVector).xyz()).Normalized();
    updatedAtLeastOnce = true;
}
void FirstPersonCamera::setCameraPos (float x, float y, float z) {
    //Reset camera
    this->camera = {x, y, z, 1.0f};

    // setCameraLookAt(lookAt.x, lookAt.y, lookAt.z);
}
void FirstPersonCamera::setCameraLookAt(float x, float y, float z) {
    this->lookAt = {x, y, z};

    auto cartesianNorm = (this->lookAt - this->camera.xyz()).Normalized();

    //ah is phi
    //90-av is theta

    av = -fromRadian(acosf(cartesianNorm.z)) + 90.0f;

    if (!feq(cartesianNorm.x, 0.0)) {
        ah = fromRadian(atanf(cartesianNorm.y / cartesianNorm.x));
        if (cartesianNorm.x < 0.0)
            ah = ah + 180.0f;
    } else {
        ah = (cartesianNorm.y > 0.0) ? 90 : -90;
    }

    while (av < -180.0f) { av += 360.0f; }
    while (av > 180.0f) { av -= 360.0f; }

    // if (av > 90) {
    //     av = 90 - (av - 90);
    //     ah += 180;
    // } else if (av < -90) {
    //     av = -90 - (av + 90)  ;
    //     ah += 180;
    // }
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
    cameraMatrices->perspectiveMat = MathHelper::createPerspectiveMat(
        fov,
        canvasAspect,
        nearPlane,
        farPlane);
    cameraMatrices->lookAtMat = lookAtMat;
    cameraMatrices->invTranspViewMat = invTranspViewMat;

    cameraMatrices->cameraPos = camera;
    cameraMatrices->lookAt = mathfu::vec4(lookAt, 1.0f);
    cameraMatrices->viewUp = mathfu::vec4(upVector, 0);
    cameraMatrices->interiorDirectLightDir = this->interiorDirectLightDir;


    return cameraMatrices;
}
