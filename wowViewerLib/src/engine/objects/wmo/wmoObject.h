//
// Created by deamon on 03.07.17.
//

#ifndef WEBWOWVIEWERCPP_WMOOBJECT_H
#define WEBWOWVIEWERCPP_WMOOBJECT_H

struct WmoGroupResult;
class WmoGroupObject;
#include <string>
#include <unordered_set>
#include <oneapi/tbb/concurrent_unordered_set.h>
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
#include "../../../include/database/dbStructs.h"

class WmoObject : public IWmoApi {

public:
    WmoObject(HApiContainer &api) : m_api(api) {
    }

	~WmoObject();
private:
    struct PortalTraverseTempData {
        FrameViewsHolder &viewsHolder;
        bool exteriorWasCreatedBeforeTraversing;
        mathfu::vec4 farPlane;
        std::vector<HInteriorView> &ivPerWMOGroup;
        mathfu::vec4 &cameraVec4;
        mathfu::vec4 &cameraLocal;
        mathfu::mat4 &transposeInverseModelMat;
        std::vector<bool> &transverseVisitedPortals;

        bool atLeastOneGroupIsDrawn = false;
    };

    HApiContainer m_api;

    HWmoMainGeom mainGeom = nullptr;
    HMapSceneBufferCreate m_sceneRenderer;
    bool m_loading = false;
    bool m_loaded = false;
    CAaBox m_bbox;

    int m_nameSet;
    int m_doodadSet = -1;

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
    std::unordered_map<int, std::shared_ptr<M2Object>> m_doodadsUnorderedMap;

    std::shared_ptr<M2Object> skyBox = nullptr;

    std::unordered_map<int, HGTexture> diffuseTextures;
    std::unordered_map<int, HGTexture> specularTextures;

    std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> m_modelWideChunk;
    std::vector<std::weak_ptr<IWMOMaterial>> m_materialCache;

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

    PointerChecker<SMOMaterial> &getMaterials() override;
    std::shared_ptr<IWMOMaterial> getMaterialInstance(int index) override;

    PointerChecker<SMOLight> &getLightArray() override;
    std::vector<PortalInfo_t> &getPortalInfos() override {
        return geometryPerPortal;
    };

    std::shared_ptr<M2Object> getSkyBoxForGroup (int groupNum);;

    void collectMeshes(std::vector<HGMesh> &renderedThisFrame);

    void createGroupObjects();


    void checkFog(mathfu::vec3 &cameraPos, std::vector<LightResult> &fogResults);

    bool doPostLoad(const HMapSceneBufferCreate &sceneRenderer);
    void update();
    void uploadGeneratorBuffers();

    void createMaterialCache();
    void updateBB() override ;

    CAaBox getAABB();

public:
    //Portal culling
    bool startTraversingWMOGroup(
        mathfu::vec4 &cameraVec4,
        const MathHelper::FrustumCullingData &frustumData,
        int groupId,
        int globalLevel,
        int &renderOrder,
        bool traversingFromInterior,
        FrameViewsHolder &viewsHolder
    );

    void checkGroupDoodads(
        int groupId,
        mathfu::vec4 &cameraVec4,
        std::vector<mathfu::vec4> &frustumPlanes,
        M2ObjectListContainer &m2Candidates);

    void addSplitChildWMOsToView(InteriorView &interiorView, int groupId);


    void traverseGroupWmo (
        int groupId,
        bool traversingStartedFromInterior,
        PortalTraverseTempData &traverseTempData,
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

struct WMOObjectHasher
{
    size_t operator()(const std::shared_ptr<WmoObject>& val)const
    {
        return std::hash<std::shared_ptr<WmoObject>>()(val);
    }
};

//typedef std::unordered_set<std::shared_ptr<WmoObject>, WMOObjectHasher> WMOObjectSetCont;
class WMOListContainer {
private:
    std::vector<std::shared_ptr<WmoObject>> wmoCandidates;
    std::vector<std::shared_ptr<WmoObject>> wmoToLoad;
    std::vector<std::shared_ptr<WmoObject>> wmoToDrawn;

    bool candCanHaveDuplicates = false;
    bool toLoadCanHaveDuplicates = false;
    bool toDrawmCanHaveDuplicates = false;

    void inline removeDuplicates(std::vector<std::shared_ptr<WmoObject>> &array) {
        if (array.size() < 1000) {
            std::sort(array.begin(), array.end());
        } else {
            tbb::parallel_sort(array.begin(), array.end(), [](auto &a, auto &b) -> bool {
                return a < b;
            });
        }
        array.erase(std::unique(array.begin(), array.end()), array.end());
        return;
    }

public:
    WMOListContainer() {
        wmoCandidates.reserve(3000);
        wmoToLoad.reserve(3000);
    }

    void addCand(const std::shared_ptr<WmoObject> &toDraw) {
        if (toDraw->isLoaded()) {
            wmoCandidates.push_back(toDraw);
            candCanHaveDuplicates = true;
        } else {
            wmoToLoad.push_back(toDraw);
            toLoadCanHaveDuplicates = true;
        }
    }

    void addToLoad(const std::shared_ptr<WmoObject> &toLoad) {
        if (!toLoad->isLoaded()) {
            wmoToLoad.push_back(toLoad);
            toLoadCanHaveDuplicates = true;
        }
    }

    const std::vector<std::shared_ptr<WmoObject>> &getCandidates() {
        if (this->candCanHaveDuplicates) {
            removeDuplicates(wmoCandidates);
            candCanHaveDuplicates = false;
        }

        return wmoCandidates;
    }

    const std::vector<std::shared_ptr<WmoObject>> &getToLoad() {
        if (this->toLoadCanHaveDuplicates) {
            removeDuplicates(wmoToLoad);
            toLoadCanHaveDuplicates = false;
        }

        return wmoToLoad;
    }

    void addToDrawn(const std::shared_ptr<WmoObject> &toDrawn) {
        if (toDrawn->isLoaded()) {
            wmoToDrawn.push_back(toDrawn);
            toDrawmCanHaveDuplicates = true;
        }
    }

    const std::vector<std::shared_ptr<WmoObject>> &getToDrawn() {
        if (this->toDrawmCanHaveDuplicates) {
            removeDuplicates(wmoToDrawn);
            toDrawmCanHaveDuplicates = false;
        }

        return wmoToDrawn;
    }

};


#endif //WEBWOWVIEWERCPP_WMOOBJECT_H
