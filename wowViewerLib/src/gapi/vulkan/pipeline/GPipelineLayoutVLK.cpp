//
// Created by Deamon on 10/17/2023.
//

#include "GPipelineLayoutVLK.h"

GPipelineLayoutVLK::GPipelineLayoutVLK(IDevice &device, VkPipelineLayout layout) :
    m_layout(layout), m_device(dynamic_cast<GDeviceVLK &>(device)) {

}

GPipelineLayoutVLK::~GPipelineLayoutVLK() {
    auto l_layout = m_layout;
    auto l_vkDevice = m_device.getVkDevice();
    m_device.addDeallocationRecord([l_layout, l_vkDevice]() {
       vkDestroyPipelineLayout(l_vkDevice, l_layout, nullptr);
    });
}
