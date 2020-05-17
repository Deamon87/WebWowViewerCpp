//
// Created by Deamon on 14.05.2020.
//

#ifndef AWEBWOWVIEWERCPP_GFRAMEBUFFERGL33_H
#define AWEBWOWVIEWERCPP_GFRAMEBUFFERGL33_H


#include <vector>
#include "../interface/IFrameBuffer.h"
#include "../interface/textures/ITexture.h"
#include "../interface/IDevice.h"

class GFrameBufferGL33 : public IFrameBuffer {
public:
    GFrameBufferGL33(std::vector<ITextureFormat> textureAttachment);



private:
    std::vector<HGTexture> texture;
};


#endif //AWEBWOWVIEWERCPP_GFRAMEBUFFERGL33_H
