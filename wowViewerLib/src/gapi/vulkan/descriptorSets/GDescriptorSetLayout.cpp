//
// Created by Deamon on 03.02.23.
//

#include <algorithm>
#include "GDescriptorSetLayout.h"
#include "../GDeviceVulkan.h"


GDescriptorSetLayout::GDescriptorSetLayout(const std::shared_ptr<IDeviceVulkan> &device,
                                           const std::vector<const shaderMetaData*> &metaDatas,
                                           int setIndex,
                                           const std::string &shaderName,
                                           const DescTypeOverride &typeOverrides) : m_device(device), m_shaderSourceName(shaderName), m_sourceSetIndex(setIndex) {
    for (auto &size : m_arraySizes) size = 0;

    //Create Layout
    auto &shaderLayoutBindings = m_shaderLayoutBindings;
    std::unordered_set<int> bindlessBindPoints;

    for (const auto p_metaData : metaDatas) {
        auto const &metaData = *p_metaData;

        VkShaderStageFlagBits vkStageFlag = [](ShaderStage stage) -> VkShaderStageFlagBits {
            switch (stage) {
                case ShaderStage::Vertex:           return VK_SHADER_STAGE_VERTEX_BIT; break;
                case ShaderStage::Fragment:         return VK_SHADER_STAGE_FRAGMENT_BIT; break;
                case ShaderStage::RayGenerate:      return VK_SHADER_STAGE_RAYGEN_BIT_KHR; break;
                case ShaderStage::RayAnyHit:        return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR; break;
                case ShaderStage::RayClosestHit:    return VK_SHADER_STAGE_ANY_HIT_BIT_KHR; break;
                case ShaderStage::RayMiss:          return VK_SHADER_STAGE_MISS_BIT_KHR; break;
                default:
                    return (VkShaderStageFlagBits)0;
            }
        }(metaData.stage);

        fillUbo(setIndex, typeOverrides, shaderLayoutBindings, p_metaData, vkStageFlag);
        fillSSBO(setIndex, typeOverrides, shaderLayoutBindings, p_metaData, vkStageFlag);

        for (int i = 0; i < p_metaData->imageBindings.size(); i++) {
            auto &imageBinding = p_metaData->imageBindings[i];

            if (imageBinding.set != setIndex) continue;

            auto it = shaderLayoutBindings.find(imageBinding.binding);
            if (it != std::end( shaderLayoutBindings )) {
                it->second.stageFlags |= vkStageFlag;
                if (it->second.descriptorType != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                    std::cerr << "Type mismatch for image in GDescriptorSetLayout" << std::endl;
                    throw std::runtime_error("types mismatch");
                }
            } else {
                VkDescriptorSetLayoutBinding imageLayoutBinding = {};
                imageLayoutBinding.binding = imageBinding.binding;
                imageLayoutBinding.descriptorCount = 1;
                imageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                imageLayoutBinding.pImmutableSamplers = nullptr;
                imageLayoutBinding.stageFlags = vkStageFlag;

                {
                    if (typeOverrides.find(imageBinding.set) != typeOverrides.end()) {
                        auto &setTypeOverrides = typeOverrides.at(imageBinding.set);
                        if (setTypeOverrides.find(imageBinding.binding) != setTypeOverrides.end()) {
                            auto const &overrideStruct = setTypeOverrides.at(imageBinding.binding);
                            assert(overrideStruct.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
                            if (overrideStruct.isBindless) {
                                m_isBindless = true;
                                imageLayoutBinding.descriptorCount = overrideStruct.descriptorCount;
                                bindlessBindPoints.insert(imageBinding.binding);
                            }
                        }
                    }
                }

                m_arraySizes[imageLayoutBinding.binding] = imageLayoutBinding.descriptorCount;

                shaderLayoutBindings.insert({imageBinding.binding, imageLayoutBinding});
                m_totalImages += imageLayoutBinding.descriptorCount;

                m_requiredBindPoints[imageBinding.binding] = true;
            }
        }
    }

    std::vector<VkDescriptorSetLayoutBinding> layouts(shaderLayoutBindings.size());
    std::transform(shaderLayoutBindings.begin(), shaderLayoutBindings.end(), layouts.begin(), [](auto &pair){return pair.second;});
    std::sort(layouts.begin(), layouts.end(), [](VkDescriptorSetLayoutBinding &a, VkDescriptorSetLayoutBinding &b) -> bool {
        return a.binding < b.binding;
    });

    m_totalUbos = 0; m_totalDynUbos = 0; m_totalImages = 0; m_totalSSBOs = 0;
    for (auto& layout : layouts) {
        if (layout.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
            m_totalUbos++;
        } if (layout.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
            m_totalSSBOs++;
        } else if (layout.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
            m_totalDynUbos++;
        } else if (layout.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
            m_totalImages+= layout.descriptorCount;
        }
    }


    VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags{};
    binding_flags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    std::vector<VkDescriptorBindingFlags> flags;

    if (m_isBindless) {
        m_bindlessDescSizes.resize(layouts.size());
        flags.resize(layouts.size());
        binding_flags.bindingCount = layouts.size();
        binding_flags.pBindingFlags = flags.data();

        for (int i = 0; i < layouts.size(); i++) {
            auto const &layout = layouts[i];
            if (bindlessBindPoints.find(layout.binding) != bindlessBindPoints.end()) {
                m_bindlessDescSizes[i] = layout.descriptorCount;
                flags[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
                    VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT ;
            } else {
                m_bindlessDescSizes[i] = 0;
                flags[i] = 0;
            }
        }
    }

    //Create VK descriptor layout
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = layouts.size();
    layoutInfo.pBindings = (!layouts.empty()) ? layouts.data() : nullptr;
    layoutInfo.pNext = nullptr;
    if (m_isBindless) {
        layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
        layoutInfo.pNext = &binding_flags;
    }

    if (vkCreateDescriptorSetLayout(m_device->getVkDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void GDescriptorSetLayout::fillUbo(int setIndex, const DescTypeOverride &typeOverrides,
                                   std::unordered_map<int, VkDescriptorSetLayoutBinding> &shaderLayoutBindings,
                                   const shaderMetaData *p_metaData, const VkShaderStageFlagBits &vkStageFlag) {
    for (int i = 0; i < p_metaData->uboBindings.size(); i++) {
        auto &uboBinding = p_metaData->uboBindings[i];

        if (uboBinding.set != setIndex) continue;

        auto uniformType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uint32_t stageOverride = 0;
        {
            if (typeOverrides.find(uboBinding.set) != typeOverrides.end()) {
                auto &setTypeOverrides = typeOverrides.at(uboBinding.set);
                if (setTypeOverrides.find(uboBinding.binding) != setTypeOverrides.end()) {
                    auto const &overrideStruct = setTypeOverrides.at(uboBinding.binding);
                    assert(overrideStruct.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
                    uniformType = overrideStruct.type;
                    if (overrideStruct.stageMask != 0) {
                        stageOverride = overrideStruct.stageMask;
                    }
                }
            }
        }


        auto it = shaderLayoutBindings.find(uboBinding.binding);
        if (it != std::end( shaderLayoutBindings )) {
            it->second.stageFlags |= vkStageFlag;
            if (it->second.descriptorType != uniformType) {
                std::cerr << "Type mismatch for ubo in GDescriptorSetLayout" << std::endl;
                throw std::runtime_error("types mismatch");
            }
        } else {
            VkDescriptorSetLayoutBinding uboLayoutBinding = {};
            uboLayoutBinding.binding = uboBinding.binding;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = uniformType;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = stageOverride == 0 ? vkStageFlag : stageOverride;

            shaderLayoutBindings.insert({uboBinding.binding, uboLayoutBinding});
            if (uboBinding.size > 0) {
                if (m_requiredUBOSize.find(uboBinding.binding) == m_requiredUBOSize.end()) {
                    m_requiredUBOSize.insert({uboBinding.binding, uboBinding.size});
                } else {
                    if (m_requiredUBOSize.at(uboBinding.binding) != uboBinding.size) {
                        std::cerr << "Size mismatch for ubo for binding " << uboBinding.binding << std::endl;
                        throw std::runtime_error("UBO size mismatch");
                    }
                }
            }

            m_arraySizes[uboLayoutBinding.binding] = uboLayoutBinding.descriptorCount;
            m_requiredBindPoints[uboBinding.binding] = true;
        }
    }
}

void GDescriptorSetLayout::fillSSBO(int setIndex, const DescTypeOverride &typeOverrides,
                                   std::unordered_map<int, VkDescriptorSetLayoutBinding> &shaderLayoutBindings,
                                   const shaderMetaData *p_metaData, const VkShaderStageFlagBits &vkStageFlag) {
    for (int i = 0; i < p_metaData->m_ssboBindings.size(); i++) {
        auto &ssboBinding = p_metaData->m_ssboBindings[i];

        if (ssboBinding.set != setIndex) continue;

        auto uniformType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        uint32_t stageOverride = 0;
        {
            if (typeOverrides.find(ssboBinding.set) != typeOverrides.end()) {
                auto &setTypeOverrides = typeOverrides.at(ssboBinding.set);
                if (setTypeOverrides.find(ssboBinding.binding) != setTypeOverrides.end()) {
                    auto const &overrideStruct = setTypeOverrides.at(ssboBinding.binding);
                    assert(overrideStruct.type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC || overrideStruct.type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
                    uniformType = overrideStruct.type;
                    if (overrideStruct.stageMask != 0) {
                        stageOverride = overrideStruct.stageMask;
                    }
                }
            }
        }

        auto it = shaderLayoutBindings.find(ssboBinding.binding);
        if (it != std::end( shaderLayoutBindings )) {
            it->second.stageFlags |= vkStageFlag;
            if (it->second.descriptorType != uniformType) {
                std::cerr << "Type mismatch for ssbo in GDescriptorSetLayout" << std::endl;
                throw std::runtime_error("types mismatch");
            }
        } else {
            VkDescriptorSetLayoutBinding ssboLayoutBinding = {};
            ssboLayoutBinding.binding = ssboBinding.binding;
            ssboLayoutBinding.descriptorCount = 1;
            ssboLayoutBinding.descriptorType = uniformType;
            ssboLayoutBinding.pImmutableSamplers = nullptr;
            ssboLayoutBinding.stageFlags = stageOverride == 0 ? vkStageFlag : stageOverride;

            shaderLayoutBindings.insert({ssboLayoutBinding.binding, ssboLayoutBinding});
            if (ssboBinding.size > 0) {
                if (m_requiredSSBOSize.find(ssboBinding.binding) == m_requiredSSBOSize.end()) {
                    m_requiredSSBOSize.insert({ssboBinding.binding, ssboBinding.size});
                } else {
                    if (m_requiredSSBOSize.at(ssboBinding.binding) != ssboBinding.size) {
                        std::cerr << "Size mismatch for SSBO for binding " << ssboBinding.binding << std::endl;
                        throw std::runtime_error("SSBO size mismatch");
                    }
                }
            }

            m_arraySizes[ssboLayoutBinding.binding] = ssboLayoutBinding.descriptorCount;
            m_requiredBindPoints[ssboBinding.binding] = true;
        }
    }
}

GDescriptorSetLayout::~GDescriptorSetLayout() {
    auto l_device = m_device->getVkDevice();
    auto l_descriptorSetLayout = m_descriptorSetLayout;
    m_device->addDeallocationRecord([l_device, l_descriptorSetLayout]{
        vkDestroyDescriptorSetLayout(l_device, l_descriptorSetLayout, nullptr);

    });
}
