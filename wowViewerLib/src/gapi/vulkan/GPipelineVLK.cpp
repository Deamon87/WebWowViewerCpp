//
// Created by Deamon on 9/26/2019.
//

#include <iostream>
#include "GPipelineVLK.h"
#include "shaders/GShaderPermutationVLK.h"
#include <array>

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
    HGVertexBufferBindings m_bindings,
    VkRenderPass renderPass,
    HGShaderPermutation shader,
    DrawElementMode element,
    int8_t backFaceCulling,
    int8_t triCCW,
    EGxBlendEnum blendMode,
    int8_t depthCulling,
    int8_t depthWrite) : m_device(dynamic_cast<GDeviceVLK &>(device))  {


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

    createPipeline(shaderVLK,
        renderPass,
        element,
        backFaceCulling,
        triCCW,
        blendMode,
        depthCulling,
        depthWrite,
        vertexBindingDescriptions, vertexAttributeDescriptions);
}

GPipelineVLK::~GPipelineVLK() {

}


void GPipelineVLK::createPipeline(
        GShaderPermutationVLK *shaderVLK,
        VkRenderPass renderPass,
        DrawElementMode m_element,
        int8_t m_backFaceCulling,
        int8_t m_triCCW,
        EGxBlendEnum m_blendMode,
        int8_t m_depthCulling,
        int8_t m_depthWrite,

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
    rasterizer.cullMode = m_backFaceCulling ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
    rasterizer.frontFace = m_triCCW ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;


    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = blendModesVLK[(char) m_blendMode].blendModeEnable ? VK_TRUE : VK_FALSE;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcColorBlendFactor = blendModesVLK[(char) m_blendMode].SrcColor;
    colorBlendAttachment.dstColorBlendFactor = blendModesVLK[(char) m_blendMode].DestColor;
    colorBlendAttachment.srcAlphaBlendFactor = blendModesVLK[(char) m_blendMode].SrcAlpha;
    colorBlendAttachment.dstAlphaBlendFactor = blendModesVLK[(char) m_blendMode].DestAlpha;

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
    depthStencil.pNext = NULL;
    depthStencil.flags = 0;
    depthStencil.depthTestEnable = m_depthCulling ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = m_depthWrite ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = m_device.getInvertZ() ? VK_COMPARE_OP_GREATER_OR_EQUAL : VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    std::array<VkDescriptorSetLayout, 2> descLayouts ;
	descLayouts[0] = shaderVLK->getUboDescriptorLayout();
	descLayouts[1] = shaderVLK->getImageDescriptorLayout();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pNext = NULL;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = NULL;
    pipelineLayoutInfo.setLayoutCount = 2;
    pipelineLayoutInfo.pSetLayouts = &descLayouts[0];

    std::cout << "Pipeline layout for "+shaderVLK->getShaderName() << std::endl;

    if (vkCreatePipelineLayout(m_device.getVkDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    std::array<VkDynamicState, 2> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext = NULL;
    dynamicState.pDynamicStates = &dynamicStateEnables[0];
    dynamicState.dynamicStateCount = 2;

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
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(m_device.getVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                  &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}