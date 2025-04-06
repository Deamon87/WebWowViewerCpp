//
// Created by Deamon on 7/18/2023.
//

#ifndef AWEBWOWVIEWERCPP_KEYSUPDATEWORKFLOW_H
#define AWEBWOWVIEWERCPP_KEYSUPDATEWORKFLOW_H

#include <thread>
#include <functional>
#include "../../../../wowViewerLib/src/engine/WowFilesCacheStorage.h"
#include "../../../persistance/CascRequestProcessor.h"

class KeysUpdateWorkflow {
public:
    KeysUpdateWorkflow(const std::shared_ptr<CascRequestProcessor> &cascRequestProcessor);
    bool render();
private:
    std::shared_ptr<CascRequestProcessor> m_cascRequestProcessor;

    bool m_showDownloading = true;
    bool m_showDownloadingSuccess = false;
    bool m_showDownloadingFailed = false;


    std::shared_ptr<std::thread> httpDownloadThread = nullptr;
    bool m_httpDownloadThreadFinished = true;
    bool m_failedToDownloadFile = false;

    bool defineDialogs();

    void keyThreadDownloadLogic();
};


#endif //AWEBWOWVIEWERCPP_KEYSUPDATEWORKFLOW_H
