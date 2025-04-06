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
#include "prodConsumerChain/ProdConsumerIOConnector.h"

class SceneComposer {
private:
    HApiContainer m_apiContainer = nullptr;
private:
    std::thread cullingThread;
    std::thread updateThread;
    std::thread loadingResourcesThread;

    bool m_supportThreads = true;
    bool m_isTerminating = false;
    bool m_firstFrame = true;

    void consumeCulling(const HFrameScenario &frameScenario);
    void consumeUpdate(const HFrameScenario &frameScenario, FrameRenderFuncs &frameRenderFunctions);
    void consumeDraw(const FrameRenderFuncs &renderFuncs, bool windowSizeChanged);

    //Flip-flop delta promises
    int frameMod = 0;


    ProdConsumerIOConnector<HFrameScenario> cullingInput = {m_isTerminating};
    ProdConsumerIOConnector<HFrameScenario> updateInput = {m_isTerminating};
    ProdConsumerIOConnector<std::shared_ptr<FrameRenderFuncs>> drawInput = {m_isTerminating};
public:
    SceneComposer(HApiContainer apiContainer, bool supportThreads = true);
    ~SceneComposer() {
        m_isTerminating = true;

        {
            cullingInput.pushInput(nullptr);
            updateInput.pushInput(nullptr);
            decltype(drawInput)::value_type nullParam = nullptr;
            drawInput.pushInput(nullParam);
        }
        if (m_supportThreads) {
            cullingThread.join();
            updateThread.join();
            loadingResourcesThread.join();
        }
    }

    void draw(const HFrameScenario &frameScenario, bool windowSizeChanged);
};


#endif //AWEBWOWVIEWERCPP_SCENECOMPOSER_H
