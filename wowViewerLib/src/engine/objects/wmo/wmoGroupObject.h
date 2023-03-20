//
// Created by deamon on 10.07.17.
//

#ifndef WEBWOWVIEWERCPP_WMOGROUPOBJECT_H
#define WEBWOWVIEWERCPP_WMOGROUPOBJECT_H

class WmoGroupObject;
class WMOGroupListContainer;

#include "../../persistance/header/wmoFileHeader.h"
#include "../iWmoApi.h"
#include "../m2/m2Object.h"
#include "../../../gapi/interface/meshes/IMesh.h"

class WmoGroupObject {
public:
    WmoGroupObject(mathfu::mat4 &modelMatrix, HApiContainer api, SMOGroupInfo &groupInfo, int groupNumber) : m_api(api){
        m_modelMatrix = &modelMatrix;
        m_groupNumber = groupNumber;
        m_main_groupInfo = &groupInfo;
        createWorldGroupBB(groupInfo.bounding_box, modelMatrix);
    }
    ~WmoGroupObject(){
//        std::cout << "WmoGroupObject destroyed" << std::endl;
    }

    void drawDebugLights();
    bool getIsLoaded() { return m_loaded; };
    CAaBox getWorldAABB() {
        return m_worldGroupBorder;
    }
    const CAaBox &getLocalAABB() {
        return m_localGroupBorder;
    }
    const HWmoGroupGeom getWmoGroupGeom() const { return m_geom; };
    const std::vector <std::shared_ptr<M2Object>> &getDoodads() const {
        return m_doodads;
    };

    void setWmoApi(IWmoApi *api);
    IWmoApi *getWmoApi() { return m_wmoApi; };
    void setModelFileName(std::string modelName);
    void setModelFileId(int fileId);

    void collectMeshes(std::vector<HGMesh> &opaqueMeshes, std::vector<HGSortableMesh> &transparentMeshes, int renderOrder);


    bool getDontUseLocalLightingForM2() { return !m_useLocalLightingForM2; };
    bool doPostLoad(const HMapSceneBufferCreate &sceneRenderer);
    void update();
    void uploadGeneratorBuffers();
    void checkGroupFrustum(bool &drawDoodads, bool &drawGroup,
                           mathfu::vec4 &cameraVec4,
                           const MathHelper::FrustumCullingData &frustumData);

    mathfu::vec4 getAmbientColor();
    void assignInteriorParams(std::shared_ptr<M2Object> m2Object);

    bool checkIfInsideGroup(mathfu::vec4 &cameraVec4,
                            mathfu::vec4 &cameraLocal,
                            PointerChecker<C3Vector> &portalVerticles,
                            PointerChecker<SMOPortal> &portalInfos,
                            PointerChecker<SMOPortalRef> &portalRels,
                            std::vector<WmoGroupResult> &candidateGroups);
private:
    HApiContainer m_api = nullptr;
    IWmoApi *m_wmoApi = nullptr;
    HWmoGroupGeom m_geom = nullptr;

    std::string m_fileName = "";
    bool useFileId = false;
    int m_modelFileId;

    CAaBox m_worldGroupBorder;
    CAaBox m_localGroupBorder;
    CAaBox m_volumeWorldGroupBorder;
    mathfu::mat4 *m_modelMatrix = nullptr;
    int m_groupNumber;

    std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> vertexModelWideUniformBuffer = nullptr;
    std::shared_ptr<IBufferChunk<WMO::modelWideBlockPS>> fragmentModelWideUniformBuffer = nullptr;
    std::vector<HGMesh> m_meshArray = {};
    std::vector<HGSortableMesh> m_sortableMeshArray = {};
    std::vector<HGSortableMesh> m_waterMeshArray = {};

    SMOGroupInfo *m_main_groupInfo;

    std::vector <std::shared_ptr<M2Object>> m_doodads = {};

    bool m_useLocalLightingForM2 = false;

    bool m_loading = false;
    bool m_loaded = false;

    bool m_recalcBoundries = false;
    int liquid_type = -1;

    void startLoading();
    void createWorldGroupBB (CAaBox &bbox, mathfu::mat4 &placementMatrix);

    void updateWorldGroupBBWithM2();
    void checkDoodads(M2ObjectListContainer &wmoM2Candidates);

    void postLoad(const HMapSceneBufferCreate &sceneRenderer);
    void createMeshes(const HMapSceneBufferCreate &sceneRenderer);
    void createWaterMeshes(const HMapSceneBufferCreate &sceneRenderer);

    int to_wmo_liquid (int x);
    void setLiquidType();

    void loadDoodads();

    bool checkIfInsidePortals(
        mathfu::vec3 point,
        const PointerChecker<SMOPortal> &portalInfos,
        const PointerChecker<SMOPortalRef> &portalRels
    );


