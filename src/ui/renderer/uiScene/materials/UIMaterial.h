//
// Created by Deamon on 27.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IUIMATERIAL_H
#define AWEBWOWVIEWERCPP_IUIMATERIAL_H

#include <memory>
#include "../../../../wowViewerLib/src/gapi/interface/textures/ITexture.h"
#include "../../../../wowViewerLib/src/gapi/interface/buffers/IBufferChunk.h"
#include "../../../../wowViewerLib/src/gapi/UniformBufferStructures.h"
#include "../../../../../wowViewerLib/src/gapi/interface/materials/IMaterial.h"

struct UIMaterialTemplate {
    HGTexture texture;

    bool operator==(const UIMaterialTemplate &other) const {
        return (texture == other.texture);
    };
};

//-------------------------------------------------------
//              HASHer and CACHE CLASS
//-------------------------------------------------------

struct UIMaterialHasher {
    std::size_t operator()(const UIMaterialTemplate& k) const {
        using std::hash;
        return hash<void*>{}(k.texture.get());
    };
};
typedef std::unordered_map<UIMaterialTemplate, std::weak_ptr<IMaterial>, UIMaterialHasher> UiMaterialCache;

#endif //AWEBWOWVIEWERCPP_IUIMATERIAL_H
