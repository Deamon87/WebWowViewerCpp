//
// Created by deamon on 15.01.20.
//

#include <iostream>
#include <future>
#include "SceneComposer.h"

SceneComposer::SceneComposer() {
#ifdef __EMSCRIPTEN__
    m_supportThreads = false;
#endif

    if (m_supportThreads) {
        loadingResourcesThread = std::thread([&]() {
            try {
                using namespace std::chrono_literals;

                while (!this->m_isTerminating) {
                    std::this_thread::sleep_for(1ms);

                    processCaches(1000);
                }
            } catch(const std::exception &e) {
                std::cerr << e.what() << std::endl;
                throw;
            } catch (...) {
                throw;
            }
        });


        cullingThread = std::thread(([&]() {
            try {
                using namespace std::chrono_literals;
                FrameCounter frameCounter;

                while (!this->m_isTerminating) {
                    auto future = nextDeltaTime.get_future();
                    future.wait();

                    //                std::cout << "update frame = " << getDevice()->getUpdateFrameNumber() << std::endl;

                    int currentFrame = m_gdevice->getCullingFrameNumber();
                    WoWFrameData *frameParam = &m_FrameParams[currentFrame];
                    frameParam->deltaTime = future.get();
                    nextDeltaTime = std::promise<float>();

                    frameCounter.beginMeasurement();
                    DoCulling();

                    frameCounter.endMeasurement("Culling thread ");

                    this->cullingFinished.set_value(true);
                }
            } catch(const std::exception &e) {
                std::cerr << e.what() << std::endl;
                throw;
            } catch (...) {
                throw;
            }
        }));

        if (device->getIsAsynBuffUploadSupported()) {
            updateThread = std::thread(([&]() {
                try {


                    while (!this->m_isTerminating) {
                        auto future = nextDeltaTimeForUpdate.get_future();
                        future.wait();
                        nextDeltaTimeForUpdate = std::promise<float>();
                        DoUpdate();

                        updateFinished.set_value(true);
                    }
                } catch(const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    throw;
                } catch (...) {
                    throw;
                }
            }));
        }
    }
}

void SceneComposer::DoCulling() {

}

void SceneComposer::DoUpdate() {

}

void SceneComposer::draw(animTime_t deltaTime) {
    //Replace the scene
    if (newScene != nullptr) {
        if (currentScene != nullptr) delete currentScene;
        for (int i = 0; i < 4; i++) {
            m_FrameParams[i] = WoWFrameData();
        }
        getDevice()->shrinkData();

        currentScene = newScene;
        newScene = nullptr;
    }

    std::future<bool> cullingFuture;
    std::future<bool> updateFuture;
    if (m_supportThreads) {
        cullingFuture = cullingFinished.get_future();

        nextDeltaTime.set_value(deltaTime);
        if (getDevice()->getIsAsynBuffUploadSupported()) {
            nextDeltaTimeForUpdate.set_value(deltaTime);
            updateFuture = updateFinished.get_future();
        }
    }

    if (currentScene == nullptr) return;

    if (needToDropCache) {
        if (cacheStorage) {
            cacheStorage->actuallDropCache();
        }
        needToDropCache = false;
    }


    IDevice *device = getDevice();
    device->reset();
    int currentFrame = device->getDrawFrameNumber();
    WoWFrameData *frameParam = &m_FrameParams[currentFrame];

    if (!m_supportThreads) {
        processCaches(10);
        frameParam->deltaTime = deltaTime;
        DoCulling();
    }

    float clearColor[4];
    m_config->getClearColor(clearColor);
    device->setClearScreenColor(clearColor[0], clearColor[1], clearColor[2]);
    device->setViewPortDimensions(0,0,this->canvWidth, this->canvHeight);
    device->beginFrame();

    device->drawMeshes(frameParam->renderedThisFrame);

    device->commitFrame();
    device->reset();

    if (!device->getIsAsynBuffUploadSupported()) {
        DoUpdate();
    }


    if (m_supportThreads) {
        cullingFuture.wait();
        cullingFinished = std::promise<bool>();

        if (device->getIsAsynBuffUploadSupported()) {
            updateFuture.wait();
            updateFinished = std::promise<bool>();
        }
    }


    device->increaseFrameNumber();
}
