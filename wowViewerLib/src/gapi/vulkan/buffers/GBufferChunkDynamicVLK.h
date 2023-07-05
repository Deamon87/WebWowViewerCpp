//
// Created by Deamon on 7/2/2023.
//

#ifndef AWEBWOWVIEWERCPP_GBUFFERCHUNKDYNAMICVLK_H
#define AWEBWOWVIEWERCPP_GBUFFERCHUNKDYNAMICVLK_H


#include "../../interface/buffers/IBufferChunk.h"
#include "GBufferVLK.h"

template<typename T>
class GBufferChunkDynamicVLK : public IBufferChunk<T>, public IBufferVLK {
public:
    GBufferChunkDynamicVLK(const HGDeviceVLK &device, const std::shared_ptr<GBufferVLK> &mainBuffer, int realSize = -1) : m_device(device) {
        m_realSize = realSize;

        if (m_realSize < 0)
            m_realSize = sizeof(T);

        for (int i = 0; i < IDevice::MAX_FRAMES_IN_FLIGHT; i++) {
            auto subBuffer = mainBuffer->getSubBuffer(m_realSize, sizeof(T));
            subBuffers[i] = subBuffer;
            pSubBuffers[i] = subBuffer.get();
        }

        iteratorUnique = pSubBuffers[0]->addOnHandleChange([&]() -> void {
            this->executeOnChange();
        });
    }

    ~GBufferChunkDynamicVLK() final {
        pSubBuffers[0]->eraseOnHandleChange(iteratorUnique);
    }
    void uploadData(const void *, int length) override {

    };
    void *getPointer() override {
        auto index = m_device->getUpdateFrameNumber();
        return pSubBuffers[index]->getPointer();
    };
    void save(int length) override{
        auto index = m_device->getUpdateFrameNumber();
        pSubBuffers[index]->save(m_realSize);
    };
    VkBuffer getGPUBuffer() override {
        auto index = m_device->getUpdateFrameNumber();
        return pSubBuffers[index]->getGPUBuffer();
    }
    size_t getOffset() override {
        auto index = m_device->getUpdateFrameNumber();
        return pSubBuffers[index]->getOffset();
    }

    size_t getSize() override {
        return m_realSize;
    }
    T &getObject() override {
        auto index = m_device->getUpdateFrameNumber();
        return *(T*)pSubBuffers[index]->getPointer();
    };
    void save() override {
        auto index = m_device->getUpdateFrameNumber();
        pSubBuffers[index]->save(m_realSize);
    };

private:
    HGDeviceVLK m_device;
    int m_realSize = 0;
    void *ptr = nullptr;
    std::array<std::shared_ptr<IBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> subBuffers = {nullptr};
    std::array<IBufferVLK *, IDevice::MAX_FRAMES_IN_FLIGHT> pSubBuffers  = {nullptr};

    std::unique_ptr<DescriptorRecord::RecordInter> iteratorUnique = nullptr;
};

namespace DynamicBufferChunkHelperVLK {
    template<typename T>
    static const inline std::shared_ptr<IBufferVLK> cast(const std::shared_ptr<IBufferChunk<T>> &chunk) {
        return std::dynamic_pointer_cast<GBufferChunkDynamicVLK<T>>(chunk);
    }

    template<typename T>
    static const inline void create(const HGDeviceVLK &device, const HGBufferVLK &parentBuffer, std::shared_ptr<IBufferChunk<T>> &chunk, int realSize = -1) {
        chunk = std::make_shared<GBufferChunkDynamicVLK<T>>(device, parentBuffer, realSize);
    }
};


#endif //AWEBWOWVIEWERCPP_GBUFFERCHUNKDYNAMICVLK_H
