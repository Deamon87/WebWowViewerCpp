#include <mutex>
#include <vector>
#include "RequestProcessor.h"

std::mutex requestMtx;           // mutex for critical section
std::mutex resultMtx;            // mutex for critical section

//1. Add request to FIFO
//2. Get filename from FIFO
//3. Add result to ResultFIFO
//4. Get ready results from FIFO
void RequestProcessor::addRequest (std::string &fileName) {
    std::unique_lock<std::mutex> lck (requestMtx,std::defer_lock);
    // critical section (exclusive access to std::cout signaled by locking lck):
    lck.lock();

    m_requestQueue.push_back(fileName);

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
            std::string &fileName = m_requestQueue.front();
            lck.unlock();

            this->processFileRequest(fileName);

            lck.lock();
            m_requestQueue.pop_front();
            lck.unlock();
        }
    } else if (!m_threaded) {
        while (!m_requestQueue.empty()) {
            std::string &fileName = m_requestQueue.front();

            this->processFileRequest(fileName);

            m_requestQueue.pop_front();
        }
    }

}

void RequestProcessor::provideResult(std::string &fileName, std::vector<unsigned char> &content) {
    std::unique_lock<std::mutex> lck (resultMtx,std::defer_lock);


    resultStruct resultStructObj;
    resultStructObj.buffer = content;
    resultStructObj.fileName = fileName;

    if (m_threaded) lck.lock();
    m_resultQueue.push_back(resultStructObj);
    if (m_threaded) lck.unlock();
}

void RequestProcessor::processResults(int limit) {
    std::unique_lock<std::mutex> lck (resultMtx,std::defer_lock);

    for (int i = 0; i < limit; i++) {
        if (m_resultQueue.empty()) break;

        if (m_threaded) lck.lock();
        resultStruct resultStructObj = m_resultQueue.front();
        m_resultQueue.pop_front();
        if (m_threaded) lck.unlock();

        m_fileRequester->provideFile(
                resultStructObj.fileName.c_str(),
                &resultStructObj.buffer[0],
                resultStructObj.buffer.size()
        );
    }

}