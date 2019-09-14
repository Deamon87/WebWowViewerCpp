//
// Created by deamon on 05.06.18.
//

#include "GBlpTextureVLK.h"
#include "../../../engine/persistance/helper/ChunkFileReader.h"
#include "../../../engine/texture/DxtDecompress.h"

GBlpTextureVLK::GBlpTextureVLK(IDevice &device, HBlpTexture texture, bool xWrapTex, bool yWrapTex)
    : GTextureVLK(device), m_texture(texture) {
    this->xWrapTex = xWrapTex;
    this->yWrapTex = yWrapTex;
}

GBlpTextureVLK::~GBlpTextureVLK() {
//    std::cout << "error!" << std::endl;
}


static int texturesUploaded = 0;
void GBlpTextureVLK::createGlTexture(TextureFormat textureFormat, const MipmapsVector &mipmaps) {
//    std::cout << "texturesUploaded = " << texturesUploaded++ << " " << this->m_texture->getTextureName() <<std::endl;

    VkFormat textureFormatGPU;
//     if (ext) {
    switch (textureFormat) {
        case TextureFormat::S3TC_RGB_DXT1:
            textureFormatGPU = VK_FORMAT_BC1_RGB_UNORM_BLOCK;
            break;

        case TextureFormat::S3TC_RGBA_DXT1:
            textureFormatGPU = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
            break;


        case TextureFormat::S3TC_RGBA_DXT3:
            textureFormatGPU = VK_FORMAT_BC2_UNORM_BLOCK;
            break;

        case TextureFormat::S3TC_RGBA_DXT5:
            textureFormatGPU = VK_FORMAT_BC3_UNORM_BLOCK;
            break;

        case TextureFormat::BGRA:
            textureFormatGPU = VK_FORMAT_B8G8R8A8_SNORM;
            break;

        default:
            debuglog("Unknown texture format found in file: ")
            break;
    }
//    }

    /* S3TC is not supported on mobile platforms */
    bool compressedTextSupported = m_device.getIsCompressedTexturesSupported();
    if (!compressedTextSupported && textureFormat !=  TextureFormat::BGRA) {
        this->decompressAndUpload(textureFormat, mipmaps);
        return;
    }

    VkMemoryRequirements memReqs = {};
    // Copy data to an optimal tiled image
    // This loads the texture data into a host local buffer that is copied to the optimal tiled image on the device

    // Create a host-visible staging buffer that contains the raw image data
    // This buffer will be the data source for copying texture data to the optimal tiled image on the device
    VkBuffer stagingBuffer;
    ///1. Create staging buffer for copy from CPU to GPU

    VkBufferCreateInfo createInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    createInfo.size = mipmaps[0].texture.size();
    createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VmaAllocation stagingIndexBufferAlloc = VK_NULL_HANDLE;

    VmaAllocationInfo stagingIndexBufferAllocInfo = {};
    ERR_GUARD_VULKAN(vmaCreateBuffer(m_device.getVMAAllocator(), &createInfo, &allocCreateInfo, &stagingBuffer,
                                     &stagingIndexBufferAlloc, &stagingIndexBufferAllocInfo));

    ///2. Create CPU memory for texture
    // Get memory requirements for the staging buffer (alignment, memory type bits)
    VkMemoryAllocateInfo memAllocInfo;

    vkGetBufferMemoryRequirements(m_device.getVkDevice(), stagingBuffer, &memReqs);
    memAllocInfo.allocationSize = memReqs.size;

    // Get memory type index for a host visible buffer


    ///3. Bind CPU staging buffer and copy data there
    ERR_GUARD_VULKAN(vkBindBufferMemory(m_device.getVkDevice(), stagingBuffer, stagingMemory, 0));

    // Copy texture data into host local staging buffer
    uint8_t *data;
    ERR_GUARD_VULKAN(vkMapMemory(m_device.getVkDevice(), stagingMemory, 0, memReqs.size, 0, (void **)&data));
    memcpy(data, tex2D.data(), tex2D.size());
    vkUnmapMemory(m_device.getVkDevice(), stagingMemory);

    ///4. Fill commands to copy from CPU staging buffer to GPU buffer
    // Setup buffer copy regions for each mip level
    std::vector<VkBufferImageCopy> bufferCopyRegions;
    uint32_t offset = 0;

    for (uint32_t i = 0; i < mipmaps.size(); i++) {
        VkBufferImageCopy bufferCopyRegion = {};
        bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        bufferCopyRegion.imageSubresource.mipLevel = i;
        bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
        bufferCopyRegion.imageSubresource.layerCount = 1;
        bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(tex2D[i].extent().x);
        bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(tex2D[i].extent().y);
        bufferCopyRegion.imageExtent.depth = 1;
        bufferCopyRegion.bufferOffset = offset;

        bufferCopyRegions.push_back(bufferCopyRegion);

        offset += static_cast<uint32_t>(tex2D[i].size());
    }

    // Create optimal tiled target image on the device
    VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = format;
    imageCreateInfo.mipLevels = texture.mipLevels;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    // Set initial layout of the image to undefined
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.extent = { texture.width, texture.height, 1 };
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    ERR_GUARD_VULKAN(vkCreateImage(m_device.getVkDevice(), &imageCreateInfo, nullptr, &texture.image));

    vkGetImageMemoryRequirements(m_device.getVkDevice(), texture.image, &memReqs);
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    ERR_GUARD_VULKAN(vkAllocateMemory(m_device.getVkDevice(), &memAllocInfo, nullptr, &texture.deviceMemory));
    ERR_GUARD_VULKAN(vkBindImageMemory(m_device.getVkDevice(), texture.image, texture.deviceMemory, 0));

    // Image memory barriers for the texture image

    // The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
    VkImageSubresourceRange subresourceRange = {};
    // Image only contains color data
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // Start at first mip level
    subresourceRange.baseMipLevel = 0;
    // We will transition on all mip levels
    subresourceRange.levelCount = texture.mipLevels;
    // The 2D texture only has one layer
    subresourceRange.layerCount = 1;

    // Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
    VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();;
    imageMemoryBarrier.image = texture.image;
    imageMemoryBarrier.subresourceRange = subresourceRange;
    imageMemoryBarrier.srcAccessMask = 0;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

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
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
    // Source pipeline stage stage is copy command exection (VK_PIPELINE_STAGE_TRANSFER_BIT)
    // Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
    vkCmdPipelineBarrier(
        m_device.getUploadCommandBuffer(),
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);

    // Store current layout for later reuse
    texture.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Clean up staging resources
//    vkFreeMemory(m_device.getVkDevice(), stagingMemory, nullptr);
//    vkDestroyBuffer(m_device.getVkDevice(), stagingBuffer, nullptr);

}



bool GBlpTextureVLK::getIsLoaded() {
    return m_loaded;
}

bool GBlpTextureVLK::postLoad() {
    if (m_loaded) return false;
    if (m_texture == nullptr) return false;
    if (!m_texture->getIsLoaded()) return false;

    m_device.bindTexture(this, 0);
    this->createGlTexture(m_texture->getTextureFormat(), m_texture->getMipmapsVector());
    m_device.bindTexture(nullptr, 0);

    m_loaded = true;
    return true;
}



void GBlpTextureVLK::decompressAndUpload(TextureFormat textureFormat, const MipmapsVector &mipmaps) {

}