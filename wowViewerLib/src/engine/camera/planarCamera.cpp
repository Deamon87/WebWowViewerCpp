//
// Created by Deamon on 10/15/2018.
//
#define _USE_MATH_DEFINES

#include "planarCamera.h"
#include "../../../3rdparty/mathfu/include/mathfu/matrix.h"
#include "../algorithms/mathHelper.h"
#include "../../../3rdparty/mathfu/include/mathfu/glsl_mappings.h"


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
void PlanarCamera::stopAllMovement() {
    MDDepthPlus = 0;
    MDDepthMinus = 0;
    MDHorizontalPlus = 0;
    MDHorizontalMinus = 0;

    MDVerticalPlus = 0;
    MDVerticalMinus = 0;
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
        delta_y = av;
    } else if (av > 89.99999f) {
        av = 89.99999f;
        delta_y = av;
    }


//    cameraRotationMat = cameraRotationMat * MathHelper::RotationX(90*M_PI/180);
    mathfu::mat4 cameraRotationMat = mathfu::mat4::Identity();
    cameraRotationMat *= MathHelper::RotationY(toRadian(-av));
    cameraRotationMat *= MathHelper::RotationZ(toRadian(ah));

    lookAtMat =
        mathfu::mat4::FromTranslationVector(mathfu::vec3(cameraViewOffset.x,cameraViewOffset.y,0)) *
        mathfu::mat4::FromTranslationVector(mathfu::vec3(0,0,-m_radius)) *
        mathfu::LookAtHelper(mathfu::vec3(1,0,0),mathfu::vec3(0,0,0),mathfu::vec3(0,0,1), 1.0f) *
        cameraRotationMat *
        mathfu::mat4::FromTranslationVector(mathfu::vec3(-cameraOffset.x, -cameraOffset.y, -cameraOffset.z)) ;
    //std::cout<<"camera " << camera[0] <<" "<<camera[1] << " " << camera[2] << " " << std::endl;

    auto invViewMat = lookAtMat.Inverse();
    auto invTranspViewMat = invViewMat.Transpose();

    this->camera = (invViewMat * mathfu::vec4(0,0,0,1)).xyz();
    this->lookAt = (lookAtMat * mathfu::vec4(0,1,0,1)).xyz();

    mathfu::vec4 interiorSunDir = mathfu::vec4(-0.30822f, -0.30822f, -0.89999998f, 0);
    interiorSunDir = invTranspViewMat * interiorSunDir;
    interiorSunDir = mathfu::vec4(interiorSunDir.xyz() * (1.0f / interiorSunDir.xyz().Length()), 0.0f);

    this->interiorDirectLightDir = interiorSunDir;

    mathfu::vec4 upVector ( 0.0, 0.0 , 1.0 , 0.0);

    this->upVector = (invTranspViewMat * upVector).xyz().Normalized();
}
void PlanarCamera::setCameraPos (float x, float y, float z) {
    //Reset camera

    m_radius = sqrt(x*x + y*y + z*z);
    float polar = asin(z/m_radius);
    float azimuth = (M_PI - atan(y/x));

    this->av = polar / ((float)(M_PI) / 180.0f);
    this->ah = azimuth/((float)(M_PI) / 180.0f);

    this->delta_x = this->ah;
    this->delta_y = this->av;


    cameraViewOffset = mathfu::vec2(0,0);

    this->lookAt[0] = 0;
    this->lookAt[1] = 0;
    this->lookAt[2] = 0;
}
void PlanarCamera::setCameraOffset(float x, float y, float z) {
    cameraOffset = mathfu::vec3(x,y,z);
}
void PlanarCamera::zoomInFromTouch(float val) {
    m_radius += val * m_moveSpeed;
    if (m_radius < 0) m_radius = 0;
}

void PlanarCamera::zoomInFromMouseScroll(float val) {
    m_radius += val;
    if (m_radius < 0) m_radius = 0;
}

void PlanarCamera::addCameraViewOffset(float x, float y) {
    cameraViewOffset += mathfu::vec2(x, y);
}

HCameraMatrices PlanarCamera::getCameraMatrices(float fov,
                                                float canvasAspect,
                                                float nearPlane,
                                                float farPlane) {
    HCameraMatrices cameraMatrices = std::make_shared<CameraMatrices>();
    cameraMatrices->cameraPos = mathfu::vec4(camera, 1.0);
    cameraMatrices->perspectiveMat = mathfu::mat4::Perspective(
        fov,
        canvasAspect,
        nearPlane,
        farPlane);
    cameraMatrices->lookAtMat = lookAtMat;

    cameraMatrices->cameraPos = mathfu::vec4(camera, 1.0);
    cameraMatrices->viewUp = mathfu::vec4(upVector, 0);
    cameraMatrices->interiorDirectLightDir = this->interiorDirectLightDir;


    return cameraMatrices;
}
