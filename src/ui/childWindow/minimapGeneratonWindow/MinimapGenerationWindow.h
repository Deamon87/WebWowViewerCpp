//
// Created by Deamon on 10/29/2022.
//

#ifndef AWEBWOWVIEWERCPP_MINIMAPGENERATIONWINDOW_H
#define AWEBWOWVIEWERCPP_MINIMAPGENERATIONWINDOW_H


#include "../../../minimapGenerator/entities.h"
#include "../../../minimapGenerator/minimapGenerator.h"
#include "../../../minimapGenerator/storage/CMinimapDataDB.h"

class MinimapGenerationWindow {
public:
    MinimapGenerationWindow(HApiContainer api, HRequestProcessor processor, bool &showMinimapGeneratorSettings) :
    m_showMinimapGeneratorSettings(showMinimapGeneratorSettings) {
        m_api = api;
        m_processor = processor;

        m_minimapDB = std::make_shared<CMinimapDataDB::DataBaseClass>("minimapdb.sqlite");
        m_minimapDB->getScenarios(sceneDefList);

        createMinimapGenerator();
    }

    void render();
    void process();
    HDrawStage getDrawStage(HFrameScenario sceneScenario);

private:
    HApiContainer m_api;
    HRequestProcessor m_processor;

    bool &m_showMinimapGeneratorSettings;

    std::shared_ptr<CMinimapDataDB::DataBaseClass> m_minimapDB;

    HMinimapGenerator minimapGenerator;
    std::vector<ScenarioDef> sceneDefList;
    HRiverColorOverrideHolder riverColorOverrides;
    ScenarioDef *sceneDef = nullptr;
    bool editTabOpened;
    float previewX = 0;
    float previewY = 0;
    float previewZoom = 1;

    int mapIndexExcludeADT = -1;
    int mapIndexExcludeADTChunk = -1;

    void restartMinimapGenPreview();
    void createMinimapGenerator();
    void editComponentsForConfig(Config * config);
    void renderMapConfigSubWindow(int mapIndex);
    void renderEditTab();

    void renderListTab();
};


#endif //AWEBWOWVIEWERCPP_MINIMAPGENERATIONWINDOW_H
