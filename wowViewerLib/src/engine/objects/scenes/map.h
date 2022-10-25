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
#include "../../SceneScenario.h"
#include "tbb/tbb.h"
#include "../../algorithms/FrameCounter.h"

enum class SceneMode {
   smMap,
   smM2,
   smWMO
};

class Map : public IScene, public IMapApi {
private:
    void initMapTiles() {
        for (auto &x : mapTiles) {
            for (auto &y : x) {
                y = nullptr;
            }
        }
    }
protected:
    FrameCounter mapProduceUpdateCounter;
    FrameCounter interiorViewCollectMeshCounter;
    FrameCounter exteriorViewCollectMeshCounter;
    FrameCounter m2CollectMeshCounter;
    FrameCounter sortMeshCounter;
    FrameCounter collectBuffersCounter;
    FrameCounter sortBuffersCounter;

    FrameCounter mapUpdateCounter;
    FrameCounter m2UpdateframeCounter;
    FrameCounter m2calcDistanceCounter;
    FrameCounter adtCleanupCounter;
    FrameCounter wmoGroupUpdate;
    FrameCounter adtUpdate;


    FrameCounter cullCreateVarsCounter;
    FrameCounter cullGetCurrentWMOCounter;
    FrameCounter cullGetCurrentZoneCounter;
    FrameCounter cullUpdateLightsFromDBCounter;
    FrameCounter cullExterior;
    FrameCounter cullExteriorSetDecl;
    FrameCounter cullExteriorWDLCull;
    FrameCounter cullExteriorGetCands;
    FrameCounter cullExterioFrustumWMO;
    FrameCounter cullExterioFrustumM2;
    FrameCounter cullSkyDoms;
    FrameCounter cullCombineAllObjects;

    HApiContainer m_api = nullptr;
    std::array<std::array<std::shared_ptr<AdtObject>, 64>, 64> mapTiles={};
    std::vector<std::array<uint8_t, 2>> m_mandatoryADT;
    std::string mapName;

    SceneMode m_sceneMode = SceneMode::smMap;

    float m_currentTime = 0;

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

    virtual void getPotentialEntities(
        const MathHelper::FrustumCullingData &frustumData,
        const mathfu::vec4 &cameraPos,
        HCullStage &cullStage,
        M2ObjectListContainer &potentialM2,
        WMOListContainer &potentialWmo);

    virtual void getCandidatesEntities(const MathHelper::FrustumCullingData &frustumData,
                                       const mathfu::vec4 &cameraPos,
                                       HCullStage &cullStage,
                                       M2ObjectListContainer &m2ObjectsCandidates,
                                       WMOListContainer &wmoCandidates);

    void checkADTCulling(int i, int j,
                         const MathHelper::FrustumCullingData &frustumData,
                         const mathfu::vec4 &cameraPos,
                         HCullStage &cullStage,
                         M2ObjectListContainer &m2ObjectsCandidates,
                         WMOListContainer &wmoCandidates);

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

    FreeStrategy adtFreeLambda;
    FreeStrategy zeroStateLambda;

    HADTRenderConfigDataHolder m_adtConfigHolder = nullptr;

protected:
    explicit Map() {
    }
public:
    explicit Map(HApiContainer api, int mapId, std::string mapName) {
        initMapTiles();

        m_mapId = mapId; m_api = api; this->mapName = mapName;
        m_sceneMode = SceneMode::smMap;
        createAdtFreeLamdas();

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

        createAdtFreeLamdas();

        m_wdtfile = api->cacheStorage->getWdtFileCache()->getFileId(wdtFileDataId);

        loadZoneLights();
    };

    explicit Map(HApiContainer api, std::string adtFileName, int i, int j, std::string mapName) {
        initMapTiles();

        m_mapId = 0; m_api = api; this->mapName = mapName;
        m_sceneMode = SceneMode::smMap;

        createAdtFreeLamdas();

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
//        std::cout << "Map destroyed " << std::endl;
	};

    void setReplaceTextureArray(std::vector<int> &replaceTextureArray) override {};
    void setMeshIdArray(std::vector<uint8_t> &meshIds) override {};
    void checkCulling(HCullStage &cullStage) override;

    void setMandatoryADTs(std::vector<std::array<uint8_t, 2>> &mandatoryADTs) override {
        m_mandatoryADT = mandatoryADTs;
    }
    void getAdtAreaId(const mathfu::vec4 &cameraPos, int &areaId, int &parentAreaId) override;
    void setAnimationId(int animationId) override {};
    void setMeshIds(std::vector<uint8_t> &meshIds) override {};

    void resetAnimation() override {

    }
    virtual void setAdtConfig(HADTRenderConfigDataHolder &adtConfig) override {
        m_adtConfigHolder = adtConfig;
    }



    void doPostLoad(HCullStage &cullStage) override;

    void update(HUpdateStage &updateStage);
    void updateBuffers(HUpdateStage &updateStage) override;
    void produceUpdateStage(HUpdateStage &updateStage) override;
    void produceDrawStage(HDrawStage &resultDrawStage, std::vector<HUpdateStage> &updateStages, std::vector<HGUniformBufferChunk> &additionalChunks) override;
private:
    void checkExterior(mathfu::vec4 &cameraPos,
                       const MathHelper::FrustumCullingData &frustumData,
                       int viewRenderOrder,
                       HCullStage cullStage);

    HDrawStage doGaussBlur(const HDrawStage &parentDrawStage, HUpdateStage &updateStage) const;


    void getLightResultsFromDB(mathfu::vec3 &cameraVec3, const Config *config, std::vector<LightResult> &lightResults, StateForConditions *stateForConditions) override;

    void createAdtFreeLamdas();
};

#endif //WEBWOWVIEWERCPP_MAP_H
