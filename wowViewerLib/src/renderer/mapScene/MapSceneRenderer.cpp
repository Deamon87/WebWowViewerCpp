//
// Created by Deamon on 11/26/2022.
//

#include "MapSceneRenderer.h"

std::shared_ptr<MapRenderPlan>
MapSceneRenderer::processCulling(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams) {
//    auto &scenes = frameInputParams->frameParameters->scene;

//    for (auto &scene : scenes) {
        auto mapScene = std::dynamic_pointer_cast<Map>(frameInputParams->frameParameters->scene);
        auto mapPlan = std::make_shared<HMapRenderPlan::element_type>();
        mapScene->makeFramePlan(*frameInputParams, mapPlan);
//    }

    return mapPlan;
}
