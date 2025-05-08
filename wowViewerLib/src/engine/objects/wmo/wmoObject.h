//
// Created by deamon on 03.07.17.
//

#ifndef WEBWOWVIEWERCPP_WMOOBJECT_H
#define WEBWOWVIEWERCPP_WMOOBJECT_H
#include <bitset>

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
#include "../lights/CEngineLight.h"

enum class WMOObjId : uintptr_t;
constexpr WMOObjId emptyWMO = static_cast<const WMOObjId>(0xFFFFFFF);

class WmoObject : public IWmoApi, public ObjectWithId<WMOObjId>{

public:
    WmoObject(const HApiContainer &api/*, int id*/) : /*SceneObjectWithId(id),*/ m_api(api) {
        //DoodadSet 0 is always active
        m_activeDoodadSets.set(0);
    }

	~WmoObject();
private:
    //One unit of portal-traversal work: a group plus the (possibly portal-clipped) frustum it
    //was reached with. Processed FIFO → breadth-first, so a group's first arrival is always a
    //shortest portal path.
    struct PortalTraversalWorkItem {
        int groupId;
        framebased::vector<mathfu::vec4> frustumPlanes;
        int globalLevel;
        int localLevel;
    };

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
        //BFS worklist; seeded by startTraversingWMOGroup, drained by drainPortalTraversal
        framebased::vector<PortalTraversalWorkItem> &portalWorkList;
        //Per portal: the last frustum contribution propagated through it. Used to suppress
        //duplicate contributions on cyclic portal paths (see drainPortalTraversal)
        framebased::vector<framebased::vector<mathfu::vec4>> &portalCrossedFrustums;
        //Batch-cull results for m_groupWorldBorders (0 = group incl. doodads is outside the
        //global frustum). Portal traversal skips descending into culled groups.
        const std::vector<uint32_t> &groupWorldVisible;

        bool atLeastOneGroupIsDrawn = false;
    };

    HApiContainer m_api;

    HWmoMainGeom mainGeom = nullptr;

    bool m_loading = false;
    bool m_loaded = false;
    CAaBox m_bbox;

    //Per-group world-space bounding boxes, owned here (contiguous) so batched SIMD culling can
    //run over them. [i] corresponds to groupObjects[i]. m_groupWorldBorders includes loaded
    //doodad M2s; m_groupVolumeWorldBorders is group geometry only.
    std::vector<CAaBox> m_groupWorldBorders;
    std::vector<CAaBox> m_groupVolumeWorldBorders;
    //Scratch buffers for batch cull results, reused across calls to avoid per-call allocation
    std::vector<uint32_t> m_groupWorldVisScratch;
    std::vector<uint32_t> m_groupVolumeVisScratch;

    //Precomputed at doPostLoad from mainGeom->groups flags. Used by Map::checkExterior to decide
    //whether a frustum-culled WMO must still be traversed for its unconditional side effects.
    bool m_hasAlwaysDrawGroups = false;
    bool m_hasAntiportalGroups = false;

    int m_nameSet;
    ActiveDoodadSets m_activeDoodadSets;

    std::vector<PortalInfo_t> geometryPerPortal;

    bool m_placementMatChanged = false;
    mathfu::mat4 m_placementMatrix;
    mathfu::mat4 m_placementInvertMatrix;

    std::string m_modelName;
    bool useFileId = false;
    int m_modelFileId;

    std::vector<std::shared_ptr<WmoGroupObject>> groupObjects = std::vector<std::shared_ptr<WmoGroupObject>>(0);
    std::vector<std::shared_ptr<WmoGroupObject>> groupObjectsLod1 = std::vector<std::shared_ptr<WmoGroupObject>>(0);
    std::vector<std::shared_ptr<WmoGroupObject>> groupObjectsLod2 = std::vector<std::shared_ptr<WmoGroupObject>>(0);
    std::vector<BlpTexture> blpTextures;

    std::vector<std::shared_ptr<CEngineLight>> m_newLights;

    std::vector<bool> drawGroupWMO;
    std::vector<int> lodGroupLevelWMO;
    robin_hood::unordered_flat_map<int, std::weak_ptr<M2Object>> m_doodadsUnorderedMap;

    std::shared_ptr<M2Object> skyBox = nullptr;

    robin_hood::unordered_flat_map<int, HGSamplableTexture> diffuseTextures;
    robin_hood::unordered_flat_map<int, HGSamplableTexture> specularTextures;

    std::shared_ptr<IWmoModelData> m_wmoModelChunk;

    bool m_interiorAmbientsChanged = false;
    std::vector<WMO::InteriorBlockData> m_groupInteriorData;

    std::vector<std::shared_ptr<IWMOMaterial>> m_materialCache;

    HGMesh transformedAntiPortals;

    //Ambient, GroundAmbient, HorizontalAmbient
    std::array<mathfu::vec3, 3> m_ambientColors;

    void createPlacementMatrix(const SMMapObjDef &mapObjDef);
    void createPlacementMatrix(const SMMapObjDefObj1 &mapObjDef);
    void createBB(CAaBox bbox);
    //Pushes m_bbox into the CAaBox component storage of wmoFactory (for batched frustum culling).
    //Must be called after every m_bbox mutation.
    void syncBBoxComponent();
    void postWmoGroupObjectLoad(int groupId, int lod) override;
    void fillLodGroup(mathfu::vec3 &cameraLocal);
    friend void attenuateTransVerts(HWmoMainGeom &mainGeom, WmoGroupGeom& wmoGroupGeom);
