//
// Created by deamon on 15.01.20.
//

#ifndef AWEBWOWVIEWERCPP_SCENECOMPOSER_H
#define AWEBWOWVIEWERCPP_SCENECOMPOSER_H


#include <thread>
#include "../include/iostuff.h"
#include "../gapi/interface/IDevice.h"
#include "SceneScenario.h"


class SceneComposer {
private:
    HWoWFilesCacheStorage cacheStorage = nullptr;
private:
    std::thread cullingThread;
    std::thread updateThread;
    std::thread loadingResourcesThread;

    bool m_supportThreads = true;
    bool m_isTerminating = false;

    std::shared_ptr<IDevice> m_gdevice;

    void DoCulling();
    void DoUpdate();
    void processCaches(int limit);

    std::promise<float> nextDeltaTime = std::promise<float>();
    std::promise<float> nextDeltaTimeForUpdate;
    std::promise<bool> cullingFinished;
    std::promise<bool> updateFinished;

    std::array<HFrameScenario, 4> m_frameScenarios;
public:
    SceneComposer(std::shared_ptr<IDevice> hDevice);

    void draw(HFrameScenario frameScenario);
};


#endif //AWEBWOWVIEWERCPP_SCENECOMPOSER_H
