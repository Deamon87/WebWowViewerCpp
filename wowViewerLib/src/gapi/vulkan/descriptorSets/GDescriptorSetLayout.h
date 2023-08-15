//
// Created by Deamon on 03.02.23.
//

#ifndef AWEBWOWVIEWERCPP_GDESCRIPTORSETLAYOUT_H
#define AWEBWOWVIEWERCPP_GDESCRIPTORSETLAYOUT_H

#include <memory>

class GDescriptorSetLayout;
class IDeviceVulkan;

#include <bitset>
#include "../context/vulkan_context.h"
#include "../IDeviceVulkan.h"
#include "../../../engine/shader/ShaderDefinitions.h"
#include "../shaders/ShaderConfig.h"

class GDescriptorSetLayout {
public:
    static const constexpr int MAX_BINDPOINT_NUMBER = 16;

    GDescriptorSetLayout(const std::shared_ptr<IDeviceVulkan> &device,
                         const std::vector<const shaderMetaData*> &metaData,
                         int setIndex,
                         const DescTypeOverride &typeOverrides);

    ~GDescriptorSetLayout();

    const VkDescriptorSetLayout getSetLayout() {return m_descriptorSetLayout;} ;

    const std::unordered_map<int,VkDescriptorSetLayoutBinding>& getShaderLayoutBindings() const {return m_shaderLayoutBindings;} ;
    const std::unordered_map<int, int>& getRequiredUBOSize() const {return m_requiredUBOSize;} ;

    int getTotalUbos() { return m_totalUbos; };
    int getTotalDynUbos() { return m_totalDynUbos; };
    int getTotalImages() { return m_totalImages; };
    std::bitset<MAX_BINDPOINT_NUMBER> getRequiredBindPoints() {return m_requiredBindPoints;};
private:
    std::unordered_map<int,VkDescriptorSetLayoutBinding> m_shaderLayoutBindings;
    std::unordered_map<int, int> m_requiredUBOSize;

    std::bitset<MAX_BINDPOINT_NUMBER> m_requiredBindPoints = 0;
    VkDescriptorSetLayout m_descriptorSetLayout;
    int m_totalImages = 0;
    int m_totalUbos = 0;
    int m_totalDynUbos = 0;

    std::shared_ptr<IDeviceVulkan> m_device;
};


#endif //AWEBWOWVIEWERCPP_GDESCRIPTORSETLAYOUT_H
