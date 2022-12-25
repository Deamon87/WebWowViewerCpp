//
// Created by Deamon on 9/5/2017.
//

#ifndef WOWVIEWERLIB_IWMOAPI_H
#define WOWVIEWERLIB_IWMOAPI_H

#include <functional>
#include "../engineClassList.h"
#include "m2/m2Object.h"

struct PortalInfo_t {
    std::vector<mathfu::vec3> sortedVericles;
    CAaBox aaBox;
};

struct WmoGroupResult {
    M2Range topBottom;
    int groupIndex;
    int WMOGroupID;
    std::vector<int> bspLeafList;
    int nodeId;
};

class IWmoApi {
public:
    virtual std::shared_ptr<M2Object> getDoodad(int index) = 0;
    virtual SMOHeader *getWmoHeader() = 0;
    virtual mathfu::vec3 getAmbientColor() = 0;
    virtual PointerChecker<SMOMaterial> &getMaterials() = 0;
    virtual bool isLoaded() = 0;
    virtual std::function<void (WmoGroupGeom& wmoGroupGeom)> getAttenFunction() = 0;
    virtual PointerChecker<SMOLight> &getLightArray() = 0;

    virtual std::vector<PortalInfo_t> &getPortalInfos() = 0;

    virtual HGTexture getTexture(int textureId, bool isSpec) = 0;
    virtual void updateBB() = 0;
    virtual void postWmoGroupObjectLoad(int groupId, int lod) = 0;

};
#endif //WOWVIEWERLIB_IWMOAPI_H