    template<typename Y>
    static void queryBspTree(CAaBox &bbox, int nodeId, Y &nodes, std::vector<int> &bspLeafIdList);


    bool getTopAndBottomTriangleFromBsp(
        mathfu::vec4 &cameraLocal,
        PointerChecker<SMOPortal> &portalInfos,
        PointerChecker<SMOPortalRef> &portalRels,
        std::vector<int> &bspLeafList, M2Range &result);

    void getBottomVertexesFromBspResult(
        const PointerChecker<SMOPortal> &portalInfos,
        const PointerChecker<SMOPortalRef> &portalRels,
        const std::vector<int> &bspLeafList, mathfu::vec4 &cameraLocal, float &topZ,
        float &bottomZ, mathfu::vec4 &colorUnderneath, bool checkPortals = true);
};
enum liquid_basic_types
{
    liquid_basic_types_water = 0,
    liquid_basic_types_ocean = 1,
    liquid_basic_types_magma = 2,
    liquid_basic_types_slime = 3,

    liquid_basic_types_MASK = 3,
};
enum liquid_types
{
    // ...
        LIQUID_WMO_Water = 13,
    LIQUID_WMO_Ocean = 14,
    LIQUID_Green_Lava = 15,
    LIQUID_WMO_Magma = 19,
    LIQUID_WMO_Slime = 20,

    LIQUID_END_BASIC_LIQUIDS = 20,
    LIQUID_FIRST_NONBASIC_LIQUID_TYPE = 21,

    LIQUID_NAXX_SLIME = 21,
    // ...
};


class WMOGroupListContainer {
private:
    std::vector<std::shared_ptr<WmoGroupObject>> wmoGroupToDraw;
    std::vector<std::shared_ptr<WmoGroupObject>> wmoGroupToCheckM2;
    std::vector<std::shared_ptr<WmoGroupObject>> wmoGroupToLoad;

    bool toDrawCanHaveDuplicates = false;
    bool toCheckM2CanHaveDuplicates = false;
    bool toLoadCanHaveDuplicates = false;

    void inline removeDuplicates(std::vector<std::shared_ptr<WmoGroupObject>> &array) {
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
    WMOGroupListContainer() {
        wmoGroupToDraw.reserve(3000);
        wmoGroupToCheckM2.reserve(3000);
        wmoGroupToLoad.reserve(3000);
    }

    void addToDraw(const std::shared_ptr<WmoGroupObject> &toDraw) {
        if (toDraw->getIsLoaded()) {
            wmoGroupToDraw.push_back(toDraw);
            toDrawCanHaveDuplicates = true;
        } else {
            wmoGroupToLoad.push_back(toDraw);
            toLoadCanHaveDuplicates = true;
        }
    }

    void addToCheckM2(const std::shared_ptr<WmoGroupObject> &toCheckM2) {
        if (toCheckM2->getIsLoaded()) {
            wmoGroupToCheckM2.push_back(toCheckM2);
            toCheckM2CanHaveDuplicates = true;
        } else {
            wmoGroupToLoad.push_back(toCheckM2);
            toLoadCanHaveDuplicates = true;
        }
    }
    void addToLoad(const std::shared_ptr<WmoGroupObject> &toLoad) {
        if (!toLoad->getIsLoaded()) {
            wmoGroupToLoad.push_back(toLoad);
            toLoadCanHaveDuplicates = true;
        }
    }

    const std::vector<std::shared_ptr<WmoGroupObject>> &getToDraw() {
        if (this->toDrawCanHaveDuplicates) {
            removeDuplicates(wmoGroupToDraw);
            toDrawCanHaveDuplicates = false;
        }

        return wmoGroupToDraw;
    }

    const std::vector<std::shared_ptr<WmoGroupObject>> &getToCheckM2() {
        if (this->toCheckM2CanHaveDuplicates) {
            removeDuplicates(wmoGroupToCheckM2);
            toCheckM2CanHaveDuplicates = false;
        }

        return wmoGroupToCheckM2;
    }

    const std::vector<std::shared_ptr<WmoGroupObject>> &getToLoad() {
        if (this->toLoadCanHaveDuplicates) {
            removeDuplicates(wmoGroupToLoad);
            toLoadCanHaveDuplicates = false;
        }

        return wmoGroupToLoad;
    }

    void addToLoadAndDraw(WMOGroupListContainer &otherList) {
        this->wmoGroupToDraw.insert(this->wmoGroupToDraw.end(), otherList.wmoGroupToDraw.begin(), otherList.wmoGroupToDraw.end());
        this->wmoGroupToLoad.insert(this->wmoGroupToLoad.end(), otherList.wmoGroupToLoad.begin(), otherList.wmoGroupToLoad.end());

        toDrawCanHaveDuplicates = true;
        toLoadCanHaveDuplicates = true;
    }
};

#endif //WEBWOWVIEWERCPP_WMOGROUPOBJECT_H
