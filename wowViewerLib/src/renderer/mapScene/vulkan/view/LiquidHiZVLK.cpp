//
// Liquid Hi-Z Min-Max Pipeline
//

#include "LiquidHiZVLK.h"
#include "../../../../gapi/vulkan/descriptorSets/GDescriptorSet.h"
#include <cmath>
#include <iostream>

LiquidHiZVLK::LiquidHiZVLK(const HGDeviceVLK &device) : m_device(device) {
}

LiquidHiZVLK::~LiquidHiZVLK() {
    cleanupMipViews();
}

void LiquidHiZVLK::cleanupMipViews() {
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
}

void LiquidHiZVLK::setup(int width, int height) {
    if (width <= 0 || height <= 0) return;
    if (m_hiZTexture &&
        (int)m_hiZTexture->getWidth() == width &&
        (int)m_hiZTexture->getHeight() == height) return;

    cleanupMipViews();

    m_hiZMipCount = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

    // Create liquid Hi-Z texture: RG32_SFLOAT with full mipchain
    // R = min depth (nearest liquid surface), G = max depth (farthest liquid surface)
    m_hiZTexture = std::make_shared<GTextureVLK>(
        *m_device,
        width, height,
        false,
        VK_FORMAT_R32G32_SFLOAT,
        VK_SAMPLE_COUNT_1_BIT,
        m_hiZMipCount,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
    );

    auto sampler = m_device->getSampler(false, false, true); // nearest filter
    m_hiZTextureSamplable = std::make_shared<ISamplableTexture>(m_hiZTexture, sampler);

    // Per-mip image views
    m_hiZMipViews.resize(m_hiZMipCount);
    for (uint32_t mip = 0; mip < m_hiZMipCount; mip++) {
        VkImageViewCreateInfo viewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        viewInfo.image = m_hiZTexture->texture.image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R32G32_SFLOAT;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = mip;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_device->getVkDevice(), &viewInfo, nullptr, &m_hiZMipViews[mip]) != VK_SUCCESS) {
            std::cerr << "Failed to create liquid Hi-Z mip view for level " << mip << std::endl;
        }
    }

    // Shader config with storage image override at binding 1 (rg32f format)
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

    // Depth copy material
    m_depthCopyMaterial = ComputeMaterialBuilderVLK::fromShader(m_device, "liquidDepthCopy", storageImgConfig)
        .toMaterial();

    // Hi-Z downsample materials — one per mip level
    m_hiZGenerateMaterials.resize(m_hiZMipCount - 1);
    for (uint32_t mip = 0; mip < m_hiZMipCount - 1; mip++) {
        auto srcView = m_hiZMipViews[mip];
        auto dstView = m_hiZMipViews[mip + 1];

        m_hiZGenerateMaterials[mip] = ComputeMaterialBuilderVLK::fromShader(m_device, "liquidHiZGenerate", storageImgConfig)
            .createDescriptorSet(0, [srcView, dstView, vkNearestSampler](std::shared_ptr<GDescriptorSet> &ds) {
                ds->beginUpdate()
                    .imageView_sampler(0, srcView, vkNearestSampler)
                    .storage_image(1, dstView);
            })
            .toMaterial();
    }
}

void LiquidHiZVLK::execute(CmdBufRecorder &cmdBuf,
                            const std::shared_ptr<GFrameBufferVLK> &liquidDepthFB) {
    if (!isReady() || !liquidDepthFB) return;
    if (!m_depthCopyMaterial) return;

    auto depthTexVlk = std::dynamic_pointer_cast<GTextureVLK>(liquidDepthFB->getDepthTexture());
    if (!depthTexVlk) return;

    VkImage depthImage = depthTexVlk->texture.image;
    VkImage hiZImage = m_hiZTexture->texture.image;
    uint32_t hiZW = m_hiZTexture->getWidth();
    uint32_t hiZH = m_hiZTexture->getHeight();

    // Engine depth formats always have a stencil aspect (D32_SFLOAT_S8_UINT / D24_UNORM_S8_UINT);
    // without separateDepthStencilLayouts, barriers must cover both aspects (VUID-VkImageMemoryBarrier-image-03320)
    VkImageAspectFlags depthAspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    // ---- Step 1: Transition liquid depth -> READ_ONLY, Hi-Z mip 0 -> GENERAL ----
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

    // ---- Step 2: Copy liquid depth -> Hi-Z mip 0 ----
    {
        auto depthSamplable = m_device->createSampledTexture(liquidDepthFB->getDepthTexture(), false, false);

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

    // ---- Step 3: Transition Hi-Z mip 0 -> SHADER_READ ----
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

    // Note: liquid depth is left in DEPTH_STENCIL_READ_ONLY_OPTIMAL
    // (ready for shader sampling in fragment shaders)
}
