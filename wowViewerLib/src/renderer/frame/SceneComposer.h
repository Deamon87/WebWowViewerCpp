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
    void consumeUpdate(const HFrameScenario &frameScenario, std::vector<std::unique_ptr<IRenderFunction>> &renderFunctions);
    void consumeDraw(const std::vector<std::unique_ptr<IRenderFunction>> &renderFuncs, bool windowSizeChanged);

    //Flip-flop delta promises
    int frameMod = 0;


    ProdConsumerIOConnector<HFrameScenario> cullingInput = {m_isTerminating};
    ProdConsumerIOConnector<HFrameScenario> updateInput = {m_isTerminating};
    ProdConsumerIOConnector<std::shared_ptr<std::vector<std::unique_ptr<IRenderFunction>>>> drawInput = {m_isTerminating};
public:
    SceneComposer(HApiContainer apiContainer);
    ~SceneComposer() {
        m_isTerminating = true;

        {
            cullingInput.pushInput(nullptr);
            updateInput.pushInput(nullptr);
            decltype(drawInput)::value_type nullParam = nullptr;
            drawInput.pushInput(nullParam);
        }
        cullingThread.join();
        updateThread.join();
        loadingResourcesThread.join();
    }

    void draw(const HFrameScenario &frameScenario, bool windowSizeChanged);
};


#endif //AWEBWOWVIEWERCPP_SCENECOMPOSER_H
