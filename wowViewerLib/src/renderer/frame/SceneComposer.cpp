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
                DoCulling();
            }
        }));

        if (m_apiContainer->hDevice->getIsAsynBuffUploadSupported()) {
            updateThread = std::thread(([&]() {
                this->m_apiContainer->hDevice->initUploadThread();
                while (!this->m_isTerminating) {


                    updateTimePerFrame.beginMeasurement();
                    DoUpdate();
                    updateTimePerFrame.endMeasurement();
                }
            }));
        }
    }
}

void SceneComposer::DoCulling() {
//    for (int i = 0; i < frameScenario->cullStages.size(); i++) {
//        auto cullStage = frameScenario->cullStages[i];
//
//        cullStage->scene->checkCulling(cullStage);
//    }
}


void SceneComposer::DoUpdate() {
//    auto device = m_apiContainer->hDevice;
//
//
//    auto frameScenario = m_frameScenarios[updateObjFrame];
//    if (frameScenario == nullptr) return;
//    device->startUpdateForNextFrame();
//    for (auto updateStage : frameScenario->updateStages) {
//        updateStage->cullResult->scene->produceUpdateStage(updateStage);
//    }
//    std::vector<HGMesh> meshes;
//    collectMeshes(frameScenario->getDrawStage(), meshes);
//
//    for (auto updateStage : frameScenario->updateStages) {
//            updateStage->cullResult->scene->updateBuffers(updateStage);
//        }
//
//    std::vector<std::vector<HGUniformBufferChunk>*> uniformChunkVec;
//    std::vector<HFrameDependantData> frameDepDataVec;
//
//    for (auto &updateStage : frameScenario->updateStages) {
//        frameDepDataVec.push_back(updateStage->cullResult->frameDependentData);
//        uniformChunkVec.push_back(&updateStage->uniformBufferChunks);
//    }
//    device->updateBuffers(uniformChunkVec, frameDepDataVec);
//    for (auto cullStage : frameScenario->cullStages) {
//            cullStage->scene->doPostLoad(cullStage); //Do post load after rendering is done!
//        }
//    device->uploadTextureForMeshes(meshes);
//
//    if (device->getIsVulkanAxisSystem()) {
//        if (frameScenario != nullptr) {
//            m_apiContainer->hDevice->drawStageAndDeps(frameScenario->getDrawStage());
//        }
//    }
//    device->endUpdateForNextFrame();
}
#define logExecution {}
//#define logExecution { \
//    std::cout << "Passed "<<__FUNCTION__<<" line " << __LINE__ << std::endl;\
//}


void SceneComposer::draw(HFrameScenario frameScenario) {
    std::future<bool> cullingFuture;
    std::future<bool> updateFuture;

    if (!m_supportThreads) {
        processCaches(10);
        DoCulling();
        DoUpdate();
    }
    m_apiContainer->hDevice->drawScenario();
    m_apiContainer->hDevice->increaseFrameNumber();
}
