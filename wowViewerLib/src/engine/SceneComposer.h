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
#include "algorithms/FrameCounter.h"

class SceneComposer {
private:
    HApiContainer m_apiContainer = nullptr;

private:


    std::thread cullingThread;
    std::thread updateThread;
    std::thread loadingResourcesThread;

    bool m_supportThreads = true;
    bool m_isTerminating = false;

    FrameCounter submitDrawCommands;
    FrameCounter updateTimePerFrame;


    FrameCounter startUpdateForNexFrame;
    FrameCounter singleUpdateCNT;
    FrameCounter produceDrawStage;
    FrameCounter meshesCollectCNT;
    FrameCounter updateBuffersCNT;
    FrameCounter updateBuffersDeviceCNT;
    FrameCounter postLoadCNT;
    FrameCounter textureUploadCNT;
    FrameCounter drawStageAndDepsCNT;
    FrameCounter endUpdateCNT;



    void DoCulling();
    void DoUpdate();
    void processCaches(int limit);

    //Flip-flop delta promises
    std::array<std::promise<float>,2> nextDeltaTime;
    std::array<std::promise<float>,2> nextDeltaTimeForUpdate;
    std::promise<bool> cullingFinished;
    std::promise<bool> updateFinished;

    std::array<HFrameScenario, 4> m_frameScenarios;

    int getPromiseInd() {
        auto frameNum = m_apiContainer->hDevice->getFrameNumber();
        auto promiseInd = frameNum & 1;
        return promiseInd;
    }
    int getNextPromiseInd() {
        auto frameNum = m_apiContainer->hDevice->getFrameNumber();
        auto promiseInd = (frameNum + 1) & 1;
        return promiseInd;
    }
public:
    SceneComposer(HApiContainer apiContainer);
    ~SceneComposer() {
        m_isTerminating = true;
        auto promiseInd = getPromiseInd();
        try {
            float delta = 1.0f;
            nextDeltaTime[promiseInd].set_value(delta);
        } catch (...) {}

        cullingThread.join();

        if (m_apiContainer->hDevice->getIsAsynBuffUploadSupported()) {
            try {
                nextDeltaTimeForUpdate[promiseInd].set_value(1.0f);
            } catch (...) {}
            updateThread.join();
        }

        loadingResourcesThread.join();
    }

    void draw(HFrameScenario frameScenario);
};


#endif //AWEBWOWVIEWERCPP_SCENECOMPOSER_H
