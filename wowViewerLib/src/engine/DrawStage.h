//
// Created by Deamon on 2/2/2020.
//

#ifndef AWEBWOWVIEWERCPP_DRAWSTAGE_H
#define AWEBWOWVIEWERCPP_DRAWSTAGE_H

struct DrawStage;
typedef std::shared_ptr<DrawStage> HDrawStage;

#include <memory>
#include <mathfu/glsl_mappings.h>
#include "../gapi/interface/IDevice.h"

struct CameraMatrices {
    mathfu::mat4 perspectiveMat;
    mathfu::mat4 lookAtMat;
    mathfu::vec3 cameraPos;
};


typedef std::shared_ptr<CameraMatrices> HCameraMatrices;

struct DrawStage {
    HCameraMatrices matricesForRendering;
    std::vector<HDrawStage> drawStageDependencies;

    bool setViewPort = false;
    struct {
        std::array<int, 2> mins;
        std::array<int, 2> maxs;
    } viewPortDimensions;

    bool clearScreen = false;
    mathfu::vec4 clearColor;

    std::vector<HGMesh> meshes;

    HFrameBuffer target;
};

#endif //AWEBWOWVIEWERCPP_DRAWSTAGE_H
