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
#include "../wowFrameData.h"

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

    bool m_lockedMap = false;

    std::vector<WmoGroupResult> m_currentInteriorGroups;
    WmoObject *m_currentWMO = nullptr;

    int m_mapId = -1;
    HWdtFile m_wdtfile;
    WdlObject * m_wdlObject;

    int m_viewRenderOrder = 0;

    ObjectCache<M2Object, int> m_m2MapObjects;
    ObjectCache<WmoObject, int> m_wmoMapObjects;

    std::vector<M2InstancingObject*> m_instanceList;
    std::unordered_map<std::string, M2InstancingObject*> m_instanceMap;

    M2Object *getM2Object(std::string fileName, SMDoodadDef &doodadDef) override ;
    M2Object *getM2Object(int fileDataId, SMDoodadDef &doodadDef) override ;
    WmoObject *getWmoObject(std::string fileName, SMMapObjDef &mapObjDef) override ;
    WmoObject *getWmoObject(int fileDataId, SMMapObjDef &mapObjDef) override ;
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

    explicit Map(IWoWInnerApi *api, std::string adtFileName, int i, int j, std::string mapName) : m_mapId(0), m_api(api), mapName(mapName){
        std::string wdtFileName = "world/maps/"+mapName+"/"+mapName+".wdt";
        std::string wdlFileName = "world/maps/"+mapName+"/"+mapName+".wdl";

        m_wdtfile = api->getWdtFileCache()->get(wdtFileName);
        m_wdlObject = new WdlObject(api, wdlFileName);
        m_wdlObject->setMapApi(this);

        m_lockedMap = true;
        std::string adtFileTemplate = "world/maps/"+mapName+"/"+mapName+"_"+std::to_string(i)+"_"+std::to_string(j);
        AdtObject * adtObject = new AdtObject(m_api, adtFileTemplate, mapName, i, j, m_wdtfile);

        adtObject->setMapApi(this);
        this->mapTiles[i][j] = adtObject;
    };

    void setReplaceTextureArray(std::vector<int> &replaceTextureArray) override {};
    void checkCulling(WoWFrameData *frameData) override;

    void collectMeshes(WoWFrameData *frameData) override;
    void draw(WoWFrameData *frameData) override;


    void doPostLoad(WoWFrameData *frameData) override;

    void update(WoWFrameData *frameData) override;
    mathfu::vec4 getAmbientColor() override {
        return m_api->getGlobalAmbientColor();
    };
    bool getCameraSettings(M2CameraResult &cameraResult) override{
        return false;
    };
    void setAmbientColorOverride(mathfu::vec4 &ambientColor, bool override) override {};
private:
    void checkExterior(mathfu::vec4 &cameraPos,
                       std::vector<mathfu::vec3> &frustumPoints,
                       std::vector<mathfu::vec3> &hullLines,
                       mathfu::mat4 &lookAtMat4,
                       mathfu::mat4 &projectionModelMat,
                       int viewRenderOrder,
                       WoWFrameData *frameData);
};


#endif //WEBWOWVIEWERCPP_MAP_H
