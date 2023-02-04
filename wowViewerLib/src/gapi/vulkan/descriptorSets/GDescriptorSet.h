//
// Created by Deamon on 10/2/2019.
//

#ifndef AWEBWOWVIEWERCPP_GDESCRIPTORSET_H
#define AWEBWOWVIEWERCPP_GDESCRIPTORSET_H

class GDeviceVLK;
class GDescriptorPoolVLK;

#include <vector>
#include <bitset>
#include "../../interface/IDevice.h"
#include "../GDeviceVulkan.h"
#include "../buffers/IBufferVLK.h"
#include "GDescriptorSetLayout.h"


class GDescriptorSet {
public:
    static const constexpr int MAX_BINDPOINT_NUMBER = 16;

    explicit GDescriptorSet(IDevice &device, std::shared_ptr<GDescriptorSetLayout> &hDescriptorSetLayout, VkDescriptorSet descriptorSet, std::shared_ptr<GDescriptorPoolVLK> parentPool);
    ~GDescriptorSet();

    void update();
    void writeToDescriptorSets(std::vector<VkWriteDescriptorSet> &descriptorWrites);
    const std::shared_ptr<GDescriptorSetLayout> getSetLayout() const { return m_hDescriptorSetLayout;};

    VkDescriptorSet getDescSet() {return m_descriptorSet;}

    class SetUpdateHelper {
    public:
        SetUpdateHelper(GDescriptorSet &set) : m_set(set) {}

        SetUpdateHelper& ubo(int bindIndex, const std::shared_ptr<IBufferVLK> &buffer);

        SetUpdateHelper& ubo_dynamic(int bindIndex, const std::shared_ptr<IBufferVLK> &buffer);

        SetUpdateHelper& ssbo(int bindIndex, const std::shared_ptr<IBufferVLK> &buffer);

        //TODO: add version of this array texture case (aka bindless)
        SetUpdateHelper& texture(int bindIndex, const std::shared_ptr<GTextureVLK> &textureVlk);

        std::bitset<MAX_BINDPOINT_NUMBER> m_updateBitSet = 0;

    private:
        GDescriptorSet &m_set;
        std::vector<VkDescriptorImageInfo> imageInfos;
        std::vector<VkDescriptorBufferInfo> bufferInfos;

        std::vector<VkWriteDescriptorSet> updates;
    };

    SetUpdateHelper beginUpdate() {
        return SetUpdateHelper(*this);
    };

private:
    GDeviceVLK &m_device;
    VkDescriptorSet m_descriptorSet;
    std::shared_ptr<GDescriptorPoolVLK> m_parentPool;

    std::shared_ptr<GDescriptorSetLayout> m_hDescriptorSetLayout;

// Scrapped idea. The VkCopyDescriptorSet can cause copy GPU -> CPU -> GPU. So it's scrapped idea
//    //Defines amount of frames that the updates has to be copied from previous frame
//    std::array<uint8_t, MAX_BINDPOINT_NUMBER> m_updatesLeft = {0};
//    std::bitset<MAX_BINDPOINT_NUMBER> m_updateBitSet = 0;
};


#endif //AWEBWOWVIEWERCPP_GDESCRIPTORSET_H
