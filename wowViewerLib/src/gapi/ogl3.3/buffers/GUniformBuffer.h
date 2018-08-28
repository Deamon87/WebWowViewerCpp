//
// Created by Deamon on 6/30/2018.
//

#ifndef AWEBWOWVIEWERCPP_GUNIFORMBUFFER_H
#define AWEBWOWVIEWERCPP_GUNIFORMBUFFER_H


#include <cstdio>
#include <cassert>
#include "../GDevice.h"
#include "../../interface/buffers/IUniformBuffer.h"

class GUniformBuffer : public IUniformBuffer {
public:
    friend class GDevice;

    explicit GUniformBuffer(GDevice &device, size_t size);
    ~GUniformBuffer();

    template<typename T>
    T &getObject() {
//        assert(sizeof(T) == m_size);
        return *(T *) getPointerForModification();
    }
    void *getPointerForModification();
    void *getPointerForUpload();

    void save(bool initialSave = false);
    void createBuffer();
private:

    void destroyBuffer();
    void bind(int bindingPoint); //Should be called only by GDevice
    void unbind();

private:
    void uploadData(void * data, int length);

private:
    GDevice &m_device;

private:
    size_t m_size;
    size_t m_offset = 0;
    void * pIdentifierBuffer;

    void * pFrameOneContent;
    void * pFrameTwoContent;
    bool m_buffCreated = false;
    bool m_dataUploaded = false;

    int m_creationIndex = 0;

    bool m_needsUpdate = false;
};


#endif //AWEBWOWVIEWERCPP_GUNIFORMBUFFER_H
