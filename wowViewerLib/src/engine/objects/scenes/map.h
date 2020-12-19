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
#include "../iScene.h"
#include "../objectCache.h"
#include "../wdl/wdlObject.h"
#include "../wowFrameData.h"
#include "../../SceneScenario.h"
#include "tbb/tbb.h"
#include "../../algorithms/FrameCounter.h"

enum class SceneMode {
   smMap,
   smM2,
   smWMO
};

class Map : public IMapApi, public IScene {
private:
    void initMapTiles() {
        for (auto &x : mapTiles) {
            for (auto &y : x) {
                y = nullptr;
            }
        }
    }
protected:

    tbb::task_scheduler_init taskScheduler;
    FrameCounter m2UpdateframeCounter;


    HApiContainer m_api = nullptr;
    std::array<std::array<std::shared_ptr<AdtObject>, 64>, 64> mapTiles={};
    std::string mapName;

    SceneMode m_sceneMode = SceneMode::smMap;

    float m_currentTime = 0;
    float m_lastTimeAdtCleanup = 0;

    bool m_lockedMap = false;

    int m_mapId = -1;
    HWdtFile m_wdtfile = nullptr;
    std::shared_ptr<WmoObject> wmoMap = nullptr;


    std::vector<std::shared_ptr<M2Object>> m_exteriorSkyBoxes;

    std::shared_ptr<WdlObject> m_wdlObject = nullptr;

    int m_viewRenderOrder = 0;

    HGVertexBufferBindings quadBindings;
    float m_skyConeAlpha = 0.0;
    HGMesh skyMesh = nullptr;
    HGMesh skyMesh0x4Sky = nullptr;

    //Map mode
    std::unordered_map<int, std::weak_ptr<M2Object>> m_m2MapObjects = {};
    std::unordered_map<int, std::weak_ptr<WmoObject>> m_wmoMapObjects = {};

    //M2 mode
    std::shared_ptr<M2Object> m_m2Object = nullptr;
    std::string m_m2Model;
    int m_cameraView;

    //Wmo mode
    std::shared_ptr<WmoObject> m_wmoObject = nullptr;
    std::string m_wmoModel;
    ///end

    bool m_suppressDrawingSky = false;

    std::shared_ptr<M2Object> getM2Object(std::string fileName, SMDoodadDef &doodadDef) override ;
    std::shared_ptr<M2Object> getM2Object(int fileDataId, SMDoodadDef &doodadDef) override ;
    std::shared_ptr<WmoObject> getWmoObject(std::string fileName, SMMapObjDef &mapObjDef) override ;
    std::shared_ptr<WmoObject> getWmoObject(int fileDataId, SMMapObjDef &mapObjDef) override ;
    std::shared_ptr<WmoObject> getWmoObject(std::string fileName, SMMapObjDefObj1 &mapObjDef) override ;
    std::shared_ptr<WmoObject> getWmoObject(int fileDataId, SMMapObjDefObj1 &mapObjDef) override ;

    int getCameraNum() override {return 0;};
    std::shared_ptr<ICamera> createCamera(int cameraNum) override { return nullptr;};

    animTime_t getCurrentSceneTime() override ;

    virtual void getPotentialEntities(const mathfu::vec4 &cameraPos, std::vector<std::shared_ptr<M2Object>> &potentialM2,
                              HCullStage &cullStage, mathfu::mat4 &lookAtMat4, mathfu::vec4 &camera4,
                              std::vector<mathfu::vec4> &frustumPlanes, std::vector<mathfu::vec3> &frustumPoints,
                              std::vector<std::shared_ptr<WmoObject>> &potentialWmo);

    virtual void getCandidatesEntities(std::vector<mathfu::vec3> &hullLines, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos,
                                       std::vector<mathfu::vec3> &frustumPoints, HCullStage &cullStage,
                                       std::vector<std::shared_ptr<M2Object>> &m2ObjectsCandidates,
                                       std::vector<std::shared_ptr<WmoObject>> &wmoCandidates);

