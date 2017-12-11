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

mathfu::vec3 FirstPersonOrthoCamera::getCameraPosition(){
    return camera;
}
mathfu::vec3 FirstPersonOrthoCamera::getCameraLookAt(){
    return lookAt;
}


void FirstPersonOrthoCamera::tick (animTime_t timeDelta) {
    mathfu::vec3 dir = {1, 0, 0};
    float moveSpeed = 1.0f / 10.0f;
    mathfu::vec3 camera = this->camera;

    double dTime = timeDelta;

    float horizontalDiff = dTime * moveSpeed * (this->MDHorizontalPlus - this->MDHorizontalMinus);
    float depthDiff      = dTime * moveSpeed * (this->MDDepthPlus - this->MDDepthMinus) + this->depthDiff;
    float verticalDiff   = dTime * moveSpeed * (this->MDVerticalPlus - this->MDVerticalMinus);

    this->depthDiff = 0;

    /* Calc look at position */

    this->av = 89.9999;

    dir = mathfu::mat3::RotationY(this->av*M_PI/180) * dir;
    dir = mathfu::mat3::RotationZ(-this->ah*M_PI/180) * dir;

    dir = mathfu::normalize(dir);


    mathfu::vec3 right_move = mathfu::mat3::RotationZ(-90*M_PI/180) * dir;
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

    this->camera = camera;
    this->lookAt = camera + dir;

//    cameraRotationMat = cameraRotationMat * MathHelper::RotationX(90*M_PI/180);
    lookAtMat = mathfu::mat4(
        right_move.x, up.x, -dir.x, 0.0f,
        right_move.y, up.y, -dir.y, 0.0f,
        right_move.z, up.z, -dir.z, 0.0f,
        0,0,0,1.0f //translation
    );

    lookAtMat *= mathfu::mat4::FromTranslationVector(-camera) ;
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