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

class WmoGroupObject {
public:
    WmoGroupObject(mathfu::mat4 &modelMatrix, IWoWInnerApi *api, std::string fileName, SMOGroupInfo &groupInfo) : m_api(api), m_fileName(fileName){
        m_modelMatrix = &modelMatrix;
        createWorldGroupBB(groupInfo.bounding_box, modelMatrix);
    }

    void draw(SMOMaterial *materials, std::function <BlpTexture&(int materialId)> m_getTextureFunc);

    void update();
    bool checkGroupFrustum(mathfu::vec4 &cameraVec4,
                           std::vector<mathfu::vec4> &frustumPlanes,
                           std::vector<mathfu::vec3> &points,
                           std::set<M2Object*> &wmoM2Candidates);
private:
    IWoWInnerApi *m_api;
    WmoGroupGeom *m_geom = nullptr;
    std::string m_fileName;

    CAaBox m_worldGroupBorder;
    CAaBox m_volumeWorldGroupBorder;
    mathfu::mat4 *m_modelMatrix;

    bool m_dontUseLocalLightingForM2 = false;

    bool m_loading = false;
    bool m_loaded = false;

    void startLoading();
    void createWorldGroupBB (CAaBox &bbox, mathfu::mat4 &placementMatrix);

    void updateWorldGroupBBWithM2();
    bool checkDoodads(std::set<M2Object*> &wmoM2Candidates);

    void postLoad();

};


#endif //WEBWOWVIEWERCPP_WMOGROUPOBJECT_H