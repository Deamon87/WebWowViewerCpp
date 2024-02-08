//
// Created by Deamon on 25.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IFRONTENDUIBUFFERCREATE_H
#define AWEBWOWVIEWERCPP_IFRONTENDUIBUFFERCREATE_H

#include "../../../../wowViewerLib/src/gapi/interface/IDevice.h"
#include "../../../../wowViewerLib/src/gapi/interface/IRendererProxy.h"
#include "materials/UIMaterial.h"
#include "../../../../wowViewerLib/src/gapi/interface/materials/IMaterial.h"

class IFrontendUIBufferCreate {
public:
    virtual HGVertexBuffer createVertexBuffer(int sizeInBytes) = 0;
    virtual HGIndexBuffer  createIndexBuffer(int sizeInBytes) = 0;

    virtual HGVertexBufferBindings createVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) = 0;

    virtual HGMesh createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) = 0;
    virtual std::shared_ptr<IUIMaterial> createUIMaterial(const HGSamplableTexture &hgtexture, bool opaque = false) = 0;
    virtual std::shared_ptr<IUIMaterial> createUIMaterialDepth(const HGSamplableTexture &hgtexture) = 0;
};
typedef std::shared_ptr<IFrontendUIBufferCreate> HFrontendUIBufferCreate;

#endif //AWEBWOWVIEWERCPP_IFRONTENDUIBUFFERCREATE_H
