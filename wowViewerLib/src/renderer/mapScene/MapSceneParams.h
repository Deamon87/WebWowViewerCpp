//
// Created by Deamon on 08.01.23.
//

#ifndef AWEBWOWVIEWERCPP_MAPSCENEPARAMS_H
#define AWEBWOWVIEWERCPP_MAPSCENEPARAMS_H


#include "../../engine/CameraMatrices.h"

struct MapSceneParams {
    std::shared_ptr<IScene> scene;
    HCameraMatrices matricesForCulling;
    HCameraMatrices cameraMatricesForRendering;
    HCameraMatrices cameraMatricesForDebugCamera;

    mathfu::vec4 clearColor;
};

typedef std::shared_ptr<MapSceneParams> HMapSceneParams;
#endif //AWEBWOWVIEWERCPP_MAPSCENEPARAMS_H
