//
// Created by deamon on 15.01.20.
//

#include <iostream>
#include <future>
#include "SceneComposer.h"
#include "algorithms/FrameCounter.h"
#include "../gapi/UniformBufferStructures.h"

void SceneComposer::processCaches(int limit) {
//    std::cout << "WoWSceneImpl::processCaches called " << std::endl;
//    std::cout << "this->adtObjectCache.m_cache.size() = " << this->adtObjectCache.m_cache.size()<< std::endl;
    if (m_apiContainer->cacheStorage) {
        m_apiContainer->cacheStorage->processCaches(limit);
    }
}

SceneComposer::SceneComposer(ApiContainer *apiContainer) : m_apiContainer(apiContainer) {
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

                    int currentFrame = m_apiContainer->hDevice->getCullingFrameNumber();
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

        if (m_apiContainer->hDevice->getIsAsynBuffUploadSupported()) {
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
    static const mathfu::vec3 upVector(0,0,1);
    static const mathfu::mat4 vulkanMatrixFix = mathfu::mat4(1, 0, 0, 0,
                                                             0, -1, 0, 0,
                                                             0, 0, 1.0/2.0, 1/2.0,
                                                             0, 0, 0, 1).Transpose();

    int currentFrame = m_apiContainer->hDevice->getCullingFrameNumber();
    auto frameScenario = m_frameScenarios[currentFrame];

    if (frameScenario == nullptr) return;

    for (int i = 0; i < frameScenario->cullStages.size(); i++) {
        auto cullStage = frameScenario->cullStages[i];
        auto config = m_apiContainer->getConfig();

        float farPlane = config->getFarPlane();
        float nearPlane = 1.0;
        float fov = toRadian(45.0);

        auto &perspectiveMatrix = cullStage->matricesForCulling->perspectiveMat;

        if (m_apiContainer->hDevice->getIsVulkanAxisSystem()) {
            perspectiveMatrix = vulkanMatrixFix * perspectiveMatrix;
        }

        cullStage->scene->checkCulling(cullStage);
    }
}

void collectMeshes(HDrawStage drawStage, std::vector<HGMesh> &meshes) {
    std::copy(
        drawStage->meshesToRender->meshes.begin(),
        drawStage->meshesToRender->meshes.end(),
        std::back_inserter(meshes)
    );
    for (auto deps : drawStage->drawStageDependencies) {
        collectMeshes(deps, meshes);
    }
}

void SceneComposer::DoUpdate() {
    FrameCounter frameCounter;

    FrameCounter singleUpdateCNT;
    FrameCounter meshesCollectCNT;

    frameCounter.beginMeasurement();

    auto device = m_apiContainer->hDevice;

    int updateObjFrame = device->getUpdateFrameNumber();

    auto frameScenario = m_frameScenarios[updateObjFrame];
    if (frameScenario == nullptr) return;

    device->startUpdateForNextFrame();

    singleUpdateCNT.beginMeasurement();
    for (auto updateStage : frameScenario->updateStages) {
        updateStage->cullResult->scene->update(updateStage);
    }
    singleUpdateCNT.endMeasurement("single update ");

    meshesCollectCNT.beginMeasurement();

    std::vector<HGUniformBufferChunk> additionalChunks;
    for (auto &link : frameScenario->drawStageLinks) {
        link.scene->produceDrawStage(link.drawStage, link.updateStage);

        additionalChunks.push_back(link.drawStage->sceneWideBlockVSPSChunk);
    }

    std::vector<HGMesh> meshes;
    collectMeshes(frameScenario->getDrawStage(), meshes);

    meshesCollectCNT.endMeasurement("collectMeshes ");

    for (auto cullStage : frameScenario->cullStages) {
        cullStage->scene->updateBuffers(cullStage);
    }
    device->updateBuffers(meshes, additionalChunks);

    for (auto cullStage : frameScenario->cullStages) {
        cullStage->scene->doPostLoad(cullStage); //Do post load after rendering is done!
    }
    device->uploadTextureForMeshes(meshes);

    device->endUpdateForNextFrame();
    frameCounter.endMeasurement("Update Thread");
}

void SceneComposer::draw(HFrameScenario frameScenario) {
    std::future<bool> cullingFuture;
    std::future<bool> updateFuture;
    if (m_supportThreads) {
        cullingFuture = cullingFinished.get_future();

        nextDeltaTime.set_value(0);
        if (m_apiContainer->hDevice->getIsAsynBuffUploadSupported()) {
            nextDeltaTimeForUpdate.set_value(0);
            updateFuture = updateFinished.get_future();
        }
    }

    if (frameScenario == nullptr) return;

//    if (needToDropCache) {
//        if (cacheStorage) {
//            cacheStorage->actuallDropCache();
//        }
//        needToDropCache = false;
//    }



    m_apiContainer->hDevice->reset();
    int currentFrame = m_apiContainer->hDevice->getDrawFrameNumber();
    auto thisFrameScenario = m_frameScenarios[currentFrame];

    m_frameScenarios[currentFrame] = frameScenario;

    if (!m_supportThreads) {
        processCaches(10);
        DoCulling();
    }

    m_apiContainer->hDevice->beginFrame();
    if (thisFrameScenario != nullptr) {
        m_apiContainer->hDevice->drawStageAndDeps(thisFrameScenario->getDrawStage());
    }
    m_apiContainer->hDevice->commitFrame();
    m_apiContainer->hDevice->reset();

    if (!m_apiContainer->hDevice->getIsAsynBuffUploadSupported()) {
        DoUpdate();
    }


    if (m_supportThreads) {
        cullingFuture.wait();
        cullingFinished = std::promise<bool>();

        if (m_apiContainer->hDevice->getIsAsynBuffUploadSupported()) {
            updateFuture.wait();
            updateFinished = std::promise<bool>();
        }
    }


    m_apiContainer->hDevice->increaseFrameNumber();
}
