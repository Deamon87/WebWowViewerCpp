//
// Created by deamon on 03.07.17.
//

#ifndef WEBWOWVIEWERCPP_WMOOBJECT_H
#define WEBWOWVIEWERCPP_WMOOBJECT_H

struct WmoGroupResult;
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

struct PortalResults {
    int groupId;
    int portalIndex;
#ifndef CULLED_NO_PORTAL_DRAWING
    std::vector<mathfu::vec3> portalVertices;
#endif
    std::vector<mathfu::vec4> frustumPlanes;
    int level;
};

class WmoObject : public IWmoApi {

public:
    WmoObject(IWoWInnerApi *api) : m_api(api) {
        m_getTextureFunc = std::bind(&WmoObject::getTexture, this, std::placeholders::_1, std::placeholders::_2);
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

    std::function <HBlpTexture (int materialId, bool isSpec)> m_getTextureFunc;

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

    std::unordered_map<int, HBlpTexture> diffuseTextures;
    std::unordered_map<int, HBlpTexture> specularTextures;

    void createPlacementMatrix(SMMapObjDef &mapObjDef);
    void createPlacementMatrix(SMMapObjDefObj1 &mapObjDef);
    void createBB(CAaBox bbox);
    void postWmoGroupObjectLoad(int groupId, int lod) override;
    void fillLodGroup(mathfu::vec3 &cameraLocal);
public:
    M2Object *getDoodad(int index) override ;
    HBlpTexture getTexture(int materialId, bool isSpec);
    void setLoadingParam( SMMapObjDef &mapObjDef);
    void setLoadingParam( SMMapObjDefObj1 &mapObjDef);

    void setModelFileName(std::string modelName);
    void setModelFileId(int fileId);

    void startLoading();
    bool isLoaded(){ return m_loaded;}
    bool hasPortals() {
        return mainGeom->header->nPortals != 0;
    }
    mathfu::vec3 getAmbientLight() {
        return mathfu::vec3(
            mainGeom->header->ambColor.r/255.0f,
            mainGeom->header->ambColor.g/255.0f,
            mainGeom->header->ambColor.b/255.0f);
    }
    int getNameSet() {
        return m_nameSet;
    }
    virtual SMOHeader *getWmoHeader() override {
        return mainGeom->header;
    }
    virtual SMOLight *getLightArray() override {
        return mainGeom->lights;
    }
    virtual std::vector<PortalInfo_t> &getPortalInfos() override {
        return geometryPerPortal;
    };

    void draw();

    void createGroupObjects();


    bool checkFrustumCulling(mathfu::vec4 &cameraPos, std::vector<mathfu::vec4> &frustumPlanes,
                             std::vector<mathfu::vec3> &frustumPoints, std::vector<M2Object *> &m2RenderedThisFrame);
    bool checkFog(mathfu::vec3 &cameraPos, CImVector &fogColor) {
        mathfu::vec3 cameraLocal = (m_placementInvertMatrix * mathfu::vec4(cameraPos, 1.0)).xyz();
        for (int i = mainGeom->fogsLen-1; i >= 0; i--) {
            SMOFog &fogRecord = mainGeom->fogs[i];
            mathfu::vec3 fogPosVec = mathfu::vec3(fogRecord.pos);

            float distanceToFog = (fogPosVec - cameraLocal).Length();
            if ((distanceToFog < fogRecord.larger_radius) /*|| fogRecord.larger_radius == 0*/) {

                fogColor.r = fogRecord.fog.color.r;
                fogColor.g = fogRecord.fog.color.g;
                fogColor.b = fogRecord.fog.color.b;
//                this.sceneApi.setFogColor(fogRecord.fog_colorF);
                //this.sceneApi.setFogStart(wmoFile.mfog.fog_end);
//                this.sceneApi.setFogEnd(fogRecord.fog_end);
                return true;
            }
        }

        return false;
    }

    void update();

    void createM2Array();
    void updateBB() override ;

public:
    std::vector<PortalResults> interiorPortals;
    std::vector<PortalResults> exteriorPortals;
    std::vector<PortalResults> antiPortals;

public:
    //Portal culling
    bool startTraversingFromInteriorWMO (
        std::vector<WmoGroupResult> &wmoGroupsResult,
        mathfu::vec4 &cameraVec4,
        mathfu::mat4 &viewPerspectiveMat,
        std::vector<M2Object*> &m2RenderedThisFrame);

    bool startTraversingFromExterior (
        mathfu::vec4 &cameraVec4,
        mathfu::mat4 &viewPerspectiveMat,
        std::vector<M2Object*> &m2RenderedThisFrame);

    void checkGroupDoodads(
        int groupId,
        mathfu::vec4 &cameraVec4,
        std::vector<mathfu::vec4> &frustumPlanes,
        int level,
        std::vector<M2Object*> &m2Candidates);

    void transverseGroupWMO (
        int groupId,
        bool fromInterior,
        mathfu::vec4 &cameraVec4,
        mathfu::vec4 &cameraLocal,
        mathfu::mat4 &inverseTransposeModelMat,
        std::vector<bool> &transverseVisitedGroups,
        std::vector<bool> &transverseVisitedPortals,
        std::vector<mathfu::vec4> &localFrustumPlanes,
        int level,
        std::vector<M2Object*> &m2ObjectSet);

    bool getGroupWmoThatCameraIsInside(mathfu::vec4 cameraVec4, WmoGroupResult &result);

    bool isGroupWmoInterior(int groupId);

    void drawTransformedPortalPoints();

    void drawDebugLights();

    void createWorldPortals();

    void drawTransformedAntiPortalPoints();
};


#endif //WEBWOWVIEWERCPP_WMOOBJECT_H
