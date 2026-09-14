//
// Created by Deamon on 5/20/2023.
//

#ifndef AWEBWOWVIEWERCPP_LIQUIDINSTANCE_H
#define AWEBWOWVIEWERCPP_LIQUIDINSTANCE_H

class IMapApi;

#include "../../ApiContainer.h"
#include "../../custom_allocators/FrameBasedStackAllocator.h"
#include "../scenes/EntityActorsFactory.h"

class LiquidInstance;

enum class LiquidInstId : uintptr_t;

typedef EntityFactory<1000, LiquidInstId, LiquidInstance> LiquidInstanceEntityFactory;
extern std::shared_ptr<LiquidInstanceEntityFactory> liquidInstanceFactory;

class LiquidInstance : public ObjectWithId<LiquidInstId> {
public:
    LiquidInstance(const HApiContainer &api,
                   const HMapSceneBufferCreate &sceneRenderer,
                   const std::shared_ptr<ILiquidMaterial> &liquidMaterial,
                   const SMLiquidInstance &liquidInstance,
                   const mathfu::vec3 &liquidBasePos,
                   const PointerChecker<char> &mH2OBlob, CAaBox &waterBBox);

    LiquidInstance(const HApiContainer &api,
                   const HMapSceneBufferCreate &sceneRenderer,
                   const HGVertexBufferBindings &binding,
                   const std::shared_ptr<ILiquidMaterial> &liquidMaterial,
                   int liquidType,
                   int indexBufferSize,
                   CAaBox &waterAaBB);

    void collectMeshes(COpaqueMeshCollector &opaqueMeshCollector);
    void collectMeshes(framebased::vector<HGSortableMesh> &transparentMeshes);

private:
    const HApiContainer &m_api;

    std::shared_ptr<ILiquidMaterial> m_liquidMaterial;
    std::vector<HGVertexBufferBindings> m_vertexWaterBufferBindings;
    std::vector<HGSortableMesh> m_liquidMeshes;
    CAaBox &m_waterBBox;

    void createAdtVertexData(const SMLiquidInstance &liquidInstance, const mathfu::vec3 &liquidBasePos,
                             const PointerChecker<char> &mH2OBlob, CAaBox &waterAaBB, int liquidVertexFormat,
                             bool generateTexCoordsFromPos, std::vector<LiquidVertexFormat> &vertexBuffer,
                             std::vector<uint16_t> &indexBuffer) const;

    HGVertexBufferBindings
    createLiquidVao(const HMapSceneBufferCreate &sceneRenderer, std::vector<LiquidVertexFormat> &vertexBuffer,
                    std::vector<uint16_t> &indexBuffer) const;

    void createMesh(
        const HMapSceneBufferCreate &sceneRenderer, int indexBufferSize,
        const HGVertexBufferBindings &vertexWaterBufferBindings
    );
};


#endif //AWEBWOWVIEWERCPP_LIQUIDINSTANCE_H
