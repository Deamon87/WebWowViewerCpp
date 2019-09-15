//
// Created by deamon on 05.06.18.
//

#include <iostream>
#include "GMeshVLK.h"
#include "../textures/GTextureVLK.h"
#include "../shaders/GShaderPermutationVLK.h"
#include "../buffers/GUniformBufferVLK.h"

struct BlendModeDescVLK {
    bool blendModeEnable;
    VkBlendFactor SrcColor;
    VkBlendFactor DestColor;
    VkBlendFactor SrcAlpha;
    VkBlendFactor DestAlpha;
};

const uint8_t M2SHADER_MAX_TEXTURES = 4;

BlendModeDescVLK blendModesVLK[(int)EGxBlendEnum::GxBlend_MAX] = {
    /*GxBlend_Opaque*/           {false,VK_BLEND_FACTOR_ONE,VK_BLEND_FACTOR_ZERO,VK_BLEND_FACTOR_ONE,VK_BLEND_FACTOR_ZERO},
    /*GxBlend_AlphaKey*/         {false,VK_BLEND_FACTOR_ONE,VK_BLEND_FACTOR_ZERO,VK_BLEND_FACTOR_ONE,VK_BLEND_FACTOR_ZERO},
    /*GxBlend_Alpha*/            {true,VK_BLEND_FACTOR_SRC_ALPHA,VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,VK_BLEND_FACTOR_ONE,VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA},
    /*GxBlend_Add*/              {true,VK_BLEND_FACTOR_SRC_ALPHA,VK_BLEND_FACTOR_ONE,VK_BLEND_FACTOR_ZERO,VK_BLEND_FACTOR_ONE},
    /*GxBlend_Mod*/              {true,VK_BLEND_FACTOR_DST_COLOR,VK_BLEND_FACTOR_ZERO,VK_BLEND_FACTOR_DST_ALPHA,VK_BLEND_FACTOR_ZERO},
    /*GxBlend_Mod2x*/            {true,VK_BLEND_FACTOR_DST_COLOR,VK_BLEND_FACTOR_SRC_COLOR,VK_BLEND_FACTOR_DST_ALPHA,VK_BLEND_FACTOR_SRC_ALPHA},
    /*GxBlend_ModAdd*/           {true,VK_BLEND_FACTOR_DST_COLOR,VK_BLEND_FACTOR_ONE,VK_BLEND_FACTOR_DST_ALPHA,VK_BLEND_FACTOR_ONE},
    /*GxBlend_InvSrcAlphaAdd*/   {true,VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,VK_BLEND_FACTOR_ONE,VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,VK_BLEND_FACTOR_ONE},
    /*GxBlend_InvSrcAlphaOpaque*/{true,VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,VK_BLEND_FACTOR_ZERO,VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,VK_BLEND_FACTOR_ZERO},
    /*GxBlend_SrcAlphaOpaque*/   {true,VK_BLEND_FACTOR_SRC_ALPHA,VK_BLEND_FACTOR_ZERO,VK_BLEND_FACTOR_SRC_ALPHA,VK_BLEND_FACTOR_ZERO},
    /*GxBlend_NoAlphaAdd*/       {true,VK_BLEND_FACTOR_ONE,VK_BLEND_FACTOR_ONE,VK_BLEND_FACTOR_ZERO,VK_BLEND_FACTOR_ONE},
    /*GxBlend_ConstantAlpha*/    {true,VK_BLEND_FACTOR_CONSTANT_ALPHA,VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,VK_BLEND_FACTOR_CONSTANT_ALPHA,VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA},
    /*GxBlend_Screen*/           {true,VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,VK_BLEND_FACTOR_ONE,VK_BLEND_FACTOR_ONE,VK_BLEND_FACTOR_ZERO},
    /*GxBlend_BlendAdd*/         {true,VK_BLEND_FACTOR_ONE,VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,VK_BLEND_FACTOR_ONE,VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA}
};



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
    GVertexBufferBindingsVLK* bufferBindingsVlk = dynamic_cast<GVertexBufferBindingsVLK *>(m_bindings.get());
    auto &arrVLKFormat = bufferBindingsVlk->getVLKFormat();

    std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
    for (auto &formatVLK : arrVLKFormat) {
        vertexBindingDescriptions.push_back(formatVLK.bindingDescription);
        for (auto &attibuteDesc : formatVLK.attributeDescription) {
            vertexAttributeDescriptions.push_back(attibuteDesc);
        }
    }

    createPipeline(shaderVLK, vertexBindingDescriptions, vertexAttributeDescriptions);
}




