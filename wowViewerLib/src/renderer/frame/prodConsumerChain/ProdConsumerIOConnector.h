//
// Created by Deamon on 3/26/2023.
//

#ifndef AWEBWOWVIEWERCPP_PRODCONSUMERIOCONNECTOR_H
#define AWEBWOWVIEWERCPP_PRODCONSUMERIOCONNECTOR_H

#include <mutex>
#include <condition_variable>
#include <queue>

template<typename T>
class ProdConsumerIOConnector {
public:
    typedef T value_type;


    ProdConsumerIOConnector(bool &terminating) : m_terminating(terminating) {

    }

    T waitForNewInput() {
        std::unique_lock<std::mutex> lock{m_queueMutex};
        auto &l_queue = m_queue;
        auto &l_terminated = this->m_terminating;
        m_condVar.wait(lock, [&l_queue, &l_terminated]() {
            return !l_queue.empty() || l_terminated;
        });


        if (l_terminated)
            return std::move<T>({});

        auto result = std::move(l_queue.front());
        m_queue.pop();
        lock.unlock();

        return std::move(result);
    }

    void pushInput(const T &newInput) {
        {
            std::lock_guard<std::mutex> l{m_queueMutex};
            m_queue.push(std::move(newInput));
            m_condVar.notify_one();
        }
    }

private:
    bool &m_terminating;
    std::mutex m_queueMutex;
    std::condition_variable m_condVar;
    std::queue<T> m_queue;
};




#endif //AWEBWOWVIEWERCPP_PRODCONSUMERIOCONNECTOR_H
