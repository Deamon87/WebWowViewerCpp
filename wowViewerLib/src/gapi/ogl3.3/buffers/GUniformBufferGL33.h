//
// Created by Deamon on 6/30/2018.
//

#ifndef AWEBWOWVIEWERCPP_GUNIFORMBUFFER_H
#define AWEBWOWVIEWERCPP_GUNIFORMBUFFER_H


#include <cstdio>
#include <cassert>
#include "../GDeviceGL33.h"
#include "../../interface/buffers/IUniformBuffer.h"

class GUniformBufferGL33 : public IUniformBuffer {
public:
    friend class GDeviceGL33;

    explicit GUniformBufferGL33(IDevice &device, size_t size);
    ~GUniformBufferGL33() override;

    void setIdentifierBuffer(GLuint glBuffIdFuture, uint32_t offset) {
        uint8_t frameIndex = static_cast<uint8_t>(m_device.getUpdateFrameNumber());
        glBuffId[frameIndex] = glBuffIdFuture;
        m_offset[frameIndex] = offset;
    }
    GLuint getIdentifierBuffer() {
        if (m_buffCreated) {
            return glBuffId[0];
        }

        uint8_t frameIndex = static_cast<uint8_t>(m_device.getDrawFrameNumber());
        return glBuffId[frameIndex];
//        return glBuffId[0];
    }

    void *getPointerForModification() override;
    void *getPointerForUpload() override;

    void save(bool initialSave = false) override;
    void createBuffer() override;
private:

    void destroyBuffer();
    void bind(int bindingPoint, int slot, int offset, int length); //Should be called only by GDevice
    void unbind();

private:
    void uploadData(void * data, int length);

private:
    IDevice &m_device;

private:
    size_t m_size;
    size_t m_offset[4] = {0, 0, 0, 0};
    GLuint glBuffId[4] = {0, 0, 0, 0};

    std::vector<char> pFrameOneContent;
	
    bool m_buffCreated = false;
    bool m_dataUploaded = false;

    int m_creationIndex = 0;

    bool m_needsUpdate = false;

    std::function<void(IUniformBuffer* self)> m_handler;
};


#endif //AWEBWOWVIEWERCPP_GUNIFORMBUFFER_H
