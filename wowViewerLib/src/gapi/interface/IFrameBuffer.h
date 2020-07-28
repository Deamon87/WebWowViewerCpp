//
// Created by deamon on 30.01.20.
//

#ifndef AWEBWOWVIEWERCPP_IFRAMEBUFFER_H
#define AWEBWOWVIEWERCPP_IFRAMEBUFFER_H

#include "../interface/IDevice.h"

class IFrameBuffer {
public:
    virtual ~IFrameBuffer(){};
    virtual HGTexture getAttachment(int index) = 0;
    virtual HGTexture getDepthTexture() = 0;
    virtual void bindFrameBuffer() = 0;
    virtual void copyRenderBufferToTexture() = 0;
};

#endif //AWEBWOWVIEWERCPP_IFRAMEBUFFER_H
