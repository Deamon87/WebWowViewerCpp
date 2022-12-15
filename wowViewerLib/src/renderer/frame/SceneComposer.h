//
// Created by deamon on 15.01.20.
//

#ifndef AWEBWOWVIEWERCPP_SCENECOMPOSER_H
#define AWEBWOWVIEWERCPP_SCENECOMPOSER_H


#include <thread>
#include <future>
#include <queue>
#include "../../include/iostuff.h"
#include "SceneScenario.h"
#include "../../engine/algorithms/FrameCounter.h"
#include "../../engine/ApiContainer.h"

class SceneComposer {
private:
    HApiContainer m_apiContainer = nullptr;
private:
    std::thread cullingThread;
    std::thread updateThread;
    std::thread loadingResourcesThread;

    bool m_supportThreads = true;
    bool m_isTerminating = false;

    FrameCounter updateTimePerFrame;

    void DoCulling();
    void DoUpdate();
    void processCaches(int limit);

    //Flip-flop delta promises
    int frameMod = 0;

    std::queue<HFrameScenario> m_cullingQueue;
    std::queue<HFrameScenario> m_updateQueue;
    std::queue<HFrameScenario> m_render;

    std::condition_variable startCulling;
    std::mutex cullingMutex;

    std::condition_variable startUpdate;
    std::mutex cullingUpdate;
public:
    SceneComposer(HApiContainer apiContainer);
    ~SceneComposer() {
        m_isTerminating = true;

        cullingThread.join();

        if (m_apiContainer->hDevice->getIsAsynBuffUploadSupported()) {
            updateThread.join();
        }

        loadingResourcesThread.join();
    }

    void draw(HFrameScenario frameScenario);

    void addInputParams(HFrameScenario rendererAndInput){};
};


#endif //AWEBWOWVIEWERCPP_SCENECOMPOSER_H
