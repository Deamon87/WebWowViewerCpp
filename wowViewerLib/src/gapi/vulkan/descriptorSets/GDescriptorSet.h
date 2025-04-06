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
#include "../utils/MutexLockedVector.h"
#include "../../../engine/custom_allocators/FrameBasedStackAllocator.h"

class BindlessUpdateAccum {
public:
    struct BindlesUpdate {
        int bindIndex;
        int arrayIndex;
    };
    void addUpdate(int bindIndex, int arrayIndex) {
        std::unique_lock<std::mutex> m_lock(m_mutex);
        m_bindlessUpdates.emplace_back() = {bindIndex, arrayIndex};
    };
    MutexLockedVector<BindlesUpdate> getUpdates() {
        return MutexLockedVector<BindlesUpdate>(m_bindlessUpdates, m_mutex, true);
    }

private:
    std::mutex m_mutex;
    std::vector<BindlesUpdate> m_bindlessUpdates;
};

class GDescriptorSet : public std::enable_shared_from_this<GDescriptorSet> {
public:
    explicit GDescriptorSet(const std::shared_ptr<IDeviceVulkan> &device, const std::shared_ptr<GDescriptorSetLayout> &hDescriptorSetLayout);
    ~GDescriptorSet();

    void update();
    void writeToDescriptorSets(framebased::vector<VkWriteDescriptorSet> &descriptorWrites, framebased::vector<VkDescriptorImageInfo> &imageInfo, framebased::vector<int> &dynamicBufferIndexes);
    const std::shared_ptr<GDescriptorSetLayout> &getDescSetLayout() const { return m_hDescriptorSetLayout;};

    VkDescriptorSet getDescSet() const {return m_descriptorSet;}
    inline void getDynamicOffsets(uint32_t *dynamicOffsets, uint32_t& dynamicOffsetsSize) {
        const auto dynBufSize = m_dynamicBuffers.size();
        dynamicOffsetsSize = dynBufSize;
//
//        auto pDynamicOffsets = dynamicOffsets.data();
//
//        assert(dynamicOffsetsSize <= 16);
//        for (int i = 0; i < dynBufSize; i++) {
//
//            auto& descriptor = boundDescriptors[m_dynamicBufferIndexes[i]];
//            assert(descriptor->descType == DescriptorRecord::DescriptorRecordType::UBODynamic);
//
//            pDynamicOffsets[i] = (descriptor->buffer->getOffset());
//        }

//        auto pDynamicOffsets = dynamicOffsets.data();
        for (int i = 0; i < dynBufSize; i++) {
            dynamicOffsets[i] = m_dynamicBuffers[i]->getOffset();
        }
    }



    class SetUpdateHelper {
    public:
        explicit SetUpdateHelper(GDescriptorSet &set, std::array<std::vector<std::unique_ptr<DescriptorRecord>>, GDescriptorSetLayout::MAX_BINDPOINT_NUMBER> &boundDescriptors,
                                 std::shared_ptr<GDescriptorSetUpdater> descriptorSetUpdater) :
            m_set(set), m_boundDescriptors(boundDescriptors), m_descriptorSetUpdater(descriptorSetUpdater) {
            //So that the resize of these vectors would not lead to pointer invalidation in updates vector
            bufferInfos.reserve(32);
            imageInfos.reserve(4096);
            updates.reserve(64);
        }
        ~SetUpdateHelper();
        // SetUpdateHelper is non-copyable
        SetUpdateHelper(const SetUpdateHelper&) = delete;
        SetUpdateHelper& operator=(const SetUpdateHelper&) = delete;

        SetUpdateHelper& ubo(int bindIndex, const std::shared_ptr<IBufferVLK> &buffer);

        SetUpdateHelper& ubo_dynamic(int bindIndex, const std::shared_ptr<IBufferVLK> &buffer);

        SetUpdateHelper& ssbo(int bindIndex, const std::shared_ptr<IBufferVLK> &buffer);

        //TODO: add version of this array texture case (aka bindless)
        SetUpdateHelper& texture(int bindIndex, const HGSamplableTexture &textureVlk, int index = 0);
        void delayUpdate();

        template <typename T>
        void assignBoundDescriptors(int bindPoint, const std::shared_ptr<T> &object, int index, DescriptorRecord::DescriptorRecordType descType) {
//            static_assert(std::is_base_of<IDSBindable, T>::value, "T should inherit from B");

            assert(index < m_boundDescriptors[bindPoint].size());

            bool createDescRecord = !m_boundDescriptors[bindPoint][index];
            if (!createDescRecord) {
                if constexpr (std::is_base_of<IBufferVLK, T>::value) {
                    createDescRecord = (m_boundDescriptors[bindPoint][index]->buffer != object);
                } else if constexpr (std::is_base_of<HGSamplableTexture::element_type, T>::value) {
                    createDescRecord = (m_boundDescriptors[bindPoint][index]->texture != object);
                }
            }
            if (createDescRecord) {
                std::function<void()> callback = createCallback(bindPoint, index);
                m_boundDescriptors[bindPoint][index] = std::make_unique<DescriptorRecord>(descType, object, callback);
            }
        }
        framebased::unordered_map<int, VkDescriptorType> &getAccumulatedTypeOverrides() {
            return typeOverrides;
        }
    private:
        bool updateCancelled = false;
        GDescriptorSet &m_set;
        std::array<std::vector<std::unique_ptr<DescriptorRecord>>, GDescriptorSetLayout::MAX_BINDPOINT_NUMBER> &m_boundDescriptors;

        framebased::vector<VkDescriptorImageInfo> imageInfos;
        framebased::vector<VkDescriptorBufferInfo> bufferInfos;
        framebased::vector<int> dynamicBufferIndexes;
        framebased::unordered_map<int, VkDescriptorType> typeOverrides;

        std::shared_ptr<GDescriptorSetUpdater> m_descriptorSetUpdater;

        framebased::vector<VkWriteDescriptorSet> updates;
        std::bitset<GDescriptorSetLayout::MAX_BINDPOINT_NUMBER> m_updateBindPoints = 0;

        std::function<void()> createCallback(int bindPoint, int arrayIndex);

        void reassignBinding(int bindPoint, int bindIndex);
    };

    SetUpdateHelper beginUpdate();

private:
    std::shared_ptr<IDeviceVulkan> m_device;
    VkDescriptorSet m_descriptorSet;
    std::shared_ptr<GDescriptorPoolVLK> m_parentPool;

    const std::shared_ptr<GDescriptorSetLayout> m_hDescriptorSetLayout;
    std::vector<int> m_dynamicBufferIndexes;
    std::vector<IBufferVLK *> m_dynamicBuffers;

    bool m_firstUpdate = true;

    std::unique_ptr<BindlessUpdateAccum> m_bindlessAccum = nullptr;

    std::array<std::vector<std::unique_ptr<DescriptorRecord>>, GDescriptorSetLayout::MAX_BINDPOINT_NUMBER> boundDescriptors;

// Scrapped idea. The VkCopyDescriptorSet can cause copy GPU -> CPU -> GPU. So it's scrapped idea
//    //Defines amount of frames that the updates has to be copied from previous frame
//    std::array<uint8_t, MAX_BINDPOINT_NUMBER> m_updatesLeft = {0};
//    std::bitset<MAX_BINDPOINT_NUMBER> m_updateBitSet = 0;
};


#endif //AWEBWOWVIEWERCPP_GDESCRIPTORSET_H
