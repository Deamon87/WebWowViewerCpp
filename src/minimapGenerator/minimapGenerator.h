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

struct ScenarioDef {
    int mapId;
    mathfu::vec4 closeOceanColor;
    mathfu::vec4 closeRiverColor;

    mathfu::vec2 minWowWorldCoord;
    mathfu::vec2 maxWowWorldCoord;

    ScenarioOrientation orientation = ScenarioOrientation::so45DegreeTick0;

    std::string folderToSave;
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



    float XToYCoef = 0;
    bool XToYCoefCalculated = false;

    int m_chunkStartX = -32;
    int m_chunkStartY= -32;
    int m_chunkWidth = 64;
    int m_chunkHeight = 64;

    std::list<HDrawStage> drawStageStack = {};
    int framesReady = 0;

    mathfu::mat4 getOrthoMatrix();
    mathfu::vec3 getLookAtVec3();
    mathfu::mat4 getScreenCoordToWoWCoordMatrix();
    void startNextScenario();
public:

    MinimapGenerator(HWoWFilesCacheStorage cacheStorage, std::shared_ptr<IDevice> hDevice, HRequestProcessor processor, IClientDatabase*);

    void startScenarios(std::vector<ScenarioDef> &scenarioListToProcess);
    void process();
    bool isDone();
    void setupCameraData();
    HDrawStage createSceneDrawStage(HFrameScenario sceneScenario);

    float GetOrthoDimension();

    void calcXtoYCoef();

    void setMinMaxXYWidhtHeight(const mathfu::vec2 &minWowWorldCoord, const mathfu::vec2 &maxWowWorldCoord);

    int XNumbering();
    int YNumbering();

    float getYScreenSpaceDimension();

    float getXScreenSpaceDimension();
};

typedef std::shared_ptr<MinimapGenerator> HMinimapGenerator;

#endif //AWEBWOWVIEWERCPP_MINIMAPGENERATOR_H
