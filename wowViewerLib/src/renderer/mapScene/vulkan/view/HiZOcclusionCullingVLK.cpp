//
// Hi-Z Occlusion Culling Pipeline
//

#include "HiZOcclusionCullingVLK.h"
#include "../../../../gapi/vulkan/descriptorSets/GDescriptorSet.h"
#include <cmath>
#include <iostream>

HiZOcclusionCullingVLK::HiZOcclusionCullingVLK(const HGDeviceVLK &device) : m_device(device) {
}

HiZOcclusionCullingVLK::~HiZOcclusionCullingVLK() {
    cleanupMipViews();
}

void HiZOcclusionCullingVLK::cleanupMipViews() {
    for (auto view : m_hiZMipViews) {
        if (view != VK_NULL_HANDLE) {
            auto l_device = m_device->getVkDevice();
            auto l_view = view;
            m_device->addDeallocationRecord([l_device, l_view]() {
                vkDestroyImageView(l_device, l_view, nullptr);
            });
        }
    }
    m_hiZMipViews.clear();
    m_hiZGenerateMaterials.clear();
    m_depthCopyMaterial = nullptr;
    m_occlusionCullMaterial = nullptr;
}

void HiZOcclusionCullingVLK::setup(int width, int height) {
    if (width <= 0 || height <= 0) return;
    if (m_hiZTexture &&
        (int)m_hiZTexture->getWidth() == width &&
        (int)m_hiZTexture->getHeight() == height) return;

    cleanupMipViews();

    m_hiZMipCount = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

    // Create Hi-Z texture: R32_SFLOAT with full mipchain
    m_hiZTexture = std::make_shared<GTextureVLK>(
        *m_device,
        width, height,
        false,
        VK_FORMAT_R32_SFLOAT,
        VK_SAMPLE_COUNT_1_BIT,
        m_hiZMipCount,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
    );

    auto sampler = m_device->getSampler(false, false, true);
    m_hiZTextureSamplable = std::make_shared<ISamplableTexture>(m_hiZTexture, sampler);

    // Per-mip image views
    m_hiZMipViews.resize(m_hiZMipCount);
    for (uint32_t mip = 0; mip < m_hiZMipCount; mip++) {
        VkImageViewCreateInfo viewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        viewInfo.image = m_hiZTexture->texture.image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R32_SFLOAT;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = mip;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_device->getVkDevice(), &viewInfo, nullptr, &m_hiZMipViews[mip]) != VK_SUCCESS) {
            std::cerr << "Failed to create Hi-Z mip view for level " << mip << std::endl;
        }
    }

    // Shader config with storage image override at binding 1
    ComputeShaderConfig storageImgConfig;
    storageImgConfig.computeShaderFolder = "compute";
    storageImgConfig.typeOverrides = {
        {0, {
            {1, {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, false, 1, VK_SHADER_STAGE_COMPUTE_BIT}}
        }}
    };

    auto nearestSampler = std::dynamic_pointer_cast<GTextureSamplerVLK>(
        m_device->getSampler(false, false, true)
    );
    VkSampler vkNearestSampler = nearestSampler->getSampler();

    // Depth copy material — descriptor set created lazily in execute()
    m_depthCopyMaterial = ComputeMaterialBuilderVLK::fromShader(m_device, "depthCopy", storageImgConfig)
        .toMaterial();

    // Hi-Z downsample materials — one per mip level
    m_hiZGenerateMaterials.resize(m_hiZMipCount - 1);
    for (uint32_t mip = 0; mip < m_hiZMipCount - 1; mip++) {
        auto srcView = m_hiZMipViews[mip];
        auto dstView = m_hiZMipViews[mip + 1];

        m_hiZGenerateMaterials[mip] = ComputeMaterialBuilderVLK::fromShader(m_device, "hiZGenerate", storageImgConfig)
            .createDescriptorSet(0, [srcView, dstView, vkNearestSampler](std::shared_ptr<GDescriptorSet> &ds) {
                ds->beginUpdate()
                    .imageView_sampler(0, srcView, vkNearestSampler)
                    .storage_image(1, dstView);
            })
            .toMaterial();
    }

    // Occlusion cull material — descriptor set created lazily in execute()
    ComputeShaderConfig occCullConfig;
    occCullConfig.computeShaderFolder = "compute";

    m_occlusionCullMaterial = ComputeMaterialBuilderVLK::fromShader(m_device, "occlusionCull", occCullConfig)
        .toMaterial();
}

