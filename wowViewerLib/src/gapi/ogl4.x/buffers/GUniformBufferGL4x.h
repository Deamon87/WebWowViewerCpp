//
// Created by Deamon on 6/30/2018.
//

#ifndef AWEBWOWVIEWERCPP_GUNIFORMBUFFER_4X_H
#define AWEBWOWVIEWERCPP_GUNIFORMBUFFER_4X_H


#include <cstdio>
#include <cassert>
#include "../GDeviceGL4x.h"
#include "../../interface/buffers/IUniformBuffer.h"

class GUniformBufferGL4x : public IUniformBuffer {
public:
    friend class GDeviceGL4x;

    explicit GUniformBufferGL4x(IDevice &device, size_t size);
    ~GUniformBufferGL4x() override;

    void setIdentifierBuffer(void * ptr) {
        pIdentifierBuffer[(m_device.getFrameNumber() + 1) & 1] = ptr;
    }
    void * getIdentifierBuffer() {
        if (m_buffCreated) {
            return pIdentifierBuffer[0];
        }

        return pIdentifierBuffer[m_device.getFrameNumber() & 1];
    }

    void *getPointerForModification() override;
    void *getPointerForUpload() override;

    void save(bool initialSave = false) override;
    void createBuffer() override;
private:

    void destroyBuffer();
    void bind(int bindingPoint); //Should be called only by GDevice
    void unbind();

    void * getIdentifierBufferForRendering() {
        if (m_buffCreated) {
            return pIdentifierBuffer[0];
        }

        return pIdentifierBuffer[(m_device.getFrameNumber() + 1) & 1];
    }

private:
    void uploadData(void * data, int length);

private:
    IDevice &m_device;

private:
    size_t m_size;
    size_t m_offset = 0;
    void * pIdentifierBuffer[2];

    void * pFrameOneContent;
    void * pFrameTwoContent;
    bool m_buffCreated = false;
    bool m_dataUploaded = false;

    void *m_perisistentPointer = nullptr;

    int m_creationIndex = 0;

    bool m_needsUpdate = false;
};


#endif //AWEBWOWVIEWERCPP_GUNIFORMBUFFER_4X_H
