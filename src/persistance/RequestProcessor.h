//
// Created by deamon on 06.09.17.
//

#ifndef WEBWOWVIEWERCPP_REQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_REQUESTPROCESSOR_H

#include "../../wowViewerLib/src/include/sharedFile.h"
#include "../../wowViewerLib/src/include/iostuff.h"
#include "../../wowViewerLib/src/renderer/frame/prodConsumerChain/ProdConsumerIOConnector.h"
#include <thread>
#include <list>
#include <vector>
#include <atomic>
#include <forward_list>
#include <unordered_set>

class RequestProcessor : public IFileRequest {
protected:
    RequestProcessor() : toBeProcessed(0){
        m_requestQueue = std::make_unique<decltype(m_requestQueue)::element_type>(dumb_boolean_always_false);
    }
    virtual ~RequestProcessor() {
    };

protected:
    IFileRequester *m_fileRequester = nullptr;

    virtual void processFileRequest(const std::string &fileName, CacheHolderType holderType, const std::weak_ptr<PersistentFile> &s_file) = 0;
public:
    void setFileRequester(IFileRequester *fileRequester) {
        m_fileRequester = fileRequester;
    }

    void initThreadQueue(bool &isTerminating) {
        m_threaded = true;
        m_requestQueue = std::make_unique<decltype(m_requestQueue)::element_type>(isTerminating);
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

    std::unique_ptr<ProdConsumerIOConnector<RequestStruct>> m_requestQueue;
    std::list<ResultStruct> m_resultQueue;
    std::unordered_set<std::string> currentlyProcessingFnames;

    bool m_threaded = false;
    bool dumb_boolean_always_false = false;
public:
    void processRequests(int limit);

    bool completedAllJobs() {
        return (m_requestQueue->empty()) && (m_resultQueue.empty()) && (toBeProcessed == 0);
    };
protected:
    std::atomic<int> toBeProcessed;
    void processResult(const std::shared_ptr<PersistentFile> &s_file, const HFileContent &content, const std::string &fileName);
};

typedef std::shared_ptr<RequestProcessor> HRequestProcessor;

#endif //WEBWOWVIEWERCPP_REQUESTPROCESSOR_H
