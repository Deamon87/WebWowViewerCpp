//
// Created by Deamon on 9/5/2017.
//

#ifndef WOWVIEWERLIB_IWMOAPI_H
#define WOWVIEWERLIB_IWMOAPI_H

#include "m2/m2Object.h"

struct PortalInfo_t {
    std::vector<mathfu::vec3> sortedVericles;
    CAaBox aaBox;
};


class IWmoApi {
public:
    virtual M2Object *getDoodad(int index) = 0;
    virtual SMOHeader *getWmoHeader() = 0;
    virtual SMOLight *getLightArray() = 0;

    virtual std::vector<PortalInfo_t> &getPortalInfos() = 0;

    virtual void updateBB() = 0;
    virtual void postWmoGroupObjectLoad(int groupId, int lod) = 0;

};
#endif //WOWVIEWERLIB_IWMOAPI_H
