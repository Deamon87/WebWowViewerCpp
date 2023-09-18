//
// Created by Deamon on 09.02.23.
//

#ifndef AWEBWOWVIEWERCPP_IDEVICEVULKAN_H
#define AWEBWOWVIEWERCPP_IDEVICEVULKAN_H

#include <functional>
#include <optional>
#include "descriptorSets/GDescriptorSetLayout.h"
#include "descriptorSets/GDescriptorPoolVLK.h"

#include "../interface/textures/ITexture.h"
#include "../interface/textures/ISamplableTexture.h"

class GDescriptorSetUpdater;

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> transferFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class IDeviceVulkan {
public:
    virtual ~IDeviceVulkan() = default;

    virtual VkDevice getVkDevice() = 0;
    virtual VkPhysicalDevice getVkPhysicalDevice() = 0;
    virtual void addDeallocationRecord(std::function<void()> callback) = 0;
    virtual VkDescriptorSet allocateDescriptorSetPrimitive(
        const std::shared_ptr<GDescriptorSetLayout> &hDescriptorSetLayout, std::shared_ptr<GDescriptorPoolVLK> &out_desciptorPool) = 0;

    virtual VmaAllocator getVMAAllocator() = 0;
    virtual VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) = 0;

    virtual HGSamplableTexture getWhiteTexturePixel() = 0;
    virtual HGSamplableTexture getBlackTexturePixel() = 0;

    virtual std::shared_ptr<GDescriptorSetUpdater> getDescriptorSetUpdater() = 0;

    //TODO:
    virtual bool getIsAnisFiltrationSupported() {return true;};
    //TODO:
    virtual bool getIsBCCompressedTexturesSupported() = 0;
    virtual float getAnisLevel() = 0;

    virtual const QueueFamilyIndices &getQueueFamilyIndices() = 0;

    virtual void setObjectName(uint64_t object, VkObjectType objectType, const char *name) = 0;
};

#endif //AWEBWOWVIEWERCPP_IDEVICEVULKAN_H
