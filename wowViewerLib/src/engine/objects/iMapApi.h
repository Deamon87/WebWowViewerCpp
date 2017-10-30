//
// Created by deamon on 21.08.17.
//

#ifndef WOWVIEWERLIB_IMAPAPI_H
#define WOWVIEWERLIB_IMAPAPI_H

class IMapApi;
#include "m2Object.h"

class IMapApi {
public:
    virtual M2Object *getM2Object(std::string fileName, SMDoodadDef &doodadDef) = 0;
    virtual M2Object *getM2Object(int fileDataId, SMDoodadDef &doodadDef) = 0;
    virtual WmoObject *getWmoObject(std::string fileName, SMMapObjDef &mapObjDef) = 0;
};


#endif //WOWVIEWERLIB_IMAPAPI_H
