//
// Created by Deamon on 7/16/2017.
//

#ifndef WEBWOWVIEWERCPP_MAP_H
#define WEBWOWVIEWERCPP_MAP_H


#include "adtObject.h"
#include "m2Object.h"
#include "wmoObject.h"
#include "iMapApi.h"
#include "iInnerSceneApi.h"
#include "objectCache.h"
#include "m2Instancing/m2InstancingObject.h"

class Map : public IMapApi, public iInnerSceneApi {
private:
    IWoWInnerApi *m_api;
    AdtObject *mapTiles[64][64]={};
    std::string mapName;

    float m_currentTime = 0;
    float lastInstanceCollect = 0;
    float m_lastTimeSort = 0;
    float m_lastTimeDistanceCalc = 0;

    std::vector<WmoGroupResult> m_currentInteriorGroups;
    WmoObject *m_currentWMO = nullptr;


    ObjectCache<M2Object, int> m_m2MapObjects;
    ObjectCache<WmoObject, int> m_wmoMapObjects;

    std::set<AdtObject*> adtRenderedThisFrame;
    std::set<M2Object*> m2RenderedThisFrame;
    std::set<WmoObject*> wmoRenderedThisFrame;

    std::vector<AdtObject*> adtRenderedThisFrameArr;
    std::vector<M2Object*> m2RenderedThisFrameArr;
    std::vector<WmoObject*> wmoRenderedThisFrameArr;

    std::set<M2Object *> m2OpaqueRenderedThisFrame;
    std::set<M2Object *> m2TranspRenderedThisFrame;

    std::vector<M2InstancingObject*> m_instanceList;
    std::unordered_map<std::string, M2InstancingObject*> m_instanceMap;

    M2Object *getM2Object(std::string fileName, SMDoodadDef &doodadDef);
    WmoObject *getWmoObject(std::string fileName, SMMapObjDef &mapObjDef);
public:
    Map(IWoWInnerApi *api, std::string mapName) : m_api(api), mapName(mapName){

    };

    void checkCulling(mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos) override;
    void draw() override;

    void update(double deltaTime, mathfu::vec3 &cameraVec3, mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat) override;
private:
    void checkExterior(mathfu::vec4 &cameraPos,
                       std::vector<mathfu::vec4> &frustumPlanes,
                       std::vector<mathfu::vec3> &frustumPoints,
                       std::vector<mathfu::vec3> &hullLines,
                       mathfu::mat4 &lookAtMat4,
                       mathfu::mat4 &projectionModelMat,
                       std::set<AdtObject*> &adtRenderedThisFrame,
                       std::set<M2Object*> &m2RenderedThisFrame,
                       std::set<WmoObject*> &wmoRenderedThisFrame);



    void drawExterior();
    void drawM2s();


    void addM2ObjectToInstanceManager(M2Object *m2Object);
};


#endif //WEBWOWVIEWERCPP_MAP_H
