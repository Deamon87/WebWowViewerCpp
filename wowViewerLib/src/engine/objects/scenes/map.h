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
#include "../wdl/wdlObject.h"
#include "../wowFrameData.h"

class Map : public IMapApi, public iInnerSceneApi {
private:
    IWoWInnerApi *m_api;
    AdtObject *mapTiles[64][64]={};
    std::string mapName;

    float m_currentTime = 0;
    float m_lastTimeLightCheck = 0;

    bool m_lockedMap = false;

    int m_mapId = -1;
    HWdtFile m_wdtfile;
    WdlObject * m_wdlObject = nullptr;

    int m_viewRenderOrder = 0;

    std::unordered_map<int, std::weak_ptr<M2Object>> m_m2MapObjects = {};
    std::unordered_map<int, std::weak_ptr<WmoObject>> m_wmoMapObjects;

    std::shared_ptr<M2Object> getM2Object(std::string fileName, SMDoodadDef &doodadDef) override ;
    std::shared_ptr<M2Object> getM2Object(int fileDataId, SMDoodadDef &doodadDef) override ;
    std::shared_ptr<WmoObject> getWmoObject(std::string fileName, SMMapObjDef &mapObjDef) override ;
    std::shared_ptr<WmoObject> getWmoObject(int fileDataId, SMMapObjDef &mapObjDef) override ;
    std::shared_ptr<WmoObject> getWmoObject(std::string fileName, SMMapObjDefObj1 &mapObjDef) override ;
    std::shared_ptr<WmoObject> getWmoObject(int fileDataId, SMMapObjDefObj1 &mapObjDef) override ;
public:
    explicit Map(IWoWInnerApi *api, int mapId, std::string mapName) : m_mapId(mapId), m_api(api), mapName(mapName){
        std::string wdtFileName = "world/maps/"+mapName+"/"+mapName+".wdt";
        std::string wdlFileName = "world/maps/"+mapName+"/"+mapName+".wdl";

        m_wdtfile = api->getWdtFileCache()->get(wdtFileName);
        m_wdlObject = new WdlObject(api, wdlFileName);
        m_wdlObject->setMapApi(this);
    };

    explicit Map(IWoWInnerApi *api, int mapId, int wdtFileDataId) : m_mapId(mapId), m_api(api), mapName(""){
        m_wdtfile = api->getWdtFileCache()->getFileId(wdtFileDataId);
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

    ~Map() override {
		for (int i = 0; i < 64; i++) {
			for (int j = 0; j < 64; j++) {
				if (mapTiles[i][j] != nullptr) {
					delete mapTiles[i][j];
				}
			}
		}
	} ;

    void setReplaceTextureArray(std::vector<int> &replaceTextureArray) override {};
    void checkCulling(WoWFrameData *frameData) override;

    void collectMeshes(WoWFrameData *frameData) override;
    void draw(WoWFrameData *frameData) override;
    void setAnimationId(int animationId) override {

    };


    void doPostLoad(WoWFrameData *frameData) override;

    void update(WoWFrameData *frameData) override;
    void updateBuffers(WoWFrameData *frameData) override;
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
                       mathfu::mat4 &viewPerspectiveMat,
                       int viewRenderOrder,
                       WoWFrameData *frameData);
};


#endif //WEBWOWVIEWERCPP_MAP_H
