//
// Created by Deamon on 3/10/2026.
//

#include <iostream>
#include "GShaderPermutationVLKBase.h"
#include "../../../engine/algorithms/hashString.h"
#include <ShaderDefinitions.h>
#include "../../UniformBufferStructures.h"
#include "../../interface/IDevice.h"


std::vector<char> GShaderPermutationVLKBase::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

GShaderPermutationVLKBase::GShaderPermutationVLKBase(const std::string &combinedName,
                                                     const std::shared_ptr<GDeviceVLK> &device,
                                                     const DescTypeOverride &typeOverrides,
                                                     const std::unordered_map<int, const std::shared_ptr<GDescriptorSetLayout>> &dsLayoutOverrides)
    : m_combinedName(combinedName), m_device(device), m_typeOverrides(typeOverrides),
      m_dsLayoutOverrides(dsLayoutOverrides) {
}

VkShaderModule GShaderPermutationVLKBase::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_device->getVkDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    m_device->setObjectName((uint64_t) shaderModule, VK_OBJECT_TYPE_SHADER_MODULE, m_combinedName.c_str());

    return shaderModule;
}

inline void makeMin(unsigned int &a, const unsigned int b) {
    a = std::min<unsigned int>(a, b);
}
inline void makeMax(unsigned int &a, const unsigned int b) {
    a = std::max<unsigned int>(a, b);
}

void GShaderPermutationVLKBase::createShaderLayout() {
    auto metaArray = createMetaArray();

    for (auto const &shaderMeta : metaArray) {

        for (int i = 0; i < shaderMeta->uboBindings.size(); i++) {
            auto &uboBinding = shaderMeta->uboBindings[i];

            auto &setLayout = m_combinedShaderLayout.setLayouts[uboBinding.set];

            auto it = setLayout.uboSizesPerBinding.find(uboBinding.binding);
            if (it != std::end(setLayout.uboSizesPerBinding)) {
                if (it->second != uboBinding.size) {
                    makeMax(it->second, uboBinding.size);
                }
            } else {
                setLayout.uboSizesPerBinding[uboBinding.binding] = uboBinding.size;

                makeMin(setLayout.uboBindings.start, uboBinding.binding);
                makeMax(setLayout.uboBindings.end, uboBinding.binding);
            }
        }

        for (int i = 0; i < shaderMeta->m_ssboBindings.size(); i++) {
            auto &ssboBinding = shaderMeta->m_ssboBindings[i];

            auto &setLayout = m_combinedShaderLayout.setLayouts[ssboBinding.set];

            auto it = setLayout.ssboSizesPerBinding.find(ssboBinding.binding);
            if (it != std::end(setLayout.ssboSizesPerBinding)) {
                if (it->second != ssboBinding.size) {
                    std::cerr << "SSBO sizes mismatch for set = " << ssboBinding.set
                              << " binding = " << ssboBinding.binding
                              << " in " << m_combinedName
                              << std::endl;
                }
            } else {
                setLayout.ssboSizesPerBinding[ssboBinding.binding] = ssboBinding.size;

                makeMin(setLayout.ssboBindings.start, ssboBinding.binding);
                makeMax(setLayout.ssboBindings.end, ssboBinding.binding);
            }
        }

        for (int i = 0; i < shaderMeta->imageBindings.size(); i++) {
            auto &imageBinding = shaderMeta->imageBindings[i];
            auto &setLayout = m_combinedShaderLayout.setLayouts[imageBinding.set];

            if (setLayout.uboSizesPerBinding.find(imageBinding.binding) != std::end(setLayout.uboSizesPerBinding)) {
                std::cerr << "types mismatch. image slot is used for UBO. for set = " << imageBinding.set
                          << " binding = " << imageBinding.binding
                          << " in " << m_combinedName
                          << std::endl;
                throw std::runtime_error("types mismatch");
            }

            makeMin(setLayout.imageBindings.start, imageBinding.binding);
            makeMax(setLayout.imageBindings.end, imageBinding.binding);
        }
    }

    //Cleanup
    for (auto &shaderLayout : m_combinedShaderLayout.setLayouts) {
        {
            auto &data = shaderLayout.uboBindings;
            if (data.start < 100) {
                data.length = data.end - data.start + 1;
            } else {
                data.start = 0;
            }
        }
        {
            auto &data = shaderLayout.ssboBindings;
            if (data.start < 100) {
                data.length = data.end - data.start + 1;
            } else {
                data.start = 0;
            }
        }
        {
            auto &data = shaderLayout.imageBindings;
            if (data.start < 100) {
                data.length = data.end - data.start + 1;
            } else {
                data.start = 0;
            }
        }
    }
}

