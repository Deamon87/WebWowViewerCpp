//
// Created by Deamon on 27.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IUIMATERIAL_H
#define AWEBWOWVIEWERCPP_IUIMATERIAL_H

#include <memory>
#include "../../../../wowViewerLib/src/gapi/interface/textures/ITexture.h"
#include "../../../../wowViewerLib/src/gapi/interface/buffers/IBufferChunk.h"
#include "../../../../wowViewerLib/src/gapi/UniformBufferStructures.h"

struct UIMaterialTemplate {
    std::shared_ptr<IBufferChunk<ImgUI::modelWideBlockVS>> uiUBO = nullptr;
    std::shared_ptr<ITexture> texture = nullptr;
};

class IUIMaterial {
public:
    virtual ~IUIMaterial() = 0;
};

typedef std::shared_ptr<IUIMaterial> HUIMaterial;

#endif //AWEBWOWVIEWERCPP_IUIMATERIAL_H
