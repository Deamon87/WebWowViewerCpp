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

    m_isSkyBox = meshTemplate.skybox;

    m_isScissorsEnabled = meshTemplate.scissorEnabled ? 1 : 0;
    if (m_isScissorsEnabled) {
        m_scissorSize = meshTemplate.scissorSize;
        m_scissorOffset = meshTemplate.scissorOffset;
    }

    m_colorMask = meshTemplate.colorMask;

    m_blendMode = meshTemplate.blendMode;
    m_isTransparent = m_blendMode > EGxBlendEnum::GxBlend_AlphaKey || !m_depthWrite ;

    m_start = meshTemplate.start;
    m_end = meshTemplate.end;
    m_element = meshTemplate.element;
    m_textureCount = meshTemplate.textureCount;

    m_texture = meshTemplate.texture;

    m_UniformBuffer[0] = meshTemplate.ubo[0];
    m_UniformBuffer[1] = meshTemplate.ubo[1];
    m_UniformBuffer[2] = meshTemplate.ubo[2];
    m_UniformBuffer[3] = meshTemplate.ubo[3];
    m_UniformBuffer[4] = meshTemplate.ubo[4];
    m_UniformBuffer[5] = meshTemplate.ubo[5];


    GShaderPermutationVLK* shaderVLK = reinterpret_cast<GShaderPermutationVLK *>(m_shader.get());
    createDescriptorSets(shaderVLK);

    //Check the buffer sizes
    std::unordered_map<int,uboBindingData> shaderLayoutBindings;
    for (int i = 0; i < shaderVLK->vertShaderMeta->uboBindings.size(); i++) {
        auto &uboVertBinding = shaderVLK->vertShaderMeta->uboBindings[i];

        auto it = shaderLayoutBindings.find(uboVertBinding.binding);
        if (it == std::end( shaderLayoutBindings )) {
            shaderLayoutBindings.insert({uboVertBinding.binding, uboVertBinding});
        }
    }
    for (int i = 0; i < shaderVLK->fragShaderMeta->uboBindings.size(); i++) {
        auto &uboFragBinding = shaderVLK->fragShaderMeta->uboBindings[i];
        auto it = shaderLayoutBindings.find(uboFragBinding.binding);
        if (it == std::end( shaderLayoutBindings )) {
            shaderLayoutBindings.insert({uboFragBinding.binding, uboFragBinding});
        }
    }

    for (int i = 0; i < 5; i++) {
        auto it = shaderLayoutBindings.find(i);
        if (it != shaderLayoutBindings.end()) {
            if ((m_UniformBuffer[i] != nullptr) && (it->second.size != (m_UniformBuffer[i]->getSize()))) {
                std::cout << "buffers missmatch! for shaderName = " << shaderVLK->getShaderName() <<
                " index = " << i <<
                " expected size " << (it->second.size) <<
                ", provided size = " << (m_UniformBuffer[i]->getSize()) <<
                std::endl;
            }
        }
    }
}

//Works under assumption that meshes do not often change the renderpass on which they are rendered
std::shared_ptr<GPipelineVLK> GMeshVLK::getPipeLineForRenderPass(std::shared_ptr<GRenderPassVLK> renderPass) {
    if (m_lastRenderPass != renderPass) {
        m_lastPipelineForRenderPass = m_device.createPipeline(m_bindings, m_shader, renderPass, m_element, m_backFaceCulling, m_triCCW, m_blendMode,m_depthCulling, m_depthWrite);
        m_lastRenderPass = renderPass;
    }

    return m_lastPipelineForRenderPass;
}


void GMeshVLK::createDescriptorSets(GShaderPermutationVLK *shaderVLK) {

    auto descLayout = shaderVLK->getImageDescriptorLayout();

    imageDescriptorSets = std::vector<std::shared_ptr<GDescriptorSets>>(4, NULL);
    for (int j = 0; j < 4; j++) {
        imageDescriptorSets[j] = m_device.createDescriptorSet(descLayout, 0, shaderVLK->getTextureCount());
    }

    descriptorSetsUpdated = std::vector<bool>(4, false);

    for (int j = 0; j < 4; j++) {
        std::vector<VkWriteDescriptorSet> descriptorWrites;

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
            writeDescriptor.dstSet = imageDescriptorSets[j]->getDescSet();
            writeDescriptor.pNext = nullptr;
            writeDescriptor.dstBinding = shaderVLK->getTextureBindingStart()+bindIndex;
            writeDescriptor.dstArrayElement = 0;
            writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeDescriptor.descriptorCount = 1;
            writeDescriptor.pBufferInfo = nullptr;
            writeDescriptor.pImageInfo = &imageInfos[bindIndex];
            writeDescriptor.pTexelBufferView = nullptr;
            descriptorWrites.push_back(writeDescriptor);
            bindIndex++;
        }

        if (!descriptorWrites.empty()) {
            imageDescriptorSets[j]->writeToDescriptorSets(descriptorWrites);
        }
//        vkUpdateDescriptorSets(m_device.getVkDevice(), static_cast<uint32_t>(descriptorWrites.size()), &descriptorWrites[0], 0, nullptr);
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
            writeDescriptor.dstSet = imageDescriptorSets[updateFrame]->getDescSet();
            writeDescriptor.pNext = nullptr;
            writeDescriptor.dstBinding = textureBegin+i;
            writeDescriptor.dstArrayElement = 0;
            writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeDescriptor.descriptorCount = 1;
            writeDescriptor.pImageInfo = &imageInfos[i];
            descriptorWrites.push_back(writeDescriptor);
        }

        if (descriptorWrites.size() > 0) {
            imageDescriptorSets[updateFrame]->writeToDescriptorSets(descriptorWrites);
//            vkUpdateDescriptorSets(m_device.getVkDevice(), static_cast<uint32_t>(descriptorWrites.size()), &descriptorWrites[0], 0, nullptr);
        }

        descriptorSetsUpdated[updateFrame] = true;
    }
}


GMeshVLK::~GMeshVLK() {

}

HGUniformBufferChunk GMeshVLK::getUniformBuffer(int slot) {
    return m_UniformBuffer[slot];
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


