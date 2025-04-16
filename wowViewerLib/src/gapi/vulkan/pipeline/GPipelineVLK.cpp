//
// Created by Deamon on 9/26/2019.
//

#include <iostream>
#include "GPipelineVLK.h"
#include "../shaders/GShaderPermutationVLK.h"
#include <array>
#include "../GRenderPassVLK.h"

struct BlendModeDescVLK {
    bool blendModeEnable;
    VkBlendFactor SrcColor;
    VkBlendFactor DestColor;
    VkBlendFactor SrcAlpha;
    VkBlendFactor DestAlpha;
};

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

GPipelineVLK::GPipelineVLK(IDevice &device,
    const HGVertexBufferBindings &m_bindings,
    const std::shared_ptr<GRenderPassVLK> &renderPass,
    const std::shared_ptr<GPipelineLayoutVLK> &pipelineLayout,
    const HGShaderPermutation &shader,
    DrawElementMode element,
    bool backFaceCulling,
    bool triCCW,
    EGxBlendEnum blendMode,
    bool depthCulling,
    bool depthWrite,
    uint8_t colorMask,
    bool stencilTestEnable,
    bool stencilWrite,
    uint8_t stencilWriteVal) : m_device(dynamic_cast<GDeviceVLK &>(device))  {

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
    GShaderPermutationVLK* shaderVLK = reinterpret_cast<GShaderPermutationVLK *>(shader.get());

    m_pipelineLayout = pipelineLayout;

    m_isTransparent = blendMode > EGxBlendEnum::GxBlend_AlphaKey || !depthWrite;

    createPipeline(shaderVLK,
        renderPass,
        element,
        backFaceCulling,
        triCCW,
        blendMode,
        depthCulling,
        depthWrite,
        colorMask,
        stencilTestEnable,
        stencilWrite,
        stencilWriteVal,
        vertexBindingDescriptions,
        vertexAttributeDescriptions);
}

GPipelineVLK::~GPipelineVLK() {
    auto l_deviceVlk = m_device.getVkDevice();
    auto l_pipelineVlk = graphicsPipeline;

    m_device.addDeallocationRecord(
    [l_deviceVlk, l_pipelineVlk]() {
        vkDestroyPipeline(l_deviceVlk, l_pipelineVlk, nullptr);
    });
}


