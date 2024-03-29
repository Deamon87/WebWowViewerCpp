//
// Created by Deamon on 7/23/2018.
//
#include <iostream>
#include <cstring>
#include "GTextureVLK.h"
#include "../../interface/IDevice.h"

GTextureVLK::GTextureVLK(IDevice &device, bool xWrapTex, bool yWrapTex) : m_device(dynamic_cast<GDeviceVLK &>(device)) {
    this->m_wrapX = xWrapTex;
    this->m_wrapY = yWrapTex;

    createBuffer();
}
GTextureVLK::GTextureVLK(IDevice &device,
                         int width, int height,
                         bool xWrapTex, bool yWrapTex,
                         bool isDepthTexture,
                         const VkFormat textureFormatGPU,
                         VkSampleCountFlagBits numSamples,
                         int vulkanMipMapCount, VkImageUsageFlags imageUsageFlags) : m_device(dynamic_cast<GDeviceVLK &>(device)) {
    //For use in frameBuffer

    this->m_wrapX = xWrapTex;
    this->m_wrapY = yWrapTex;

    m_width = width;
    m_height = height;

    createBuffer();

    createVulkanImageObject(
        isDepthTexture,
        textureFormatGPU,
        numSamples,
        vulkanMipMapCount,
        imageUsageFlags
    );

    //this is texture for use in framebuffer, that's why it is set as initialized
    m_loaded = true;
    m_uploaded = true;
    stagingBufferCreated = false;
}



GTextureVLK::~GTextureVLK() {
    destroyBuffer();
}

void GTextureVLK::createBuffer() {
}

void GTextureVLK::destroyBuffer() {
    if (!m_uploaded) return;

    auto *l_device = &m_device;
    auto &l_texture = texture;

    auto &l_imageAllocation = imageAllocation;
    auto &l_stagingBuffer = stagingBuffer;
    auto &l_stagingBufferAlloc = stagingBufferAlloc;
    auto l_stagingBufferCreated = stagingBufferCreated;

    m_device.addDeallocationRecord(
        [l_device, l_texture, l_imageAllocation, l_stagingBuffer, l_stagingBufferAlloc, l_stagingBufferCreated]() {
            vkDestroyImageView(l_device->getVkDevice(), l_texture.view, nullptr);
            vkDestroySampler(l_device->getVkDevice(), l_texture.sampler, nullptr);
            vmaDestroyImage(l_device->getVMAAllocator(), l_texture.image, l_imageAllocation);

            if (l_stagingBufferCreated) {
                vmaDestroyBuffer(l_device->getVMAAllocator(), l_stagingBuffer, l_stagingBufferAlloc);
            }
    });
}

void GTextureVLK::bind() {

}

void GTextureVLK::unbind() {
}

bool GTextureVLK::getIsLoaded() {
    return m_loaded;
}

static int pureTexturesUploaded = 0;
void GTextureVLK::loadData(int width, int height, void *data, ITextureFormat textureFormat) {
//    std::cout << "pureTexturesUploaded = " << pureTexturesUploaded++ << std::endl;
    std::vector<uint8_t > unifiedBuffer((uint8_t *)data, (uint8_t *)data + (width*height*4));


    HMipmapsVector mipmapsVector = std::make_shared<std::vector<mipmapStruct_t>>();
    mipmapStruct_t mipmap;
    mipmap.height = height;
    mipmap.width = width;
    mipmap.texture = unifiedBuffer;

    mipmapsVector->push_back(mipmap);


    createTexture(mipmapsVector, VK_FORMAT_R8G8B8A8_UNORM, unifiedBuffer);
}

