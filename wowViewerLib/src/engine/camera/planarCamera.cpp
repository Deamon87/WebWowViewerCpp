//
// Created by Deamon on 10/15/2018.
//

#include "planarCamera.h"


void PlanarCamera::addForwardDiff(float val) {
    this->depthDiff = this->depthDiff + val;
}

void PlanarCamera::addHorizontalViewDir(float val) {
    delta_x += val;
}
void PlanarCamera::addVerticalViewDir(float val) {
    delta_y += val;
}

void PlanarCamera::startMovingForward(){
    this->MDDepthPlus = 1;
}
void PlanarCamera::stopMovingForward(){
    this->MDDepthPlus = 0;
}
void PlanarCamera::startMovingBackwards(){
    this->MDDepthMinus = 1;
}
void PlanarCamera::stopMovingBackwards(){
    this->MDDepthMinus = 0;
}

void PlanarCamera::startStrafingLeft(){
    this->MDHorizontalMinus = 1;
}
void PlanarCamera::stopStrafingLeft(){
    this->MDHorizontalMinus = 0;
}
void PlanarCamera::startStrafingRight(){
    this->MDHorizontalPlus = 1;
}
void PlanarCamera::stopStrafingRight(){
    this->MDHorizontalPlus = 0;
}

void PlanarCamera::startMovingUp(){
    this->MDVerticalPlus = 1;
}
void PlanarCamera::stopMovingUp(){
    this->MDVerticalPlus = 0;
}
void PlanarCamera::startMovingDown(){
    this->MDVerticalMinus = 1;
}
void PlanarCamera::stopMovingDown(){
    this->MDVerticalMinus = 0;
}

mathfu::vec3 PlanarCamera::getCameraPosition(){
    return camera;
}
mathfu::vec3 PlanarCamera::getCameraLookAt(){
    return lookAt;
}

void PlanarCamera::setMovementSpeed(float value) {
    this->m_moveSpeed = value;
};



void PlanarCamera::tick (animTime_t timeDelta) {
    const float springiness = 300; // tweak to taste.

    double d = 1-exp(log(0.5)*springiness*timeDelta);


    ah += (delta_x-ah)*d;
    av += (delta_y-av)*d;
    if (av < -89.99999f) {
        av = -89.99999f;
    } else if (av > 89.99999f) {
        av = 89.99999f;
    }
    float polar = this->av * (float)(M_PI) / 180.0f;
    float azimuth = -this->ah * (float)(M_PI) / 180.0f;

    float cosPolar = cos(polar);

    mathfu::vec3 cameraDirection = mathfu::vec3(
        cosPolar * cos(azimuth),
        cosPolar * sin(azimuth),
        sin(polar)
    );

    mathfu::vec3 camera = cameraDirection * mathfu::vec3(m_radius);


    /* Calc look at position */
    mathfu::vec3 right_move = -mathfu::mat3::RotationZ((float) (-90.0f * M_PI / 180.0f)) * cameraDirection;
    right_move[2] = 0;
    right_move = mathfu::normalize(right_move);

    mathfu::vec3 up = {0, 0, 1};
    up = mathfu::normalize(mathfu::vec3::CrossProduct(right_move,-cameraDirection));

    this->camera = camera;
    this->lookAt = {0, 0, 0};

//    cameraRotationMat = cameraRotationMat * MathHelper::RotationX(90*M_PI/180);
    lookAtMat = mathfu::mat4(
        right_move.x, up.x, cameraDirection.x, 0.0f,
        right_move.y, up.y, cameraDirection.y, 0.0f,
        right_move.z, up.z, cameraDirection.z, 0.0f,
        0,0,0,1.0f //translation
    );

    lookAtMat *= mathfu::mat4::FromTranslationVector(-camera) ;
    lookAtMat =
        mathfu::mat4::FromTranslationVector(mathfu::vec3(cameraOffset.x, cameraOffset.z/2.0f, cameraOffset.y)) *
        lookAtMat *
        mathfu::mat4::FromTranslationVector(mathfu::vec3(-cameraOffset.x, -cameraOffset.y, -cameraOffset.z)) ;
    //std::cout<<"camera " << camera[0] <<" "<<camera[1] << " " << camera[2] << " " << std::endl;

}
void PlanarCamera::setCameraPos (float x, float y, float z) {
    //Reset camera

    m_radius = sqrt(x*x + y*y + z*z);
    float polar = asin(z/m_radius);
    float azimuth = M_PI_2 - atan(y/x);

    this->av = polar / ((float)(M_PI) / 180.0f);
    this->ah = -azimuth/((float)(M_PI) / 180.0f);


    this->lookAt[0] = 0;
    this->lookAt[1] = 0;
    this->lookAt[2] = 0;

    this->av = 0;
    this->ah = 0;
}
void PlanarCamera::setCameraOffset(float x, float y, float z) {
    cameraOffset = mathfu::vec3(x,y,z);
}
void PlanarCamera::zoomInFromTouch(float val) {
    m_radius += val;
}

void PlanarCamera::zoomInFromMouseScroll(float val) {
    m_radius += val;
}
