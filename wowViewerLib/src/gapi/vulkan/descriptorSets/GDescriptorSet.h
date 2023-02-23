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
#include "../IDeviceVulkan.h"
#include "../buffers/IBufferVLK.h"
#include "../textures/GTextureVLK.h"
#include "GDescriptorSetLayout.h"
#include "DescriptorRecord.h"
#include "../bindable/DSBindable.h"


class GDescriptorSet : public std::enable_shared_from_this<GDescriptorSet> {
public:
    explicit GDescriptorSet(const std::shared_ptr<IDeviceVulkan> &device, const std::shared_ptr<GDescriptorSetLayout> &hDescriptorSetLayout);
    ~GDescriptorSet();

    void update();
    void writeToDescriptorSets(std::vector<VkWriteDescriptorSet> &descriptorWrites);
    const std::shared_ptr<GDescriptorSetLayout> &getDescSetLayout() const { return m_hDescriptorSetLayout;};

    VkDescriptorSet getDescSet() const {return m_descriptorSet;}



    class SetUpdateHelper {
    public:
        explicit SetUpdateHelper(GDescriptorSet &set, std::array<std::unique_ptr<DescriptorRecord>, GDescriptorSetLayout::MAX_BINDPOINT_NUMBER> &boundDescriptors) :
            m_set(set), m_boundDescriptors(boundDescriptors) {}
        ~SetUpdateHelper();

        SetUpdateHelper& ubo(int bindIndex, const std::shared_ptr<IBufferVLK> &buffer);

        SetUpdateHelper& ubo_dynamic(int bindIndex, const std::shared_ptr<IBufferVLK> &buffer);

        SetUpdateHelper& ssbo(int bindIndex, const std::shared_ptr<IBufferVLK> &buffer);

        //TODO: add version of this array texture case (aka bindless)
        SetUpdateHelper& texture(int bindIndex, const std::shared_ptr<GTextureVLK> &textureVlk);

        template <typename T>
        void assignBoundDescriptors(int bindPoint, const std::shared_ptr<T> &object, DescriptorRecord::DescriptorRecordType descType) {
            static_assert(std::is_base_of<IDSBindable, T>::value, "T should inherit from B");

            bool createDescRecord = !m_boundDescriptors[bindPoint];
            if (!createDescRecord) {
                if constexpr (std::is_base_of<IBufferVLK, T>::value) {
                    createDescRecord = (m_boundDescriptors[bindPoint]->buffer != object);
                } else if constexpr (std::is_base_of<GTextureVLK, T>::value) {
                    createDescRecord = (m_boundDescriptors[bindPoint]->textureVlk != object);
                }
            }
            if (createDescRecord) {
                auto ds_weak = m_set.weak_from_this();

                auto callback = ([ds_weak]() -> void {
                    if (auto ds = ds_weak.lock()) {
                        ds->update();
                    }
                    });

                m_boundDescriptors[bindPoint] = std::make_unique<DescriptorRecord>(descType, object, callback);
            }
        }
    private:
        GDescriptorSet &m_set;
        std::array<std::unique_ptr<DescriptorRecord>, GDescriptorSetLayout::MAX_BINDPOINT_NUMBER> &m_boundDescriptors;

        std::vector<VkDescriptorImageInfo> imageInfos;
        std::vector<VkDescriptorBufferInfo> bufferInfos;

        std::vector<VkWriteDescriptorSet> updates;
        std::bitset<GDescriptorSetLayout::MAX_BINDPOINT_NUMBER> m_updateBindPoints = 0;
    };

    SetUpdateHelper beginUpdate();

private:
    std::shared_ptr<IDeviceVulkan> m_device;
    VkDescriptorSet m_descriptorSet;
    std::shared_ptr<GDescriptorPoolVLK> m_parentPool;

    const std::shared_ptr<GDescriptorSetLayout> m_hDescriptorSetLayout;

    bool m_firstUpdate = true;

    std::array<std::unique_ptr<DescriptorRecord>, GDescriptorSetLayout::MAX_BINDPOINT_NUMBER> boundDescriptors;

// Scrapped idea. The VkCopyDescriptorSet can cause copy GPU -> CPU -> GPU. So it's scrapped idea
//    //Defines amount of frames that the updates has to be copied from previous frame
//    std::array<uint8_t, MAX_BINDPOINT_NUMBER> m_updatesLeft = {0};
//    std::bitset<MAX_BINDPOINT_NUMBER> m_updateBitSet = 0;
};


#endif //AWEBWOWVIEWERCPP_GDESCRIPTORSET_H
