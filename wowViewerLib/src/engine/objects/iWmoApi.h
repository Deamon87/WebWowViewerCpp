//
// Created by Deamon on 9/5/2017.
//

#ifndef WOWVIEWERLIB_IWMOAPI_H
#define WOWVIEWERLIB_IWMOAPI_H

#include "m2Object.h"

class IWmoApi {
public:
    virtual M2Object *getDoodad(int index) = 0;
    virtual SMOHeader *getWmoHeader() = 0;
    virtual SMOLight *getLightArray() = 0;

    virtual void postWmoGroupObjectLoad(int groupId, int lod) = 0;
};
#endif //WOWVIEWERLIB_IWMOAPI_H
