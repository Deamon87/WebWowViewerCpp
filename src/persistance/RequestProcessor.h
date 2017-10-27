//
// Created by deamon on 06.09.17.
//

#ifndef WEBWOWVIEWERCPP_REQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_REQUESTPROCESSOR_H

#include "../../wowViewerLib/src/include/wowScene.h"
#include <thread>
#include <list>

class RequestProcessor : public IFileRequest {
protected:
    RequestProcessor() {
//        auto receiver = [](int count, PolyM::Queue& q) {
//            while (true) {
//                auto msg = q.get();
//                auto &dm = dynamic_cast<PolyM::DataMsg<int> &>(*msg);
//
//            }
//        };
//
        loaderThread = new std::thread(([&](){
            this->processRequests();
        }));
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

public:
    void processResults(int limit);


protected:
    void addRequest (std::string &fileName);
    void processRequests();

    void provideResult(std::string fileName, std::vector<unsigned char> &content);

};

#endif //WEBWOWVIEWERCPP_REQUESTPROCESSOR_H
