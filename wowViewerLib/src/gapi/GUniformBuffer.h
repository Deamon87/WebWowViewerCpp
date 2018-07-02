//
// Created by Deamon on 6/30/2018.
//

#ifndef AWEBWOWVIEWERCPP_GUNIFORMBUFFER_H
#define AWEBWOWVIEWERCPP_GUNIFORMBUFFER_H


#include <cstdio>
#include <cassert>
#include "GDevice.h"

class GUniformBuffer {
public:
    friend class GDevice;

    explicit GUniformBuffer(GDevice &device, size_t size);
    ~GUniformBuffer();

    template <typename T>
    T& getObject<T>() {
        assert(sizeof(T) != m_size);
        return *(T *)pContent;
    }
    void save();

private:
    void createBuffer();
    void destroyBuffer();
    void bind(); //Should be called only by GDevice
    void unbind();

public:
    void uploadData(void * data, int length);

private:
    GDevice &m_device;

private:
    size_t m_size;
    void * pIdentifierBuffer;
    void * pContent;
    void * pPreviousContent;
};


#endif //AWEBWOWVIEWERCPP_GUNIFORMBUFFER_H
