#include <mutex>
#include <vector>
#include <iostream>
#include <sstream>
#include "RequestProcessor.h"

std::mutex requestMtx;           // mutex for critical section
std::mutex resultMtx;            // mutex for critical section

std::mutex setProcessingMtx;     // mutex for critical section

//1. Add request to FIFO
//2. Get filename from FIFO
//3. Add result to ResultFIFO
//4. Get ready results from FIFO
void
RequestProcessor::requestFile(std::string &fileName, CacheHolderType holderType, std::weak_ptr<PersistentFile> s_file) {
    std::unique_lock<std::mutex> setLck (setProcessingMtx,std::defer_lock);
    setLck.lock();
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

    std::unique_lock<std::mutex> lck (requestMtx,std::defer_lock);
    // critical section (exclusive access to std::cout signaled by locking lck):
    lck.lock();

    m_requestQueue.push_back({fileName, holderType, s_file});
    toBeProcessed++;
    lck.unlock();
}

void RequestProcessor::processRequests (bool calledFromThread) {
    using namespace std::chrono_literals;
    // critical section (exclusive access to std::cout signaled by locking lck):
    std::unique_lock<std::mutex> lck (requestMtx,std::defer_lock);

    if (calledFromThread){
        while (!this->isTerminating) {
            if (m_requestQueue.empty()) {
                std::this_thread::sleep_for(1ms);
                continue;
            }

            lck.lock(); 
            auto it = m_requestQueue.front();
            m_requestQueue.pop_front();
            lck.unlock();

            this->processFileRequest(it.fileName, it.holderType, it.s_file);

            std::unique_lock<std::mutex> setLck (setProcessingMtx,std::defer_lock);
            setLck.lock();
            currentlyProcessingFnames.erase(it.fileName);
            setLck.unlock();
        }
    } else if (!m_threaded) {
        while (!m_requestQueue.empty()) {
            lck.lock();
            auto it = m_requestQueue.front();
            m_requestQueue.pop_front();
            lck.unlock();

            this->processFileRequest(it.fileName, it.holderType, it.s_file);

            std::unique_lock<std::mutex> setLck (setProcessingMtx,std::defer_lock);
            setLck.lock();
            currentlyProcessingFnames.erase(it.fileName);
            setLck.unlock();
        }
    }

}

void
RequestProcessor::processResult(std::shared_ptr<PersistentFile> s_file, HFileContent content, const std::string &fileName) {
    if (s_file->getStatus() == FileStatus::FSLoaded) {
        std::cout << "sharedPtr->getStatus == FileStatus::FSLoaded " << fileName << std::endl;
    } if (s_file->getStatus() == FileStatus::FSRejected) {
        std::cout << "sharedPtr->getStatus == FileStatus::FSRejected" << fileName << std::endl;
    } else {
        s_file->process(content, fileName);
    }

}
