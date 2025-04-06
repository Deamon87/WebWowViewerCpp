//
// Created by Deamon on 11.02.23.
//

#include "GFenceVLK.h"
#include "../../interface/IDevice.h"

GFenceVLK::GFenceVLK(const std::shared_ptr<IDeviceVulkan> &deviceVulkan, bool createInSignalledState) : m_device(deviceVulkan) {
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = NULL;
    fenceInfo.flags =
        (createInSignalledState ? VK_FENCE_CREATE_SIGNALED_BIT : 0) |
        0;

    ERR_GUARD_VULKAN(vkCreateFence(m_device->getVkDevice(), &fenceInfo, nullptr, &m_fence));
}

GFenceVLK::~GFenceVLK() {
    auto l_deviceVlk = m_device->getVkDevice();
    auto l_fence = m_fence;

    m_device->addDeallocationRecord([l_deviceVlk, l_fence]() {
        vkDestroyFence(l_deviceVlk, l_fence, nullptr);
    });
}

constexpr int FENCES_COUNT = 1;

void GFenceVLK::wait(uint64_t maxWaitTime) {
    auto result = vkWaitForFences(m_device->getVkDevice(), FENCES_COUNT, &m_fence, VK_TRUE, maxWaitTime);

    if (result == VK_SUCCESS) {
        for (auto const &callback : onFinish) {
            callback();
        }
        onFinish.clear();
    }
    ERR_GUARD_VULKAN(result);
}

void GFenceVLK::reset() {
    ERR_GUARD_VULKAN(vkResetFences(m_device->getVkDevice(), FENCES_COUNT, &m_fence));
}
