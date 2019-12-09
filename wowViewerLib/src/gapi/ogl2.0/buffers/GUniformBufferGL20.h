//
// Created by Deamon on 6/30/2018.
//

#ifndef AWEBWOWVIEWERCPP_GUNIFORMBUFFER20_H
#define AWEBWOWVIEWERCPP_GUNIFORMBUFFER20_H


#include <cstdio>
#include <cassert>
#include "../GDeviceGL20.h"
#include "../../interface/buffers/IUniformBuffer.h"

class GUniformBufferGL20 : public IUniformBuffer {
public:
    friend class GDeviceGL20;

    explicit GUniformBufferGL20(IDevice &device, size_t size);
    ~GUniformBufferGL20() override;

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
    void bind(int bindingPoint); //Should be called only by GDevice
    void unbind();

private:
    void uploadData(void * data, int length);
    void setUpdateHandler(std::function<void(IUniformBuffer* self)>) override;
    void update() override;

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
