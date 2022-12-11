//
// Created by deamon on 13.12.19.
//

#ifndef AWEBWOWVIEWERCPP_GUNFORMBUFFERCHUNK33_H
#define AWEBWOWVIEWERCPP_GUNFORMBUFFERCHUNK33_H

#include "../../interface/buffers/IUniformBufferChunk.h"
#include "../../interface/IDevice.h"

class GUniformBufferChunk33 : public IUniformBufferChunk {
private:
    HGUniformBuffer m_uniformBuffer = nullptr;


public:
    GUniformBufferChunk33(size_t size, size_t realSize = -1) : IUniformBufferChunk(size, realSize) {}

    void setUniformBuffer(HGUniformBuffer uniformBuffer) {
        m_uniformBuffer = uniformBuffer;
    }
    HGUniformBuffer getUniformBuffer() {
        return m_uniformBuffer;
    }
};

#endif //AWEBWOWVIEWERCPP_GUNFORMBUFFERCHUNK33_H
