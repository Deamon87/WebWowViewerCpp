//
// Created by Deamon on 5/20/2023.
//

#ifndef AWEBWOWVIEWERCPP_LIQUIDINSTANCE_H
#define AWEBWOWVIEWERCPP_LIQUIDINSTANCE_H


#include "../../ApiContainer.h"
#include "../iMapApi.h"

class LiquidInstance {
public:
    LiquidInstance(const HApiContainer &api,
                   const HMapSceneBufferCreate &sceneRenderer,
                   const SMLiquidInstance &liquidInstance,
                   std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &waterPlacementChunk,
                   const mathfu::vec3 &liquidBasePos,
                   const PointerChecker<char> &mH2OBlob, CAaBox &waterBBox);

    void updateLiquidMaterials(const HFrameDependantData &frameDependantData, IMapApi *mapApi);
    void collectMeshes(std::vector<HGSortableMesh> &transparentMeshes);
private:
    const HApiContainer &m_api;
    const HMapSceneBufferCreate &m_sceneRenderer;
    const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> m_waterPlacementChunk;
    std::vector<std::shared_ptr<IWaterMaterial>> m_liquidMaterials;
    std::vector<HGSortableMesh> m_liquidMeshes;
    CAaBox &m_waterBBox;

    uint16_t liquid_object_or_lvf;

    void createAdtVertexData(const SMLiquidInstance &liquidInstance, const mathfu::vec3 &liquidBasePos,
                             const PointerChecker<char> &mH2OBlob, CAaBox &waterAaBB, int liquidVertexFormat,
                             bool generateTexCoordsFromPos, std::vector<LiquidVertexFormat> &vertexBuffer,
                             std::vector<uint16_t> &indexBuffer) const;

    HGVertexBufferBindings
    createLiquidVao(const HMapSceneBufferCreate &sceneRenderer, std::vector<LiquidVertexFormat> &vertexBuffer,
                    std::vector<uint16_t> &indexBuffer) const;

    void
    getInfoFromDatabase(const SMLiquidInstance &liquidInstance, int &basetextureFDID, mathfu::vec3 &color,
                        int &liquidFlags,
                        int &liquidVertexFormat, bool &generateTexCoordsFromPos);
};


#endif //AWEBWOWVIEWERCPP_LIQUIDINSTANCE_H
