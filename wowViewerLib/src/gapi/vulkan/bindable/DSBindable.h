//
// Created by Deamon on 2/21/2023.
//

#ifndef AWEBWOWVIEWERCPP_DSBINDABLE_H
#define AWEBWOWVIEWERCPP_DSBINDABLE_H

#include <list>
#include <functional>
#include <mutex>

class IDSBindable {
public:
    IDSBindable(bool clearListOnExecution) : m_clearListExecution(clearListOnExecution){
    }

    std::unique_ptr<std::list<std::function<void()>>::const_iterator> addOnHandleChange(const std::function<void()> &callback) {
        std::lock_guard lock(m_callbackMtx);

        m_onHandleChangeList.push_back(callback);
        if (m_clearListExecution) {
            return nullptr;
        } else {
            return std::make_unique<std::list<std::function<void()>>::const_iterator>(
                std::prev(m_onHandleChangeList.end())
            );
        }
    };

    void eraseOnHandleChange(std::unique_ptr<std::list<std::function<void()>>::const_iterator> &iterator) {
        std::lock_guard lock(m_callbackMtx);

        m_onHandleChangeList.erase(*iterator);
        iterator = nullptr;
    };

    void executeOnChange() {
        std::lock_guard lock(m_callbackMtx);

        for (auto &callBack : m_onHandleChangeList) {
            callBack();
        }

        if (m_clearListExecution) {
            m_onHandleChangeList.clear();
        }
    }

private:
    bool m_clearListExecution;

    std::mutex m_callbackMtx;
    std::list<std::function<void()>> m_onHandleChangeList;
};

#endif //AWEBWOWVIEWERCPP_DSBINDABLE_H
