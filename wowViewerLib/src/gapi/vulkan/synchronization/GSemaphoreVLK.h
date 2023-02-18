//
// Created by Deamon on 11.02.23.
//

#ifndef AWEBWOWVIEWERCPP_GSEMAPHOREVLK_H
#define AWEBWOWVIEWERCPP_GSEMAPHOREVLK_H


#include "../IDeviceVulkan.h"

class GSemaphoreVLK {
public:
    GSemaphoreVLK(const std::shared_ptr<IDeviceVulkan> &deviceVulkan);
    ~GSemaphoreVLK();

    VkSemaphore getNativeSemaphore() {return m_semaphore;};
private:
    std::shared_ptr<IDeviceVulkan> m_device;
    VkSemaphore m_semaphore;

};


#endif //AWEBWOWVIEWERCPP_GSEMAPHOREVLK_H
