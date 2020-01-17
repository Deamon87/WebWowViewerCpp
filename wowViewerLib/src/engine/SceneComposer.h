//
// Created by deamon on 15.01.20.
//

#ifndef AWEBWOWVIEWERCPP_SCENECOMPOSER_H
#define AWEBWOWVIEWERCPP_SCENECOMPOSER_H


#include <thread>
#include "../include/iostuff.h"
#include "../gapi/interface/IDevice.h"


class SceneComposer {
private:
    std::thread cullingThread;
    std::thread updateThread;
    std::thread loadingResourcesThread;

    bool m_supportThreads = true;

    std::unique_ptr<IDevice> m_gdevice;

    void DoCulling();
    void DoUpdate();

    std::promise<float> nextDeltaTime = std::promise<float>();
    std::promise<float> nextDeltaTimeForUpdate;
    std::promise<bool> cullingFinished;
    std::promise<bool> updateFinished;
public:
    SceneComposer();

    void draw(animTime_t deltaTime);
};


#endif //AWEBWOWVIEWERCPP_SCENECOMPOSER_H
