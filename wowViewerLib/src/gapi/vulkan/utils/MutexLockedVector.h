//
// Created by Deamon on 2/21/2023.
//

#ifndef AWEBWOWVIEWERCPP_MUTEXLOCKEDVECTOR_H
#define AWEBWOWVIEWERCPP_MUTEXLOCKEDVECTOR_H

#include <vector>
#include <mutex>

template <typename T>
class MutexLockedVector {
public:
    MutexLockedVector(std::vector<T> &vec, std::mutex &m, bool clearOnDestroy) : m_vec(vec), m_lockGuard(std::lock_guard(m)), m_clearOnDestroy(clearOnDestroy) {

    }
    ~MutexLockedVector() {
        if (m_clearOnDestroy)
            m_vec.clear();
    };
    const std::vector<T> &get() const {
        return m_vec;
    }
private:
    bool m_clearOnDestroy;
    std::vector<T> &m_vec;
    std::lock_guard<std::mutex> m_lockGuard;
};

#endif //AWEBWOWVIEWERCPP_MUTEXLOCKEDVECTOR_H
