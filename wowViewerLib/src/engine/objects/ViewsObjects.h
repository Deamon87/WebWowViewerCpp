//
// Created by deamon on 22.10.19.
//

#ifndef AWEBWOWVIEWERCPP_VIEWSOBJECTS_H
#define AWEBWOWVIEWERCPP_VIEWSOBJECTS_H

#include <memory>

class GeneralView;
class InteriorView;
class ExteriorView;
class ADTObjRenderRes;
class FrameViewsHolder;

typedef std::shared_ptr<GeneralView> HGeneralView;
typedef std::shared_ptr<InteriorView> HInteriorView;
typedef std::shared_ptr<ExteriorView> HExteriorView;

#include <vector>
#include "wmo/wmoGroupObject.h"
#include "m2/m2Object.h"

class ADTObjRenderRes {
public:
    std::shared_ptr<AdtObject> adtObject;
    bool wasLoaded = false;
    std::array<bool, 256> drawChunk = {false};
    std::array<bool, 256> drawWaterChunk = {false};
    std::array<bool, 256> checkRefs = {false};
};

class GeneralView {
public:
    WMOGroupListContainer wmoGroupArray;
    M2ObjectListContainer m2List;

    //Support several frustum planes because of how portal culling works
    std::vector<std::vector<mathfu::vec3>> worldPortalVertices = {};
    MathHelper::FrustumCullingData frustumData;

    int level = -1;
    int renderOrder = -1;
    struct PortalPointsFrame {
        HGIndexBuffer m_indexVBO;
        HGVertexBuffer m_bufferVBO;

        HGVertexBufferBindings m_bindings;
        std::vector<HGSortableMesh> m_meshes = {};
    } portalPointsFrame;

    virtual void collectMeshes(bool renderADT, bool renderAdtLiquid, bool renderWMO, std::vector<HGMesh> &opaqueMeshes, std::vector<HGSortableMesh> &transparentMeshes);
    virtual void setM2Lights(std::shared_ptr<M2Object> &m2Object);

    void produceTransformedPortalMeshes(const HMapSceneBufferCreate &sceneRenderer, const HApiContainer &apiContainer);
    void addM2FromGroups(const MathHelper::FrustumCullingData &frustumData, mathfu::vec4 &cameraPos);
};

class InteriorView : public GeneralView {
public:
    std::vector<int> portalIndexes;
    std::shared_ptr<WmoGroupObject> ownerGroupWMO = {}; //Wmos which portals belong to
    void setM2Lights(std::shared_ptr<M2Object> &m2Object) override;
};

class ExteriorView : public GeneralView {
public:
	std::vector<std::shared_ptr<ADTObjRenderRes>> drawnADTs = {};
    std::vector<HGMesh> m_adtOpaqueMeshes = {};
    std::vector<HGSortableMesh> m_adtLiquidTransparentMeshes = {};

public:
    void collectMeshes(bool renderADT, bool renderAdtLiquid, bool renderWMO, std::vector<HGMesh> &opaqueMeshes, std::vector<HGSortableMesh> &transparentMeshes) override;
};

class FrameViewsHolder {
public:
    HExteriorView getOrCreateExterior(const MathHelper::FrustumCullingData &frustumData);
    HExteriorView getExterior();
    HExteriorView getSkybox();

    HInteriorView createInterior(const  MathHelper::FrustumCullingData &frustumData);

    const std::vector<HInteriorView> &getInteriorViews() {
        return interiorViews;
    }
private:
    HExteriorView exteriorView = nullptr ;
    HExteriorView skyBoxView = nullptr ;
    std::vector<HInteriorView> interiorViews = {};


};


#endif //AWEBWOWVIEWERCPP_VIEWSOBJECTS_H