void HiZOcclusionCullingVLK::execute(CmdBufRecorder &cmdBuf,
                                      const std::shared_ptr<GFrameBufferVLK> &gBufferFB,
                                      const mathfu::mat4 &viewProj,
                                      const HGBufferVLK &aabbBuffer,
                                      const HGBufferVLK &visibilityBuffer,
                                      uint32_t objectCount,
                                      bool invertZ) {
    if (!isReady() || !gBufferFB || objectCount == 0) return;
    if (!m_depthCopyMaterial || !m_occlusionCullMaterial) return;

    auto depthTexVlk = std::dynamic_pointer_cast<GTextureVLK>(gBufferFB->getDepthTexture());
    if (!depthTexVlk) return;

    VkImage depthImage = depthTexVlk->texture.image;
    VkImage hiZImage = m_hiZTexture->texture.image;
    uint32_t hiZW = m_hiZTexture->getWidth();
    uint32_t hiZH = m_hiZTexture->getHeight();

    VkImageAspectFlags depthAspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    // ---- Step 1: Transition depth → READ_ONLY, Hi-Z mip 0 → GENERAL ----
    {
        std::vector<VkImageMemoryBarrier> barriers(2);

        barriers[0] = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barriers[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        barriers[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barriers[0].oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        barriers[0].newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        barriers[0].image = depthImage;
        barriers[0].subresourceRange = {depthAspect, 0, 1, 0, 1};

        barriers[1] = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barriers[1].srcAccessMask = 0;
        barriers[1].dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        barriers[1].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barriers[1].newLayout = VK_IMAGE_LAYOUT_GENERAL;
        barriers[1].image = hiZImage;
        barriers[1].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

        cmdBuf.recordPipelineImageBarrier(
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            barriers
        );
    }

    // ---- Step 2: Compute copy depth → Hi-Z mip 0 ----
    {
        auto depthSamplable = m_device->createSampledTexture(gBufferFB->getDepthTexture(), false, false);

        if (m_depthCopyMaterial->descriptorSets.empty()) {
            auto ds = std::make_shared<GDescriptorSet>(m_device, m_depthCopyMaterial->shader->getDescriptorLayout(0));
            ds->beginUpdate()
                .texture_depth(0, depthSamplable)
                .storage_image(1, m_hiZMipViews[0]);
            m_depthCopyMaterial->descriptorSets.push_back(ds);
        } else {
            m_depthCopyMaterial->descriptorSets[0]->beginUpdate()
                .texture_depth(0, depthSamplable)
                .storage_image(1, m_hiZMipViews[0]);
        }

        cmdBuf.bindComputePipeline(m_depthCopyMaterial->pipeline);
        cmdBuf.bindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, m_depthCopyMaterial->descriptorSets);

        mathfu::vec4i pushData(hiZW, hiZH, 0, 0);
        cmdBuf.pushConstants(
            m_depthCopyMaterial->pipeline->getLayout()->getLayout(),
            VK_SHADER_STAGE_COMPUTE_BIT,
            0, sizeof(mathfu::vec4i), &pushData
        );

        cmdBuf.dispatch((hiZW + 7) / 8, (hiZH + 7) / 8, 1);
    }

    // ---- Step 3: Transition Hi-Z mip 0 → SHADER_READ ----
    {
        VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.image = hiZImage;
        barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

        cmdBuf.recordPipelineImageBarrier(
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            {barrier}
        );
    }

    // ---- Step 4: Generate mip chain ----
    uint32_t mipW = hiZW;
    uint32_t mipH = hiZH;

    for (uint32_t mip = 0; mip < m_hiZMipCount - 1; mip++) {
        uint32_t dstW = std::max(1u, mipW / 2);
        uint32_t dstH = std::max(1u, mipH / 2);

        {
            VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.image = hiZImage;
            barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, mip + 1, 1, 0, 1};

            cmdBuf.recordPipelineImageBarrier(
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                {barrier}
            );
        }

        auto &mat = m_hiZGenerateMaterials[mip];
        cmdBuf.bindComputePipeline(mat->pipeline);
        cmdBuf.bindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, mat->descriptorSets);

        mathfu::vec4i pushData(mipW, mipH, dstW, dstH);
        cmdBuf.pushConstants(
            mat->pipeline->getLayout()->getLayout(),
            VK_SHADER_STAGE_COMPUTE_BIT,
            0, sizeof(mathfu::vec4i), &pushData
        );

        cmdBuf.dispatch((dstW + 7) / 8, (dstH + 7) / 8, 1);

        {
            VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.image = hiZImage;
            barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, mip + 1, 1, 0, 1};

            cmdBuf.recordPipelineImageBarrier(
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                {barrier}
            );
        }

        mipW = dstW;
        mipH = dstH;
    }

    // ---- Step 5: Transition depth back to ATTACHMENT_OPTIMAL ----
    {
        VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        barrier.image = depthImage;
        barrier.subresourceRange = {depthAspect, 0, 1, 0, 1};

        cmdBuf.recordPipelineImageBarrier(
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            {barrier}
        );
    }

    // ---- Step 6: Dispatch occlusion culling ----
    {
        if (m_occlusionCullMaterial->descriptorSets.empty()) {
            auto ds = std::make_shared<GDescriptorSet>(m_device, m_occlusionCullMaterial->shader->getDescriptorLayout(0));
            ds->beginUpdate()
                .ssbo(0, aabbBuffer)
                .ssbo(1, visibilityBuffer)
                .texture(2, m_hiZTextureSamplable);
            m_occlusionCullMaterial->descriptorSets.push_back(ds);
        } else {
            m_occlusionCullMaterial->descriptorSets[0]->beginUpdate()
                .ssbo(0, aabbBuffer)
                .ssbo(1, visibilityBuffer)
                .texture(2, m_hiZTextureSamplable);
        }

        cmdBuf.bindComputePipeline(m_occlusionCullMaterial->pipeline);
        cmdBuf.bindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, m_occlusionCullMaterial->descriptorSets);

        struct OcclusionCullPushConstants {
            mathfu::mat4 viewProj;
            uint32_t objectCount;
            uint32_t hiZMipCount;
            uint32_t hiZWidth;
            uint32_t hiZHeight;
            float viewportDepthMin;
            float viewportDepthRange;
            float pad0;
            float pad1;
        };

        // Match viewport depth range from CmdBufRecorder::createViewPortTypes (vp_usual)
        const float vpMinDepth = invertZ ? 0.06f : 0.0f;
        const float vpMaxDepth = invertZ ? 1.0f  : 0.990f;

        OcclusionCullPushConstants pc;
        pc.viewProj = viewProj;
        pc.objectCount = objectCount;
        pc.hiZMipCount = m_hiZMipCount;
        pc.hiZWidth = hiZW;
        pc.hiZHeight = hiZH;
        pc.viewportDepthMin = vpMinDepth;
        pc.viewportDepthRange = vpMaxDepth - vpMinDepth;
        pc.pad0 = 0;
        pc.pad1 = 0;

        cmdBuf.pushConstants(
            m_occlusionCullMaterial->pipeline->getLayout()->getLayout(),
            VK_SHADER_STAGE_COMPUTE_BIT,
            0, sizeof(OcclusionCullPushConstants), &pc
        );

        uint32_t groupCount = (objectCount + 63) / 64;
        cmdBuf.dispatch(groupCount, 1, 1);

        // Barrier: occlusion cull write → indirect cull read
        std::vector<VkBufferMemoryBarrier> barriers = {{
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            nullptr,
            VK_ACCESS_SHADER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            visibilityBuffer->getGPUBuffer(),
            0,
            objectCount * sizeof(uint32_t)
        }};

        cmdBuf.recordPipelineBufferBarrier(
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            barriers
        );
    }
}
