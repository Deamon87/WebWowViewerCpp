//
// Created by deamon on 10.07.17.
//

#ifndef WEBWOWVIEWERCPP_WMOGROUPOBJECT_H
#define WEBWOWVIEWERCPP_WMOGROUPOBJECT_H

class WmoObject;
class WmoGroupObject;

#include "../../wowInnerApi.h"
#include "../../persistance/header/wmoFileHeader.h"
#include "wmoObject.h"
#include "../iWmoApi.h"
#include "../../../gapi/meshes/GMesh.h"

class WmoGroupObject {
public:
    WmoGroupObject(mathfu::mat4 &modelMatrix, IWoWInnerApi *api, SMOGroupInfo &groupInfo, int groupNumber) : m_api(api){
        m_modelMatrix = &modelMatrix;
        m_groupNumber = groupNumber;
        m_main_groupInfo = &groupInfo;
        createWorldGroupBB(groupInfo.bounding_box, modelMatrix);
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
    const std::vector <M2Object *> *getDoodads() const { return &m_doodads; };

    void setWmoApi(IWmoApi *api);
    void setModelFileName(std::string modelName);
    void setModelFileId(int fileId);

    void collectMeshes(std::vector<HGMesh> &renderedThisFrame);


    bool getDontUseLocalLightingForM2() { return m_dontUseLocalLightingForM2; };
    void doPostLoad();
    void update();
    bool checkGroupFrustum(mathfu::vec4 &cameraVec4,
                           std::vector<mathfu::vec4> &frustumPlanes,
                           std::vector<mathfu::vec3> &points);

    mathfu::vec4 getAmbientColor();

    bool checkIfInsideGroup(mathfu::vec4 &cameraVec4,
                            mathfu::vec4 &cameraLocal,
                            C3Vector *portalVerticles,
                            SMOPortal *portalInfos,
                            SMOPortalRef *portalRels,
                            std::vector<WmoGroupResult> &candidateGroups);
private:
    IWoWInnerApi *m_api = nullptr;
    IWmoApi *m_wmoApi = nullptr;
    HWmoGroupGeom m_geom = nullptr;

    std::string m_fileName;
    bool useFileId = false;
    int m_modelFileId;

    CAaBox m_worldGroupBorder;
    CAaBox m_localGroupBorder;
    CAaBox m_volumeWorldGroupBorder;
    mathfu::mat4 *m_modelMatrix;
    int m_groupNumber;

    HGUniformBuffer vertexModelWideUniformBuffer = nullptr;
    std::vector<HGMesh> m_meshArray;

    SMOGroupInfo *m_main_groupInfo;

    std::vector <M2Object *> m_doodads = std::vector<M2Object *>(0);

    bool m_dontUseLocalLightingForM2 = false;

    bool m_loading = false;
    bool m_loaded = false;

    bool m_recalcBoundries = false;

    void startLoading();
    void createWorldGroupBB (CAaBox &bbox, mathfu::mat4 &placementMatrix);

    void updateWorldGroupBBWithM2();
    bool checkDoodads(std::vector<M2Object*> &wmoM2Candidates);

    void postLoad();
    void createMeshes();

    void loadDoodads();

    bool checkIfInsidePortals(mathfu::vec3 point, SMOPortal *portalInfos, SMOPortalRef *portalRels);


    static void queryBspTree(CAaBox &bbox, int nodeId, t_BSP_NODE *nodes, std::vector<int> &bspLeafIdList);

    bool getTopAndBottomTriangleFromBsp(mathfu::vec4 &cameraLocal, SMOPortal *portalInfos,
                                        SMOPortalRef *portalRels, std::vector<int> &bspLeafList, M2Range &result);
};


#endif //WEBWOWVIEWERCPP_WMOGROUPOBJECT_H
