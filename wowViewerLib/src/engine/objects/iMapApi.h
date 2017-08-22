//
// Created by deamon on 21.08.17.
//

#ifndef WOWVIEWERLIB_IMAPAPI_H
#define WOWVIEWERLIB_IMAPAPI_H

class IMapApi;
#include "m2Object.h"

class IMapApi {
public:
    virtual M2Object getM2Object(SMDoodadDef &doodadDef) = 0;
    virtual M2Object getWmoObject(SMMapObjDef &mapObjDef) = 0;
};


#endif //WOWVIEWERLIB_IMAPAPI_H
