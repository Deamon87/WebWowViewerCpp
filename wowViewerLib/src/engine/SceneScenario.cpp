//
// Created by deamon on 30.01.20.
//

#include "SceneScenario.h"

HCullStage FrameScenario::addCullStage(HCameraMatrices matricesForCulling, IScene *scene) {
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
