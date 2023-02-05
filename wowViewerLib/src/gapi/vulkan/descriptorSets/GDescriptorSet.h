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

    explicit GDescriptorSet(const std::shared_ptr<GDeviceVLK> &device, std::shared_ptr<GDescriptorSetLayout> &hDescriptorSetLayout);
    ~GDescriptorSet();

    void update();
    void writeToDescriptorSets(std::vector<VkWriteDescriptorSet> &descriptorWrites);
    const std::shared_ptr<GDescriptorSetLayout> &getSetLayout() const { return m_hDescriptorSetLayout;};

    VkDescriptorSet getDescSet() {return m_descriptorSet;}

    struct DescriptorRecord {
        enum class DescriptorRecordType {
            None, UBO, UBODynamic, Texture
        };

        DescriptorRecordType descType = DescriptorRecordType::None;
        std::shared_ptr<IBufferVLK> buffer = nullptr;
        std::shared_ptr<GTextureVLK> textureVlk = nullptr;
    };

    class SetUpdateHelper {
    public:
        explicit SetUpdateHelper(GDescriptorSet &set, std::array<DescriptorRecord, 16> &boundDescriptors) :
            m_set(set), m_boundDescriptors(boundDescriptors) {}
        ~SetUpdateHelper();

        SetUpdateHelper& ubo(int bindIndex, const std::shared_ptr<IBufferVLK> &buffer);

        SetUpdateHelper& ubo_dynamic(int bindIndex, const std::shared_ptr<IBufferVLK> &buffer);

        SetUpdateHelper& ssbo(int bindIndex, const std::shared_ptr<IBufferVLK> &buffer);

        //TODO: add version of this array texture case (aka bindless)
        SetUpdateHelper& texture(int bindIndex, const std::shared_ptr<GTextureVLK> &textureVlk);

    private:
        GDescriptorSet &m_set;
        std::array<DescriptorRecord, MAX_BINDPOINT_NUMBER> &m_boundDescriptors;

        std::vector<VkDescriptorImageInfo> imageInfos;
        std::vector<VkDescriptorBufferInfo> bufferInfos;

        std::vector<VkWriteDescriptorSet> updates;
        std::bitset<MAX_BINDPOINT_NUMBER> m_updateBindPoints = 0;
    };

    SetUpdateHelper beginUpdate();

private:
    std::shared_ptr<GDeviceVLK> m_device;
    VkDescriptorSet m_descriptorSet;
    std::shared_ptr<GDescriptorPoolVLK> m_parentPool;

    std::shared_ptr<GDescriptorSetLayout> m_hDescriptorSetLayout;

    bool m_firstUpdate = true;

    std::array<DescriptorRecord, MAX_BINDPOINT_NUMBER> boundDescriptors;

// Scrapped idea. The VkCopyDescriptorSet can cause copy GPU -> CPU -> GPU. So it's scrapped idea
//    //Defines amount of frames that the updates has to be copied from previous frame
//    std::array<uint8_t, MAX_BINDPOINT_NUMBER> m_updatesLeft = {0};
//    std::bitset<MAX_BINDPOINT_NUMBER> m_updateBitSet = 0;
};


#endif //AWEBWOWVIEWERCPP_GDESCRIPTORSET_H
