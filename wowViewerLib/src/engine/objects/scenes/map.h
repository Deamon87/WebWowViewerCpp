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
#include "tbb/tbb.h"
#include "../../algorithms/FrameCounter.h"
#include "../../../renderer/frame/FrameInputParams.h"
#include "../../../renderer/mapScene/MapScenePlan.h"
#include "../../../renderer/mapScene/MapSceneParams.h"
#include "../wdt/wdtLightsObject.h"
#include "dayNightDataHolder/DayNightLightHolder.h"

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
    HApiContainer m_api = nullptr;
    std::array<std::array<std::shared_ptr<AdtObject>, 64>, 64> mapTiles={};
    std::vector<std::array<uint8_t, 2>> m_mandatoryADT;
    std::string mapName;

    std::shared_ptr<IBufferChunk<sceneWideBlockVSPS>> m_sceneWideBlockVSPSChunk;

    SceneMode m_sceneMode = SceneMode::smMap;

    float m_currentTime = 0;

    bool m_lockedMap = false;

    int m_mapId = -1;
    HWdtFile m_wdtfile = nullptr;
    std::shared_ptr<WmoObject> wmoMap = nullptr;

    bool useWeightedBlend = false;
    bool has0x200000Flag = false;

    std::vector<std::shared_ptr<M2Object>> m_exteriorSkyBoxes;

    std::shared_ptr<WdlObject> m_wdlObject = nullptr;
    std::shared_ptr<WdtLightsObject> m_wdtLightObject = nullptr;

    int m_viewRenderOrder = 0;

    HGMesh skyMesh = nullptr;
    std::shared_ptr<ISkyMeshMaterial> skyMeshMat = nullptr;

    HGMesh skyMesh0x4Sky = nullptr;
    std::shared_ptr<ISkyMeshMaterial> skyMeshMat0x4 = nullptr;

    //Map mode
    std::unordered_map<int, std::weak_ptr<M2Object>> m_m2MapObjects = {};
    std::unordered_map<int, std::weak_ptr<WmoObject>> m_wmoMapObjects = {};

    //M2 mode
    std::shared_ptr<M2Object> m_m2Object = nullptr;
    std::string m_m2Model;

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


    virtual void getPotentialEntities(
        const MathHelper::FrustumCullingData &frustumData,
        const mathfu::vec4 &cameraPos,
        const HMapRenderPlan &mapRenderPlan,
        M2ObjectListContainer &potentialM2,
        WMOListContainer &potentialWmo);

    virtual void getCandidatesEntities(const MathHelper::FrustumCullingData &frustumData,
                                       const mathfu::vec4 &cameraPos,
                                       const HMapRenderPlan &mapRenderPlan,
                                       M2ObjectListContainer &m2ObjectsCandidates,
                                       WMOListContainer &wmoCandidates);

    void checkADTCulling(int i, int j,
                         const MathHelper::FrustumCullingData &frustumData,
                         const mathfu::vec4 &cameraPos,
                         const HMapRenderPlan &mapRenderPlan,
                         M2ObjectListContainer &m2ObjectsCandidates,
                         WMOListContainer &wmoCandidates);

    virtual void updateLightAndSkyboxData(const HMapRenderPlan &mapRenderPlan, MathHelper::FrustumCullingData &frustumData,
                                          StateForConditions &stateForConditions, const AreaRecord &areaRecord);

    DayNightLightHolder dayNightLightHolder;

    FreeStrategy adtFreeLambda;
    FreeStrategy zeroStateLambda;

    HADTRenderConfigDataHolder m_adtConfigHolder = nullptr;

protected:
    explicit Map() : dayNightLightHolder(nullptr, -1) {
    }
public:
    explicit Map(HApiContainer api, int mapId, const std::string &mapName);

    explicit Map(HApiContainer api, int mapId, int wdtFileDataId) : dayNightLightHolder(api, mapId) {
        initMapTiles();

        m_mapId = mapId; m_api = api; mapName = "";
        m_sceneMode = SceneMode::smMap;

        MapRecord mapRecord;
        api->databaseHandler->getMapById(mapId, mapRecord);
        useWeightedBlend = (mapRecord.flags0 & 0x4) > 0;
        has0x200000Flag = (mapRecord.flags0 & 0x200000) > 0;

        createAdtFreeLamdas();

        m_wdtfile = api->cacheStorage->getWdtFileCache()->getFileId(wdtFileDataId);

        dayNightLightHolder.loadZoneLights();
    };

    explicit Map(HApiContainer api, std::string adtFileName, int i, int j, std::string mapName) : dayNightLightHolder(api, 0) {
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
        auto adtObject = adtObjectFactory.createObject(m_api, adtFileTemplate, mapName, i, j, false, m_wdtfile);

        adtObject->setMapApi(this);
        this->mapTiles[i][j] = adtObject;
    };

    ~Map() override {
//        std::cout << "Map destroyed " << std::endl;
	};
    animTime_t getCurrentSceneTime() override;

    void makeFramePlan(const FrameInputParams<MapSceneParams> &frameInputParams, const HMapRenderPlan &mapRenderPlan);

    void setMandatoryADTs(std::vector<std::array<uint8_t, 2>> &mandatoryADTs) {
        m_mandatoryADT = mandatoryADTs;
    }
    void getAdtAreaId(const mathfu::vec4 &cameraPos, int &areaId, int &parentAreaId);

    void setAdtConfig(HADTRenderConfigDataHolder &adtConfig) {
        m_adtConfigHolder = adtConfig;
    }

    void doPostLoad(const HMapSceneBufferCreate &sceneRenderer, const HMapRenderPlan &renderPlan);

    void update(const HMapRenderPlan &renderPlan);
    void updateBuffers(const HMapSceneBufferCreate &sceneRenderer, const HMapRenderPlan &renderPlan);
private:
    void checkExterior(mathfu::vec4 &cameraPos,
                       const MathHelper::FrustumCullingData &frustumData,
                       int viewRenderOrder,
                       const HMapRenderPlan &mapRenderPlan);

//    HDrawStage doGaussBlur(const HDrawStage &parentDrawStage, std::vector<HGUniformBufferChunk> &uniformBufferChunks) const;

    void createAdtFreeLamdas();
};
typedef std::shared_ptr<Map> HMapScene;

#endif //WEBWOWVIEWERCPP_MAP_H
