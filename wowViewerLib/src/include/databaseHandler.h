//
// Created by Deamon on 12/30/2019.
//

#ifndef AWEBWOWVIEWERCPP_DATABASEHANDLER_H
#define AWEBWOWVIEWERCPP_DATABASEHANDLER_H

#include <vector>
#include <string>
#include "database/dbStructs.h"

class IClientDatabase {
public:
    virtual void getMapArray(std::vector<MapRecord> &mapRecords) = 0;
    virtual bool getMapById(int mapId, MapRecord &mapRecord) = 0;
    virtual AreaRecord getArea(int areaId) = 0;
    virtual bool getWmoArea(int wmoId, int nameId, int groupId, AreaRecord &result) = 0;

    virtual void getAllLightByMap(int mapId, std::vector<LightResult> &lightResults) = 0;
    virtual void getLightById(int lightId, LightResult &lightResult) = 0;
    virtual void getEnvInfo(int mapId, float x, float y, float z, std::vector<LightResult> &lightResults) = 0;

    virtual bool getLightParamData(int lightParamId, int time, LightParamData &lightParamData) = 0;

    virtual void getLiquidObjectData(int liquidObjectId, int fallbackliquidTypeId, LiquidObjectRec &loData) = 0;
    virtual void getLiquidTypeData(int liquidTypeId, LiquidTypeAndMat &loData, std::vector<LiquidTextureData> &textures) = 0;
    virtual void getZoneLightsForMap(int mapId, std::vector<ZoneLight> &zoneLights) = 0;

    virtual void getGameObjectsForMap(int mapId, std::vector<GameObjectRecord> &gameObjects) = 0;
    virtual bool getGameObjectDisplayInfo(int displayId, GameObjectDisplayInfoRecord &result) = 0;

    // All SkySceneXPlayerCondition rows (skySceneID -> playerConditionID assignments).
    // Implementations are expected to cache: this is queried per loaded sky scene.
    virtual void getSkySceneXPlayerConditions(std::vector<SkySceneXPlayerConditionRecord> &result) = 0;
};

#endif //AWEBWOWVIEWERCPP_DATABASEHANDLER_H
