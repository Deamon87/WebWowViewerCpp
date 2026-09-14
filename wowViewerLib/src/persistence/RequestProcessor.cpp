#include <mutex>
#include <vector>
#include <chrono>
#include <iostream>
#include <sstream>
#include "RequestProcessor.h"

std::mutex setProcessingMtx;     // mutex for critical section

//1. Add request to FIFO
//2. Get filename from FIFO
//3. Add result to ResultFIFO
//4. Get ready results from FIFO
void
RequestProcessor::requestFile(CacheHolderType holderType, std::weak_ptr<PersistentFile> w_file) {
#if CHECK_FOR_DUPLICATES == 1
    std::unique_lock<std::mutex> setLck (setProcessingMtx,std::defer_lock);
    setLck.lock();

    auto s_file = w_file.lock();

    if (currentlyProcessingFnames.count(fileName) > 0) {
        uint32_t fileDataId = 0;
        if (fileName.find("File") == 0) {
            std::stringstream ss;
            std::string fileDataIdHex = fileName.substr(4, fileName.find(".") - 4);

            ss << std::hex << fileDataIdHex;
            ss >> fileDataId;
        }
        std::cout << "RequestProcessor::addRequest : duplicate detected for fileName = " << fileName
                  << " " << ((fileDataId > 0) ? ("(fileDataId = "+std::to_string(fileDataId)+")"): "")
                  << " holderTypeReq = " << (int)holderType
                  <<std::endl;
        return;
    }
    currentlyProcessingFnames.insert(fileName);

    setLck.unlock();
#endif

    m_requestQueue->pushInput({holderType, w_file});
    toBeProcessed++;
}
void RequestProcessor::iterateAllFiles( std::unique_ptr<IterateFilesRequest> &iterateFilesRequest ) {
    m_iterateFilesRequest = std::move(iterateFilesRequest);

    m_requestQueue->pushInput({});
}

void RequestProcessor::processRequests (int limit) {
    using namespace std::chrono_literals;
    // critical section (exclusive access to std::cout signaled by locking lck):
    std::unique_lock<std::mutex> setLck (setProcessingMtx,std::defer_lock);

    if (m_iterateFilesRequest != nullptr) {
//        std::cout << "m_iterateFilesRequest lol " << std::endl;
//        std::this_thread::sleep_for(10000ms);

        iterateFilesInternal(m_iterateFilesRequest->process, m_iterateFilesRequest->callback);

        m_iterateFilesRequest = nullptr;
//        std::cout << "m_iterateFilesRequest destroyed" << std::endl;
    }

    if (m_threaded) {
        m_requestQueue->waitAndProcess([&](const RequestStruct &it) {

            this->processFileRequest(it.holderType, it.s_file);

#if CHECK_FOR_DUPLICATES == 1
            setLck.lock();
            currentlyProcessingFnames.erase(it.fileName);
            setLck.unlock();
#endif
        });
    } else {
        m_requestQueue->blockProcessWithoutWait(limit, [&](const RequestStruct &it) {
            this->processFileRequest(it.holderType, it.s_file);
#if CHECK_FOR_DUPLICATES == 1
            setLck.lock();
            currentlyProcessingFnames.erase(it.fileName);
            setLck.unlock();
#endif
        });
    }
}

void
RequestProcessor::processResult(const std::shared_ptr<PersistentFile> &s_file, const HFileContent &content) {
    if (s_file->getStatus() == FileStatus::FSLoaded) {
        std::cout << "sharedPtr->getStatus == FileStatus::FSLoaded " << s_file->getFileNameOrDataId() << std::endl;
    } if (s_file->getStatus() == FileStatus::FSRejected) {
        std::cout << "sharedPtr->getStatus == FileStatus::FSRejected" << s_file->getFileNameOrDataId() << std::endl;
    } else {
        s_file->process(content);
    }
}
