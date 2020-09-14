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

    updateStages.push_back(newUpdateStage);

    return newUpdateStage;
}

HDrawStage FrameScenario::getDrawStage() {
    return lastDrawStage;
}

//FrameScenario::addDrawStage(std::shared_ptr<UpdateStage>, std::shared_ptr<IScene>, std::shared_ptr<CameraMatrices>, std::vector<std::shared_ptr<DrawStage>, std::allocator<std::shared_ptr<DrawStage>>> const&, bool, ViewPortDimensions const&, bool, mathfu::Vector<float, 4> const&, std::shared_ptr<IFrameBuffer>)
//FrameScenario::addDrawStage(std::shared_ptr<UpdateStage>, std::shared_ptr<IScene>, std::shared_ptr<CameraMatrices>, std::__debug::vector<std::shared_ptr<DrawStage>, std::allocator<std::shared_ptr<DrawStage>>> const&, bool, ViewPortDimensions const&, bool, mathfu::Vector<float, 4> const&, std::shared_ptr<IFrameBuffer>)
HDrawStage FrameScenario::addDrawStage(HUpdateStage updateStage, HScene scene, HCameraMatrices matricesForDrawing,
                                       std::vector<HDrawStage> const &drawStageDependencies, bool setViewPort,
                                        ViewPortDimensions const &viewPortDimensions, bool clearScreen, bool invertedZ,
                                        mathfu::vec4 const &clearColor, HFrameBuffer fbTarget) {
    HDrawStage drawStage = std::make_shared<DrawStage>();

    drawStage->drawStageDependencies = drawStageDependencies;
    drawStage->matricesForRendering = matricesForDrawing;
    drawStage->setViewPort = setViewPort;
    drawStage->viewPortDimensions = viewPortDimensions;
    drawStage->clearScreen = clearScreen;
    drawStage->clearColor = clearColor;
    drawStage->invertedZ = invertedZ;
    drawStage->target = fbTarget;

//    drawStage->sceneWideBlockVSPSChunk;

    drawStageLinks.push_back({scene, updateStage, drawStage});

    this->lastDrawStage = drawStage;

    return drawStage;
}