void GMeshVLK::createPipeline(GShaderPermutationVLK *shaderVLK,
                              const std::vector<VkVertexInputBindingDescription> &vertexBindingDescriptions,
                              const std::vector<VkVertexInputAttributeDescription> &vertexAttributeDescriptions) {
    auto swapChainExtent = m_device.getCurrentExtent();
    auto renderPass = m_device.getRenderPass();


    //Create Graphic pipeline for Vulkan
    VkShaderModule vertShaderModule = shaderVLK->getVertexModule();
    VkShaderModule fragShaderModule = shaderVLK->getFragmentModule();

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;


    vertexInputInfo.vertexBindingDescriptionCount = vertexBindingDescriptions.size();
    vertexInputInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptions.size();
    vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescriptions[0];
    vertexInputInfo.pVertexAttributeDescriptions = &vertexAttributeDescriptions[0];

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

    switch (m_element) {
        case DrawElementMode::TRIANGLES:
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        case DrawElementMode::TRIANGLE_STRIP:
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            break;
        default:
            throw new std::runtime_error("unknown DrawElementMode");
    }
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = m_backFaceCulling ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
    rasterizer.frontFace = m_triCCW ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE  ;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = blendModesVLK->blendModeEnable ? VK_TRUE : VK_FALSE;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcColorBlendFactor = blendModesVLK[(char)m_blendMode].SrcColor;
    colorBlendAttachment.dstColorBlendFactor = blendModesVLK[(char)m_blendMode].DestColor;
    colorBlendAttachment.srcAlphaBlendFactor = blendModesVLK[(char)m_blendMode].SrcAlpha;
    colorBlendAttachment.dstAlphaBlendFactor = blendModesVLK[(char)m_blendMode].DestAlpha;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = m_depthCulling ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = m_depthWrite ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    auto descLayout = shaderVLK->getDescriptorLayout();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descLayout;

    std::cout << "Pipeline layout for "+((GShaderPermutationVLK *)m_shader.get())->getShaderName() << std::endl;

    if (vkCreatePipelineLayout(m_device.getVkDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState  = &depthStencil;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(m_device.getVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                  &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }


    //Create descriptor pool
    std::array<VkDescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(5*4);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(M2SHADER_MAX_TEXTURES * 4);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
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
        std::vector<VkDescriptorImageInfo> imageInfos(M2SHADER_MAX_TEXTURES);

        auto blackTexture = m_device.getBlackPixelTexture();
        GTextureVLK *blackTextureVlk = reinterpret_cast<GTextureVLK *>(blackTexture.get());

        int bindIndex = 0;
        for (int i = 0; i < M2SHADER_MAX_TEXTURES; i++) {
            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = blackTextureVlk->texture.view;
            imageInfo.sampler = blackTextureVlk->texture.sampler;
            imageInfos[bindIndex] = imageInfo;

            VkWriteDescriptorSet writeDescriptor;
            writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptor.dstSet = descriptorSets[j];
            writeDescriptor.pNext = nullptr;
            writeDescriptor.dstBinding = 5+bindIndex;
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

    int updateFrame = m_device.getUpdateFrameNumber();
    if (descriptorSetsUpdated[updateFrame]) return;

    for (auto& texture : m_texture) {
        allTexturesAreReady &= texture->getIsLoaded();
    }

    if (allTexturesAreReady) {
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        std::vector<VkDescriptorImageInfo> imageInfos(m_texture.size());

        int bindIndex = 0;
        for (auto& texture : m_texture) {
            GTextureVLK *textureVlk = reinterpret_cast<GTextureVLK *>(texture.get());

            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureVlk->texture.view;
            imageInfo.sampler = textureVlk->texture.sampler;
            imageInfos[bindIndex] = imageInfo;

            VkWriteDescriptorSet writeDescriptor;
            writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptor.dstSet = descriptorSets[updateFrame];
            writeDescriptor.pNext = nullptr;
            writeDescriptor.dstBinding = 5+bindIndex;
            writeDescriptor.dstArrayElement = 0;
            writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeDescriptor.descriptorCount = 1;
            writeDescriptor.pImageInfo = &imageInfos[bindIndex];
            descriptorWrites.push_back(writeDescriptor);
            bindIndex++;
        }

        vkUpdateDescriptorSets(m_device.getVkDevice(), static_cast<uint32_t>(descriptorWrites.size()), &descriptorWrites[0], 0, nullptr);

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