    virtual void updateLightAndSkyboxData(const HCullStage &cullStage, mathfu::vec3 &cameraVec3,
                                          StateForConditions &stateForConditions, const AreaRecord &areaRecord);

    struct mapInnerZoneLightRecord {
        int ID;
        std::string name;
        int LightID;
        CAaBox aabb;
        std::vector<mathfu::vec2> points;
        std::vector<mathfu::vec2> lines;
    };
    std::vector<mapInnerZoneLightRecord> m_zoneLights;
    void loadZoneLights();

public:

    explicit Map() : taskScheduler(10){
    }

    explicit Map(HApiContainer api, int mapId, std::string mapName) : taskScheduler(10) {
        initMapTiles();

        m_mapId = mapId; m_api = api; this->mapName = mapName;
        m_sceneMode = SceneMode::smMap;

        std::string wdtFileName = "world/maps/"+mapName+"/"+mapName+".wdt";
        std::string wdlFileName = "world/maps/"+mapName+"/"+mapName+".wdl";

        m_wdtfile = api->cacheStorage->getWdtFileCache()->get(wdtFileName);
        m_wdlObject = std::make_shared<WdlObject>(api, wdlFileName);
        m_wdlObject->setMapApi(this);

        loadZoneLights();
    };

    explicit Map(HApiContainer api, int mapId, int wdtFileDataId) {
        initMapTiles();

        m_mapId = mapId; m_api = api; mapName = "";
        m_sceneMode = SceneMode::smMap;

        m_wdtfile = api->cacheStorage->getWdtFileCache()->getFileId(wdtFileDataId);

        loadZoneLights();
    };

    explicit Map(HApiContainer api, std::string adtFileName, int i, int j, std::string mapName) {
        initMapTiles();

        m_mapId = 0; m_api = api; this->mapName = mapName;
        m_sceneMode = SceneMode::smMap;

        std::string wdtFileName = "world/maps/"+mapName+"/"+mapName+".wdt";
        std::string wdlFileName = "world/maps/"+mapName+"/"+mapName+".wdl";

        m_wdtfile = api->cacheStorage->getWdtFileCache()->get(wdtFileName);
        m_wdlObject = std::make_shared<WdlObject>(api, wdlFileName);
        m_wdlObject->setMapApi(this);

        m_lockedMap = true;
        std::string adtFileTemplate = "world/maps/"+mapName+"/"+mapName+"_"+std::to_string(i)+"_"+std::to_string(j);
        auto adtObject = std::make_shared<AdtObject>(m_api, adtFileTemplate, mapName, i, j, m_wdtfile);

        adtObject->setMapApi(this);
        this->mapTiles[i][j] = adtObject;
    };

    ~Map() override {

	} ;

    void setReplaceTextureArray(std::vector<int> &replaceTextureArray) override {};
    void setMeshIdArray(std::vector<uint8_t> &meshIds) override {};
    void checkCulling(HCullStage cullStage) override;


    void setAnimationId(int animationId) override {

    };
    void resetAnimation() override {

    }


    void doPostLoad(HCullStage cullStage) override;

    void update(HUpdateStage updateStage);
    void updateBuffers(HUpdateStage updateStage) override;
    void produceUpdateStage(HUpdateStage updateStage) override;
    void produceDrawStage(HDrawStage resultDrawStage, HUpdateStage updateStage, std::vector<HGUniformBufferChunk> &additionalChunks) override;
private:
    void checkExterior(mathfu::vec4 &cameraPos,
                       std::vector<mathfu::vec3> &frustumPoints,
                       std::vector<mathfu::vec3> &hullLines,
                       mathfu::mat4 &lookAtMat4,
                       mathfu::mat4 &viewPerspectiveMat,
                       int viewRenderOrder,
                       HCullStage cullStage);

    HDrawStage doGaussBlur(const HDrawStage parentDrawStage, HUpdateStage &updateStage) const;


    void getLightResultsFromDB(mathfu::vec3 &cameraVec3, const Config *config, std::vector<LightResult> &lightResults) override;
};

#endif //WEBWOWVIEWERCPP_MAP_H
