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

class COpaqueMeshCollector {
public:
    virtual void addM2Mesh(const HGM2Mesh &mesh) = 0;
    virtual void addWMOMesh(const HGMesh &mesh) = 0;
    virtual void addWaterMesh(const HGMesh &mesh) = 0;
    virtual void addADTMesh(const HGMesh &mesh) = 0;

    virtual void addMesh(const HGMesh &mesh) = 0;
};

class GeneralView {
public:
    WMOGroupListContainer wmoGroupArray;
    M2ObjectListContainer m2List;

    //Support several frustum planes because of how portal culling works
    std::vector<std::vector<mathfu::vec3>> worldPortalVertices = {};
    std::vector<std::vector<mathfu::vec3>> worldAntiPortalVertices = {};
    MathHelper::FrustumCullingData frustumData;

    int level = -1;
    int renderOrder = -1;
    struct PortalPointsFrame {
        HGIndexBuffer m_indexVBO;
        HGVertexBuffer m_bufferVBO;

        HGVertexBufferBindings m_bindings;
    };
    std::vector<HGSortableMesh> m_portalMeshes = {};

    std::vector<PortalPointsFrame> portals;

    std::vector<HGSortableMesh> liquidMeshes = {};

    virtual void collectMeshes(bool renderADT, bool renderAdtLiquid, bool renderWMO, std::vector<HGMesh> &opaqueMeshes, std::vector<HGSortableMesh> &transparentMeshes, std::vector<HGSortableMesh> &liquidMeshes);
    void collectPortalMeshes(std::vector<HGSortableMesh> &transparentMeshes);
    virtual void setM2Lights(std::shared_ptr<M2Object> &m2Object);

    void produceTransformedPortalMeshes(const HMapSceneBufferCreate &sceneRenderer, const HApiContainer &apiContainer,
                                        const std::vector<std::vector<mathfu::vec3>> &portalsVerts, bool isAntiportal = false);
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
public:
    void collectMeshes(bool renderADT, bool renderAdtLiquid, bool renderWMO, std::vector<HGMesh> &opaqueMeshes, std::vector<HGSortableMesh> &transparentMeshes, std::vector<HGSortableMesh> &liquidMeshes) override;
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
