//
// Created by deamon on 15.01.20.
//

#ifndef AWEBWOWVIEWERCPP_SCENECOMPOSER_H
#define AWEBWOWVIEWERCPP_SCENECOMPOSER_H


#include <thread>
#include <future>
#include "../include/iostuff.h"
#include "../gapi/interface/IDevice.h"
#include "SceneScenario.h"


class SceneComposer {
private:
    ApiContainer *m_apiContainer = nullptr;
private:
    std::thread cullingThread;
    std::thread updateThread;
    std::thread loadingResourcesThread;

    bool m_supportThreads = true;
    bool m_isTerminating = false;



    void DoCulling();
    void DoUpdate();
    void processCaches(int limit);

    std::promise<float> nextDeltaTime;
    std::promise<float> nextDeltaTimeForUpdate;
    std::promise<bool> cullingFinished;
    std::promise<bool> updateFinished;

    std::array<HFrameScenario, 4> m_frameScenarios;
public:
    SceneComposer(ApiContainer *apiContainer);


    void draw(HFrameScenario frameScenario);
};


#endif //AWEBWOWVIEWERCPP_SCENECOMPOSER_H
