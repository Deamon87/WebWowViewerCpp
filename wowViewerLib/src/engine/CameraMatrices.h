//
// Created by deamon on 06.02.20.
//

#ifndef AWEBWOWVIEWERCPP_CAMERAMATRICES_H
#define AWEBWOWVIEWERCPP_CAMERAMATRICES_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <mathfu/glsl_mappings.h>

struct CameraMatrices {
    mathfu::mat4 perspectiveMat;
    mathfu::mat4 lookAtMat;
    mathfu::vec4 cameraPos;
    mathfu::vec4 viewUp;
    mathfu::vec4 interiorDirectLightDir;
};

typedef std::shared_ptr<CameraMatrices> HCameraMatrices;

#endif //AWEBWOWVIEWERCPP_CAMERAMATRICES_H
