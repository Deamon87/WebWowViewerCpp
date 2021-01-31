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

    HScene m_currentScene = nullptr;
    int m_width = 1024;
    int m_height = 1024;

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

    HADTBoundingBoxHolder m_boundingBoxHolder = nullptr;

    //Per X dimension, per Y dimension, vector of mandatory adt {x, y} coordinates
    std::vector<std::vector<std::vector<std::array<uint8_t, 2>>>> mandatoryADTMap;

    std::array<HCullStage, 4> stackOfCullStages;

    HDrawStage m_candidateDS = nullptr;
    HCullStage m_candidateCS = nullptr;
    int framesReady = 0;
    bool prepearCandidate = false;

    mathfu::mat4 getOrthoMatrix();
    mathfu::vec3 getLookAtVec3();
    void startNextScenario();

    mathfu::mat4 genTempProjectMatrix();
public:

    MinimapGenerator(HWoWFilesCacheStorage cacheStorage, std::shared_ptr<IDevice> hDevice, HRequestProcessor processor, IClientDatabase*, HADTBoundingBoxHolder boundingBoxHolder);

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

    void setMinMaxXYWidhtHeight(const mathfu::vec2 &minWowWorldCoord, const mathfu::vec2 &maxWowWorldCoord);

    void setupScenarioData();

    void setLookAtPoint(float x, float y);
    void setZoom(float zoom);

    void reload();

    void startPreview(ScenarioDef &scenarioDef);
    void stopPreview();

    void startBoundingBoxCalc(ScenarioDef &scenarioDef);
    void stopBoundingBoxCalc();

    void saveDrawStageToFile(std::string folderToSave, const std::shared_ptr<DrawStage> &lastFrameIt);

};

typedef std::shared_ptr<MinimapGenerator> HMinimapGenerator;

#endif //AWEBWOWVIEWERCPP_MINIMAPGENERATOR_H
