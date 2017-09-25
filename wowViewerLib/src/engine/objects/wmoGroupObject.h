//
// Created by deamon on 10.07.17.
//

#ifndef WEBWOWVIEWERCPP_WMOGROUPOBJECT_H
#define WEBWOWVIEWERCPP_WMOGROUPOBJECT_H

class WmoObject;
class WmoGroupObject;

#include "../wowInnerApi.h"
#include "../persistance/header/wmoFileHeader.h"
#include "wmoObject.h"
#include "iWmoApi.h"

class WmoGroupObject {
public:
    WmoGroupObject(mathfu::mat4 &modelMatrix, IWoWInnerApi *api, std::string fileName, SMOGroupInfo &groupInfo) : m_api(api), m_fileName(fileName){
        m_modelMatrix = &modelMatrix;
        createWorldGroupBB(groupInfo.bounding_box, modelMatrix);
    }

    void draw(SMOMaterial *materials, std::function <BlpTexture&(int materialId)> m_getTextureFunc);
    bool getIsLoaded() { return m_loaded; };
    const WmoGroupGeom *getWmoGroupGeom() const { return m_geom; };
    const std::vector <M2Object *> *getDoodads() const { return &m_doodads; };

    void setWmoApi(IWmoApi *api);
    void update();
    bool checkGroupFrustum(mathfu::vec4 &cameraVec4,
                           std::vector<mathfu::vec4> &frustumPlanes,
                           std::vector<mathfu::vec3> &points,
                           std::set<M2Object*> &wmoM2Candidates);

    bool checkIfInsideGroup(mathfu::vec4 &cameraVec4,
                            mathfu::vec4 &cameraLocal,
                            C3Vector *portalVerticles,
                            SMOPortal *portalInfos,
                            SMOPortalRef *portalRels,
                            std::vector<WmoGroupResult> &candidateGroups);
private:
    IWoWInnerApi *m_api = nullptr;
    IWmoApi *m_wmoApi = nullptr;
    WmoGroupGeom *m_geom = nullptr;
    std::string m_fileName;

    CAaBox m_worldGroupBorder;
    CAaBox m_volumeWorldGroupBorder;
    mathfu::mat4 *m_modelMatrix;

    std::vector <M2Object *> m_doodads = std::vector<M2Object *>(0);

    bool m_dontUseLocalLightingForM2 = false;

    bool m_loading = false;
    bool m_loaded = false;

    void startLoading();
    void createWorldGroupBB (CAaBox &bbox, mathfu::mat4 &placementMatrix);

    void updateWorldGroupBBWithM2();
    bool checkDoodads(std::set<M2Object*> &wmoM2Candidates);

    void postLoad();

    void loadDoodads();

    bool checkIfInsidePortals(mathfu::vec3 point,C3Vector *portalVerticles,
                              SMOPortal *portalInfos, SMOPortalRef *portalRels);

    bool
    checkIfInsidePortals(mathfu::vec4 point, C3Vector *portalVerticles, SMOPortal *portalInfos, SMOPortalRef *portalRels);

    static void queryBspTree(CAaBox &bbox, int nodeId, t_BSP_NODE *nodes, std::vector<int> &bspLeafIdList);

    bool getTopAndBottomTriangleFromBsp(mathfu::vec4 &cameraLocal, C3Vector *portalVerticles, SMOPortal *portalInfos,
                                        SMOPortalRef *portalRels, std::vector<int> &bspLeafList, M2Range &result);
};


#endif //WEBWOWVIEWERCPP_WMOGROUPOBJECT_H
