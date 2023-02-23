//
// Created by Deamon on 27.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IUIMATERIAL_H
#define AWEBWOWVIEWERCPP_IUIMATERIAL_H

#include <memory>
#include "../../../../wowViewerLib/src/gapi/interface/textures/ITexture.h"
#include "../../../../wowViewerLib/src/gapi/interface/buffers/IBufferChunk.h"
#include "../../../../wowViewerLib/src/gapi/UniformBufferStructures.h"
#include "../../../../wowViewerLib/src/gapi/interface/materials/IMaterial.h"
#include "../../../../wowViewerLib/src/include/custom_container_key.h"


typedef std::unordered_map<wtf::KeyContainer<WGTexture>, std::weak_ptr<IMaterial>> UiMaterialCache;

#endif //AWEBWOWVIEWERCPP_IUIMATERIAL_H
