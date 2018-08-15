//
// Created by deamon on 03.07.17.
//

#ifndef WEBWOWVIEWERCPP_WMOOBJECT_H
#define WEBWOWVIEWERCPP_WMOOBJECT_H

struct WmoGroupResult;
class WmoGroupObject;
class WmoGroupObject;
#include <string>
#include <unordered_set>
#include "../../persistance/header/adtFileHeader.h"
#include "mathfu/glsl_mappings.h"
#include "wmoGroupObject.h"
#include "../m2/m2Object.h"
#include "../../geometry/wmoMainGeom.h"
#include "../iWmoApi.h"
#include "../../persistance/header/wmoFileHeader.h"


struct WmoGroupResult {
    M2Range topBottom;
    int groupIndex;
    int WMOGroupID;
    std::vector<int> bspLeafList;
    int nodeId;
};

class GeneralView {
public:
    bool viewCreated = false;
    std::vector<WmoGroupObject *> drawnWmos;
    std::vector<M2Object *> drawnM2s;
    //Support several frustum planes because of how portal culling works
    std::vector<std::vector<mathfu::vec3>> portalVertices;
    std::vector<std::vector<mathfu::vec4>> frustumPlanes;
    int level = -1;
    int renderOrder = -1;

    virtual void collectMeshes(std::vector<HGMesh> &renderedThisFrame);
    virtual void setM2Lights(M2Object * m2Object){};
    void addM2FromGroups(mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos);
};

class InteriorView : public GeneralView {
public:
    int portalIndex;
    void setM2Lights(M2Object * m2Object) override;;
};

class ExteriorView : public GeneralView {
public:
    std::vector<AdtObject *> drawnADTs;
    std::vector<HGMesh> drawnChunks;

public:
    void collectMeshes(std::vector<HGMesh> &renderedThisFrame) override;
};


class WmoObject : public IWmoApi {

public:
    WmoObject(IWoWInnerApi *api) : m_api(api) {
    }
private:
    IWoWInnerApi *m_api;

    HWmoMainGeom mainGeom = nullptr;
    bool m_loading = false;
    bool m_loaded = false;
    CAaBox m_bbox;

    int m_nameSet;
    int m_doodadSet;

    std::vector<PortalInfo_t> geometryPerPortal;

    mathfu::mat4 m_placementMatrix;
    mathfu::mat4 m_placementInvertMatrix;

    std::string m_modelName;
    bool useFileId = false;
    int m_modelFileId;

    std::vector<WmoGroupObject*> groupObjects = std::vector<WmoGroupObject*>(0);
    std::vector<WmoGroupObject*> groupObjectsLod1 = std::vector<WmoGroupObject*>(0);
    std::vector<WmoGroupObject*> groupObjectsLod2 = std::vector<WmoGroupObject*>(0);
    std::vector<BlpTexture> blpTextures;

    std::vector<bool> drawGroupWMO;
    std::vector<int> lodGroupLevelWMO;
    std::vector<M2Object*> m_doodadsArray;

    std::unordered_map<int, HGTexture> diffuseTextures;
    std::unordered_map<int, HGTexture> specularTextures;

    // Portal culling stuff begin
    std::vector<bool> transverseVisitedPortals;
    std::vector<int> collectedM2s;
    // Portal culling stuff end

    void createPlacementMatrix(SMMapObjDef &mapObjDef);
    void createPlacementMatrix(SMMapObjDefObj1 &mapObjDef);
    void createBB(CAaBox bbox);
    void postWmoGroupObjectLoad(int groupId, int lod) override;
    void fillLodGroup(mathfu::vec3 &cameraLocal);
    friend void attenuateTransVerts(HWmoMainGeom &mainGeom, WmoGroupGeom& wmoGroupGeom);
public:
    M2Object *getDoodad(int index) override ;
    HGTexture getTexture(int materialId, bool isSpec) override;
    void setLoadingParam( SMMapObjDef &mapObjDef);
    void setLoadingParam( SMMapObjDefObj1 &mapObjDef);

    void setModelFileName(std::string modelName);
    void setModelFileId(int fileId);

    void startLoading();
    bool isLoaded() override { return m_loaded;}
    bool hasPortals();
    int getNameSet() {
        return m_nameSet;
    }

    virtual std::function<void (WmoGroupGeom& wmoGroupGeom)> getAttenFunction() override;
    virtual SMOHeader *getWmoHeader() override;

    virtual SMOMaterial *getMaterials() override;

    virtual SMOLight *getLightArray() override;
    virtual std::vector<PortalInfo_t> &getPortalInfos() override {
        return geometryPerPortal;
    };

    void collectMeshes(std::vector<HGMesh> &renderedThisFrame);

    void createGroupObjects();


    bool checkFog(mathfu::vec3 &cameraPos, CImVector &fogColor);

    void doPostLoad();
    void update();

    void createM2Array();
    void updateBB() override ;

public:
    //Portal culling
//    bool startTraversingFromInteriorWMO (
//        std::vector<WmoGroupResult> &wmoGroupsResult,
//        mathfu::vec4 &cameraVec4,
//        mathfu::mat4 &viewPerspectiveMat,
//        std::vector<M2Object*> &m2RenderedThisFrame);
//
//    bool startTraversingFromExterior (
//        mathfu::vec4 &cameraVec4,
//        mathfu::mat4 &viewPerspectiveMat,
//        std::vector<M2Object*> &m2RenderedThisFrame);

    void resetTraversedWmoGroups();
    bool startTraversingWMOGroup(
        mathfu::vec4 &cameraVec4,
        mathfu::mat4 &viewPerspectiveMat,
        int groupId,
        int globalLevel,
        int &renderOrder,
        bool traversingFromInterior,
        std::vector<InteriorView> &interiorViews,
        ExteriorView &exteriorView
    );

    void checkGroupDoodads(
        int groupId,
        mathfu::vec4 &cameraVec4,
        std::vector<mathfu::vec4> &frustumPlanes,
        std::vector<M2Object*> &m2Candidates);

    void transverseGroupWMO (
        int groupId,
        bool traversingStartedFromInterior,
        std::vector<InteriorView> &allInteriorViews, //GroupIndex as index
        ExteriorView &exteriorView,
        mathfu::vec4 &cameraVec4,
        mathfu::vec4 &cameraLocal,
        mathfu::mat4 &inverseTransposeModelMat,
        std::vector<bool> &transverseVisitedPortals,
        std::vector<mathfu::vec4> &localFrustumPlanes,
        int globalLevel,
        int localLevel
    );

    bool getGroupWmoThatCameraIsInside(mathfu::vec4 cameraVec4, WmoGroupResult &result);

    bool isGroupWmoInterior(int groupId);

    void drawTransformedPortalPoints();

    void drawDebugLights();

    void createWorldPortals();

    void drawTransformedAntiPortalPoints();
};


#endif //WEBWOWVIEWERCPP_WMOOBJECT_H
