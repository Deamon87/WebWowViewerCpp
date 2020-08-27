//
// Created by Deamon on 22.07.2020.
//

#include "m2TiedCamera.h"

void m2TiedCamera::getCameraPosition(float *position) {
    position[0] = m_lastCameraResult.target_position.x;
    position[1] = m_lastCameraResult.target_position.y;
    position[2] = m_lastCameraResult.target_position.z;
}

void m2TiedCamera::tick(animTime_t timeDelta) {
    if (m_m2Object == nullptr) return;

    m_lastCameraResult = m_m2Object->updateCamera(0, m_cameraNum);
}

HCameraMatrices m2TiedCamera::getCameraMatrices(float fov, float canvasAspect, float nearPlane, float farPlane) {
    mathfu::mat4 lookAtMat4 =
        mathfu::mat4::LookAt(
            -m_lastCameraResult.target_position.xyz()+m_lastCameraResult.position.xyz(),
            mathfu::vec3(0,0,0),
            mathfu::vec3(0,0,1)) * mathfu::mat4::FromTranslationVector(-m_lastCameraResult.position.xyz());

    mathfu::mat4 rotateMat = MathHelper::RotationY(m_lastCameraResult.roll);
    lookAtMat4 = rotateMat * lookAtMat4;

    mathfu::vec4 upVector ( 0.0, 0.0 , 1.0 , 0.0);
    mathfu::mat3 lookAtRotation = mathfu::mat4::ToRotationMatrix(lookAtMat4);
    mathfu::vec3 upVectorTranformed = (lookAtRotation * upVector.xyz());

    farPlane = m_lastCameraResult.far_clip;
    nearPlane = m_lastCameraResult.near_clip;

//    canvasAspect = 1.0f / canvasAspect;
    fov = m_lastCameraResult.diagFov / sqrt(1.0f + canvasAspect*canvasAspect);

    HCameraMatrices cameraMatrices = std::make_shared<CameraMatrices>();
    cameraMatrices->perspectiveMat = mathfu::mat4::Perspective(
        fov,
        canvasAspect,
        nearPlane,
        farPlane);
    cameraMatrices->lookAtMat = lookAtMat4;

    cameraMatrices->cameraPos = mathfu::vec4(m_lastCameraResult.position.xyz(), 1.0);
    cameraMatrices->viewUp = mathfu::vec4(upVectorTranformed, 0);
    cameraMatrices->interiorDirectLightDir = mathfu::vec4(0,0,0,0);

    return cameraMatrices;
}
