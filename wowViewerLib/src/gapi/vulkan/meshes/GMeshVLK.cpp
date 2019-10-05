//
// Created by deamon on 05.06.18.
//

#include <iostream>
#include <array>
#include "GMeshVLK.h"
#include "../textures/GTextureVLK.h"
#include "../shaders/GShaderPermutationVLK.h"
#include "../buffers/GUniformBufferVLK.h"





GMeshVLK::GMeshVLK(IDevice &device,
             const gMeshTemplate &meshTemplate
) : m_device(dynamic_cast<GDeviceVLK &>(device)), m_shader(meshTemplate.shader), m_meshType(meshTemplate.meshType) {

    m_bindings = meshTemplate.bindings;

    m_depthWrite = (int8_t) (meshTemplate.depthWrite ? 1u : 0u);
    m_depthCulling = (int8_t) (meshTemplate.depthCulling ? 1 : 0);
    m_backFaceCulling = (int8_t) (meshTemplate.backFaceCulling ? 1 : 0);
    m_triCCW = meshTemplate.triCCW;

    m_colorMask = meshTemplate.colorMask;

    m_blendMode = meshTemplate.blendMode;
    m_isTransparent = m_blendMode > EGxBlendEnum::GxBlend_AlphaKey || !m_depthWrite ;

    m_start = meshTemplate.start;
    m_end = meshTemplate.end;
    m_element = meshTemplate.element;
    m_textureCount = meshTemplate.textureCount;

    m_texture = meshTemplate.texture;

    m_vertexUniformBuffer[0] = meshTemplate.vertexBuffers[0];
    m_vertexUniformBuffer[1] = meshTemplate.vertexBuffers[1];
    m_vertexUniformBuffer[2] = meshTemplate.vertexBuffers[2];

    m_fragmentUniformBuffer[0] = meshTemplate.fragmentBuffers[0];
    m_fragmentUniformBuffer[1] = meshTemplate.fragmentBuffers[1];
    m_fragmentUniformBuffer[2] = meshTemplate.fragmentBuffers[2];


    GShaderPermutationVLK* shaderVLK = reinterpret_cast<GShaderPermutationVLK *>(m_shader.get());
    createDescriptorSets(shaderVLK);
    hgPipelineVLK = m_device.createPipeline(m_bindings, m_shader, m_element, m_backFaceCulling, m_triCCW, m_blendMode,m_depthCulling, m_depthWrite);
}




