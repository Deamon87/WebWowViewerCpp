//
// Created by Deamon on 7/16/2017.
//

#ifndef WEBWOWVIEWERCPP_MAP_H
#define WEBWOWVIEWERCPP_MAP_H


#include <unordered_set>
#include "../adt/adtObject.h"
#include "../m2/m2Object.h"
#include "../wmo/wmoObject.h"
#include "../iMapApi.h"
#include "../iInnerSceneApi.h"
#include "../objectCache.h"
#include "../m2/m2Instancing/m2InstancingObject.h"
#include "../wdl/wdlObject.h"

class Map : public IMapApi, public iInnerSceneApi {
private:
    IWoWInnerApi *m_api;
    AdtObject *mapTiles[64][64]={};
    std::string mapName;

    float m_currentTime = 0;
    float lastInstanceCollect = 0;
    float m_lastTimeSort = 0;
    float m_lastTimeDistanceCalc = 0;
    float m_lastTimeLightCheck = 0;

    std::vector<WmoGroupResult> m_currentInteriorGroups;
    WmoObject *m_currentWMO = nullptr;

    int m_mapId = -1;
    WdtFile * m_wdtfile;
    WdlObject * m_wdlObject;

    ObjectCache<M2Object, int> m_m2MapObjects;
    ObjectCache<WmoObject, int> m_wmoMapObjects;

    std::unordered_set<AdtObject*> adtRenderedThisFrame;
    std::unordered_set<M2Object*> m2RenderedThisFrame;
    std::unordered_set<WmoObject*> wmoRenderedThisFrame;

    std::vector<AdtObject*> adtRenderedThisFrameArr;
    std::vector<M2Object*> m2RenderedThisFrameArr;
    std::vector<WmoObject*> wmoRenderedThisFrameArr;

    std::unordered_set<M2Object *> m2OpaqueRenderedThisFrame;
    std::unordered_set<M2Object *> m2TranspRenderedThisFrame;

    std::vector<M2InstancingObject*> m_instanceList;
    std::unordered_map<std::string, M2InstancingObject*> m_instanceMap;

    M2Object *getM2Object(std::string fileName, SMDoodadDef &doodadDef) override ;
    M2Object *getM2Object(int fileDataId, SMDoodadDef &doodadDef) override ;
    WmoObject *getWmoObject(std::string fileName, SMMapObjDef &mapObjDef) override ;
    WmoObject *getWmoObject(std::string fileName, SMMapObjDefObj1 &mapObjDef) override ;
    WmoObject *getWmoObject(int fileDataId, SMMapObjDefObj1 &mapObjDef) override ;
public:
    explicit Map(IWoWInnerApi *api, int mapId, std::string mapName) : m_mapId(mapId), m_api(api), mapName(mapName){
        std::string wdtFileName = "world/maps/"+mapName+"/"+mapName+".wdt";
        std::string wdlFileName = "world/maps/"+mapName+"/"+mapName+".wdl";

        m_wdtfile = api->getWdtFileCache()->get(wdtFileName);
        m_wdlObject = new WdlObject(api, wdlFileName);
        m_wdlObject->setMapApi(this);
    };

    void checkCulling(mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos) override;
    void draw() override;

    void update(double deltaTime, mathfu::vec3 &cameraVec3, mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat) override;
    mathfu::vec4 getAmbientColor() override {
        return m_api->getGlobalAmbientColor();
    };
    bool getCameraSettings(M2CameraResult &cameraResult) override{
        return false;
    };
    void setAmbientColorOverride(mathfu::vec4 &ambientColor, bool override) override {};
private:
    void checkExterior(mathfu::vec4 &cameraPos,
                       std::vector<mathfu::vec4> &frustumPlanes,
                       std::vector<mathfu::vec3> &frustumPoints,
                       std::vector<mathfu::vec3> &hullLines,
                       mathfu::mat4 &lookAtMat4,
                       mathfu::mat4 &projectionModelMat,
                       std::unordered_set<AdtObject*> &adtRenderedThisFrame,
                       std::unordered_set<M2Object*> &m2RenderedThisFrame,
                       std::unordered_set<WmoObject*> &wmoRenderedThisFrame);

    void drawExterior();
    void drawM2s();

    void addM2ObjectToInstanceManager(M2Object *m2Object);
};


#endif //WEBWOWVIEWERCPP_MAP_H
