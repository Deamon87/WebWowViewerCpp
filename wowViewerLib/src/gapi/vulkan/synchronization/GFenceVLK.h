//
// Created by Deamon on 11.02.23.
//

#ifndef AWEBWOWVIEWERCPP_GFENCEVLK_H
#define AWEBWOWVIEWERCPP_GFENCEVLK_H

#include <memory>
#include "../context/vulkan_context.h"
#include "../IDeviceVulkan.h"

class GFenceVLK {
public:
    GFenceVLK(const std::shared_ptr<IDeviceVulkan> &deviceVulkan, bool createInSignalledState);
    ~GFenceVLK();

    void wait(uint64_t maxWaitTime);
    void reset();
    void addOnFinish(const std::function<void()>& callback) {onFinish.push_back(callback);};

    VkFence getNativeFence() {return m_fence;};
private:
    std::shared_ptr<IDeviceVulkan> m_device;
    VkFence m_fence;
    std::vector<std::function<void()>> onFinish;
};


#endif //AWEBWOWVIEWERCPP_GFENCEVLK_H
