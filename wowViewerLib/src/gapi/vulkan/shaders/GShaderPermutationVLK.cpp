//
// Created by Deamon on 7/1/2018.
//

#include <iostream>
#include "GShaderPermutationVLK.h"
#include "../../../engine/stringTrim.h"
#include "../../../engine/algorithms/hashString.h"
#include <ShaderDefinitions.h>
#include "../../UniformBufferStructures.h"
#include "../../interface/IDevice.h"
#include <unordered_map>

static std::vector<char> readFile(const std::string& filename) {
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

VkShaderModule GShaderPermutationVLK::createShaderModule(const std::vector<char>& code) {
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

GShaderPermutationVLK::GShaderPermutationVLK(const std::string &shaderVertName, const std::string &shaderFragName,
                                             const std::shared_ptr<GDeviceVLK> &device,
                                             const ShaderConfig &shaderConf,
                                             const std::unordered_map<int, const std::shared_ptr<GDescriptorSetLayout>> &dsLayoutOverrides
                                             ) :
    m_device(device), m_combinedName(shaderVertName + " "+ shaderFragName), m_shaderConf(shaderConf),
    m_shaderNameVert(shaderVertName), m_shaderNameFrag(shaderFragName),
    m_dsLayoutOverrides(dsLayoutOverrides) {
                                             

}

std::vector<const shaderMetaData *> GShaderPermutationVLK::createMetaArray() {
    return {fragShaderMeta, vertShaderMeta};
}

void GShaderPermutationVLK::createSetDescriptorLayouts() {
    std::vector<const shaderMetaData *> metas = createMetaArray();
    for (int i = 0; i < combinedShaderLayout.setLayouts.size(); i++) {
        if (m_dsLayoutOverrides.find(i) != m_dsLayoutOverrides.end()) {
            descriptorSetLayouts[i] = m_dsLayoutOverrides.at(i);
        } else {
            auto &setLayout = combinedShaderLayout.setLayouts[i];
            if (setLayout.imageBindings.length == 0 && setLayout.uboBindings.length == 0) continue;

            descriptorSetLayouts[i] = std::make_shared<GDescriptorSetLayout>(m_device, metas, i,
                                                                             m_shaderConf.typeOverrides);
        }
    }
}

void GShaderPermutationVLK::compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) {
    vertShaderName = m_shaderConf.vertexShaderFolder +"/" + m_shaderNameVert;
    vertShaderFrag = m_shaderConf.fragmentShaderFolder +"/" + m_shaderNameFrag;

    auto vertShaderCode = readFile("spirv/" + vertShaderName + ".vert.spv");
    auto fragShaderCode = readFile("spirv/" + vertShaderFrag + ".frag.spv");

    vertShaderModule = createShaderModule(vertShaderCode);
    fragShaderModule = createShaderModule(fragShaderCode);

    vertShaderMeta = &shaderMetaInfo.at("./"+vertShaderName + ".vert.spv");
    fragShaderMeta = &shaderMetaInfo.at("./"+vertShaderFrag + ".frag.spv");

    this->createShaderLayout();

    this->createSetDescriptorLayouts();

    this->createPipelineLayout();
}

inline void makeMin(unsigned int &a, const unsigned int b) {
    a = std::min<unsigned int>(a, b);
}
inline void makeMax(unsigned int &a, const unsigned int b) {
    a = std::max<unsigned int>(a, b);
}

void GShaderPermutationVLK::createShaderLayout() {
    //UBO stuff
    auto metaArray = createMetaArray();

    for (auto const &shaderMeta : metaArray) {

        for (int i = 0; i < shaderMeta->uboBindings.size(); i++) {
            auto &uboBinding = shaderMeta->uboBindings[i];

            auto &setLayout = combinedShaderLayout.setLayouts[uboBinding.set];

            auto it = setLayout.uboSizesPerBinding.find(uboBinding.binding);
            if (it != std::end(setLayout.uboSizesPerBinding)) {
                if (it->second != uboBinding.size) {
                    std::cerr << "UBO sizes mismatch for set = " << uboBinding.set
                              << " binding = " << uboBinding.binding
                              << " between " << m_shaderNameVert << " and " << m_shaderNameFrag
                              << std::endl;
                }
            } else {
                setLayout.uboSizesPerBinding[uboBinding.binding] = uboBinding.size;

                makeMin(setLayout.uboBindings.start, uboBinding.binding);
                makeMax(setLayout.uboBindings.end, uboBinding.binding);
            }
        }

        for (int i = 0; i < shaderMeta->ssboBindingData.size(); i++) {
            auto &ssboBinding = shaderMeta->ssboBindingData[i];

            auto &setLayout = combinedShaderLayout.setLayouts[ssboBinding.set];

            auto it = setLayout.ssboSizesPerBinding.find(ssboBinding.binding);
            if (it != std::end(setLayout.ssboSizesPerBinding)) {
                if (it->second != ssboBinding.size) {
                    std::cerr << "SSBO sizes mismatch for set = " << ssboBinding.set
                              << " binding = " << ssboBinding.binding
                              << " between " << m_shaderNameVert << " and " << m_shaderNameFrag
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
            auto &setLayout = combinedShaderLayout.setLayouts[imageBinding.set];

            if (setLayout.uboSizesPerBinding.find(imageBinding.binding) != std::end(setLayout.uboSizesPerBinding)) {
                std::cerr << "types mismatch. image slot is used for UBO. for set = " << imageBinding.set
                          << " binding = " << imageBinding.binding
                          << " in " << m_shaderNameVert << " and " << m_shaderNameFrag
                          << std::endl;
                throw std::runtime_error("types mismatch");
            }

            makeMin(setLayout.imageBindings.start, imageBinding.binding);
            makeMax(setLayout.imageBindings.end, imageBinding.binding);
        }
    }


    //Cleanup
    for (auto &shaderLayout : combinedShaderLayout.setLayouts) {
        {
            auto &data = shaderLayout.uboBindings;
            if (shaderLayout.uboBindings.start < 100) {
                data.length = data.end - data.start + 1;
            } else {
                data.start = 0;
            }
        }
        {
            auto &data = shaderLayout.imageBindings;
            if (shaderLayout.uboBindings.start < 100) {
                data.length = data.end - data.start + 1;
            } else {
                data.start = 0;
            }
        }
    }
}

std::shared_ptr<GPipelineLayoutVLK>
GShaderPermutationVLK::createPipelineLayoutOverrided(const std::unordered_map<int, const std::shared_ptr<GDescriptorSet>> &dses) {
    std::vector<VkDescriptorSetLayout> descLayouts;
    descLayouts.reserve(combinedShaderLayout.setLayouts.size());
    for (int i = 0; i < combinedShaderLayout.setLayouts.size(); i++) {

        if (dses.find(i) != dses.end()) {
            descLayouts.push_back(dses.at(i)->getDescSetLayout()->getSetLayout());
        } else {
            descLayouts.push_back(this->getDescriptorLayout(i)->getSetLayout());
        }
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pNext = NULL;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = NULL;
    pipelineLayoutInfo.setLayoutCount = descLayouts.size();
    pipelineLayoutInfo.pSetLayouts = descLayouts.data();

    std::cout << "Pipeline layout for "+this->getShaderCombinedName() << std::endl;

    VkPipelineLayout pipelineLayout;
    if (vkCreatePipelineLayout(m_device->getVkDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    auto combinedName = this->getShaderCombinedName();
    m_device->setObjectName(reinterpret_cast<uint64_t>(pipelineLayout), VK_OBJECT_TYPE_PIPELINE_LAYOUT, combinedName.c_str());

    return std::make_shared<GPipelineLayoutVLK>(*m_device, pipelineLayout);
}

void GShaderPermutationVLK::createPipelineLayout() {
    m_pipelineLayout = createPipelineLayoutOverrided({});
}

const std::shared_ptr<GDescriptorSetLayout>
GShaderPermutationVLK::getDescriptorLayout(int bindPoint) {
    return descriptorSetLayouts[bindPoint];
}

