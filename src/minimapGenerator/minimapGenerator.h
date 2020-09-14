//
// Created by Deamon on 9/8/2020.
//

#ifndef AWEBWOWVIEWERCPP_MINIMAPGENERATOR_H
#define AWEBWOWVIEWERCPP_MINIMAPGENERATOR_H

#include "../../wowViewerLib/src/engine/ApiContainer.h"
#include "../../wowViewerLib/src/engine/SceneScenario.h"


class MinimapGenerator {
private:
    ApiContainer m_apiContainer;
    int currentScenario = -1;

    HScene m_currentScene = nullptr;
    int m_width = 1024;
    int m_height = 1024;
public:
    MinimapGenerator(ApiContainer &apiContainer);

    void startScenario(int scenarioId);

    HDrawStage createSceneDrawStage(HFrameScenario sceneScenario);
};


#endif //AWEBWOWVIEWERCPP_MINIMAPGENERATOR_H
