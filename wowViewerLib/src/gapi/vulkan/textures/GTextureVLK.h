//
// Created by Deamon on 7/23/2018.
//

#ifndef AWEBWOWVIEWERCPP_GTEXTUREVLK_H
#define AWEBWOWVIEWERCPP_GTEXTUREVLK_H

class GFrameBufferVLK;
class GCommandBuffer;

#include "../IDeviceVulkan.h"
#include "../../interface/textures/ITexture.h"
#include "../bindable/DSBindable.h"


class GTextureVLK : public ITexture, public std::enable_shared_from_this<GTextureVLK>, public IDSBindable {
    friend class GFrameBufferVLK;
public:
    //Used for rendering to texture in framebuffer
    explicit GTextureVLK(IDeviceVulkan &device,
                         int width, int height,
                         bool isDepthTexture,
                         const VkFormat textureFormatGPU,
                         VkSampleCountFlagBits numSamples,
                         int vulkanMipMapCount,
                         VkImageUsageFlags imageUsageFlags);

    //Dumb param is introduced only so that compiler would not invoke constructor with two bools, instead with one
    explicit GTextureVLK(IDeviceVulkan &device,
                         const VkImage &image,
                         const VkImageView &imageView,
                         bool dumbParam);
    GTextureVLK(const GTextureVLK&) = delete;
    GTextureVLK(const GTextureVLK&&) = delete;

    uint32_t getWidth() override {return m_width;};
    uint32_t getHeight() override {return m_height;};
    bool getIsSamplable() override {return m_samplable;};

    explicit GTextureVLK(IDeviceVulkan &device, const std::function<void(const std::weak_ptr<GTextureVLK>&)> &onUpdateCallback);

    void createTexture(const HMipmapsVector &mipmaps, const VkFormat &textureFormatGPU, const std::vector<uint8_t> &unitedBuffer);
private:
    struct updateData {
        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferAlloc = VK_NULL_HANDLE;
        VmaAllocationInfo stagingBufferAllocInfo = {};

        std::vector<VkBufferImageCopy> bufferCopyRegions = {};
    };

public:
    ~GTextureVLK() override;

    void readData(std::vector<uint8_t> &buff) override {};
    void loadData(int width, int height, void *data, ITextureFormat textureFormat) override;
    bool getIsLoaded() override;
    void createTexture(TextureFormat textureFormat, const HMipmapsVector &mipmaps) override {
        throw "Not Implemented in this class";
    }

    const std::unique_ptr<updateData> & getAndPlanDestroy() {
        if (!stagingBufferCreated || m_tempUpdateData == nullptr) {
            throw "Something went wrong";
        }
        auto *l_device = &m_device;

        auto &l_stagingBuffer = m_tempUpdateData->stagingBuffer;
        auto &l_stagingBufferAlloc = m_tempUpdateData->stagingBufferAlloc;

        auto w_this = weak_from_this();

        m_device.addDeallocationRecord([w_this, l_device, l_stagingBuffer, l_stagingBufferAlloc]() {
            vmaDestroyBuffer(l_device->getVMAAllocator(), l_stagingBuffer, l_stagingBufferAlloc);

            if (auto texture = w_this.lock()) {
                texture->m_tempUpdateData = nullptr;
            }
        });

        m_uploaded = true;
        m_loaded = true;

        return m_tempUpdateData;
    }
    TextureStatus postLoad() override;

    struct Texture {
        VkImage image;
        VkImageView view;
    } texture;
private:
    void destroyBuffer();

    std::unique_ptr<updateData> m_tempUpdateData = nullptr;

    VmaAllocation imageAllocation = VK_NULL_HANDLE;
    VmaAllocationInfo imageAllocationInfo = {};

    std::function<void(const std::weak_ptr<GTextureVLK>&)> m_onDataUpdate = nullptr;

protected:
    IDeviceVulkan &m_device;

    bool m_isFrameBufferImage = false;
    bool stagingBufferCreated = false;

    bool m_uploaded = false;
    bool m_loaded = false;

    bool m_samplable = true;

    int m_width = 0;
    int m_height = 0;

    std::string m_debugName;

    void createVulkanImageObject(
        bool isDepthTexture,
        const VkFormat textureFormatGPU,
        VkSampleCountFlagBits numSamples,
        int vulkanMipMapCount,
        VkImageUsageFlags imageUsageFlags
    );

    virtual void freeMipmaps() {return;};
};

typedef std::shared_ptr<GTextureVLK> HGTextureVLK;


#endif //AWEBWOWVIEWERCPP_GTEXTUREVLK_H
