//
// Created by Deamon on 22.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IMAPSCENEBUFFERCREATE_H
#define AWEBWOWVIEWERCPP_IMAPSCENEBUFFERCREATE_H

#include <memory>
#include "../../gapi/interface/IDevice.h"

class IMapSceneBufferCreate {
public:
    virtual HGVertexBuffer createM2VertexBuffer(int sizeInBytes) = 0;
    virtual HGIndexBuffer  createM2IndexBuffer(int sizeInBytes) = 0;

    virtual HGVertexBuffer createADTVertexBuffer(int sizeInBytes) = 0;
    virtual HGIndexBuffer  createADTIndexBuffer(int sizeInBytes) = 0;

    virtual HGVertexBuffer createWMOVertexBuffer(int sizeInBytes) = 0;
    virtual HGIndexBuffer  createWMOIndexBuffer(int sizeInBytes) = 0;

    virtual HGVertexBuffer createWaterVertexBuffer(int sizeInBytes) = 0;
    virtual HGIndexBuffer  createWaterIndexBuffer(int sizeInBytes) = 0;
};
typedef std::shared_ptr<IMapSceneBufferCreate> HMapSceneBufferCreate;

#endif //AWEBWOWVIEWERCPP_IMAPSCENEBUFFERCREATE_H
