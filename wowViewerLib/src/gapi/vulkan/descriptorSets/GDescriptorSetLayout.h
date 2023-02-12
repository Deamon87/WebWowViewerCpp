//
// Created by Deamon on 03.02.23.
//

#ifndef AWEBWOWVIEWERCPP_GDESCRIPTORSETLAYOUT_H
#define AWEBWOWVIEWERCPP_GDESCRIPTORSETLAYOUT_H

#include <memory>

class GDescriptorSetLayout;
class IDeviceVulkan;

#include "../context/vulkan_context.h"
#include "../IDeviceVulkan.h"
#include "../../../engine/shader/ShaderDefinitions.h"

class GDescriptorSetLayout {
public:
    GDescriptorSetLayout(const std::shared_ptr<IDeviceVulkan> &device, const std::vector<const shaderMetaData*> &metaData, int setIndex);

    ~GDescriptorSetLayout();

    const VkDescriptorSetLayout getSetLayout() {return m_descriptorSetLayout;} ;

    const std::unordered_map<int,VkDescriptorSetLayoutBinding>& getShaderLayoutBindings() const {return m_shaderLayoutBindings;} ;

    int getTotalUbos() { return m_totalUbos; };
    int getTotalImages() { return m_totalImages; };
private:
    std::unordered_map<int,VkDescriptorSetLayoutBinding> m_shaderLayoutBindings;
    VkDescriptorSetLayout m_descriptorSetLayout;
    int m_totalImages = 0;
    int m_totalUbos = 0;

    std::shared_ptr<IDeviceVulkan> m_device;
};


#endif //AWEBWOWVIEWERCPP_GDESCRIPTORSETLAYOUT_H
