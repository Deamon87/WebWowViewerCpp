//
// Created by Deamon on 8/27/2022.
//

#ifndef AWEBWOWVIEWERCPP_DATABASEUPDATEWORKFLOW_H
#define AWEBWOWVIEWERCPP_DATABASEUPDATEWORKFLOW_H

#include <thread>
#include <functional>
#include "../../../../wowViewerLib/src/engine/WowFilesCacheStorage.h"

class DatabaseUpdateWorkflow {
public:
    DatabaseUpdateWorkflow(HApiContainer &api, bool isClassic);

    void render();
    bool isDatabaseUpdated() { return m_databaseUpdated;};
private:
    //Showing elements state.
    bool m_showDBDPrompt = true;
    bool m_needToUpdateDBD = false;
    bool m_needToUpdateDatabase = false;
    bool m_showOkModal = false;
    bool m_databaseUpdated = false;

private:
    HApiContainer m_api;
    bool m_isClassic = false;

    void defineDialogs();

    void dbdThreadDownloadLogic();
    void db2UpdateLogic();


    int m_currentDBDFile = 0;
    std::shared_ptr<std::thread> httpDownloadThread = nullptr;

    bool m_httpDownloadThreadFinished = true;
    std::string m_failedToDownloadFile = "";


    int m_currentDB2File = 0;
    std::function<bool()> checkDB2Lambda = nullptr;
    std::function<bool(std::string, std::shared_ptr<Db2File>)> addTableLambda = nullptr;
    bool m_db2ThreadFinished = true;
    std::string m_db2FailedMessage = "";
};


#endif //AWEBWOWVIEWERCPP_DATABASEUPDATEWORKFLOW_H
