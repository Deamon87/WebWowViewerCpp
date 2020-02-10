//
// Created by deamon on 30.01.20.
//

#include "SceneScenario.h"

HCullStage FrameScenario::addCullStage(HCameraMatrices matricesForCulling, std::shared_ptr<IScene> scene) {
    HCullStage newCullStage = std::make_shared<CullStage>();
    newCullStage->matricesForCulling = matricesForCulling;
    newCullStage->scene = scene;

    cullStages.push_back(newCullStage);

    return newCullStage;
}

HUpdateStage
FrameScenario::addUpdateStage(HCullStage cullStage, animTime_t deltaTime, HCameraMatrices matricesForUpdate) {
    HUpdateStage newUpdateStage = std::make_shared<UpdateStage>();

    newUpdateStage->cullResult = cullStage;
    newUpdateStage->delta = deltaTime;
    newUpdateStage->cameraMatrices = matricesForUpdate;
    newUpdateStage->meshes = std::make_shared<MeshesToRender>();

    updateStages.push_back(newUpdateStage);

    return newUpdateStage;
}

HDrawStage FrameScenario::addDrawStage(HUpdateStage updateStage,
    HCameraMatrices matricesForDrawing,
    std::vector<HDrawStage> drawStageDependencies,
    bool setViewPort,
    ViewPortDimensions viewPortDimensions,
    bool clearScreen, mathfu::vec4 clearColor) {
    HDrawStage drawStage = std::make_shared<DrawStage>();

    drawStage->drawStageDependencies = drawStageDependencies;
    drawStage->matricesForRendering = matricesForDrawing;
    drawStage->setViewPort = setViewPort;
    drawStage->viewPortDimensions = viewPortDimensions;
    drawStage->clearScreen = clearScreen;
    drawStage->clearColor = clearColor;

    drawStage->sceneWideBlockVSPSChunk;

    drawStageLinks.push_back({updateStage, drawStage});

    this->lastDrawStage = drawStage;

    return drawStage;
}

HDrawStage FrameScenario::getDrawStage() {
    return lastDrawStage;
}
