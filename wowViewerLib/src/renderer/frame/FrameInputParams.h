//
// Created by Deamon on 11.12.22.
//

#ifndef AWEBWOWVIEWERCPP_FRAMEINPUTPARAMS_H
#define AWEBWOWVIEWERCPP_FRAMEINPUTPARAMS_H

#include "../../engine/CameraMatrices.h"
#include "../../engine/objects/iScene.h"
#include "../../include/iostuff.h"

struct ViewPortDimensions{
    std::array<int, 2> mins;
    std::array<int, 2> maxs;
};

//template<typename T, typename R>
struct FrameInputParams {
    HCameraMatrices matricesForCulling;
    HCameraMatrices cameraMatricesForRendering;
    HCameraMatrices cameraMatricesForDebugCamera;
    HScene scene;

//    std::shared_ptr<R> additionalData;

    //Time advance
    animTime_t delta = 0;

    //Parameters for framebuffer
    ViewPortDimensions viewPortDimensions = {{0,0}, {64, 64}};
    bool invertedZ = false;
    bool clearScreen = false;
};

#endif //AWEBWOWVIEWERCPP_FRAMEINPUTPARAMS_H
