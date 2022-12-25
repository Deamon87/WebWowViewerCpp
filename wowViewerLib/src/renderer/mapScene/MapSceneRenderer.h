//
// Created by Deamon on 11/26/2022.
//

#ifndef AWEBWOWVIEWERCPP_MAPSCENERENDERER_H
#define AWEBWOWVIEWERCPP_MAPSCENERENDERER_H


#include "../IRenderer.h"
#include "../IRenderParameters.h"
#include "../../engine/objects/scenes/map.h"
#include "IMapSceneBufferCreate.h"

class MapSceneRenderer : public IRenderer, public IMapSceneBufferCreate, public IRendererParameters<MapRenderPlan, void>  {
public:
    MapSceneRenderer() = default;
    ~MapSceneRenderer() override = 0;
};

//typedef FrameInputParams<MapRenderPlan, void> MapSceneRendererInputParams;
typedef FrameInputParams MapSceneRendererInputParams;
typedef std::shared_ptr<MapSceneRenderer> HMapSceneRenderer;

#endif //AWEBWOWVIEWERCPP_MAPSCENERENDERER_H