void GShaderPermutationVLKBase::createSetDescriptorLayouts() {
    std::vector<const shaderMetaData *> metas = createMetaArray();
    for (int i = 0; i < m_combinedShaderLayout.setLayouts.size(); i++) {
        if (m_dsLayoutOverrides.find(i) != m_dsLayoutOverrides.end()) {
            m_descriptorSetLayouts[i] = m_dsLayoutOverrides.at(i);
        } else {
            auto &setLayout = m_combinedShaderLayout.setLayouts[i];
            if (setLayout.imageBindings.length == 0 && setLayout.uboBindings.length == 0 && setLayout.ssboBindings.length == 0) continue;

            m_descriptorSetLayouts[i] = std::make_shared<GDescriptorSetLayout>(m_device, metas, i,
                                                                             m_combinedName,
                                                                             m_typeOverrides);
        }
    }
}

void GShaderPermutationVLKBase::buildLayoutsFromMeta() {
    this->createShaderLayout();
    this->createSetDescriptorLayouts();
    this->createPipelineLayout();
}

std::shared_ptr<GPipelineLayoutVLK>
GShaderPermutationVLKBase::createPipelineLayoutOverrided(const std::unordered_map<int, const std::shared_ptr<GDescriptorSet>> &dses) {
    // Find the highest occupied descriptor set index
    int maxSetIndex = -1;
    for (int i = 0; i < m_combinedShaderLayout.setLayouts.size(); i++) {
        if (dses.find(i) != dses.end() || this->getDescriptorLayout(i) != nullptr) {
            maxSetIndex = i;
        }
    }

    std::vector<VkDescriptorSetLayout> descLayouts;
    descLayouts.reserve(maxSetIndex + 1);
    for (int i = 0; i <= maxSetIndex; i++) {
        if (dses.find(i) != dses.end()) {
            descLayouts.push_back(dses.at(i)->getDescSetLayout()->getSetLayout());
        } else {
            auto dsLayout = this->getDescriptorLayout(i);
            if (dsLayout) {
                descLayouts.push_back(dsLayout->getSetLayout());
            } else {
                std::cerr << "detected a gap in DSes. Pretty bad. No DS layout for index " << i << std::endl << std::flush;
                throw "detected a gap in DSes. Pretty bad";
            }
        }
    };

    // Collect push constant ranges from shader metadata
    std::vector<VkPushConstantRange> pushConstantRanges;
    {
        auto metas = createMetaArray();
        for (auto *meta : metas) {
            if (meta && meta->pushConstantSize > 0) {
                VkShaderStageFlags stageFlag = 0;
                switch (meta->stage) {
                    case ShaderStage::Vertex:   stageFlag = VK_SHADER_STAGE_VERTEX_BIT; break;
                    case ShaderStage::Fragment: stageFlag = VK_SHADER_STAGE_FRAGMENT_BIT; break;
                    case ShaderStage::Compute:  stageFlag = VK_SHADER_STAGE_COMPUTE_BIT; break;
                    default: stageFlag = VK_SHADER_STAGE_ALL; break;
                }
                pushConstantRanges.push_back({stageFlag, 0, meta->pushConstantSize});
            }
        }
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pNext = NULL;
    pipelineLayoutInfo.pushConstantRangeCount = pushConstantRanges.size();
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.empty() ? nullptr : pushConstantRanges.data();
    pipelineLayoutInfo.setLayoutCount = descLayouts.size();
    pipelineLayoutInfo.pSetLayouts = descLayouts.data();

    std::cout << "Pipeline layout for "+this->getShaderCombinedName() << std::endl;

    VkPipelineLayout pipelineLayout;
    if (vkCreatePipelineLayout(m_device->getVkDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    m_device->setObjectName(reinterpret_cast<uint64_t>(pipelineLayout), VK_OBJECT_TYPE_PIPELINE_LAYOUT, m_combinedName.c_str());

    return std::make_shared<GPipelineLayoutVLK>(*m_device, pipelineLayout);
}

void GShaderPermutationVLKBase::createPipelineLayout() {
    m_pipelineLayout = createPipelineLayoutOverrided({});
}

const std::shared_ptr<GDescriptorSetLayout>
GShaderPermutationVLKBase::getDescriptorLayout(int bindPoint) {
    return m_descriptorSetLayouts[bindPoint];
}
