//
// Created by deamon on 21.08.17.
//

#ifndef WOWVIEWERLIB_IMAPAPI_H
#define WOWVIEWERLIB_IMAPAPI_H

class IMapApi;
#include "m2/m2Object.h"
#include "wmo/wmoObject.h"

struct StateForConditions {
    int currentAreaId = 0;
    int currentParentAreaId = 0;
    std::vector<int> currentSkyboxIds = {};
    std::vector<int> currentLightParams = {};
    std::vector<int> currentZoneLights = {};
};

class IMapApi {
public:
    virtual std::shared_ptr<M2Object> getM2Object(std::string fileName, SMDoodadDef &doodadDef) = 0;
    virtual std::shared_ptr<M2Object> getM2Object(int fileDataId, SMDoodadDef &doodadDef) = 0;
    virtual std::shared_ptr<WmoObject> getWmoObject(std::string fileName, SMMapObjDef &mapObjDef) = 0;
    virtual std::shared_ptr<WmoObject> getWmoObject(int fileDataId, SMMapObjDef &mapObjDef) = 0;
    virtual std::shared_ptr<WmoObject> getWmoObject(std::string fileName, SMMapObjDefObj1 &mapObjDef) = 0;
    virtual std::shared_ptr<WmoObject> getWmoObject(int fileDataId, SMMapObjDefObj1 &mapObjDef) = 0;
    virtual void getLightResultsFromDB(mathfu::vec3 &cameraVec3, const Config *config, std::vector<LightResult> &lightResults,StateForConditions *stateForConditions) = 0;


    virtual animTime_t getCurrentSceneTime() = 0;
};


#endif //WOWVIEWERLIB_IMAPAPI_H
