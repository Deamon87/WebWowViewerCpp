//
// Created by deamon on 15.01.20.
//

#include <iostream>
#include <future>
#include "SceneComposer.h"
#include "FrameProfile.h"
#include "tbb/tbb.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

SceneComposer::SceneComposer(HApiContainer apiContainer, bool supportThreads) : m_apiContainer(apiContainer) {
    m_supportThreads = supportThreads;
#ifdef __EMSCRIPTEN__
    m_supportThreads = false;
//    m_supportThreads = emscripten_run_script_int("(SharedArrayBuffer != null) ? 1 : 0") == 1;
#endif

    if (m_supportThreads) {
        loadingResourcesThread = std::thread([&]() {
            setThreadName("ResourceLoader");
            using namespace std::chrono_literals;

            HRequestProcessor currentProcessor = nullptr;

            while (!this->m_isTerminating) {
                if (currentProcessor != m_apiContainer->requestProcessor) {
                    currentProcessor = m_apiContainer->requestProcessor;
                    if (currentProcessor != nullptr)
                        currentProcessor->initThreadQueue(this->m_isTerminating);
                }

                if (currentProcessor != nullptr) {
                    m_apiContainer->requestProcessor->processRequests(INT_MAX);
                } else {
                    std::this_thread::sleep_for(1ms);
                }
            }
        });

        cullingThread = std::thread(([&]() {
//            tbb::global_control global_limit(oneapi::tbb::global_control::max_allowed_parallelism,
//                                             2*apiContainer->getConfig()->hardwareThreadCount());

            using namespace std::chrono_literals;
            setThreadName("Culling");

            while (!this->m_isTerminating) {
                auto frameScenario = cullingInput.waitForNewInput();
                if (m_isTerminating)
                    continue;

                //Do the culling and make function for further pipeline
                if (frameScenario != nullptr) {
                    consumeCulling(frameScenario);
                }

                //Pass the culling result down the pipeline
                updateInput.pushInput(frameScenario);
            }
        }));

        updateThread = std::thread(([&]() {
//            tbb::global_control global_limit(oneapi::tbb::global_control::max_allowed_parallelism,
//                                             apiContainer->getConfig()->hardwareThreadCount());

            using namespace std::chrono_literals;
            setThreadName("Update");

            while (!this->m_isTerminating) {
                auto frameScenario = updateInput.waitForNewInput();
                if (m_isTerminating)
                    continue;

                //Do the culling and make function for further pipeline
                std::shared_ptr<FrameRenderFuncs> frameRenderFuncs = std::make_shared<decltype(frameRenderFuncs)::element_type>();
                if (frameScenario != nullptr) {
                    consumeUpdate(frameScenario, *frameRenderFuncs);
                    frameRenderFuncs->onFinish = frameScenario->onFinish;
                }

                //Pass the culling result down the pipeline
                drawInput.pushInput(frameRenderFuncs);
            }
        }));
    }


    //Insert one temp element for balancing
    updateInput.pushInput(nullptr);
//    drawInput.pushInput(nullptr);
//    drawInput.pushInput(nullptr);
}

void SceneComposer::consumeCulling(const HFrameScenario &frameScenario) {
    ZoneScoped ;

    if (frameScenario == nullptr)
        return;

    for (int i = 0; i < frameScenario->cullFunctions.size(); i++) {
        frameScenario->drawUpdateFunction.push_back(frameScenario->cullFunctions[i]());
    }
}


void SceneComposer::consumeUpdate(const HFrameScenario &frameScenario, FrameRenderFuncs &frameRenderFunctions) {
    ZoneScoped ;

    if (frameScenario == nullptr)
        return;

    auto &renderFuncs = frameRenderFunctions.renderFuncs;
    for (int i = 0; i < frameScenario->drawUpdateFunction.size(); i++) {
        renderFuncs.push_back(std::move(frameScenario->drawUpdateFunction[i]()));
    }
}

void SceneComposer::consumeDraw(const FrameRenderFuncs &renderFuncs, bool windowSizeChanged) {
    ZoneScoped ;
    m_apiContainer->hDevice->drawFrame(renderFuncs, windowSizeChanged);
}


#define logExecution {}
//#define logExecution { \
//    std::cout << "Passed "<<__FUNCTION__<<" line " << __LINE__ << std::endl;\
//}


void SceneComposer::draw(const HFrameScenario &frameScenario, bool windowSizeChanged) {
    ZoneScoped ;

    if (!m_supportThreads) {
        consumeCulling(frameScenario);
        FrameRenderFuncs renderFuncs = {};
        m_apiContainer->requestProcessor->processRequests(10);
        consumeUpdate(frameScenario, renderFuncs);
        consumeDraw(renderFuncs, windowSizeChanged);
    } else {
        cullingInput.pushInput(frameScenario);
        {
//            if (!m_firstFrame) {
//                auto frameScenario = updateInput.waitForNewInput();
                auto renderFuncs = drawInput.waitForNewInput();
//                std::shared_ptr<std::vector<std::unique_ptr<IRenderFunction>>> renderFuncs = std::make_shared<decltype(renderFuncs)::element_type>();
//                if (frameScenario != nullptr) {
//                    consumeUpdate(frameScenario, *renderFuncs);
//                }
                if (renderFuncs != nullptr) {
                    consumeDraw(*renderFuncs, windowSizeChanged);
                }
            }
            m_firstFrame = false;
//        }
    }


//    m_apiContainer->hDevice->submitDrawCommands();
    m_apiContainer->hDevice->increaseFrameNumber();
}
