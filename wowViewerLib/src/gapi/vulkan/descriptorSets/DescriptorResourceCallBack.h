//
// Created by Deamon on 2/21/2023.
//

#ifndef AWEBWOWVIEWERCPP_DESCRIPTORRESOURCECALLBACK_H
#define AWEBWOWVIEWERCPP_DESCRIPTORRESOURCECALLBACK_H

#include <list>
#include <memory>
#include <functional>
#include "GDescriptorSet.h"

class DescriptorResourceCallback {
public:
    DescriptorResourceCallback() {
    }

    ~DescriptorResourceCallback() {
        if (m_onRelease) {
            m_onRelease(m_iterator);
        }
    }
private:
    std::list<std::weak_ptr<GDescriptorSet>>::const_iterator m_iterator;
    std::function<void(typename std::list<std::weak_ptr<GDescriptorSet>>::const_iterator)> m_onRelease;
};


#endif //AWEBWOWVIEWERCPP_DESCRIPTORRESOURCECALLBACK_H
