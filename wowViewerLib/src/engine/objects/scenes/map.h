//
// Created by Deamon on 7/16/2017.
//

#ifndef WEBWOWVIEWERCPP_MAP_H
#define WEBWOWVIEWERCPP_MAP_H


#include <unordered_set>
#include <map>
#include <set>
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
#include "../liquid/liquidMaterials/LiquidMaterialManager.h"
#include "../worldObject/WorldObjectManager.h"

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
    std::unique_ptr<LiquidMaterialManager> m_liquidMaterialManager;

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

    std::shared_ptr<WdlObject> m_wdlObject = nullptr;
    std::shared_ptr<WdtLightsObject> m_wdtLightObject = nullptr;

    int m_viewRenderOrder = 0;

    HGMesh skyMesh = nullptr;
    std::shared_ptr<ISkyMeshMaterial> skyMeshMat = nullptr;

    HGMesh skyMesh0x4Sky = nullptr;
    std::shared_ptr<ISkyMeshMaterial> skyMeshMat0x4 = nullptr;

    // Planets (sun/moon discs) + stars
    std::array<HGSamplableTexture, 3> m_planetTextures = {nullptr, nullptr, nullptr}; // sun, moon1, moon2
    std::array<HGMesh, 3> m_planetMeshes = {nullptr, nullptr, nullptr};
    std::array<std::shared_ptr<IPlanetMaterial>, 3> m_planetMats = {nullptr, nullptr, nullptr};
    std::shared_ptr<M2Object> m_starsModel = nullptr;

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

    std::shared_ptr<M2Object> getM2Object(std::string fileName, const SMDoodadDef &doodadDef) override ;
    std::shared_ptr<M2Object> getM2Object(int fileDataId, const SMDoodadDef &doodadDef) override ;
    std::shared_ptr<WmoObject> getWmoObject(std::string fileName, const SMMapObjDef &mapObjDef, const PointerChecker<MWDR> &m_MWDR, const PointerChecker<uint16_t> &m_MWDS) override ;
    std::shared_ptr<WmoObject> getWmoObject(int fileDataId, const SMMapObjDef &mapObjDef, const PointerChecker<MWDR> &m_MWDR, const PointerChecker<uint16_t> &m_MWDS) override ;
    std::shared_ptr<WmoObject> getWmoObject(std::string fileName, const SMMapObjDefObj1 &mapObjDef, const PointerChecker<MWDR> &m_MWDR, const PointerChecker<uint16_t> &m_MWDS) override ;
    std::shared_ptr<WmoObject> getWmoObject(int fileDataId, const SMMapObjDefObj1 &mapObjDef, const PointerChecker<MWDR> &m_MWDR, const PointerChecker<uint16_t> &m_MWDS) override ;


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

    void getPossibleHeight(const mathfu::vec4 &cameraPos, float &height);

    virtual void updateLightAndSkyboxData(const HMapRenderPlan &mapRenderPlan, MathHelper::FrustumCullingData &frustumData,
                                          StateForConditions &stateForConditions, const AreaRecord &areaRecord);

    FreeStrategy adtFreeLambda;
    FreeStrategy zeroStateLambda;

    HADTRenderConfigDataHolder m_adtConfigHolder = nullptr;
    HWorldObjectManager m_worldObjectManager = nullptr;

protected:
    explicit Map(const HApiContainer &m_api) : m_dayNightLightHolder(m_api, -1) {
    }
    DayNightLightHolder m_dayNightLightHolder;
public:
    explicit Map(HApiContainer api, int mapId, const std::string &mapName);

    explicit Map(HApiContainer api, int mapId, int wdtFileDataId);;

    explicit Map(HApiContainer api, std::string adtFileName, int i, int j, std::string mapName);;
    void initialize(HApiContainer api, int mapId);

    ~Map() override {
//        std::cout << "Map destroyed " << std::endl;
	};
    animTime_t getCurrentSceneTime() override;
    mathfu::vec3 getGlobalOffset() override;

    void makeFramePlan(const FrameInputParams<MapSceneParams> &frameInputParams, const HMapRenderPlan &mapRenderPlan);

    void setMandatoryADTs(std::vector<std::array<uint8_t, 2>> &mandatoryADTs) {
        m_mandatoryADT = mandatoryADTs;
    }
    void getAdtAreaId(const mathfu::vec4 &cameraPos, int &areaId, int &parentAreaId);

    void setAdtConfig(HADTRenderConfigDataHolder &adtConfig) {
        m_adtConfigHolder = adtConfig;
    }

    void setWorldObjectManager(HWorldObjectManager worldObjectManager) {
        m_worldObjectManager = worldObjectManager;
    }
    HWorldObjectManager getWorldObjectManager() const {
        return m_worldObjectManager;
    }

    // Sky scene ids of the map's WDL grouped by the player condition that gates them
    // (SkySceneXPlayerCondition db2). Empty when the map has no WDL. Built once at WDL
    // load and static for the WDL's lifetime.
    const std::map<int, std::set<int>> &getSkyScenesByPlayerCondition() const {
        static const std::map<int, std::set<int>> empty = {};
        return m_wdlObject ? m_wdlObject->getSkyScenesByPlayerCondition() : empty;
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
