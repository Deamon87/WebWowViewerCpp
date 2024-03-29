//
// Created by deamon on 15.01.20.
//

#include <iostream>
#include <future>
#include "SceneComposer.h"
#include "algorithms/FrameCounter.h"
#include "../gapi/UniformBufferStructures.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void SceneComposer::processCaches(int limit) {
//    std::cout << "WoWSceneImpl::processCaches called " << std::endl;
//    std::cout << "this->adtObjectCache.m_cache.size() = " << this->adtObjectCache.m_cache.size()<< std::endl;
    if (m_apiContainer->cacheStorage) {
        m_apiContainer->cacheStorage->processCaches(limit);
    }
}

SceneComposer::SceneComposer(HApiContainer apiContainer) : m_apiContainer(apiContainer) {
#ifdef __EMSCRIPTEN__
    m_supportThreads = false;
//    m_supportThreads = emscripten_run_script_int("(SharedArrayBuffer != null) ? 1 : 0") == 1;
#endif
    nextDeltaTime[0] = std::promise<float>();
    nextDeltaTime[1] = std::promise<float>();
    nextDeltaTimeForUpdate[0] = std::promise<float>();
    nextDeltaTimeForUpdate[1] = std::promise<float>();

    if (m_supportThreads) {
        loadingResourcesThread = std::thread([&]() {
//            try {
                using namespace std::chrono_literals;

                while (!this->m_isTerminating) {
                    std::this_thread::sleep_for(1ms);

                    processCaches(1000);
                }
//            } catch(const std::exception &e) {
//                std::cerr << e.what() << std::endl;
//                throw;
//            } catch (...) {
//                throw;
//            }
        });


        cullingThread = std::thread(([&]() {
//            try {
                using namespace std::chrono_literals;
                FrameCounter frameCounter;


                //NOTE: it's required to have separate counting process here with getPromiseInd/getNextPromiseInd,
                //cause it's not known if the frameNumber will be updated from main thread before
                //new currIndex is calculated (and if that doenst happen, desync in numbers happens)
                auto currIndex = getPromiseInd();

                while (!this->m_isTerminating) {
                    //std::cout << "cullingThread " << currIndex << std::endl;
                    auto future = nextDeltaTime[currIndex].get_future();
                    future.wait();
                    auto nextIndex = getNextPromiseInd();

                    //                std::cout << "update frame = " << getDevice()->getUpdateFrameNumber() << std::endl;

                    int currentFrame = m_apiContainer->hDevice->getCullingFrameNumber();

                    frameCounter.beginMeasurement();
                    DoCulling();
                    frameCounter.endMeasurement();
                    m_apiContainer->getConfig()->cullingTimePerFrame = frameCounter.getTimePerFrame();

                    this->cullingFinished.set_value(true);
                    currIndex = nextIndex;
                    
                }
//            } catch(const std::exception &e) {
//                std::cerr << e.what() << std::endl;
//                throw;
//            } catch (...) {
//                throw;
//            }
        }));

        if (m_apiContainer->hDevice->getIsAsynBuffUploadSupported()) {
            updateThread = std::thread(([&]() {
//                try {
                    this->m_apiContainer->hDevice->initUploadThread();
                    //NOTE: Refer to comment in cullingThread code
                    auto currIndex = getPromiseInd();
                    while (!this->m_isTerminating) {

                        //std::cout << "updateThread " << currIndex << std::endl;
                        auto future = nextDeltaTimeForUpdate[currIndex].get_future();
                        future.wait();
                        auto nextIndex = getNextPromiseInd();

                        updateTimePerFrame.beginMeasurement();
                        DoUpdate();
                        updateTimePerFrame.endMeasurement();

                        updateFinished.set_value(true);
                        currIndex = nextIndex;
                    }
//                } catch(const std::exception &e) {
//                    std::cerr << e.what() << std::endl;
//                    throw;
//                } catch (...) {
//                    throw;
//                }
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

        cullStage->scene->checkCulling(cullStage);
    }
}

void collectMeshes(HDrawStage drawStage, std::vector<HGMesh> &meshes) {
    if (drawStage->opaqueMeshes != nullptr) {
        std::copy(
            drawStage->opaqueMeshes->meshes.begin(),
            drawStage->opaqueMeshes->meshes.end(),
            std::back_inserter(meshes)
        );
    }
    if (drawStage->transparentMeshes != nullptr) {
        std::copy(
            drawStage->transparentMeshes->meshes.begin(),
            drawStage->transparentMeshes->meshes.end(),
            std::back_inserter(meshes)
        );
    }
    for (auto deps : drawStage->drawStageDependencies) {
        collectMeshes(deps, meshes);
    }
}
#define logExecution {}
//#define logExecution { \
//    std::cout << "Passed "<<__FUNCTION__<<" line " << __LINE__ << std::endl;\
//}

void SceneComposer::DoUpdate() {
    logExecution
    auto device = m_apiContainer->hDevice;
    logExecution

    int updateObjFrame = device->getUpdateFrameNumber();
    logExecution
    auto frameScenario = m_frameScenarios[updateObjFrame];
    if (frameScenario == nullptr) return;
    logExecution
    startUpdateForNexFrame.beginMeasurement();
    device->startUpdateForNextFrame();
    startUpdateForNexFrame.endMeasurement();
    logExecution
    singleUpdateCNT.beginMeasurement();
    logExecution
    for (auto updateStage : frameScenario->updateStages) {
        updateStage->cullResult->scene->produceUpdateStage(updateStage);
    }
    logExecution
    singleUpdateCNT.endMeasurement();
    logExecution
    produceDrawStage.beginMeasurement();
    logExecution
    std::vector<HGUniformBufferChunk> additionalChunks;
    logExecution
    for (auto &link : frameScenario->drawStageLinks) {
        logExecution
        link.scene->produceDrawStage(link.drawStage, link.updateStage, additionalChunks);
        logExecution
    }
    produceDrawStage.endMeasurement();

    meshesCollectCNT.beginMeasurement();
    std::vector<HGMesh> meshes;
    logExecution
    collectMeshes(frameScenario->getDrawStage(), meshes);
    logExecution
    meshesCollectCNT.endMeasurement();
    logExecution

    updateBuffersCNT.beginMeasurement();
    logExecution
    for (auto updateStage : frameScenario->updateStages) {
        logExecution
        updateStage->cullResult->scene->updateBuffers(updateStage);
        logExecution
    }
    updateBuffersCNT.endMeasurement();
    logExecution

    updateBuffersDeviceCNT.beginMeasurement();
    logExecution
    std::vector<HFrameDepedantData> frameDepDataVec = {};
    logExecution
    std::vector<std::vector<HGUniformBufferChunk>*> uniformChunkVec = {};
    logExecution
    for (auto updateStage : frameScenario->updateStages) {
        frameDepDataVec.push_back(updateStage->cullResult->frameDepedantData);
        uniformChunkVec.push_back(&updateStage->uniformBufferChunks);
    }
    logExecution
    device->updateBuffers(uniformChunkVec, frameDepDataVec);
    logExecution
    updateBuffersDeviceCNT.endMeasurement();
    logExecution
    postLoadCNT.beginMeasurement();
    for (auto cullStage : frameScenario->cullStages) {
        logExecution
        cullStage->scene->doPostLoad(cullStage); //Do post load after rendering is done!
        logExecution
    }
    postLoadCNT.endMeasurement();
    logExecution
    textureUploadCNT.beginMeasurement();
    logExecution
    device->uploadTextureForMeshes(meshes);
    logExecution
    textureUploadCNT.endMeasurement();

    if (device->getIsVulkanAxisSystem()) {
        drawStageAndDepsCNT.beginMeasurement();
        if (frameScenario != nullptr) {
            m_apiContainer->hDevice->drawStageAndDeps(frameScenario->getDrawStage());
        }
        drawStageAndDepsCNT.endMeasurement();
    }
    logExecution
    endUpdateCNT.beginMeasurement();
    device->endUpdateForNextFrame();
    logExecution
    endUpdateCNT.endMeasurement();
    logExecution

    auto config = m_apiContainer->getConfig();
    config->startUpdateForNexFrame = startUpdateForNexFrame.getTimePerFrame();
    config->updateTimePerFrame = updateTimePerFrame.getTimePerFrame();
    config->singleUpdateCNT = singleUpdateCNT.getTimePerFrame();
    config->produceDrawStage = produceDrawStage.getTimePerFrame();;
    config->meshesCollectCNT = meshesCollectCNT.getTimePerFrame();;
    config->updateBuffersCNT = updateBuffersCNT.getTimePerFrame();;
    config->updateBuffersDeviceCNT = updateBuffersDeviceCNT.getTimePerFrame();;
    config->postLoadCNT = postLoadCNT.getTimePerFrame();;
    config->textureUploadCNT = textureUploadCNT.getTimePerFrame();;
    config->drawStageAndDepsCNT = drawStageAndDepsCNT.getTimePerFrame();;
    config->endUpdateCNT = endUpdateCNT.getTimePerFrame();;
    logExecution

}
#define logExecution {}
//#define logExecution { \
//    std::cout << "Passed "<<__FUNCTION__<<" line " << __LINE__ << std::endl;\
//}


void SceneComposer::draw(HFrameScenario frameScenario) {
//    std::cout << __FILE__ << ":" << __LINE__<< "m_apiContainer == nullptr :" << ((m_apiContainer == nullptr) ? "true" : "false") << std::endl;
//    std::cout << __FILE__ << ":" << __LINE__<< "hdevice == nullptr :" << ((m_apiContainer->hDevice == nullptr) ? "true" : "false") << std::endl;

    std::future<bool> cullingFuture;
    std::future<bool> updateFuture;
    logExecution
    if (m_supportThreads) {
        cullingFuture = cullingFinished.get_future();
        logExecution
        nextDeltaTime[getNextPromiseInd()] = std::promise<float>();
        logExecution
        nextDeltaTime[getPromiseInd()].set_value(1.0f);
        //std::cout << "set new nextDeltaTime value for " << getPromiseInd() << "frame " << std::endl;
        //std::cout << "set new nextDeltaTime promise for " << getNextPromiseInd() << "frame " << std::endl;
        logExecution
        if (m_apiContainer->hDevice->getIsAsynBuffUploadSupported()) {
            logExecution
            nextDeltaTimeForUpdate[getNextPromiseInd()] = std::promise<float>();
            logExecution
            nextDeltaTimeForUpdate[getPromiseInd()].set_value(1.0f);

            //std::cout << "set new nextDeltaTime value for " << getPromiseInd() << "frame " << std::endl;
            //std::cout << "set new nextDeltaTimeForUpdate promise for " << getNextPromiseInd() << "frame " << std::endl;
            logExecution
            updateFuture = updateFinished.get_future();
            logExecution
        }
    }

    //if (frameScenario == nullptr) return;

//    if (needToDropCache) {
//        if (cacheStorage) {
//            cacheStorage->actuallDropCache();
//        }
//        needToDropCache = false;
//    }


    m_apiContainer->hDevice->reset();
    logExecution
    int currentFrame = m_apiContainer->hDevice->getDrawFrameNumber();
    auto thisFrameScenario = m_frameScenarios[currentFrame];
    logExecution
    m_frameScenarios[currentFrame] = frameScenario;
    logExecution
    if (!m_supportThreads) {
        processCaches(10);
        DoCulling();
    }
    logExecution
    m_apiContainer->hDevice->beginFrame();
    if (thisFrameScenario != nullptr && !m_apiContainer->hDevice->getIsVulkanAxisSystem()) {
        drawStageAndDepsCNT.beginMeasurement();

        m_apiContainer->hDevice->drawStageAndDeps(thisFrameScenario->getDrawStage());

        drawStageAndDepsCNT.endMeasurement();
    }
    logExecution

    m_apiContainer->hDevice->commitFrame();
    logExecution
    m_apiContainer->hDevice->reset();
    logExecution
    if (!m_apiContainer->hDevice->getIsAsynBuffUploadSupported()) {
        logExecution
        updateTimePerFrame.beginMeasurement();
        DoUpdate();
        updateTimePerFrame.endMeasurement();
        logExecution
    }
    logExecution

    if (m_supportThreads) {
        cullingFuture.wait();
        logExecution
        cullingFinished = std::promise<bool>();
        logExecution
        if (m_apiContainer->hDevice->getIsAsynBuffUploadSupported()) {
            updateFuture.wait();
            logExecution
            updateFinished = std::promise<bool>();
            logExecution
        }
    }

    logExecution
    m_apiContainer->hDevice->increaseFrameNumber();
    logExecution
}
