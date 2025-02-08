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
#include "../SceneObjectWithID.h"
#include "../lights/CWmoNewLight.h"

enum class WMOObjId : uintptr_t;
constexpr WMOObjId emptyWMO = static_cast<const WMOObjId>(0xFFFFFFF);

class WmoObject : public IWmoApi, public ObjectWithId<WMOObjId>{

public:
    WmoObject(HApiContainer &api/*, int id*/) : /*SceneObjectWithId(id),*/ m_api(api) {
    }

	~WmoObject();
private:
    struct PortalTraverseTempData {
        FrameViewsHolder &viewsHolder;
        bool exteriorWasCreatedBeforeTraversing;
        mathfu::vec4 farPlane;
        framebased::vector<HInteriorView> &ivPerWMOGroup;
        mathfu::vec4 &cameraVec4;
        mathfu::vec4 &cameraLocal;
        mathfu::mat4 &transposeInverseModelMat;
        mathfu::mat4 &MVPMat;
        mathfu::mat4 &MVPMatInv;
        framebased::vector<bool> &transverseVisitedPortals;

        bool atLeastOneGroupIsDrawn = false;
    };

    HApiContainer m_api;

    HWmoMainGeom mainGeom = nullptr;

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

    std::vector<std::shared_ptr<CWmoNewLight>> m_newLights;

    std::vector<bool> drawGroupWMO;
    std::vector<int> lodGroupLevelWMO;
    robin_hood::unordered_flat_map<int, std::shared_ptr<M2Object>> m_doodadsUnorderedMap;

    std::shared_ptr<M2Object> skyBox = nullptr;

    robin_hood::unordered_flat_map<int, HGSamplableTexture> diffuseTextures;
    robin_hood::unordered_flat_map<int, HGSamplableTexture> specularTextures;

    std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> m_modelWideChunk;
    std::vector<std::shared_ptr<IWMOMaterial>> m_materialCache;

    HGMesh transformedAntiPortals;

    void createPlacementMatrix(SMMapObjDef &mapObjDef);
    void createPlacementMatrix(SMMapObjDefObj1 &mapObjDef);
    void createBB(CAaBox bbox);
    void postWmoGroupObjectLoad(int groupId, int lod) override;
    void fillLodGroup(mathfu::vec3 &cameraLocal);
    friend void attenuateTransVerts(HWmoMainGeom &mainGeom, WmoGroupGeom& wmoGroupGeom);
public:
    std::shared_ptr<M2Object> getDoodad(int index) override ;
    HGSamplableTexture getTexture(int materialId, bool isSpec) override;
    void setLoadingParam( SMMapObjDef &mapObjDef);
    void setLoadingParam( SMMapObjDefObj1 &mapObjDef);

    std::string getModelFileName();
    void setModelFileName(std::string modelName);
    int getModelFileId();
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

    std::function<void (WmoGroupGeom& wmoGroupGeom)> getAttenFunction() override;
    SMOHeader *getWmoHeader() override;
    int getActiveDoodadSet() override {
        return m_doodadSet;
    }
    mathfu::vec3 getAmbientColor() override;

    PointerChecker<SMOMaterial> &getMaterials() override;
    std::shared_ptr<IWMOMaterial> getMaterialInstance(int index, const HMapSceneBufferCreate &sceneRenderer) override;

    PointerChecker<SMOLight> &getLightArray() override;
    std::vector<PortalInfo_t> &getPortalInfos() override {
        return geometryPerPortal;
    };
    std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> getPlacementBuffer() override {
        return m_modelWideChunk;
    }

    std::shared_ptr<M2Object> getSkyBoxForGroup (int groupNum);;
    void collectMeshes(std::vector<HGMesh> &renderedThisFrame);

    void createGroupObjects();
    void checkFog(mathfu::vec3 &cameraPos, std::vector<SMOFog_Data> &wmoFogData);

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
        framebased::vector<mathfu::vec4> &localFrustumPlanes,
        int globalLevel,
        int localLevel
    );

    bool getGroupWmoThatCameraIsInside(mathfu::vec4 cameraVec4, WmoGroupResult &result);

    bool isGroupWmoInterior(int groupId);
    bool isGroupWmoExteriorLit(int groupId);
    bool isGroupWmoExtSkybox(int groupId);

    void drawDebugLights();

    void createWorldPortals();
    void createNewLights();
    std::shared_ptr<CWmoNewLight> getNewLight(int index) override;
};

typedef EntityFactory<2000, WMOObjId, WmoObject> WMOEntityFactory;

extern std::shared_ptr<WMOEntityFactory> wmoFactory;

class WMOListContainer {
    using wmoContainer = framebased::vector<WMOObjId>;
//    using wmoContainer = std::vector<std::shared_ptr<WmoObject>>;
private:
    wmoContainer wmoCandidates;
    wmoContainer wmoToLoad;
    wmoContainer wmoToDrawn;

    bool candCanHaveDuplicates = false;
    bool toLoadCanHaveDuplicates = false;
    bool toDrawmCanHaveDuplicates = false;

    bool m_locked = false;

    void inline removeDuplicates(wmoContainer &array) {
        if (array.size() < 1000) {
            std::sort(array.begin(), array.end(), [](auto &a, auto &b) -> bool {
                return a < b;
            });
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
        wmoToDrawn.reserve(3000);
    }

    void addCand(const std::shared_ptr<WmoObject> &toDraw) {
        if (m_locked) {
            throw "oops";
        }

        if (toDraw->isLoaded()) {
            wmoCandidates.push_back(toDraw->getObjectId());
            candCanHaveDuplicates = true;
        } else {
            wmoToLoad.push_back(toDraw->getObjectId());
            toLoadCanHaveDuplicates = true;
        }
    }

    void addToLoad(const std::shared_ptr<WmoObject> &toLoad) {
        if (m_locked) {
            throw "oops";
        }

        if (!toLoad->isLoaded()) {
            wmoToLoad.push_back(toLoad->getObjectId());
            toLoadCanHaveDuplicates = true;
        }
    }

    void addToDrawn(const std::shared_ptr<WmoObject> &toDrawn) {
        if (m_locked) {
            throw "oops";
        }

        if (toDrawn->isLoaded()) {
            wmoToDrawn.push_back(toDrawn->getObjectId());
            toDrawmCanHaveDuplicates = true;
        }
    }
    void addToDrawn(WmoObject* toDrawn) {
        if (m_locked) {
            throw "oops";
        }

        if (toDrawn->isLoaded()) {
            wmoToDrawn.push_back(toDrawn->getObjectId());
            toDrawmCanHaveDuplicates = true;
        }
    }

    const wmoContainer &getCandidates() {
        if (this->candCanHaveDuplicates) {
            removeDuplicates(wmoCandidates);
            candCanHaveDuplicates = false;
        }

        return wmoCandidates;
    }

    const wmoContainer &getToLoad() {
        if (this->toLoadCanHaveDuplicates) {
            removeDuplicates(wmoToLoad);
            toLoadCanHaveDuplicates = false;
        }

        return wmoToLoad;
    }


    const wmoContainer &getToDrawn() {
        if (this->toDrawmCanHaveDuplicates) {
            removeDuplicates(wmoToDrawn);
            toDrawmCanHaveDuplicates = false;
        }

        return wmoToDrawn;
    }

    void lock() {
        getToDrawn();
        getToLoad();
        getCandidates();

        m_locked = true;
    }

};


#endif //WEBWOWVIEWERCPP_WMOOBJECT_H