public:
    std::shared_ptr<M2Object> getDoodad(int index, int fromGroupIndex) override ;
    void applyLightingParamsToDoodad(const SMODoodadDef *doodadDef, M2Object* doodad, float mddiVal, int fromGroupIndex);
    void applyColorFromMOLT(
        const SMODoodadDef *doodadDef,
        M2Object *doodad,
        std::array<mathfu::vec3, 3> &interiorAmbients,
        mathfu::vec3 &color,
        bool &hasDoodad0x4Flag,
        int fromGroupIndex);

    HGSamplableTexture getTexture(int materialId, bool isSpec) override;
    void setLoadingParam(const SMMapObjDef &mapObjDef, const PointerChecker<MWDR> &MWDR, const PointerChecker<uint16_t> &MWDS);
    void setLoadingParam(const SMMapObjDefObj1 &mapObjDef, const PointerChecker<MWDR> &MWDR, const PointerChecker<uint16_t> &MWDS);

    void setLoadingParam(mathfu::vec3 pos, mathfu::vec3 scaleVec, mathfu::mat4 *rotationMatrix,
                                const mathfu::vec4 &localAABBMin, const mathfu::vec4 &localAABBMax);

    std::string getModelFileName();
    void setModelFileName(std::string modelName);
    int getModelFileId();
    void setModelFileId(int fileId);

    void startLoading();
    bool isLoaded() override { return m_loaded;}
    bool isFailedToLoadGeom() { return mainGeom != nullptr && mainGeom->getStatus() == FileStatus::FSRejected; }
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
    ActiveDoodadSets getActiveDoodadSet() override {
        return m_activeDoodadSets;
    }
    std::array<mathfu::vec3, 3> getAmbientColors() override;

    PointerChecker<SMOMaterial> &getMaterials() override;
    std::shared_ptr<IWMOMaterial> getMaterialInstance(int index, const HMapSceneBufferCreate &sceneRenderer) override;

    PointerChecker<SMOLight> &getLightArray() override;
    std::vector<PortalInfo_t> &getPortalInfos() override {
        return geometryPerPortal;
    };
    std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> getPlacementBuffer() override {
        return m_wmoModelChunk->m_placementMatrix;
    }
    uint32_t getPickObjectId() override {
        return static_cast<uint32_t>(this->ObjectWithId<WMOObjId>::getObjectId());
    }

    std::shared_ptr<M2Object> getSkyBoxForGroup (int groupNum);;
    void collectMeshes(std::vector<HGMesh> &renderedThisFrame);

    void createGroupObjects();

    //Result of WMO fog query
    struct WmoFogBlendResult {
        bool fogFound = false;          // a WMO fog record applies at the camera position
        bool insideInterior = false;    // camera is inside a non-exterior(-lit) group of this WMO
        float distToExit = 0.0f;        // distance from camera to nearest portal of the interior group(s)
        SMOFog fog = {};                // blended WMO fog record (fog + underwater_fog)
    };
    void checkFog(const mathfu::vec3 &cameraPos, int currentGroupIndex, WmoFogBlendResult &result);

    bool doPostLoad(const HMapSceneBufferCreate &sceneRenderer);
    void update();
    void uploadGeneratorBuffers();

    void createMaterialCache();
    void updateBB() override ;

    CAaBox getAABB();

    bool hasAlwaysDrawGroups() const { return m_hasAlwaysDrawGroups; }
    bool hasAntiportalGroups() const { return m_hasAntiportalGroups; }

    const CAaBox &getGroupWorldBorder(int groupId) override { return m_groupWorldBorders[groupId]; }
    const CAaBox &getGroupVolumeWorldBorder(int groupId) override { return m_groupVolumeWorldBorders[groupId]; }
    void recalcGroupBorders(int groupId) override;

    //Queues not-yet-loaded exterior groups for loading without doing any culling work.
    //Used when the whole WMO was culled at the WMO bbox level: mirrors the load-trigger
    //side effects startTraversingWMOGroup has for such groups.
    void triggerExteriorGroupLoads(WMOGroupListContainer &wmoGroupArray);

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


    //Breadth-first portal traversal: drains traverseTempData.portalWorkList. A group re-entered
    //through a second visible portal path gets its outgoing portals re-evaluated with the new
    //frustum, so contributions are not lost on diamond paths (1->2 and 1->3->2 both visible).
    void drainPortalTraversal (
        bool traversingStartedFromInterior,
        PortalTraverseTempData &traverseTempData
    );

    bool getGroupWmoThatCameraIsInside(mathfu::vec4 cameraVec4, WmoGroupResult &result, float &bottomBorder);

    bool isGroupWmoInterior(int groupId);
    bool isGroupWmoExteriorLit(int groupId);
    bool isGroupWmoExtSkybox(int groupId);

    void createWorldPortals();
    void createNewLights();
    void calculateAmbient();
    std::shared_ptr<CEngineLight> getNewLight(int index) override;
    void setInteriorAmbientColor(int groupIndex,
        bool isExteriorLighted,
        const mathfu::vec3 &ambient,
        const mathfu::vec3 &horizontAmbient,
        const mathfu::vec3 &groundAmbient
    ) override;
};

typedef EntityFactory<2000, WMOObjId, WmoObject, CAaBox> WMOEntityFactory;

extern std::shared_ptr<WMOEntityFactory> wmoFactory;

static const CAaBox wmoNonexistingAabb = CAaBox(
    mathfu::vec3_packed(mathfu::vec3(999999, 999999, 999999)),
    mathfu::vec3_packed(mathfu::vec3(-999999, -999999, -999999))
);

template<>
inline const CAaBox &retrieveAABB<>(const WMOObjId &objectId) {
    auto * ptr = wmoFactory->getObjectByIdConst<1>(objectId);
    return ptr ? *ptr : wmoNonexistingAabb;
}

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
            if (toDraw->isFailedToLoadGeom())
                return;

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
