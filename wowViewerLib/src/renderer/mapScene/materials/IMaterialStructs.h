//
// Created by Deamon on 3/5/2023.
//

#ifndef AWEBWOWVIEWERCPP_IMATERIALSTRUCTS_H
#define AWEBWOWVIEWERCPP_IMATERIALSTRUCTS_H

#include "../../../gapi/interface/materials/IMaterial.h"
#include "../../../gapi/UniformBufferStructures.h"

struct M2MaterialTemplate {
    const HGTexture texture0 = nullptr;
    const HGTexture texture1 = nullptr;
    const HGTexture texture2 = nullptr;
};

class IM2ModelData {
public:
    std::shared_ptr<IBufferChunk<M2::PlacementMatrix>> m_placementMatrix = nullptr;
    std::shared_ptr<IBufferChunk<M2::Bones>> m_bonesData = nullptr;
    std::shared_ptr<IBufferChunk<M2::modelWideBlockPS>> m_modelFragmentData = nullptr;
};

class IM2Material : public IMaterial {
public:
    std::shared_ptr<IBufferChunk<M2::meshWideBlockVS>> m_vertexData = nullptr;
    std::shared_ptr<IBufferChunk<M2::meshWideBlockPS>> m_fragmentData = nullptr;
};

class ISkyMeshMaterial : public IMaterial {
public:
    std::shared_ptr<IBufferChunk<DnSky::meshWideBlockVS>> m_skyColors = nullptr;
};

#endif //AWEBWOWVIEWERCPP_IMATERIALSTRUCTS_H