void GMeshVLK::createDescriptorSets(GShaderPermutationVLK *shaderVLK) {

    auto descLayout = shaderVLK->getDescriptorLayout();
    //Create descriptor pool
    std::array<VkDescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(5*4);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(shaderVLK->getTextureCount() * 4);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = (shaderVLK->getTextureCount() != 0) ? static_cast<uint32_t>(poolSizes.size()) : 1;
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(4);

    if (vkCreateDescriptorPool(m_device.getVkDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }


    //Create descriptor sets
    std::vector<VkDescriptorSetLayout> descLAyouts(4, descLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(4);
    allocInfo.pSetLayouts = &descLAyouts[0];

    descriptorSets.resize(4);
    descriptorSetsUpdated = std::vector<bool>(4, false);
    if (vkAllocateDescriptorSets(m_device.getVkDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }


    for (int j = 0; j < 4; j++) {
        std::array<VkDescriptorBufferInfo, 5> bufferInfos;
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        int bufferInfoInd = 0;

        for (int i = 0; i < 3; i++ ) {
            GUniformBufferVLK * buffer = (GUniformBufferVLK *) this->getVertexUniformBuffer(i).get();
            if (buffer != nullptr) {
                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = buffer->g_buf[j];
                bufferInfo.offset = 0;
                bufferInfo.range = buffer->getSize();
                bufferInfos[bufferInfoInd] = bufferInfo;

                VkWriteDescriptorSet writeDescriptor;
                writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptor.dstSet = descriptorSets[j];
                writeDescriptor.pNext = nullptr;
                writeDescriptor.dstBinding = i;
                writeDescriptor.dstArrayElement = 0;
                writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                writeDescriptor.descriptorCount = 1;
                writeDescriptor.pBufferInfo = &bufferInfos[bufferInfoInd++];;
                descriptorWrites.push_back(writeDescriptor);

            }
        }
        for (int i = 1; i < 3; i++ ) {
            GUniformBufferVLK *buffer = (GUniformBufferVLK *) this->getFragmentUniformBuffer(i).get();
            if (buffer != nullptr) {
                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = buffer->g_buf[j];
                bufferInfo.offset = 0;
                bufferInfo.range = buffer->getSize();
                bufferInfos[bufferInfoInd]= bufferInfo;

                VkWriteDescriptorSet writeDescriptor;
                writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptor.dstSet = descriptorSets[j];
                writeDescriptor.pNext = nullptr;
                writeDescriptor.dstBinding = 2 + i;
                writeDescriptor.dstArrayElement = 0;
                writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                writeDescriptor.descriptorCount = 1;
                writeDescriptor.pBufferInfo = &bufferInfos[bufferInfoInd++];
                descriptorWrites.push_back(writeDescriptor);
            }
        }

        //Bind Black pixel texture
        std::vector<VkDescriptorImageInfo> imageInfos(shaderVLK->getTextureCount());

        auto blackTexture = m_device.getBlackPixelTexture();
        GTextureVLK *blackTextureVlk = reinterpret_cast<GTextureVLK *>(blackTexture.get());

        int bindIndex = 0;
        for (int i = 0; i < shaderVLK->getTextureCount(); i++) {
            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = blackTextureVlk->texture.view;
            imageInfo.sampler = blackTextureVlk->texture.sampler;
            imageInfos[bindIndex] = imageInfo;

            VkWriteDescriptorSet writeDescriptor;
            writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptor.dstSet = descriptorSets[j];
            writeDescriptor.pNext = nullptr;
            writeDescriptor.dstBinding = shaderVLK->getTextureBindingStart()+bindIndex;
            writeDescriptor.dstArrayElement = 0;
            writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeDescriptor.descriptorCount = 1;
            writeDescriptor.pImageInfo = &imageInfos[bindIndex];
            descriptorWrites.push_back(writeDescriptor);
            bindIndex++;
        }


        vkUpdateDescriptorSets(m_device.getVkDevice(), static_cast<uint32_t>(descriptorWrites.size()), &descriptorWrites[0], 0, nullptr);
    }
}

void GMeshVLK::updateDescriptor() {
    bool allTexturesAreReady = true;

    int textureBegin = ((GShaderPermutationVLK *)m_shader.get())->getTextureBindingStart();

    int updateFrame = m_device.getUpdateFrameNumber();
    if (descriptorSetsUpdated[updateFrame]) return;

    for (auto& texture : m_texture) {
        if (texture == nullptr) continue;
        allTexturesAreReady &= texture->getIsLoaded();
    }

    if (allTexturesAreReady) {
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        std::vector<VkDescriptorImageInfo> imageInfos(m_texture.size());

        if (((GShaderPermutationVLK *)m_shader.get())->getTextureCount() == 0) return;

        for (size_t i = 0; i < m_texture.size(); i++) {
            GTextureVLK *textureVlk = reinterpret_cast<GTextureVLK *>(m_texture[i].get());
            if (textureVlk == nullptr) continue;

            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureVlk->texture.view;
            imageInfo.sampler = textureVlk->texture.sampler;
            imageInfos[i] = imageInfo;

            VkWriteDescriptorSet writeDescriptor;
            writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptor.dstSet = descriptorSets[updateFrame];
            writeDescriptor.pNext = nullptr;
            writeDescriptor.dstBinding = textureBegin+i;
            writeDescriptor.dstArrayElement = 0;
            writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeDescriptor.descriptorCount = 1;
            writeDescriptor.pImageInfo = &imageInfos[i];
            descriptorWrites.push_back(writeDescriptor);
        }

        if (descriptorWrites.size() > 0) {
            vkUpdateDescriptorSets(m_device.getVkDevice(), static_cast<uint32_t>(descriptorWrites.size()), &descriptorWrites[0], 0, nullptr);
        }

        descriptorSetsUpdated[updateFrame] = true;
    }
}


GMeshVLK::~GMeshVLK() {

}

HGUniformBuffer GMeshVLK::getVertexUniformBuffer(int slot) {
    return m_vertexUniformBuffer[slot];
}

HGUniformBuffer GMeshVLK::getFragmentUniformBuffer(int slot) {
    return m_fragmentUniformBuffer[slot];
}

EGxBlendEnum GMeshVLK::getGxBlendMode() { return m_blendMode; }

bool GMeshVLK::getIsTransparent() { return m_isTransparent; }

MeshType GMeshVLK::getMeshType() {
    return m_meshType;
}

void GMeshVLK::setRenderOrder(int renderOrder) {
    m_renderOrder = renderOrder;
}

void GMeshVLK::setStart(int start) {m_start = start; }
void GMeshVLK::setEnd(int end) {m_end = end; }


