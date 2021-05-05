//
// Created by deamon on 10.07.17.
//

#ifndef WEBWOWVIEWERCPP_WMOGROUPOBJECT_H
#define WEBWOWVIEWERCPP_WMOGROUPOBJECT_H

class WmoObject;
class WmoGroupObject;

#include "../../persistance/header/wmoFileHeader.h"
#include "wmoObject.h"
#include "../iWmoApi.h"
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
    const std::vector <std::shared_ptr<M2Object>> *getDoodads() const {
        return &m_doodads;
    };

    void setWmoApi(IWmoApi *api);
    IWmoApi *getWmoApi() { return m_wmoApi; };
    void setModelFileName(std::string modelName);
    void setModelFileId(int fileId);

    void collectMeshes(std::vector<HGMesh> &opaqueMeshes, std::vector<HGMesh> &transparentMeshes, int renderOrder);


    bool getDontUseLocalLightingForM2() { return !m_useLocalLightingForM2; };
    bool doPostLoad();
    void update();
    void uploadGeneratorBuffers();
    void checkGroupFrustum(bool &drawDoodads, bool &drawGroup,
                           mathfu::vec4 &cameraVec4,
                           std::vector<mathfu::vec4> &frustumPlanes,
                           std::vector<mathfu::vec3> &points);

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

    HGUniformBufferChunk vertexModelWideUniformBuffer = nullptr;
    HGUniformBufferChunk fragmentModelWideUniformBuffer = nullptr;
    std::vector<HGMesh> m_meshArray = {};
    std::vector<HGMesh> m_waterMeshArray = {};

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
    void checkDoodads(std::vector<std::shared_ptr<M2Object>> &wmoM2Candidates);

    void postLoad();
    void createMeshes();
    void createWaterMeshes();

    int to_wmo_liquid (int x);
    void setLiquidType();

    void loadDoodads();

    bool checkIfInsidePortals(
        mathfu::vec3 point,
        const PointerChecker<SMOPortal> &portalInfos,
        const PointerChecker<SMOPortalRef> &portalRels
    );


    static void queryBspTree(CAaBox &bbox, int nodeId, t_BSP_NODE *nodes, std::vector<int> &bspLeafIdList);
    static void queryBspTree(CAaBox &bbox, int nodeId, PointerChecker<t_BSP_NODE> &nodes, std::vector<int> &bspLeafIdList);

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

#endif //WEBWOWVIEWERCPP_WMOGROUPOBJECT_H
