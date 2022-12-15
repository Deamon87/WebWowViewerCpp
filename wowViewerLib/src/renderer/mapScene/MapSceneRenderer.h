//
// Created by Deamon on 11/26/2022.
//

#ifndef AWEBWOWVIEWERCPP_MAPSCENERENDERER_H
#define AWEBWOWVIEWERCPP_MAPSCENERENDERER_H


#include "../../engine/objects/scenes/map.h"
#include "../IRenderer.h"
#include "../IRenderParameters.h"

class MapSceneRenderer : public IRenderer, public IRendererParameters<MapRenderPlan>  {
public:
    MapSceneRenderer() = default;
    ~MapSceneRenderer() override = 0;

    void putIntoQueue(FrameInputParams<MapRenderPlan> &cullStage) override = 0 ;
};


#endif //AWEBWOWVIEWERCPP_MAPSCENERENDERER_H
