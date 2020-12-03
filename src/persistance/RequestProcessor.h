//
// Created by deamon on 06.09.17.
//

#ifndef WEBWOWVIEWERCPP_REQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_REQUESTPROCESSOR_H

#include "../../wowViewerLib/src/include/sharedFile.h"
#include "../../wowViewerLib/src/include/iostuff.h"
#include <thread>
#include <list>
#include <vector>
#include <forward_list>
#include <unordered_set>

class RequestProcessor : public IFileRequest {
protected:
    RequestProcessor() {

    }
    ~RequestProcessor() {
        if (loaderThread != nullptr) {
            isTerminating = true;
            loaderThread->join();
            loaderThread = nullptr;
        }
    };

protected:
    IFileRequester *m_fileRequester = nullptr;

    virtual void processFileRequest(std::string &fileName, CacheHolderType holderType) = 0;
public:
    void setFileRequester(IFileRequester *fileRequester) {
        m_fileRequester = fileRequester;
    }

private:
    class RequestStruct {
    public:
        std::string fileName;
        CacheHolderType holderType;
    };

    class ResultStruct {
    public:
        ResultStruct(){};
        ResultStruct (const ResultStruct &old_obj) {
            fileName = old_obj.fileName;
            holderType = old_obj.holderType;
            buffer = old_obj.buffer;
        }
        std::string fileName;
        CacheHolderType holderType;
        HFileContent buffer = nullptr;
    };

    bool isTerminating = false;
    std::shared_ptr<std::thread> loaderThread = nullptr;

    std::list<RequestStruct> m_requestQueue;
    std::list<ResultStruct> m_resultQueue;
    std::unordered_set<std::string> currentlyProcessingFnames;

    bool m_threaded = false;

    int currentlyProcessing = 0;
public:
    void processResults(int limit);
    void processRequests(bool calledFromThread);

    bool queuesNotEmpty() {
        return (!m_requestQueue.empty()) || (!m_resultQueue.empty());
    };

    bool getThreaded() {
        return m_threaded;
    }
    void setThreaded(bool value) {
        m_threaded = value;
        if (value) {
            loaderThread = std::make_shared<std::thread>(([&](){
                this->processRequests(true);
            }));
        }
    }

protected:
    void addRequest (std::string &fileName, CacheHolderType holderType);

    void provideResult(std::string &fileName, HFileContent content, CacheHolderType holderType);
};

typedef std::shared_ptr<RequestProcessor> HRequestProcessor;

#endif //WEBWOWVIEWERCPP_REQUESTPROCESSOR_H
