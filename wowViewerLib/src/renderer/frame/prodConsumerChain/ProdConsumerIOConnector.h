//
// Created by Deamon on 3/26/2023.
//

#ifndef AWEBWOWVIEWERCPP_PRODCONSUMERIOCONNECTOR_H
#define AWEBWOWVIEWERCPP_PRODCONSUMERIOCONNECTOR_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include "../FrameProfile.h"

template<typename T, typename Container = std::deque<T>>
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
    };

    void blockProcessWithoutWait(int limit, const std::function<void (T& input)> &callback) {
        ZoneScoped;
        int processed = 0;
        std::unique_lock<std::mutex> lock{m_queueMutex, std::defer_lock};
        while ((processed < limit) && (!m_queue.empty())) {
            lock.lock();
            auto result = std::move(m_queue.front());
            m_queue.pop();
            lock.unlock();

            callback(result);
            processed++;
        }
    };

    void waitAndProcess(const std::function<void (T& input)> &callback) {
        std::unique_lock<std::mutex> lock{m_queueMutex};
        auto &l_queue = m_queue;
        auto &l_terminated = this->m_terminating;
        {
            using namespace std::chrono_literals;
            m_condVar.wait_for(lock, 20ms, [&l_queue, &l_terminated]()
            {
                return !l_queue.empty() || l_terminated;
            });
            lock.unlock();
        }

        blockProcessWithoutWait(INT_MAX, callback);
    };

    bool empty() {return m_queue.empty(); }

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
    std::queue<T, Container> m_queue = std::queue<T, Container>();
};




#endif //AWEBWOWVIEWERCPP_PRODCONSUMERIOCONNECTOR_H
