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
            while (!this->m_isTerminating) {
                std::unique_lock<std::mutex> lock{cullingQueueMutex};
                auto &l_cullingQueue = m_cullingQueue;
                cullingCondVar.wait(lock, [&l_cullingQueue]() { return !l_cullingQueue.empty(); });

                auto frameScenario = m_cullingQueue.front();
                m_cullingQueue.pop();
                lock.unlock();


                consumeCulling(frameScenario);

                //Push into updateRenderQueue
                {
                    std::lock_guard<std::mutex> l{updateRenderQueueMutex};
                    m_updateRenderQueue.push(frameScenario);
                    updateRenderCondVar.notify_one();
                }
            }
        }));

        //Insert one temp element for balancing
        m_updateRenderQueue.push(std::make_shared<HFrameScenario::element_type>());
    }
}

void SceneComposer::consumeCulling(HFrameScenario &frameScenario) {
    if (frameScenario == nullptr)
        return;

    for (int i = 0; i < frameScenario->cullFunctions.size(); i++) {
        frameScenario->drawUpdateFunction.push_back(frameScenario->cullFunctions[i]());
    }
}


void SceneComposer::consumeDrawAndUpdate(HFrameScenario &frameScenario) {
    if (frameScenario == nullptr)
        return;

    std::vector<std::unique_ptr<IRenderFunction>> renderFunctions;
    for (int i = 0; i < frameScenario->drawUpdateFunction.size(); i++) {
        renderFunctions.push_back(std::move(frameScenario->drawUpdateFunction[i]()));
    }

    m_apiContainer->hDevice->drawFrame(renderFunctions);
}
#define logExecution {}
//#define logExecution { \
//    std::cout << "Passed "<<__FUNCTION__<<" line " << __LINE__ << std::endl;\
//}


void SceneComposer::draw(HFrameScenario frameScenario) {
    if (!m_supportThreads) {
        processCaches(10);
        consumeCulling(frameScenario);
        consumeDrawAndUpdate(frameScenario);
    } else {
        {
            //Add to Queue
            std::lock_guard<std::mutex> l{cullingQueueMutex};
            m_cullingQueue.push(frameScenario);
            cullingCondVar.notify_one();
        }

        {
            std::unique_lock<std::mutex> lock{updateRenderQueueMutex};
            auto &l_updateRenderQueue = m_updateRenderQueue;
            updateRenderCondVar.wait(lock, [&l_updateRenderQueue]() { return !l_updateRenderQueue.empty(); });

            auto frameScenario = m_updateRenderQueue.front();
            m_updateRenderQueue.pop();
            lock.unlock();

            updateTimePerFrame.beginMeasurement();
            consumeDrawAndUpdate(frameScenario);
            updateTimePerFrame.endMeasurement();
        }
    }


//    m_apiContainer->hDevice->submitDrawCommands();
    m_apiContainer->hDevice->increaseFrameNumber();
}
