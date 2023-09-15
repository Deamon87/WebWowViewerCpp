//
// Created by Deamon on 7/2/2023.
//

#ifndef AWEBWOWVIEWERCPP_GBUFFERCHUNKDYNAMICVERSIONEDVLK_H
#define AWEBWOWVIEWERCPP_GBUFFERCHUNKDYNAMICVERSIONEDVLK_H


#include "../../interface/buffers/IBufferChunk.h"
#include "GBufferVLK.h"
#include "../../interface/buffers/IBufferVersioned.h"

template<typename T>
class GBufferChunkDynamicVersionedVLK : public IBufferVLK, public IBufferChunkVersioned<T> {
public:
    GBufferChunkDynamicVersionedVLK(const HGDeviceVLK &device, int versionAmount, const std::shared_ptr<GBufferVLK> &mainBuffer, int realSize = -1) : m_device(device) {
        m_realSize = realSize;

        if (m_realSize < 0)
            m_realSize = sizeof(T);

        subBufferVersions.resize(versionAmount);
        for (auto &subBuffers : subBufferVersions) {
            for (int i = 0; i < IDevice::MAX_FRAMES_IN_FLIGHT; i++) {
                auto subBuffer = mainBuffer->getSubBuffer(m_realSize, sizeof(T));
                subBuffers[i] = subBuffer;
            }
        }

        iteratorUnique = subBufferVersions[0][0]->addOnHandleChange([&]() -> void {
            this->executeOnChange();
        });
    }

    ~GBufferChunkDynamicVersionedVLK() final {
        subBufferVersions[0][0]->eraseOnHandleChange(iteratorUnique);
    }
    void uploadData(const void *, int length) override {
    };
    void *getPointer() override {
        auto index = m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
        return subBufferVersions[m_currentVersion][index]->getPointer();
    };
    void save(int length) override{
        auto index = m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
        subBufferVersions[m_currentVersion][index]->save(m_realSize);
    };
    VkBuffer getGPUBuffer() override {
        auto index = m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
        return subBufferVersions[m_currentVersion][index]->getGPUBuffer();
    }
    size_t getOffset() override {
        auto index = m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
        return subBufferVersions[m_currentVersion][index]->getOffset();
    }

    size_t getIndex() override {
        auto index = m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
        return subBufferVersions[m_currentVersion][index]->getIndex();
    }

    size_t getSize() override {
        return m_realSize;
    }
    void setCurrentVersion(int version) {
        if (version > subBufferVersions.size()) throw "wrong version";
        m_currentVersion = version;
    }

    T &getObject(int version) override {
        if (version > subBufferVersions.size()) throw "wrong version";
        auto index = m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
        return *(T*)subBufferVersions[version][index]->getPointer();
    };
    void saveVersion(int version) {
        auto index = m_device->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
        subBufferVersions[version][index]->save(m_realSize);
    }

private:
    HGDeviceVLK m_device;
    int m_realSize = 0;
    int m_currentVersion = 0;

    std::vector<std::array<std::shared_ptr<IBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT>> subBufferVersions;
    std::unique_ptr<DescriptorRecord::RecordInter> iteratorUnique = nullptr;
};


#endif //AWEBWOWVIEWERCPP_GBUFFERCHUNKDYNAMICVERSIONEDVLK_H
