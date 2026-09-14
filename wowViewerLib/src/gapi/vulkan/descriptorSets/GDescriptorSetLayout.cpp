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
                case ShaderStage::Compute:          return VK_SHADER_STAGE_COMPUTE_BIT; break;
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
        fillImages(setIndex, typeOverrides, shaderLayoutBindings, bindlessBindPoints, p_metaData, vkStageFlag);
    }

    // Compute combined stage flags from all shader stages in metadata
    VkShaderStageFlags combinedStageFlags = 0;
    for (const auto p_metaData : metaDatas) {
        auto const &metaData = *p_metaData;
        switch (metaData.stage) {
            case ShaderStage::Vertex:   combinedStageFlags |= VK_SHADER_STAGE_VERTEX_BIT; break;
            case ShaderStage::Fragment: combinedStageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT; break;
            case ShaderStage::Compute:  combinedStageFlags |= VK_SHADER_STAGE_COMPUTE_BIT; break;
            default: break;
        }
    }
    if (combinedStageFlags == 0)
        combinedStageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    fillMissingBindingsFromOverrides(
        setIndex, typeOverrides, shaderLayoutBindings, bindlessBindPoints,
        combinedStageFlags
    );

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

    if (vkCreateDescriptorSetLayout(device->getVkDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void GDescriptorSetLayout::fillBindings(
    int setIndex,
    const DescTypeOverride &typeOverrides,
    std::unordered_map<int, VkDescriptorSetLayoutBinding> &shaderLayoutBindings,
    std::unordered_set<int> &bindlessBindPoints,
    const shaderMetaData *p_metaData,
    const VkShaderStageFlagBits &vkStageFlag,
    const std::vector<bindingData> &metaBindings,
    VkDescriptorType defaultType,
    std::unordered_map<int, int> *sizeMap // nullptr if not applicable
) {

    // 1️⃣  Process all existing bindings from shader metadata
    for (auto &binding : metaBindings) {
        if (binding.set != setIndex) continue;

        VkDescriptorType descType = defaultType;
        uint32_t stageOverride = 0;
        uint32_t descriptorCount = 1;
        bool isBindless = false;

        // Check for overrides
        if (auto setIt = typeOverrides.find(binding.set); setIt != typeOverrides.end()) {
            const auto &setOverrides = setIt->second;
            if (auto bindIt = setOverrides.find(binding.binding); bindIt != setOverrides.end()) {
                const auto &ov = bindIt->second;
                descType = ov.type;
                descriptorCount = ov.descriptorCount;
                stageOverride = ov.stageMask;
                isBindless = ov.isBindless;
            }
        }

        // Check if binding already exists
        auto it = shaderLayoutBindings.find(binding.binding);
        if (it != shaderLayoutBindings.end()) {
            it->second.stageFlags |= vkStageFlag;
            if (it->second.descriptorType != descType) {
                std::cerr << "Type mismatch for binding " << binding.binding << std::endl;
                throw std::runtime_error("Descriptor type mismatch");
            }
        } else {
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = binding.binding;
            layoutBinding.descriptorCount = descriptorCount;
            layoutBinding.descriptorType = descType;
            layoutBinding.pImmutableSamplers = nullptr;
            layoutBinding.stageFlags = stageOverride ? stageOverride : vkStageFlag;

            shaderLayoutBindings[binding.binding] = layoutBinding;
            m_arraySizes[binding.binding] = descriptorCount;
            m_requiredBindPoints[binding.binding] = true;

            if (isBindless) {
                this->m_isBindless = true;
                bindlessBindPoints.insert(binding.binding);
            }

            // Handle size validation for UBO/SSBO
            if (sizeMap && binding.size > 0) {
                auto &map = *sizeMap;
                auto itSize = map.find(binding.binding);
                if (itSize == map.end()) {
                    map[binding.binding] = binding.size;
                } else if (itSize->second != binding.size) {
                    std::cerr << "Size mismatch for binding " << binding.binding << std::endl;
                    throw std::runtime_error("Descriptor size mismatch");
                }
            }

            // Count images for statistics
            if (descType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
                this->m_totalImages += descriptorCount;
        }
    }
}

void GDescriptorSetLayout::fillMissingBindingsFromOverrides(int setIndex, const DescTypeOverride &typeOverrides,
                                                            std::unordered_map<int, VkDescriptorSetLayoutBinding> &shaderLayoutBindings,
                                                            std::unordered_set<int> &bindlessBindPoints,
                                                            const VkShaderStageFlags &vkStageFlag) {
    if (auto setIt = typeOverrides.find(setIndex); setIt != typeOverrides.end()) {
        const auto &setOverrides = setIt->second;
        for (auto &[binding, ov] : setOverrides) {
            if (shaderLayoutBindings.find(binding) != shaderLayoutBindings.end())
                continue; // Already handled above

            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = binding;
            layoutBinding.descriptorCount = ov.descriptorCount;
            layoutBinding.descriptorType = ov.type;
            layoutBinding.pImmutableSamplers = nullptr;
            layoutBinding.stageFlags = ov.stageMask ? ov.stageMask : vkStageFlag;

            shaderLayoutBindings[binding] = layoutBinding;
            this->m_arraySizes[binding] = layoutBinding.descriptorCount;
            this->m_requiredBindPoints[binding] = true;

            if (ov.isBindless) {
                this->m_isBindless = true;
                bindlessBindPoints.insert(binding);
            }

            //The size is not being overriden as of now
//            if (sizeMap && ov.size > 0) {
//                (*sizeMap)[binding] = ov.size;
//            }

            if (ov.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
                this->m_totalImages += layoutBinding.descriptorCount;
        }
    }
}


void GDescriptorSetLayout::fillImages(int setIndex, const DescTypeOverride &typeOverrides,
                                      std::unordered_map<int, VkDescriptorSetLayoutBinding> &shaderLayoutBindings,
                                      std::unordered_set<int> &bindlessBindPoints,
                                      const shaderMetaData *p_metaData,
                                      const VkShaderStageFlagBits &vkStageFlag) {
    fillBindings(setIndex, typeOverrides, shaderLayoutBindings, bindlessBindPoints,
                 p_metaData, vkStageFlag, p_metaData->imageBindings,
                 VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nullptr);
}

void GDescriptorSetLayout::fillUbo(int setIndex, const DescTypeOverride &typeOverrides,
                                   std::unordered_map<int, VkDescriptorSetLayoutBinding> &shaderLayoutBindings,
                                   const shaderMetaData *p_metaData,
                                   const VkShaderStageFlagBits &vkStageFlag) {
    std::unordered_set<int> dummyBindless;
    fillBindings(setIndex, typeOverrides, shaderLayoutBindings, dummyBindless,
                 p_metaData, vkStageFlag, p_metaData->uboBindings,
                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &m_requiredUBOSize);
}

void GDescriptorSetLayout::fillSSBO(int setIndex, const DescTypeOverride &typeOverrides,
                                    std::unordered_map<int, VkDescriptorSetLayoutBinding> &shaderLayoutBindings,
                                    const shaderMetaData *p_metaData,
                                    const VkShaderStageFlagBits &vkStageFlag) {
    std::unordered_set<int> dummyBindless;
    fillBindings(setIndex, typeOverrides, shaderLayoutBindings, dummyBindless,
                 p_metaData, vkStageFlag, p_metaData->m_ssboBindings,
                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &m_requiredSSBOSize);
}

GDescriptorSetLayout::~GDescriptorSetLayout() {
    auto s_device = m_device.lock();
    if (s_device) return;
    
    auto l_descriptorSetLayout = m_descriptorSetLayout;
    auto l_device = m_device;
    s_device->addDeallocationRecord([l_device, l_descriptorSetLayout]{
        auto s_device = l_device.lock();
        if (s_device) return;

        vkDestroyDescriptorSetLayout(s_device->getVkDevice(), l_descriptorSetLayout, nullptr);
    });
}
