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

    void *getPointerForModification() override;
    void *getPointerForUpload() override;

    void save(bool initialSave = false) override;
    void createBuffer() override;
private:

    void destroyBuffer();
    void unbind();

private:
    void uploadData(void * data, int length);

private:
    IDevice &m_device;

private:
    size_t m_size;

    std::vector<char> pFrameOneContent;
	
    bool m_buffCreated = false;
    bool m_dataUploaded = false;
};


#endif //AWEBWOWVIEWERCPP_GUNIFORMBUFFER_H
