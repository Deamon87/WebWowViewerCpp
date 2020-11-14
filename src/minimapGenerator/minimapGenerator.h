//
// Created by Deamon on 9/8/2020.
//

#ifndef AWEBWOWVIEWERCPP_MINIMAPGENERATOR_H
#define AWEBWOWVIEWERCPP_MINIMAPGENERATOR_H

#include "../../wowViewerLib/src/engine/ApiContainer.h"
#include "../../wowViewerLib/src/engine/SceneScenario.h"
#include "../persistance/RequestProcessor.h"


class MinimapGenerator {
private:
    HApiContainer m_apiContainer;
    HRequestProcessor m_processor;
    int currentScenario = -1;

    HScene m_currentScene = nullptr;
    int m_width = 1024;
    int m_height = 1024;

    //Position that's being rendered
    int m_x = 0;
    int m_y = 0;

    float XToYCoef = 0;
    bool XToYCoefCalculated = false;

    int m_chunkStartX = -32;
    int m_chunkStartY= -32;
    int m_chunkWidth = 64;
    int m_chunkHeight = 64;

    std::list<HDrawStage> drawStageStack = {};
    int framesReady = 0;

    mathfu::mat4 getOrthoMatrix();
public:

    MinimapGenerator(HWoWFilesCacheStorage cacheStorage, std::shared_ptr<IDevice> hDevice, HRequestProcessor processor, IClientDatabase*);

    void startScenario(int scenarioId);
    void process();
    bool isDone();
    void setupCameraData();
    HDrawStage createSceneDrawStage(HFrameScenario sceneScenario);

    float GetOrthoDimension();
};

typedef std::shared_ptr<MinimapGenerator> HMinimapGenerator;

#endif //AWEBWOWVIEWERCPP_MINIMAPGENERATOR_H
