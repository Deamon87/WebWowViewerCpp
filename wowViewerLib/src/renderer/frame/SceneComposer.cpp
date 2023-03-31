//
// Created by deamon on 15.01.20.
//

#include <iostream>
#include <future>
#include "SceneComposer.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void SceneComposer::processCaches(int limit) {
    if (m_apiContainer->cacheStorage) {
        m_apiContainer->cacheStorage->processCaches(limit);
    }
}

SceneComposer::SceneComposer(HApiContainer apiContainer) : m_apiContainer(apiContainer) {
#ifdef __EMSCRIPTEN__
    m_supportThreads = false;
//    m_supportThreads = emscripten_run_script_int("(SharedArrayBuffer != null) ? 1 : 0") == 1;
#endif

    if (m_supportThreads) {
        loadingResourcesThread = std::thread([&]() {
            using namespace std::chrono_literals;
            while (!this->m_isTerminating) {
                std::this_thread::sleep_for(1ms);
                processCaches(1000);
            }
        });


        cullingThread = std::thread(([&]() {
            using namespace std::chrono_literals;
            FrameCounter frameCounter;

            while (!this->m_isTerminating) {
                auto frameScenario = cullingInput.waitForNewInput();
                if (m_isTerminating)
                    continue;


                frameCounter.beginMeasurement();

                //Do the culling and make function for further pipeline
                if (frameScenario != nullptr) {
                    consumeCulling(frameScenario);
                }

                //Pass the culling result down the pipeline
                updateInput.pushInput(frameScenario);

                frameCounter.endMeasurement();
                m_apiContainer->getConfig()->cullingTimePerFrame = frameCounter.getTimePerFrame();
            }
        }));

//        updateThread = std::thread(([&]() {
//            using namespace std::chrono_literals;
//            FrameCounter frameCounter;
//
//            while (!this->m_isTerminating) {
//                auto frameScenario = updateInput.waitForNewInput();
//                if (m_isTerminating)
//                    continue;
//
//                frameCounter.beginMeasurement();
//
//                //Do the culling and make function for further pipeline
//                std::shared_ptr<std::vector<std::unique_ptr<IRenderFunction>>> renderFuncs = std::make_shared<decltype(renderFuncs)::element_type>();
//                if (frameScenario != nullptr) {
//                    consumeUpdate(frameScenario, *renderFuncs);
//                }
//
//                //Pass the culling result down the pipeline
//                drawInput.pushInput(renderFuncs);
//
//                frameCounter.endMeasurement();
////            m_apiContainer->getConfig()->cullingTimePerFrame = frameCounter.getTimePerFrame();
//            }
//        }));
    }


    //Insert one temp element for balancing
    updateInput.pushInput(nullptr);
    drawInput.pushInput(nullptr);
    drawInput.pushInput(nullptr);
}

void SceneComposer::consumeCulling(HFrameScenario &frameScenario) {
    if (frameScenario == nullptr)
        return;

    for (int i = 0; i < frameScenario->cullFunctions.size(); i++) {
        frameScenario->drawUpdateFunction.push_back(frameScenario->cullFunctions[i]());
    }
}


void SceneComposer::consumeUpdate(HFrameScenario &frameScenario, std::vector<std::unique_ptr<IRenderFunction>> &renderFunctions) {
    if (frameScenario == nullptr)
        return;

    drawFuncGeneration.beginMeasurement();

    for (int i = 0; i < frameScenario->drawUpdateFunction.size(); i++) {
        renderFunctions.push_back(std::move(frameScenario->drawUpdateFunction[i]()));
    }
    drawFuncGeneration.endMeasurement();
    m_apiContainer->getConfig()->drawFuncGeneration = drawFuncGeneration.getTimePerFrame();
}

void SceneComposer::consumeDraw(const std::vector<std::unique_ptr<IRenderFunction>> &renderFuncs) {
    deviceDrawFrame.beginMeasurement();
    m_apiContainer->hDevice->drawFrame(renderFuncs);
    deviceDrawFrame.endMeasurement();

    m_apiContainer->getConfig()->deviceDrawFrame = deviceDrawFrame.getTimePerFrame();
}


#define logExecution {}
//#define logExecution { \
//    std::cout << "Passed "<<__FUNCTION__<<" line " << __LINE__ << std::endl;\
//}


void SceneComposer::draw(HFrameScenario frameScenario) {
    composerDrawTimePerFrame.beginMeasurement();

    if (!m_supportThreads) {
        processCaches(10);
        consumeCulling(frameScenario);
        std::vector<std::unique_ptr<IRenderFunction>> renderFuncs = {};
        consumeUpdate(frameScenario,renderFuncs);
        consumeDraw(renderFuncs);

    } else {
        cullingInput.pushInput(frameScenario);
        {
//            if (!m_firstFrame) {
                auto frameScenario = updateInput.waitForNewInput();
//                auto renderFuncs = drawInput.waitForNewInput();
                std::shared_ptr<std::vector<std::unique_ptr<IRenderFunction>>> renderFuncs = std::make_shared<decltype(renderFuncs)::element_type>();
                if (frameScenario != nullptr) {
                    consumeUpdate(frameScenario, *renderFuncs);
                }
                if (renderFuncs != nullptr) {
                    consumeDraw(*renderFuncs);
                }
            }
            m_firstFrame = false;
//        }
    }


//    m_apiContainer->hDevice->submitDrawCommands();
    m_apiContainer->hDevice->increaseFrameNumber();
    composerDrawTimePerFrame.endMeasurement();
    m_apiContainer->getConfig()->composerDrawTimePerFrame = composerDrawTimePerFrame.getTimePerFrame();
}
