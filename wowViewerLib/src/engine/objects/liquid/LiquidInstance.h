//
// Created by Deamon on 5/20/2023.
//

#ifndef AWEBWOWVIEWERCPP_LIQUIDINSTANCE_H
#define AWEBWOWVIEWERCPP_LIQUIDINSTANCE_H

class IMapApi;

#include "../../ApiContainer.h"

class LiquidInstance {
public:
    LiquidInstance(const HApiContainer &api,
                   const HMapSceneBufferCreate &sceneRenderer,
                   const SMLiquidInstance &liquidInstance,
                   const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &waterPlacementChunk,
                   const mathfu::vec3 &liquidBasePos,
                   const PointerChecker<char> &mH2OBlob, CAaBox &waterBBox);

    LiquidInstance(const HApiContainer &api,
                   const HMapSceneBufferCreate &sceneRenderer,
                   const HGVertexBufferBindings &binding,
                   int liquidType,
                   int indexBufferSize,
                   const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &waterPlacementChunk,
                   CAaBox &waterAaBB);

    void updateLiquidMaterials(const HFrameDependantData &frameDependantData, animTime_t mapCurrentTime);
    void collectMeshes(std::vector<HGSortableMesh> &transparentMeshes);
private:
    const HApiContainer &m_api;
    const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> m_waterPlacementChunk;
    std::vector<std::shared_ptr<IWaterMaterial>> m_liquidMaterials;
    std::vector<HGVertexBufferBindings> m_vertexWaterBufferBindings;
    std::vector<HGSortableMesh> m_liquidMeshes;
    CAaBox &m_waterBBox;

    uint16_t liquid_object;
    uint32_t liquidType;

    LiquidTypeAndMat m_liqMatAndType;
    std::vector<LiquidTextureData> m_liquidTextureData;

    void createAdtVertexData(const SMLiquidInstance &liquidInstance, const mathfu::vec3 &liquidBasePos,
                             const PointerChecker<char> &mH2OBlob, CAaBox &waterAaBB, int liquidVertexFormat,
                             bool generateTexCoordsFromPos, std::vector<LiquidVertexFormat> &vertexBuffer,
                             std::vector<uint16_t> &indexBuffer) const;

    HGVertexBufferBindings
    createLiquidVao(const HMapSceneBufferCreate &sceneRenderer, std::vector<LiquidVertexFormat> &vertexBuffer,
                    std::vector<uint16_t> &indexBuffer) const;

    void
    getInfoFromDatabase(int liquid_object_or_lvf, int liquid_type, bool &generateTexCoordsFromPos);

    void
    createMaterialAndMesh(const HMapSceneBufferCreate &sceneRenderer,
                          int indexBufferSize,
                          const HGVertexBufferBindings &vertexWaterBufferBindings);

    mathfu::mat4 GetTexScrollMtx(int time, mathfu::vec2 scrollVec);
};


#endif //AWEBWOWVIEWERCPP_LIQUIDINSTANCE_H
