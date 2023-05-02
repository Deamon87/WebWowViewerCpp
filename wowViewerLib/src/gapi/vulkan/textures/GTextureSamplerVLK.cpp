//
// Created by Deamon on 4/29/2023.
//

#include <algorithm>
#include "GTextureSamplerVLK.h"
#include "../../interface/IDevice.h"

GTextureSamplerVLK::GTextureSamplerVLK(IDeviceVulkan &deviceVlk, bool xWrapTex, bool yWrapTex) : m_device(deviceVlk) {

    // Create a texture sampler
    // In Vulkan textures are accessed by samplers
    // This separates all the sampling information from the texture data. This means you could have multiple sampler objects for the same texture with different settings
    // Note: Similar to the samplers available with OpenGL 3.3
    VkSamplerCreateInfo sampler = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    sampler.magFilter = VK_FILTER_LINEAR;
    sampler.minFilter = VK_FILTER_LINEAR;
    sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler.addressModeU = xWrapTex ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler.addressModeV = yWrapTex ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler.mipLodBias = 0.0f;
    sampler.compareOp = VK_COMPARE_OP_NEVER;
    sampler.minLod = 0.0f;
    // Set max level-of-detail to mip level count of the texture
    sampler.maxLod = 64.0f;//vulkanMipMapCount;
    // Enable anisotropic filtering
    // This feature is optional, so we must check if it's supported on the device
    if (deviceVlk.getIsAnisFiltrationSupported()) {
        // Use max. level of anisotropy for this example
        sampler.maxAnisotropy = std::min<float>(deviceVlk.getAnisLevel(), 16.0);
        sampler.anisotropyEnable = VK_TRUE;
    } else {
        // The device does not support anisotropic filtering
        sampler.maxAnisotropy = 1.0;
        sampler.anisotropyEnable = VK_FALSE;
    }
    sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    ERR_GUARD_VULKAN(vkCreateSampler(m_device.getVkDevice(), &sampler, nullptr, &m_sampler));
}

    GTextureSamplerVLK::~GTextureSamplerVLK() {
    auto *l_device = &m_device;
    auto &l_sampler = m_sampler;

    m_device.addDeallocationRecord([l_device, l_sampler]() -> void {
        vkDestroySampler(l_device->getVkDevice(), l_sampler, nullptr);
    });
}
