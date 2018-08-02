//
// Created by deamon on 18.05.17.
//

#include <iostream>
#include "firstPersonCamera.h"
#include "math.h"
#include "../algorithms/mathHelper.h"

void FirstPersonCamera::addForwardDiff(float val) {
    this->depthDiff = this->depthDiff + val;
}

void FirstPersonCamera::addHorizontalViewDir(float val) {
    this->ah = ah + val;
}
void FirstPersonCamera::addVerticalViewDir(float val) {
    float av = this->av;
    av += val;

    if (av < -89.99999f) {
        av = -89.99999f;
    } else if (av > 89.99999f) {
        av = 89.99999f;
    }
    this->av = av;
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

mathfu::vec3 FirstPersonCamera::getCameraPosition(){
    return camera;
}
mathfu::vec3 FirstPersonCamera::getCameraLookAt(){
    return lookAt;
}

void FirstPersonCamera::setMovementSpeed(float value) {
    this->m_moveSpeed = value;
};


void FirstPersonCamera::tick (animTime_t timeDelta) {
    mathfu::vec3 dir = {1, 0, 0};
    mathfu::vec3 up = {0, 0, 1};
    float moveSpeed = m_moveSpeed * 1.0f / 30.0f;
    mathfu::vec3 camera = this->camera;

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


    //std::cout<<"camera " << camera[0] <<" "<<camera[1] << " " << camera[2] << " " << std::endl;

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