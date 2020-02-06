//
// Created by deamon on 06.02.20.
//

#ifndef AWEBWOWVIEWERCPP_CAMERAMATRICES_H
#define AWEBWOWVIEWERCPP_CAMERAMATRICES_H

#include <mathfu/glsl_mappings.h>

struct CameraMatrices {
    mathfu::mat4 perspectiveMat;
    mathfu::mat4 lookAtMat;
    mathfu::vec3 cameraPos;
};

typedef std::shared_ptr<CameraMatrices> HCameraMatrices;

#endif //AWEBWOWVIEWERCPP_CAMERAMATRICES_H
