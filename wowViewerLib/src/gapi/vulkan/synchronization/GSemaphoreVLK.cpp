//
// Created by Deamon on 11.02.23.
//

#include "GSemaphoreVLK.h"
#include "../../interface/IDevice.h"

GSemaphoreVLK::GSemaphoreVLK(const std::shared_ptr<IDeviceVulkan> &deviceVulkan) {
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = NULL;

    ERR_GUARD_VULKAN(vkCreateSemaphore(deviceVulkan->getVkDevice(), &semaphoreInfo, nullptr, &m_semaphore));
}

GSemaphoreVLK::~GSemaphoreVLK() {
    auto l_deviceVlk = m_device->getVkDevice();
    auto l_semaphore = m_semaphore;

    m_device->addDeallocationRecord([l_deviceVlk, l_semaphore]() {
        vkDestroySemaphore(l_deviceVlk, l_semaphore, nullptr);
    });
}
