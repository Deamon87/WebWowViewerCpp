//
// Created by Deamon on 7/23/2018.
//

#ifndef AWEBWOWVIEWERCPP_GTEXTUREVLK_H
#define AWEBWOWVIEWERCPP_GTEXTUREVLK_H

class GDeviceVLK;
#include "../GDeviceVulkan.h"
#include "../../interface/textures/ITexture.h"

class GTextureVLK : public ITexture {
    friend class GDeviceVLK;
protected:
    explicit GTextureVLK(IDevice &device);
    void createTexture(const MipmapsVector &mipmaps, const VkFormat &textureFormatGPU, std::vector<uint8_t> unitedBuffer);
public:
    ~GTextureVLK() override;

    void loadData(int width, int height, void *data) override;
    bool getIsLoaded() override;
    void createGlTexture(TextureFormat textureFormat, const MipmapsVector &mipmaps) override {
        throw "Not Implemented in this class";
    }
    bool postLoad() override;;

    struct Texture {
        VkSampler sampler;
        VkImage image;
        VkImageLayout imageLayout;
        VkDeviceMemory deviceMemory;
        VkImageView view;
    } texture;
private:
    void createBuffer();
    void destroyBuffer();
    virtual void bind(); //Should be called only by GDevice
    void unbind();

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferAlloc = VK_NULL_HANDLE;
    VmaAllocationInfo stagingBufferAllocInfo = {};


    VmaAllocation imageAllocation = VK_NULL_HANDLE;
    VmaAllocationInfo imageAllocationInfo = {};
protected:
    GDeviceVLK &m_device;

    bool m_uploaded = false;
    bool m_loaded = false;
    bool m_wrapX = true;
    bool m_wrapY = true;
};


#endif //AWEBWOWVIEWERCPP_GTEXTUREVLK_H