void GPipelineVLK::createPipeline(
        GShaderPermutationVLK *shaderVLK,
        const std::shared_ptr<GRenderPassVLK> &renderPass,
        DrawElementMode m_element,
        bool backFaceCulling,
        bool triCCW,
        EGxBlendEnum m_blendMode,
        bool depthCulling,
        bool depthWrite,
        uint8_t colorMask,
        bool stencilTestEnable,
        bool stencilWrite,
        uint8_t stencilWriteVal,
        const std::vector<VkVertexInputBindingDescription> &vertexBindingDescriptions,
        const std::vector<VkVertexInputAttributeDescription> &vertexAttributeDescriptions) {

    auto swapChainExtent = m_device.getCurrentExtent();


    //Create Graphic pipeline for Vulkan
    VkShaderModule vertShaderModule = shaderVLK->getVertexModule();
    VkShaderModule fragShaderModule = shaderVLK->getFragmentModule();

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main"; //entry point in SPIR-V

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main"; //entry point in SPIR-V

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;


    vertexInputInfo.vertexBindingDescriptionCount = vertexBindingDescriptions.size();
    vertexInputInfo.pVertexBindingDescriptions =    vertexBindingDescriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount =   vertexAttributeDescriptions.size();
    vertexInputInfo.pVertexAttributeDescriptions =      vertexAttributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

    switch (m_element) {
        case DrawElementMode::LINE:
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
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
    viewport.y = 0.0f;
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;

//    if (!skyBoxMod) {
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
//    } else {
//        viewport.minDepth = 0.998f;
//        viewport.maxDepth = 1.0f;
//    }


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
    rasterizer.cullMode = backFaceCulling ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
    rasterizer.frontFace = triCCW ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = renderPass->getSampleCountBit();

    std::vector <VkPipelineColorBlendAttachmentState> colorBlendAttachments = {};

    //Color attachment 0
    {
        auto &colorBlendAttachment = colorBlendAttachments.emplace_back();
        colorBlendAttachment.colorWriteMask =
            (((colorMask & 1) > 0) ? VK_COLOR_COMPONENT_R_BIT : 0) |
            (((colorMask & 2) > 0) ? VK_COLOR_COMPONENT_G_BIT : 0) |
            (((colorMask & 4) > 0) ? VK_COLOR_COMPONENT_B_BIT : 0) |
            (((colorMask & 8) > 0) ? VK_COLOR_COMPONENT_A_BIT : 0);
        colorBlendAttachment.blendEnable = blendModesVLK[(char) m_blendMode].blendModeEnable ? VK_TRUE : VK_FALSE;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcColorBlendFactor = blendModesVLK[(char) m_blendMode].SrcColor;
        colorBlendAttachment.dstColorBlendFactor = blendModesVLK[(char) m_blendMode].DestColor;
        colorBlendAttachment.srcAlphaBlendFactor = blendModesVLK[(char) m_blendMode].SrcAlpha;
        colorBlendAttachment.dstAlphaBlendFactor = blendModesVLK[(char) m_blendMode].DestAlpha;
    }

    for (size_t i = 1; i < renderPass->getColorAttachmentsCount(); i++) {
        //Do not blend attachments other than in 0
        auto &colorBlendAttachment = colorBlendAttachments.emplace_back();
        colorBlendAttachment.blendEnable = VK_FALSE;

        colorBlendAttachment.colorWriteMask =
            (((colorMask & 1) > 0) ? VK_COLOR_COMPONENT_R_BIT : 0) |
            (((colorMask & 2) > 0) ? VK_COLOR_COMPONENT_G_BIT : 0) |
            (((colorMask & 4) > 0) ? VK_COLOR_COMPONENT_B_BIT : 0) |
            (((colorMask & 8) > 0) ? VK_COLOR_COMPONENT_A_BIT : 0);
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcColorBlendFactor = blendModesVLK[(char) 0].SrcColor;
        colorBlendAttachment.dstColorBlendFactor = blendModesVLK[(char) 0].DestColor;
        colorBlendAttachment.srcAlphaBlendFactor = blendModesVLK[(char) 0].SrcAlpha;
        colorBlendAttachment.dstAlphaBlendFactor = blendModesVLK[(char) 0].DestAlpha;
    }

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = colorBlendAttachments.size();
    colorBlending.pAttachments = colorBlendAttachments.data();
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.pNext = NULL;
    depthStencil.flags = 0;
    depthStencil.depthTestEnable = depthCulling ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = depthWrite ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = renderPass->getInvertZ() ? VK_COMPARE_OP_GREATER_OR_EQUAL : VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = (stencilTestEnable || stencilWrite) ? VK_TRUE : VK_FALSE;
    depthStencil.front = {
        .failOp = VK_STENCIL_OP_KEEP,
        .depthFailOp = VK_STENCIL_OP_KEEP,
        .passOp = stencilWrite ? VK_STENCIL_OP_REPLACE : VK_STENCIL_OP_KEEP,
        .compareOp = stencilTestEnable ? VK_COMPARE_OP_GREATER : VK_COMPARE_OP_ALWAYS,
        .reference = stencilWriteVal,
        .compareMask = 0xFF,
        .writeMask = 0xFF
    };

    static const std::array<VkDynamicState, 2> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext = NULL;
    dynamicState.pDynamicStates = dynamicStateEnables.data();
    dynamicState.dynamicStateCount = dynamicStateEnables.size();

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
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout->getLayout();
    pipelineInfo.renderPass = renderPass->getRenderPass();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    auto res = vkCreateGraphicsPipelines(
        m_device.getVkDevice(),
        VK_NULL_HANDLE, 1,
        &pipelineInfo, nullptr,
        &graphicsPipeline);

    if (res != VK_SUCCESS) {
        std::cerr << "failed to create graphics pipeline!" << std::endl;
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    auto combinedName = shaderVLK->getShaderCombinedName();
    m_device.setObjectName(reinterpret_cast<uint64_t>(graphicsPipeline), VK_OBJECT_TYPE_PIPELINE, combinedName.c_str());
}