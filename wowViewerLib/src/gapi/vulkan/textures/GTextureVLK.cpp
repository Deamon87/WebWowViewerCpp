//
// Created by Deamon on 7/23/2018.
//
#include <iostream>
#include <cstring>
#include "GTextureVLK.h"
#include "../../interface/IDevice.h"

GTextureVLK::GTextureVLK(IDeviceVulkan &device, const std::function<void(const std::weak_ptr<GTextureVLK>&)> &onUpdateCallback)
    : m_device(device), m_onDataUpdate(onUpdateCallback), IDSBindable(false) {

}
GTextureVLK::GTextureVLK(IDeviceVulkan &device,
                         int width, int height,
                         bool isDepthTexture,
                         const VkFormat textureFormatGPU,
                         VkSampleCountFlagBits numSamples,
                         int vulkanMipMapCount, VkImageUsageFlags imageUsageFlags) : m_device(device), IDSBindable(false) {
    //For use in frameBuffer
    m_isFrameBufferImage = true;

    m_width = width;
    m_height = height;

    m_samplable = (imageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) > 0;

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

GTextureVLK::GTextureVLK(IDeviceVulkan &device,
                         const VkImage &image,
                         const VkImageView &imageView,
                         bool dumbParam) :
                         m_device(device), IDSBindable(false) {

    //This image is used as holder for framebuffer data (swapchain framebuffer one)
    texture.image = image;
    texture.view = imageView;
    m_samplable = false;

    //this is texture for use in framebuffer, that's why it is set as initialized
    m_loaded = true;
    m_uploaded = true;
    stagingBufferCreated = false;
}

GTextureVLK::~GTextureVLK() {
    destroyBuffer();
}

void GTextureVLK::destroyBuffer() {
    if (!m_uploaded) return;

    auto *l_device = &m_device;
    auto l_textureImage = texture.image;
    auto l_textureView = texture.view;

    auto l_imageAllocation = imageAllocation;

    m_device.addDeallocationRecord(
        [l_device, l_textureImage, l_textureView, l_imageAllocation]() {
            vkDestroyImageView(l_device->getVkDevice(), l_textureView, nullptr);
            if (l_imageAllocation != nullptr) {
                vmaDestroyImage(l_device->getVMAAllocator(), l_textureImage, l_imageAllocation);
            } else {
                //This is swapchain image. Deleting swapchain image leads to exception
//                vkDestroyImage(l_device->getVkDevice(), l_texture.image, nullptr);
            }
    });
}

bool GTextureVLK::getIsLoaded() {
    return m_loaded;
}
static int pureTexturesUploaded = 0;

void GTextureVLK::loadData(int width, int height, void *data, ITextureFormat textureFormat) {
    std::vector<uint8_t > unifiedBuffer((uint8_t *)data, (uint8_t *)data + (width*height*4));


    HMipmapsVector mipmapsVector = std::make_shared<std::vector<mipmapStruct_t>>(1);
    mipmapStruct_t &mipmap = mipmapsVector->at(0);
    mipmap.height = height;
    mipmap.width = width;
    mipmap.texture = unifiedBuffer;

    createTexture(mipmapsVector, VK_FORMAT_R8G8B8A8_UNORM, unifiedBuffer);
}

void GTextureVLK::createTexture(const HMipmapsVector &hmipmaps, const VkFormat &textureFormatGPU, const std::vector<uint8_t> &unitedBuffer) {// Copy data to an optimal tiled image
    if (m_uploaded) {
        std::cout << "oops!" << std::endl << std::flush;
    }

    m_tempUpdateData = std::make_unique<updateData>();

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

    ERR_GUARD_VULKAN(vmaCreateBuffer(m_device.getVMAAllocator(), &createInfo, &allocCreateInfo,
                                     &m_tempUpdateData->stagingBuffer,
                                     &m_tempUpdateData->stagingBufferAlloc,
                                     &m_tempUpdateData->stagingBufferAllocInfo));

    // Copy texture data into host local staging buffer


    memcpy(m_tempUpdateData->stagingBufferAllocInfo.pMappedData, unitedBuffer.data(), unitedBuffer.size());


    ///2. Create regions to copy from CPU staging buffer to GPU buffer
// Setup buffer copy regions for each mip level

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

        VkBufferImageCopy &bufferCopyRegion = m_tempUpdateData->bufferCopyRegions.emplace_back();
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

        offset += static_cast<uint32_t>(mipmaps[i].texture.size());
        vulkanMipMapCount++;
    }

    // Create optimal tiled target image on the device
    createVulkanImageObject(
        false,
        textureFormatGPU,
        VK_SAMPLE_COUNT_1_BIT,
        vulkanMipMapCount,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
    );

    stagingBufferCreated = true;

    if (m_onDataUpdate)
        m_onDataUpdate(this->weak_from_this());
}

void GTextureVLK::createVulkanImageObject(bool isDepthTexture, const VkFormat textureFormatGPU,
                                          VkSampleCountFlagBits numSamples, int vulkanMipMapCount,
                                          VkImageUsageFlags imageUsageFlags) {
    if (!isDepthTexture) {
        m_device.findSupportedFormat(
            {textureFormatGPU},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT
        );
    }

    //3. Create Image on GPU side
    VkImageCreateInfo imageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = textureFormatGPU;
    imageCreateInfo.mipLevels = vulkanMipMapCount;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = numSamples;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

    // Set initial layout of the image to undefined
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.extent = {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height), 1};
    imageCreateInfo.usage = imageUsageFlags;

    imageCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    imageCreateInfo.queueFamilyIndexCount = 2;

    auto &indicies = m_device.getQueueFamilyIndices();
    std::array<uint32_t, 2> families = {indicies.graphicsFamily.value(), indicies.transferFamily.value()};
    imageCreateInfo.pQueueFamilyIndices = families.data();

    if (indicies.graphicsFamily.value() == indicies.transferFamily.value()) {
        imageCreateInfo.queueFamilyIndexCount = 1;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VmaAllocationCreateInfo allocImageCreateInfo = {};
    allocImageCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
//    allocImageCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT; //this bit forces to create per one texture per memory
    if (m_isFrameBufferImage) {
        allocImageCreateInfo.flags = VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
    }


    vmaCreateImage(m_device.getVMAAllocator(), &imageCreateInfo, &allocImageCreateInfo, &texture.image,
                 &imageAllocation, &imageAllocationInfo);
    if (!m_debugName.empty()) {
        m_device.setObjectName((uint64_t) texture.image, VK_OBJECT_TYPE_IMAGE, m_debugName.c_str());
    }
#ifdef DUMP_SELECTION_OF_MEMTYPE
    std::cout << "Texture "<< m_debugName <<", memtype = " << imageAllocationInfo.memoryType << std::endl;
#endif

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
        view.components = {VK_COMPONENT_SWIZZLE_R};
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

TextureStatus GTextureVLK::postLoad() {
    if (m_loaded) return TextureStatus::TSLoaded;

    if (m_uploaded) {
        m_loaded = true;
        return TextureStatus::TSLoaded;
    }

    return TextureStatus::TSNotLoaded;
}

