
#include <mutex>
#include <vector>
#include <iostream>
#include "RequestProcessor.h"

std::mutex requestMtx;           // mutex for critical section
std::mutex resultMtx;            // mutex for critical section

std::mutex setProcessingMtx;     // mutex for critical section

//1. Add request to FIFO
//2. Get filename from FIFO
//3. Add result to ResultFIFO
//4. Get ready results from FIFO
void RequestProcessor::addRequest (std::string &fileName, CacheHolderType holderType) {
//    std::cout << "RequestProcessor::addRequest : fileName = " << fileName << std::endl;

    std::unique_lock<std::mutex> setLck (setProcessingMtx,std::defer_lock);
    setLck.lock();
    if (currentlyProcessingFnames.count(fileName) > 0) {
//        std::cout << "RequestProcessor::addRequest : duplicate detected for fileName = " << fileName << std::endl;
        return;
    }
    currentlyProcessingFnames.insert(fileName);
    setLck.unlock();

    std::unique_lock<std::mutex> lck (requestMtx,std::defer_lock);
    // critical section (exclusive access to std::cout signaled by locking lck):
    lck.lock();


    m_requestQueue.push_back({fileName, holderType});

    lck.unlock();
}

void RequestProcessor::processRequests (bool calledFromThread) {
    using namespace std::chrono_literals;
    // critical section (exclusive access to std::cout signaled by locking lck):
    if (calledFromThread){
        std::unique_lock<std::mutex> lck (requestMtx,std::defer_lock);


    } else if (!m_threaded) {
        while (!m_requestQueue.empty()) {
            auto it = m_requestQueue.front();
            m_requestQueue.pop_front();

            this->processFileRequest(it.fileName, it.holderType);
        }
    }

}

void RequestProcessor::provideResult(std::string &fileName, HFileContent content, CacheHolderType holderType) {
    std::unique_lock<std::mutex> lck (resultMtx,std::defer_lock);

    ResultStruct resultStructObj;
    resultStructObj.buffer = content;
    resultStructObj.fileName = fileName;
    resultStructObj.holderType = holderType;

    if (m_threaded) lck.lock();
    m_resultQueue.push_back(resultStructObj);
    if (m_threaded) lck.unlock();
}

void RequestProcessor::processResults(int limit) {
    std::unique_lock<std::mutex> lck (resultMtx,std::defer_lock);

    for (int i = 0; i < limit; i++) {
        if (m_resultQueue.empty()) break;

        if (m_threaded) lck.lock();
        auto it = &m_resultQueue.front();
        if (m_threaded) lck.unlock();

//        std::cout << "it->buffer.use_count() = " << it->buffer.use_count() << std::endl << std::flush;

        HFileContent bufferCpy = it->buffer;
        m_fileRequester->provideFile(
            it->holderType,
            it->fileName.c_str(),
            bufferCpy
        );

        std::unique_lock<std::mutex> setLck (setProcessingMtx,std::defer_lock);
        setLck.lock();
        currentlyProcessingFnames.erase(it->fileName);
        setLck.unlock();


        if (m_threaded) lck.lock();
        m_resultQueue.pop_front();
        if (m_threaded) lck.unlock();
    }


}