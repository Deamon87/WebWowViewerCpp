//
// Created by deamon on 06.09.17.
//

#ifndef WEBWOWVIEWERCPP_REQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_REQUESTPROCESSOR_H

#include "../../wowViewerLib/src/include/wowScene.h"
#include <thread>
#include <list>
#include <vector>

class RequestProcessor : public IFileRequest {
protected:
    RequestProcessor() {

    }

protected:
    IFileRequester *m_fileRequester = nullptr;

    virtual void processFileRequest(std::string &fileName) = 0;
public:
    void setFileRequester(IFileRequester *fileRequester) {
        m_fileRequester = fileRequester;
    }

private:
    struct resultStruct {
        std::string fileName;
        std::vector<unsigned char> buffer;
    };

    std::thread *loaderThread;

    std::list<std::string> m_requestQueue;
    std::list<resultStruct> m_resultQueue;

    bool m_threaded = false;

    int currentlyProcessing = 0;
public:
    void processResults(int limit);
    void processRequests(bool calledFromThread);

    void setThreaded(bool value) {
        m_threaded = value;
        if (value) {
            loaderThread = new std::thread(([&](){
                this->processRequests(true);
            }));
        }
    }

protected:
    void addRequest (std::string &fileName);

    void provideResult(std::string &fileName, std::vector<unsigned char> &content);

};

#endif //WEBWOWVIEWERCPP_REQUESTPROCESSOR_H
