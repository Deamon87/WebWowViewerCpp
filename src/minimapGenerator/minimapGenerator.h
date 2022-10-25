//
// Created by Deamon on 9/8/2020.
//

#ifndef AWEBWOWVIEWERCPP_MINIMAPGENERATOR_H
#define AWEBWOWVIEWERCPP_MINIMAPGENERATOR_H

#include "../../wowViewerLib/src/engine/ApiContainer.h"
#include "../../wowViewerLib/src/engine/SceneScenario.h"
#include "../persistance/RequestProcessor.h"
#include "entities.h"


class MinimapGenerator {
private:
    HApiContainer m_apiContainer;
    HRequestProcessor m_processor;

    std::vector<ScenarioDef> scenarioListToProcess;
    ScenarioDef currentScenario;

    int m_width = 1024;
    int m_height = 1024;

    int m_mapIndex = 0;

    //Position that's being rendered
    int m_x = 0;
    int m_y = 0;

    float m_zoom;

    int m_chunkStartX = -32;
    int m_chunkStartY= -32;
    int m_chunkWidth = 64;
    int m_chunkHeight = 64;

    EMGMode m_mgMode = EMGMode::eNone;

    HDrawStage m_lastDraw = nullptr;

    struct PerSceneData {
        int mapIndex = -1;
        HScene scene;
        //Per X dimension, per Y dimension, vector of mandatory adt {x, y} coordinates
        std::vector<std::vector<std::vector<std::array<uint8_t, 2>>>> mandatoryADTMap;
    };

    std::vector<PerSceneData> mapRuntimeInfo;

    std::array<HCullStage, 4> stackOfCullStages;

    std::vector<HUpdateStage> m_candidateUS = {};
    HDrawStage m_candidateDS = nullptr;
    std::vector<HCullStage> m_candidateCS = {};

    float m_zFar = 3000.0f;
    float m_maxZ = 1000.0f;
    float m_minZ = -1000.0f;


    int framesReady = 0;
    bool prepearCandidate = false;

    mathfu::mat4 getOrthoMatrix();
    mathfu::vec3 getLookAtVec3();
    void startNextScenario();
    void resetCandidate();

    void calcBB(const HCullStage &cullStage, mathfu::vec3 &minCoord,
                mathfu::vec3 &maxCoord, const CAaBox &adtBox2d,
                int adt_x, int adt_y, bool applyAdtChecks);

    mathfu::mat4 genTempProjectMatrix();
public:

    MinimapGenerator(HWoWFilesCacheStorage cacheStorage,
                     const HGDevice &hDevice,
                     HRequestProcessor processor,
                     std::shared_ptr<IClientDatabase> dbhandler);

    void startScenarios(std::vector<ScenarioDef> &scenarioListToProcess);
    void process();
    EMGMode getCurrentMode() { return m_mgMode;}
    void setupCameraData();
    HDrawStage createSceneDrawStage(HFrameScenario sceneScenario);
    HDrawStage getLastDrawStage();
    Config *getConfig();

    void getCurrentTileCoordinates(int &x, int &y, int &maxX, int &maxY) {
        x = m_x;
        y = m_y;
        maxX = m_chunkWidth;
        maxY = m_chunkHeight;
    }

    void getCurrentFDData(int &areaId, int &parentAreaId, mathfu::vec4 &riverColor);

    float GetOrthoDimension();

    void calcXtoYCoef();

    void setMinMaxXYWidthHeight(const mathfu::vec2 &minWowWorldCoord, const mathfu::vec2 &maxWowWorldCoord);

    void setupScenarioData();

    void setLookAtPoint(float x, float y);
    void setZoom(float zoom);

    void reload();

    void startPreview(ScenarioDef &scenarioDef);
    void stopPreview();

    void startBoundingBoxCalc(ScenarioDef &scenarioDef);
    void stopBoundingBoxCalc();

    void saveDrawStageToFile(std::string folderToSave, const std::shared_ptr<DrawStage> &lastFrameIt);

    void setupCamera(const mathfu::vec3 &lookAtPoint2D, std::shared_ptr<ICamera> &camera);
};

typedef std::shared_ptr<MinimapGenerator> HMinimapGenerator;

#endif //AWEBWOWVIEWERCPP_MINIMAPGENERATOR_H
