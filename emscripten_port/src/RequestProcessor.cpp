#include <mutex>
#include <vector>
#include <iostream>
#include "RequestProcessor.h"

std::mutex requestMtx;           // mutex for critical section
std::mutex resultMtx;            // mutex for critical section

//1. Add request to FIFO
//2. Get filename from FIFO
//3. Add result to ResultFIFO
//4. Get ready results from FIFO
void RequestProcessor::addRequest (std::string &fileName, CacheHolderType holderType) {
//    std::cout << fileName;
    std::unique_lock<std::mutex> lck (requestMtx,std::defer_lock);
    // critical section (exclusive access to std::cout signaled by locking lck):
    lck.lock();

    m_requestQueue.push_front({fileName, holderType});

    lck.unlock();
}

void RequestProcessor::processRequests (bool calledFromThread) {
    using namespace std::chrono_literals;
    // critical section (exclusive access to std::cout signaled by locking lck):
    if (calledFromThread){
        std::unique_lock<std::mutex> lck (requestMtx,std::defer_lock);

        while (true) {
            if (m_requestQueue.empty()) {
                std::this_thread::sleep_for(1ms);
                continue;
            }

            lck.lock();
            auto it = m_requestQueue.begin();
            lck.unlock();

            this->processFileRequest(it->fileName, it->holderType);

            lck.lock();
            m_requestQueue.erase(it);
            lck.unlock();
        }
    } else if (!m_threaded) {
        while (!m_requestQueue.empty()) {
            auto it = m_requestQueue.begin();

//            std::cout << "currentlyProcessing = " << currentlyProcessing << std::endl;
            if (currentlyProcessing >= 5) break;

            currentlyProcessing++;

            this->processFileRequest(it->fileName, it->holderType);

            m_requestQueue.erase(it);
        }
    }

}

void RequestProcessor::provideResult(std::string &fileName, std::vector<unsigned char> &content, CacheHolderType holderType) {
    std::unique_lock<std::mutex> lck (resultMtx,std::defer_lock);

    ResultStruct resultStructObj;
    resultStructObj.buffer = content;
    resultStructObj.fileName = fileName;
    resultStructObj.holderType = holderType;

    if (m_threaded) lck.lock();
    m_resultQueue.push_front(resultStructObj);
    if (m_threaded) lck.unlock();
}

void RequestProcessor::processResults(int limit) {
//    std::unique_lock<std::mutex> lck (resultMtx,std::defer_lock);

    for (int i = 0; i < limit; i++) {
        if (m_resultQueue.empty()) break;

//        if (m_threaded) lck.lock();
        auto it = m_resultQueue.begin();
//        if (m_threaded) lck.unlock();

        m_fileRequester->provideFile(
            it->holderType,
            it->fileName.c_str(),
            &it->buffer[0],
            it->buffer.size()
        );

//        if (m_threaded) lck.lock();
        m_resultQueue.erase(it);
//        if (m_threaded) lck.unlock();
    }
}