void GTextureVLK::createTexture(const HMipmapsVector &hmipmaps, const VkFormat &textureFormatGPU, std::vector<uint8_t> unitedBuffer) {// Copy data to an optimal tiled image
    if (m_uploaded) {
        std::cout << "oops!" << std::endl << std::flush;
    }

    auto &mipmaps = *hmipmaps;

    m_width = mipmaps[0].width;
    m_height = mipmaps[0].height;

    // This loads the texture data into a host local buffer that is copied to the optimal tiled image on the device

    // Create a host-visible staging buffer that contains the raw image data
    // This buffer will be the data source for copying texture data to the optimal tiled image on the device
    ///1. Create staging buffer for copy from CPU to GPU
    VkBufferCreateInfo createInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    createInfo.size = unitedBuffer.size();
    createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    ERR_GUARD_VULKAN(vmaCreateBuffer(m_device.getVMAAllocator(), &createInfo, &allocCreateInfo, &stagingBuffer,
                                     &stagingBufferAlloc, &stagingBufferAllocInfo));

    // Copy texture data into host local staging buffer


    memcpy(stagingBufferAllocInfo.pMappedData, unitedBuffer.data(), unitedBuffer.size());


    ///2. Create regions to copy from CPU staging buffer to GPU buffer
// Setup buffer copy regions for each mip level
    std::vector<VkBufferImageCopy> bufferCopyRegions;
    uint32_t offset = 0;

    int vulkanMipMapCount = 0;
    for (uint32_t i = 0; i < mipmaps.size(); i++) {
        if (
            (textureFormatGPU != VK_FORMAT_B8G8R8A8_SNORM) &&
            (textureFormatGPU != VK_FORMAT_B8G8R8A8_UNORM) &&
            (textureFormatGPU != VK_FORMAT_R8G8B8A8_UNORM) &&
            ((mipmaps[i].width < 4) || (mipmaps[i].height < 4))
            )
            break;

        VkBufferImageCopy bufferCopyRegion;
        bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        bufferCopyRegion.imageSubresource.mipLevel = i;
        bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
        bufferCopyRegion.imageSubresource.layerCount = 1;
        bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(mipmaps[i].width);
        bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(mipmaps[i].height);
        bufferCopyRegion.imageExtent.depth = 1;
        bufferCopyRegion.bufferRowLength = 0;
        bufferCopyRegion.bufferImageHeight = 0;
        bufferCopyRegion.bufferOffset = offset;
        bufferCopyRegion.imageOffset = {0, 0, 0};

        bufferCopyRegions.push_back(bufferCopyRegion);

        offset += static_cast<uint32_t>(mipmaps[i].texture.size());
        vulkanMipMapCount++;
    }

    // Create optimal tiled target image on the device
    auto indicies = m_device.getQueueFamilyIndices();

    createVulkanImageObject(
        false,
        textureFormatGPU,
        VK_SAMPLE_COUNT_1_BIT,
        vulkanMipMapCount,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
    );
    ///4. Fill commands to copy from CPU staging buffer to GPU buffer

    // Image memory barriers for the texture image

    // The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
    VkImageSubresourceRange subresourceRange = {};
    // Image only contains color data
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // Start at first mip level
    subresourceRange.baseMipLevel = 0;
    // We will transition on all mip levels
    subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    // The 2D texture only has one layer
    subresourceRange.layerCount = 1;

    // Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
    VkImageMemoryBarrier imageMemoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    imageMemoryBarrier.subresourceRange = subresourceRange;
    imageMemoryBarrier.srcAccessMask = 0;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.image = texture.image;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
    // Source pipeline stage is host write/read exection (VK_PIPELINE_STAGE_HOST_BIT)
    // Destination pipeline stage is copy command exection (VK_PIPELINE_STAGE_TRANSFER_BIT)
    vkCmdPipelineBarrier(
        m_device.getUploadCommandBuffer(),
        VK_PIPELINE_STAGE_HOST_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);

    // Copy mip levels from staging buffer
    vkCmdCopyBufferToImage(
        m_device.getUploadCommandBuffer(),
        stagingBuffer,
        texture.image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        static_cast<uint32_t>(bufferCopyRegions.size()),
        bufferCopyRegions.data());

    // Once the data has been uploaded we transfer to the texture image to the shader read layout, so it can be sampled from

    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT ;
    imageMemoryBarrier.dstAccessMask = 0;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    if (m_device.canUploadInSeparateThread()) {
        imageMemoryBarrier.srcQueueFamilyIndex = indicies.transferFamily.value();
        imageMemoryBarrier.dstQueueFamilyIndex = indicies.graphicsFamily.value();
    } else {
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    }
    ///SeparateUploadQueue reference: https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples
    /// "Upload data from the CPU to an image sampled in a fragment shader"


    // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
    // Source pipeline stage stage is copy command exection (VK_PIPELINE_STAGE_TRANSFER_BIT)
    // Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)

    if (m_device.canUploadInSeparateThread()) {
        vkCmdPipelineBarrier(
            m_device.getTextureTransferCommandBuffer(),
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);

        m_device.signalTextureTransferCommandRecorded();
    } else {
        vkCmdPipelineBarrier(
                m_device.getUploadCommandBuffer(),
                VK_PIPELINE_STAGE_TRANSFER_BIT ,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &imageMemoryBarrier);
    }

    // Store current layout for later reuse
    texture.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    m_uploaded = true;
    stagingBufferCreated = true;
}

