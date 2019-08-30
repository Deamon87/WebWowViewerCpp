//
// Created by Deamon on 6/30/2018.
//

#ifndef AWEBWOWVIEWERCPP_GUNIFORMBUFFER_H
#define AWEBWOWVIEWERCPP_GUNIFORMBUFFER_H


#include <cstdio>
#include <cassert>
#include "../GDeviceVulkan.h"
#include "../../interface/buffers/IUniformBuffer.h"

class GUniformBufferVLK : public IUniformBuffer {
public:
    friend class GDeviceVLK;

    explicit GUniformBufferVLK(IDevice &device, size_t size);
    ~GUniformBufferVLK() override;

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

private:
    IDevice &m_device;

private:
    size_t m_size;
    size_t m_offset[4] = {0, 0, 0, 0};

    void * pFrameOneContent;
    bool m_buffCreated = false;
    bool m_dataUploaded = false;

    int m_creationIndex = 0;

    bool m_needsUpdate = false;
};


#endif //AWEBWOWVIEWERCPP_GUNIFORMBUFFER_H
