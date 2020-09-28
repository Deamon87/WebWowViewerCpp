//
// Created by deamon on 03.07.17.
//

#ifndef WEBWOWVIEWERCPP_WMOOBJECT_H
#define WEBWOWVIEWERCPP_WMOOBJECT_H

struct WmoGroupResult;
class WmoGroupObject;
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include "../../geometry/wmoMainGeom.h"

#include "../../persistance/header/adtFileHeader.h"
#include "mathfu/glsl_mappings.h"
#include "wmoGroupObject.h"
#include "../m2/m2Object.h"
#include "../iWmoApi.h"
#include "../../persistance/header/wmoFileHeader.h"
#include "../ViewsObjects.h"


struct WmoGroupResult {
    M2Range topBottom;
    int groupIndex;
    int WMOGroupID;
    std::vector<int> bspLeafList;
    int nodeId;
};

class WmoObject : public IWmoApi {

public:
    WmoObject(ApiContainer *api) : m_api(api) {
    }

	~WmoObject();
private:
    ApiContainer *m_api;

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

    std::vector<std::shared_ptr<WmoGroupObject>> groupObjects = std::vector<std::shared_ptr<WmoGroupObject>>(0);
    std::vector<std::shared_ptr<WmoGroupObject>> groupObjectsLod1 = std::vector<std::shared_ptr<WmoGroupObject>>(0);
    std::vector<std::shared_ptr<WmoGroupObject>> groupObjectsLod2 = std::vector<std::shared_ptr<WmoGroupObject>>(0);
    std::vector<BlpTexture> blpTextures;

    std::vector<bool> drawGroupWMO;
    std::vector<int> lodGroupLevelWMO;
    std::vector<std::shared_ptr<M2Object>> m_doodadsArray;

    std::shared_ptr<M2Object> skyBox = nullptr;

    std::unordered_map<int, HGTexture> diffuseTextures;
    std::unordered_map<int, HGTexture> specularTextures;

    // Portal culling stuff begin
    std::vector<bool> transverseVisitedPortals;
    // Portal culling stuff end

    HGMesh transformedAntiPortals;

    void createPlacementMatrix(SMMapObjDef &mapObjDef);
    void createPlacementMatrix(SMMapObjDefObj1 &mapObjDef);
    void createBB(CAaBox bbox);
    void postWmoGroupObjectLoad(int groupId, int lod) override;
    void fillLodGroup(mathfu::vec3 &cameraLocal);
    friend void attenuateTransVerts(HWmoMainGeom &mainGeom, WmoGroupGeom& wmoGroupGeom);
public:
    std::shared_ptr<M2Object> getDoodad(int index) override ;
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
    int getWmoId() {
        if (m_loaded) {
            return mainGeom->header->wmoID;
        }
        return 0;
    }
    int getWmoGroupId (int groupNum);

    virtual std::function<void (WmoGroupGeom& wmoGroupGeom)> getAttenFunction() override;
    virtual SMOHeader *getWmoHeader() override;
    mathfu::vec3 getAmbientColor() override;

    virtual PointerChecker<SMOMaterial> &getMaterials() override;

    virtual PointerChecker<SMOLight> &getLightArray() override;
    virtual std::vector<PortalInfo_t> &getPortalInfos() override {
        return geometryPerPortal;
    };

    std::shared_ptr<M2Object> getSkyBoxForGroup (int groupNum);;

    void collectMeshes(std::vector<HGMesh> &renderedThisFrame);

    void createGroupObjects();


    void checkFog(mathfu::vec3 &cameraPos, std::vector<LightResult> &fogResults);

    bool doPostLoad(int &processedThisFrame);
    void update();
    void uploadGeneratorBuffers();

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
        std::vector<std::shared_ptr<M2Object>> &m2Candidates);

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
    bool isGroupWmoExteriorLit(int groupId);
    bool isGroupWmoExtSkybox(int groupId);

    void drawTransformedPortalPoints();

    void drawDebugLights();

    void createWorldPortals();

    void createTransformedAntiPortalMesh();
    void updateTransformedAntiPortalPoints();
};


#endif //WEBWOWVIEWERCPP_WMOOBJECT_H
