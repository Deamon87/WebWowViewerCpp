//
// Created by Deamon on 9/8/2020.
//

#ifndef AWEBWOWVIEWERCPP_MINIMAPGENERATOR_H
#define AWEBWOWVIEWERCPP_MINIMAPGENERATOR_H

#include "../../wowViewerLib/src/engine/ApiContainer.h"
#include "../../wowViewerLib/src/engine/SceneScenario.h"
#include "../persistance/RequestProcessor.h"

enum class ScenarioOrientation {
    soTopDownOrtho,
    so45DegreeTick0,
    so45DegreeTick1,
    so45DegreeTick2,
    so45DegreeTick3,
};
enum class EMGMode {
    eNone,
    eBoundingBoxCalculation,
    eScreenshotGeneration,
    ePreview,
};

struct ScenarioDef {
    EMGMode mode;
    int mapId;
    mathfu::vec4 closeOceanColor;
    mathfu::vec4 closeRiverColor;

    mathfu::vec2 minWowWorldCoord;
    mathfu::vec2 maxWowWorldCoord;

    int imageHeight;
    int imageWidth;

    float zoom = 1.0f;
    bool doBoundingBoxPreCalc = false;

    ScenarioOrientation orientation = ScenarioOrientation::so45DegreeTick0;

    std::string folderToSave;

    HADTBoundingBoxHolder boundingBoxHolder = nullptr;
};



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


    float XToYCoef = 0;
    bool XToYCoefCalculated = false;

    int m_chunkStartX = -32;
    int m_chunkStartY= -32;
    int m_chunkWidth = 64;
    int m_chunkHeight = 64;

    EMGMode m_mgMode = EMGMode::eNone;

    HDrawStage m_lastDraw = nullptr;
    std::list<HDrawStage> drawStageStack = {};
    std::list<HCullStage> cullStageStack = {};
    int framesReady = 0;
    bool pauseAddingToStack = false;

    mathfu::mat4 getOrthoMatrix();
    mathfu::vec3 getLookAtVec3();
    mathfu::mat4 getScreenCoordToWoWCoordMatrix();
    void startNextScenario();
public:

    MinimapGenerator(HWoWFilesCacheStorage cacheStorage, std::shared_ptr<IDevice> hDevice, HRequestProcessor processor, IClientDatabase*);

    void startScenarios(std::vector<ScenarioDef> &scenarioListToProcess);
    void process();
    bool isDone();
    EMGMode getCurrentMode() { return m_mgMode;}
    void setupCameraData();
    HDrawStage createSceneDrawStage(HFrameScenario sceneScenario);
    HDrawStage getLastDrawStage();
    Config *getConfig();

    void getCurrentTileCoordinates(int &x, int &y, int &maxX, int &maxY) {
        x = (XNumbering() > 0) ? (m_x - m_chunkStartX) : ((m_chunkWidth  - 1) - (m_x - m_chunkStartX));
        y = (YNumbering() > 0) ? (m_y - m_chunkStartY) : ((m_chunkHeight - 1) - (m_y - m_chunkStartY));
        maxX = m_chunkWidth;
        maxY = m_chunkHeight;
    }

    float GetOrthoDimension();

    void calcXtoYCoef();

    void setMinMaxXYWidhtHeight(const mathfu::vec2 &minWowWorldCoord, const mathfu::vec2 &maxWowWorldCoord);

    int XNumbering();
    int YNumbering();

    float getYScreenSpaceDimension();
    float getXScreenSpaceDimension();


    void setupScenarioData();

    void setLookAtPoint(float x, float y);
    void setZoom(float zoom);

    void startPreview(ScenarioDef &scenarioDef);
    void stopPreview();

    void startBoundingBoxCalc(ScenarioDef &scenarioDef);
    void stopBoundingBoxCalc();

    void saveDrawStageToFile(std::string folderToSave, const std::shared_ptr<DrawStage> &lastFrameIt);
};

typedef std::shared_ptr<MinimapGenerator> HMinimapGenerator;

#endif //AWEBWOWVIEWERCPP_MINIMAPGENERATOR_H
