//
// Created by Deamon on 08.01.23.
//

#ifndef AWEBWOWVIEWERCPP_MAPSCENEPARAMS_H
#define AWEBWOWVIEWERCPP_MAPSCENEPARAMS_H


#include "../../engine/CameraMatrices.h"
#include "../../engine/objects/iScene.h"

class IRenderView {
public:
    virtual ~IRenderView() = default;
};

struct MapSceneParams {
    std::shared_ptr<IScene> scene;
    HCameraMatrices matricesForCulling;

    struct RenderTuple {
        HCameraMatrices cameraMatricesForRendering = nullptr;
        std::shared_ptr<IRenderView> target = nullptr;
        ViewPortDimensions viewPortDimensions = {{0,0}, {64, 64}};
        bool clearTarget = false;
    };
    std::vector<RenderTuple> renderTargets;

    mathfu::vec4 clearColor;
};

typedef std::shared_ptr<MapSceneParams> HMapSceneParams;
#endif //AWEBWOWVIEWERCPP_MAPSCENEPARAMS_H
