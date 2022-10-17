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
#include <atomic>
#include <forward_list>
#include <unordered_set>

class RequestProcessor : public IFileRequest {
protected:
    RequestProcessor() : toBeProcessed(0){

    }
    virtual ~RequestProcessor() {
        if (loaderThread != nullptr) {
            isTerminating = true;
            loaderThread->join();
            loaderThread = nullptr;
        }
    };

protected:
    IFileRequester *m_fileRequester = nullptr;

    virtual void processFileRequest(std::string &fileName, CacheHolderType holderType, std::weak_ptr<PersistentFile> s_file) = 0;
public:
    void setFileRequester(IFileRequester *fileRequester) {
        m_fileRequester = fileRequester;
    }

    void requestFile(std::string &fileName, CacheHolderType holderType, std::weak_ptr<PersistentFile> s_file) override;
private:
    class RequestStruct {
    public:
        std::string fileName;
        CacheHolderType holderType;
        std::weak_ptr<PersistentFile> s_file;
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
    void processRequests(bool calledFromThread);

    bool completedAllJobs() {
        return (m_requestQueue.empty()) && (m_resultQueue.empty()) && (toBeProcessed == 0);
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
    std::atomic<int> toBeProcessed;
    void processResult( std::shared_ptr<PersistentFile> s_file, HFileContent content, const std::string &fileName);
};

typedef std::shared_ptr<RequestProcessor> HRequestProcessor;

#endif //WEBWOWVIEWERCPP_REQUESTPROCESSOR_H
