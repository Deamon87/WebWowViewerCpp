//
// Created by Deamon on 7/1/2018.
//

#include <iostream>
#include "GShaderPermutationVLK.h"
#include "../../../engine/stringTrim.h"
#include "../../../engine/algorithms/hashString.h"
#include "../../../engine/shader/ShaderDefinitions.h"
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

    return shaderModule;
}

GShaderPermutationVLK::GShaderPermutationVLK(std::string &shaderName, IDevice * device) :
    m_device(dynamic_cast<GDeviceVLK *>(device)), m_shaderName(shaderName), m_shaderNameVert(shaderName), m_shaderNameFrag(shaderName){

}

GShaderPermutationVLK::GShaderPermutationVLK(std::string &shaderName, std::string &shaderVertName, std::string &shaderFragName,
                                             IDevice *device) :
    m_device(dynamic_cast<GDeviceVLK *>(device)), m_shaderName(shaderName), m_shaderNameVert(shaderVertName), m_shaderNameFrag(shaderFragName){
                                             

}

void GShaderPermutationVLK::createUBODescriptorLayout() {
    std::unordered_map<int,VkDescriptorSetLayoutBinding> shaderLayoutBindings;
    for (int i = 0; i < vertShaderMeta->uboBindings.size(); i++) {
        auto &uboVertBinding = vertShaderMeta->uboBindings[i];

        auto it = shaderLayoutBindings.find(uboVertBinding.binding);
        if (it != std::end( shaderLayoutBindings )) {
            it->second.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
        } else {
            VkDescriptorSetLayoutBinding uboLayoutBinding = {};
            uboLayoutBinding.binding = uboVertBinding.binding;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            shaderLayoutBindings.insert({uboVertBinding.binding, uboLayoutBinding});
        }

        hasBondUBO[uboVertBinding.binding] = true;

    }
    for (int i = 0; i < fragShaderMeta->uboBindings.size(); i++) {
        auto &uboFragBinding = fragShaderMeta->uboBindings[i];

        auto it = shaderLayoutBindings.find(uboFragBinding.binding);
        if (it != std::end( shaderLayoutBindings )) {
            it->second.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        } else {
            VkDescriptorSetLayoutBinding uboLayoutBinding = {};
            uboLayoutBinding.binding = uboFragBinding.binding;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            shaderLayoutBindings.insert({uboFragBinding.binding, uboLayoutBinding});
        }

        hasBondUBO[uboFragBinding.binding] = true;
    }

    std::vector<VkDescriptorSetLayoutBinding> shaderLayoutBindingsVec;
    shaderLayoutBindingsVec.reserve(shaderLayoutBindings.size());
    for(auto elem : shaderLayoutBindings) {
        shaderLayoutBindingsVec.push_back(elem.second);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = NULL;
    layoutInfo.bindingCount = shaderLayoutBindingsVec.size();
    layoutInfo.pBindings = &shaderLayoutBindingsVec[0];

    if (vkCreateDescriptorSetLayout(m_device->getVkDevice(), &layoutInfo, nullptr, &uboDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void GShaderPermutationVLK::createImageDescriptorLayout() {
    std::vector<VkDescriptorSetLayoutBinding> shaderLayoutBindings;

    for (int i = 0; i < getTextureCount(); i++) {
        VkDescriptorSetLayoutBinding imageLayoutBinding = {};
        imageLayoutBinding.binding = getTextureBindingStart()+i;
        imageLayoutBinding.descriptorCount = 1;
        imageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imageLayoutBinding.pImmutableSamplers = nullptr;
        imageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT  | VK_SHADER_STAGE_VERTEX_BIT;

        shaderLayoutBindings.push_back(imageLayoutBinding);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = shaderLayoutBindings.size();
    layoutInfo.pBindings = (shaderLayoutBindings.size() > 0) ? &shaderLayoutBindings[0] : nullptr;

    if (vkCreateDescriptorSetLayout(m_device->getVkDevice(), &layoutInfo, nullptr, &imageDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void GShaderPermutationVLK::updateDescriptorSet(int index) {
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    std::vector<VkWriteDescriptorSet> descriptorWrites;

    auto *uploadBuffer = ((IBufferVLK *) m_device->getUploadBuffer(index).get());
    if (uploadBuffer == nullptr) return;

    for (int i = 0; i < vertShaderMeta->uboBindings.size(); i++) {
        auto &uboVertBinding = vertShaderMeta->uboBindings[i];

        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uploadBuffer->getGPUBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = uboVertBinding.size;
        bufferInfos.push_back(bufferInfo);
    }
    for (int i = 0; i < fragShaderMeta->uboBindings.size(); i++) {
        auto &uboFragBinding = fragShaderMeta->uboBindings[i];

        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uploadBuffer->getGPUBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = uboFragBinding.size;
        bufferInfos.push_back(bufferInfo);
    }

    int buffInd = 0;
    for (int i = 0; i < vertShaderMeta->uboBindings.size(); i++) {
        auto &uboVertBinding = vertShaderMeta->uboBindings[i];

        VkWriteDescriptorSet writeDescriptor;
        writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptor.dstSet = uboDescriptorSets[index]->getDescSet();
        writeDescriptor.pNext = nullptr;
        writeDescriptor.dstBinding = uboVertBinding.binding;
        writeDescriptor.dstArrayElement = 0;
        writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        writeDescriptor.descriptorCount = 1;
        writeDescriptor.pBufferInfo = &bufferInfos[buffInd++];
        writeDescriptor.pImageInfo = nullptr;
        writeDescriptor.pTexelBufferView = nullptr;
        descriptorWrites.push_back(writeDescriptor);

    }
    for (int i = 0; i < fragShaderMeta->uboBindings.size(); i++) {
        auto &uboFragBinding = fragShaderMeta->uboBindings[i];

        VkWriteDescriptorSet writeDescriptor;
        writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptor.dstSet = uboDescriptorSets[index]->getDescSet();
        writeDescriptor.pNext = nullptr;
        writeDescriptor.dstBinding = uboFragBinding.binding;
        writeDescriptor.dstArrayElement = 0;
        writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        writeDescriptor.descriptorCount = 1;
        writeDescriptor.pBufferInfo = &bufferInfos[buffInd++];
        writeDescriptor.pImageInfo = nullptr;
        writeDescriptor.pTexelBufferView = nullptr;
        descriptorWrites.push_back(writeDescriptor);
    }

    uboDescriptorSets[index]->writeToDescriptorSets(descriptorWrites);
}

void GShaderPermutationVLK::createUboDescriptorSets() {
    for (int j = 0; j < 4; j++) {
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        uboDescriptorSets[j] = m_device->createDescriptorSet(uboDescriptorSetLayout, 5, 0);
        updateDescriptorSet(j);
    }
}

void GShaderPermutationVLK::compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) {
    auto vertShaderCode = readFile("spirv/" + m_shaderNameVert + ".vert.spv");
    auto fragShaderCode = readFile("spirv/" + m_shaderNameFrag + ".frag.spv");

    vertShaderModule = createShaderModule(vertShaderCode);
    fragShaderModule = createShaderModule(fragShaderCode);

    vertShaderMeta = &shaderMetaInfo.at(m_shaderNameVert + ".vert.spv");
    fragShaderMeta = &shaderMetaInfo.at(m_shaderNameFrag + ".frag.spv");

    this->createShaderLayout();
    this->createUBODescriptorLayout();
    this->createImageDescriptorLayout();
    this->createUboDescriptorSets();
}

int GShaderPermutationVLK::getTextureBindingStart() {
    return shaderLayout.setLayouts[1].imageBindings.start;
}

int GShaderPermutationVLK::getTextureCount() {
    return shaderLayout.setLayouts[1].imageBindings.length;
}

inline void makeMin(unsigned int &a, const unsigned int b) {
    a = std::min<unsigned int>(a, b);
}
inline void makeMax(unsigned int &a, const unsigned int b) {
    a = std::max<unsigned int>(a, b);
}

void GShaderPermutationVLK::createShaderLayout() {
    //Check the buffer sizes

    //UBO stuff
    for (int i = 0; i < this->vertShaderMeta->uboBindings.size(); i++) {
        auto &uboVertBinding = this->vertShaderMeta->uboBindings[i];

        auto &setLayout = shaderLayout.setLayouts[uboVertBinding.set];

        setLayout.uboSizesPerBinding[uboVertBinding.binding] = uboVertBinding.size;
        makeMin(setLayout.uboBindings.start, uboVertBinding.binding);
        makeMax(setLayout.uboBindings.end, uboVertBinding.binding);
    }
    for (int i = 0; i < this->fragShaderMeta->uboBindings.size(); i++) {
        auto &uboFragBinding = this->fragShaderMeta->uboBindings[i];

        auto &setLayout = shaderLayout.setLayouts[uboFragBinding.set];

        auto it = setLayout.uboSizesPerBinding.find(uboFragBinding.binding);
        if (it != std::end(setLayout.uboSizesPerBinding)) {
            if (it->second != uboFragBinding.size) {
                std::cerr << "sizes mismatch for set = " << uboFragBinding.set
                          << " binding = " << uboFragBinding.binding
                          << " between " << m_shaderNameVert << " and " << m_shaderNameFrag
                          << std::endl;
            }
        } else {
            setLayout.uboSizesPerBinding[uboFragBinding.binding] = uboFragBinding.size;

            makeMin(setLayout.uboBindings.start, uboFragBinding.binding);
            makeMax(setLayout.uboBindings.end, uboFragBinding.binding);
        }
    }

    //Image stuff
    for (int i = 0; i < this->vertShaderMeta->imageBindings.size(); i++) {
        auto &imageVertBinding = this->vertShaderMeta->imageBindings[i];
        auto &setLayout = shaderLayout.setLayouts[imageVertBinding.set];

        if (setLayout.uboSizesPerBinding.find(imageVertBinding.binding) != std::end(setLayout.uboSizesPerBinding)) {
            std::cerr << "types mismatch. image slot is used for UBO. for set = " << imageVertBinding.set
                      << " binding = " << imageVertBinding.binding
                      << " in " << m_shaderNameVert << " and " << m_shaderNameFrag
                      << std::endl;
        }

        makeMin(setLayout.imageBindings.start, imageVertBinding.binding);
        makeMax(setLayout.imageBindings.end, imageVertBinding.binding);
    }
    for (int i = 0; i < this->fragShaderMeta->uboBindings.size(); i++) {
        auto &imageFragBinding = this->fragShaderMeta->imageBindings[i];

        auto &setLayout = shaderLayout.setLayouts[imageFragBinding.set];

        if (setLayout.uboSizesPerBinding.find(imageFragBinding.binding) != std::end(setLayout.uboSizesPerBinding)) {
            std::cerr << "types mismatch. image slot is used for UBO. for set = " << imageFragBinding.set
                      << " binding = " << imageFragBinding.binding
                      << " in " << m_shaderNameVert << " and " << m_shaderNameFrag
                      << std::endl;
        }

        makeMin(setLayout.imageBindings.start, imageFragBinding.binding);
        makeMax(setLayout.imageBindings.end, imageFragBinding.binding);
    }
    //Cleanup
    for (auto &shaderLayout : shaderLayout.setLayouts) {
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