void GTextureVLK::createVulkanImageObject(bool isDepthTexture, const VkFormat textureFormatGPU,
                                          VkSampleCountFlagBits numSamples, int vulkanMipMapCount,
                                          VkImageUsageFlags imageUsageFlags) {
    //3. Create Image on GPU side
    VkImageCreateInfo imageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = textureFormatGPU;
    imageCreateInfo.mipLevels = vulkanMipMapCount;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = numSamples;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    // Set initial layout of the image to undefined
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.extent = {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height), 1};
    imageCreateInfo.usage = imageUsageFlags;
    //    imageCreateInfo.queueFamilyIndexCount = 2;
    //    std::array<uint32_t, 2> families = {indicies.graphicsFamily.value(), indicies.transferFamily.value()};
    //    imageCreateInfo.pQueueFamilyIndices = &families[0];

    VmaAllocationCreateInfo allocImageCreateInfo = {};
    allocImageCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;


    vmaCreateImage(m_device.getVMAAllocator(), &imageCreateInfo, &allocImageCreateInfo, &texture.image,
                 &imageAllocation, &imageAllocationInfo);

    // Create a texture sampler
    // In Vulkan textures are accessed by samplers
    // This separates all the sampling information from the texture data. This means you could have multiple sampler objects for the same texture with different settings
    // Note: Similar to the samplers available with OpenGL 3.3
    VkSamplerCreateInfo sampler = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    sampler.magFilter = VK_FILTER_LINEAR;
    sampler.minFilter = VK_FILTER_LINEAR;
    sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler.addressModeU = m_wrapX ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler.addressModeV = m_wrapY ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler.mipLodBias = 0.0f;
    sampler.compareOp = VK_COMPARE_OP_NEVER;
    sampler.minLod = 0.0f;
    // Set max level-of-detail to mip level count of the texture
    sampler.maxLod = (float)vulkanMipMapCount;
    // Enable anisotropic filtering
    // This feature is optional, so we must check if it's supported on the device
    if (m_device.getIsAnisFiltrationSupported()) {
      // Use max. level of anisotropy for this example
      sampler.maxAnisotropy = std::min<float>(m_device.getAnisLevel(), vulkanMipMapCount);
      sampler.anisotropyEnable = VK_TRUE;
    } else {
      // The device does not support anisotropic filtering
      sampler.maxAnisotropy = 1.0;
      sampler.anisotropyEnable = VK_FALSE;
    }
    sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    ERR_GUARD_VULKAN(vkCreateSampler(m_device.getVkDevice(), &sampler, nullptr, &texture.sampler));

    // Create image view
    // Textures are not directly accessed by the shaders and
    // are abstracted by image views containing additional
    // information and sub resource ranges
    VkImageViewCreateInfo view = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view.pNext = nullptr;
    view.flags = 0;
    view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view.format = textureFormatGPU;
    if (!isDepthTexture) {
        view.components = {VK_COMPONENT_SWIZZLE_R,
                           VK_COMPONENT_SWIZZLE_G,
                           VK_COMPONENT_SWIZZLE_B,
                           VK_COMPONENT_SWIZZLE_A};
    } else {
        view.components = {VK_COMPONENT_SWIZZLE_IDENTITY};
    }
    // The subresource range describes the set of mip levels (and array layers) that can be accessed through this image view
    // It's possible to create multiple image views for a single image referring to different (and/or overlapping) ranges of the image
    view.subresourceRange.aspectMask = !isDepthTexture ? VK_IMAGE_ASPECT_COLOR_BIT : (VK_IMAGE_ASPECT_DEPTH_BIT);
    view.subresourceRange.baseMipLevel = 0;
    view.subresourceRange.baseArrayLayer = 0;
    view.subresourceRange.layerCount = 1;
    // Linear tiling usually won't support mip maps
    // Only set mip map count if optimal tiling is used
    view.subresourceRange.levelCount = vulkanMipMapCount;
    // The view will be based on the texture's image
    view.image = texture.image;
    ERR_GUARD_VULKAN(vkCreateImageView(m_device.getVkDevice(), &view, nullptr, &texture.view));
}

bool GTextureVLK::postLoad() {
    if (m_loaded) return false;

    if (m_uploaded) {
        m_loaded = true;
    }

    return false;

}


