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
    std::vector<std::vector<mathfu::vec3>> worldAntiPortalVertices = {};
    MathHelper::FrustumCullingData frustumData;

    int level = -1;
    int renderOrder = -1;
    struct PortalPointsFrame {
        HGIndexBuffer m_indexVBO;
        HGVertexBuffer m_bufferVBO;

        HGVertexBufferBindings m_bindings;
    };
    framebased::vector<HGSortableMesh> m_portalMeshes = {};

    std::vector<PortalPointsFrame> portals;

    virtual void collectMeshes(bool renderADT, bool renderAdtLiquid, bool renderWMO, COpaqueMeshCollector &opaqueMeshCollector, framebased::vector<HGSortableMesh> &transparentMeshes);
    virtual void collectLights(std::vector<LocalLight> &pointLights, std::vector<SpotLight> &spotLights, std::vector<std::shared_ptr<CWmoNewLight>> &newWmoLights);
    void collectPortalMeshes(framebased::vector<HGSortableMesh> &transparentMeshes);

    void produceTransformedPortalMeshes(const HMapSceneBufferCreate &sceneRenderer, const HApiContainer &apiContainer,
                                        const std::vector<std::vector<mathfu::vec3>> &portalsVerts, bool isAntiportal = false);
    void addM2FromGroups(const MathHelper::FrustumCullingData &frustumData, mathfu::vec4 &cameraPos);
};

class InteriorView : public GeneralView {
public:
    std::vector<int> portalIndexes;
    std::shared_ptr<WmoGroupObject> ownerGroupWMO = {}; //Wmos which portals belong to
};

class ExteriorView : public GeneralView {
public:
    void collectMeshes(bool renderADT, bool renderAdtLiquid, bool renderWMO, COpaqueMeshCollector &opaqueMeshCollector, framebased::vector<HGSortableMesh> &transparentMeshes) override;
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
