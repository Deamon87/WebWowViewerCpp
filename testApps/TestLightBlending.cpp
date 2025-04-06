//
// Created by Deamon on 12/28/2024.
//



#include <iostream>
#include "../wowViewerLib/src/engine/objects/scenes/dayNightDataHolder/LightParamCalculate.h"
#include "../src/database/CSqliteDB.h"
#include "../wowViewerLib/3rdparty/mathfu/include/mathfu/glsl_mappings.h"



void dumpStateConditions(const StateForConditions &stateForConditions) {
    std::cout << "List of current ZoneLight.db2 ids:"<< std::endl;
    for (const auto &zoneLight : stateForConditions.currentZoneLights) {
        std::cout << zoneLight.id << " " << zoneLight.blend << std::endl;
    }

    std::cout << "List of current Light.db2 ids:"<< std::endl;

    for (const auto &lightId : stateForConditions.currentLightIds) {
        std::cout << lightId.id << " " << lightId.blend << std::endl;
    }

    std::cout << "List of current LightParams.db2 ids:"<< std::endl;
    for (const auto &lightParamId : stateForConditions.currentLightParams) {
        std::cout << lightParamId.id << " " << lightParamId.blend << std::endl;
    }

    std::cout << "List of current LightSkyBox.db2 ids:"<< std::endl;
    for (const auto &skyboxId : stateForConditions.currentSkyboxIds) {
        std::cout << skyboxId.id << " " << skyboxId.blend << std::endl;
    }
}

struct LightParamTestSettings {
    int mapId;
    mathfu::vec3 pos;
    int expectedLightParamId;
    float expectedBlend;
};

std::vector<LightParamTestSettings> lightParamTestSuites = {
    {
        2601,
        {2731.013916f, -2380.905029f, 375.916168f},
        6213,
        0.69f
    },
    {
        2601,
        {2683.917969f, -2302.646973f, 375.725250f},
        6295,
        0.96f
    },
    {
        2601,
        {2731.013916f, -2380.905029f, 375.916168f},
        6295,
        0.99f
    },
    {
        2601,
        {2731.013916f, -2380.905029f, 375.916168f},
        6295,
        0.74f
    }
};

int main() {

    auto databaseHandler = std::make_shared<CSqliteDB>("./export.db3");

    int TestIndex = 0;
    for (const LightParamTestSettings &testSuite : lightParamTestSuites)
    {
//        if (TestIndex != 1 ) { TestIndex ++;continue;}

        const int mapId = testSuite.mapId;
        const int lightParamIndex = 0;

        StateForConditions stateForConditions;
        
        auto zoneLights = loadZoneLightRecs(databaseHandler, mapId);

        const auto paramBlends = calculateLightParamBlends(
            databaseHandler,
            mapId,
            testSuite.pos,
            &stateForConditions,
            zoneLights,
            lightParamIndex
        );

        stateForConditions.currentLightParams = paramBlends;

        bool testPassed = false;
        for (const IdAndBlendAndPriority &paramBlend : paramBlends) {
            if (paramBlend.id == testSuite.expectedLightParamId && feq(paramBlend.blend, testSuite.expectedBlend)) {
                std::cout << "Test " << TestIndex << " passed" << std::endl;
                testPassed = true;
                break;
            }
        }
        
        if (!testPassed) {
            std::cout << "Test "<< TestIndex << " FAILED" << std::endl;
            std::cout << "Expected "<< testSuite.expectedLightParamId
                      << " with value " << testSuite.expectedBlend
                      << " Not found " << std::endl;
            dumpStateConditions(stateForConditions);
        }

        TestIndex++;
    }

    return 0;
}