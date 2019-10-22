#include "wowScene.h"
#include "algorithms/mathHelper.h"
#include "objects/scenes/m2Scene.h"
#include "objects/scenes/wmoScene.h"
#include "androidLogSupport.h"

#include "mathfu/glsl_mappings.h"
#include "persistance/db2/DB2Light.h"
#include "persistance/db2/DB2WmoAreaTable.h"
#include "./../gapi/UniformBufferStructures.h"
#include "objects/GlobalThreads.h"
#include "../gapi/IDeviceFactory.h"
#include "algorithms/FrameCounter.h"
//#include "objects/scenes/creatureScene.h"
#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>
#include <future>
#include <fstream>



void WoWSceneImpl::processCaches(int limit) {
//    std::cout << "WoWSceneImpl::processCaches called " << std::endl;
//    std::cout << "this->adtObjectCache.m_cache.size() = " << this->adtObjectCache.m_cache.size()<< std::endl;
    this->adtObjectCache.processCacheQueue(limit);
    this->wdtCache.processCacheQueue(limit);
    this->wdlCache.processCacheQueue(limit);
    this->wmoGeomCache.processCacheQueue(limit);
    this->wmoMainCache.processCacheQueue(limit);
    this->skinGeomCache.processCacheQueue(limit);
    this->animCache.processCacheQueue(limit);
    this->skelCache.processCacheQueue(limit);
    this->m2GeomCache.processCacheQueue(limit);
    this->textureCache.processCacheQueue(limit);
    this->db2Cache.processCacheQueue(limit);
}

void WoWSceneImpl::DoCulling() {
    if (currentScene == nullptr) return;

    float farPlane = m_config->getFarPlane();
    float nearPlane = 1.0;
    float fov = toRadian(45.0);

    static const mathfu::vec3 upVector(0,0,1);

    IDevice *device = getDevice();
    int currentFrame = device->getCullingFrameNumber();
    WoWFrameData *frameParam = &m_FrameParams[currentFrame];

    M2CameraResult cameraResult;
    mathfu::mat4 lookAtMat4;
    mathfu::vec4 cameraVec4;

    m_firstCamera.setMovementSpeed(m_config->getMovementSpeed());
    if (controllable == nullptr) return;
//    if (!m_config->getUseSecondCamera()){
    ((ICamera *)this->controllable)->tick(frameParam->deltaTime);
//    } else {
//        this->m_secondCamera.tick(frameParam->deltaTime);
//    }

    if ( currentScene->getCameraSettings(cameraResult)) {
//        farPlane = cameraResult.far_clip * 100;
        farPlane = 300;
        nearPlane = cameraResult.near_clip;

        fov = cameraResult.diagFov/ sqrt(1 + canvAspect*canvAspect);

        lookAtMat4 =
                mathfu::mat4::LookAt(
                        -cameraResult.target_position.xyz()+cameraResult.position.xyz(),
                        mathfu::vec3(0,0,0),
                        upVector) * mathfu::mat4::FromTranslationVector(-cameraResult.position.xyz());
        cameraVec4 = cameraResult.position;
        frameParam->m_lookAtMat4 = lookAtMat4;

    } else {

        cameraVec4 = mathfu::vec4(((ICamera *)controllable)->getCameraPosition(), 1);
        lookAtMat4 = ((ICamera *)this->controllable)->getLookatMat();

        frameParam->m_lookAtMat4 = lookAtMat4;
    }

    frameParam->m_perspectiveMatrixForCulling =
            mathfu::mat4::Perspective(
                    fov,
                    this->canvAspect,
                    nearPlane,
                    m_config->getFarPlaneForCulling());
    //Camera for rendering
    mathfu::mat4 perspectiveMatrixForCameraRender =
            mathfu::mat4::Perspective(fov,
                                      this->canvAspect,
                                      nearPlane,
                                      farPlane);
    mathfu::mat4 viewCameraForRender =
            perspectiveMatrixForCameraRender * lookAtMat4;

    frameParam->m_secondLookAtMat =
            mathfu::mat4::LookAt(
                    this->m_secondCamera.getCameraPosition(),
                    this->m_secondCamera.getCameraLookAt(),
                    upVector);

    mathfu::mat4 perspectiveMatrix =
         mathfu::mat4::Perspective(
                    fov,
                    this->canvAspect,
                    nearPlane,
                    farPlane);

    static const mathfu::mat4 vulkanMatrixFix = mathfu::mat4(1, 0, 0, 0,
                     0, -1, 0, 0,
                     0, 0, 1.0/2.0, 1/2.0,
                     0, 0, 0, 1).Transpose();

     if (device->getIsVulkanAxisSystem()) {
        perspectiveMatrix = vulkanMatrixFix * perspectiveMatrix;
     }

    frameParam->m_perspectiveMatrix = perspectiveMatrix;

    frameParam->m_viewCameraForRender = viewCameraForRender;

    frameParam->m_cameraVec3 = cameraVec4.xyz();

    float ambient[4];
    m_config->getAmbientColor(ambient);
    frameParam->m_globalAmbientColor = mathfu::vec4(ambient[0],ambient[1],ambient[2],ambient[3]);

    float sunColor[4];
    m_config->getSunColor(sunColor);
    frameParam->m_globalSunColor = mathfu::vec4(sunColor[0],sunColor[1],sunColor[2],sunColor[3]);

    float fogColor[4];
    m_config->getFogColor(fogColor);
    frameParam->m_fogColor = mathfu::vec4(fogColor);

    if (frameParam->uFogStart < 0) {
        frameParam->uFogStart = 3.0 * farPlane;
    }
    if (frameParam->uFogEnd < 0) {
        frameParam->uFogEnd = 4.0 * farPlane;
    }

    this->SetDirection(*frameParam);


    currentScene->checkCulling(frameParam);
}

void WoWSceneImpl::setScene(int sceneType, std::string name, int cameraNum) {
    if (sceneType == 0) {
        m_usePlanarCamera = cameraNum == -1;
        if (m_usePlanarCamera) {
            controllable = &m_planarCamera;
        }
        newScene = new M2Scene(this, name , cameraNum);
    } else if (sceneType == 1) {
        controllable = &m_firstCamera;
        m_usePlanarCamera = false;
        newScene = new WmoScene(this, name);
    } else if (sceneType == 2) {
        std::string &adtFileName = name;

        size_t lastSlashPos = adtFileName.find_last_of("/");
        size_t underscorePosFirst = adtFileName.find_last_of("_");
        size_t underscorePosSecond = adtFileName.find_last_of("_", underscorePosFirst-1);
        std::string mapName = adtFileName.substr(lastSlashPos+1, underscorePosSecond-lastSlashPos-1);

        int i = std::stoi(adtFileName.substr(underscorePosSecond+1, underscorePosFirst-underscorePosSecond-1));
        int j = std::stoi(adtFileName.substr(underscorePosFirst+1, adtFileName.size()-underscorePosFirst-5));

        float adt_x_min = AdtIndexToWorldCoordinate(j);
        float adt_x_max = AdtIndexToWorldCoordinate(j+1);

        float adt_y_min = AdtIndexToWorldCoordinate(i);
        float adt_y_max = AdtIndexToWorldCoordinate(i+1);

        m_firstCamera.setCameraPos(
            (adt_x_min+adt_x_max) / 2.0,
            (adt_y_min+adt_y_max) / 2.0,
            200
        );

        controllable = &m_firstCamera;
        m_usePlanarCamera = false;

        newScene = new Map(this, adtFileName, i, j, mapName);
    }
}

void WoWSceneImpl::setMap(int mapId, int wdtFileId, float x, float y, float z) {
    controllable = &m_firstCamera;
    m_usePlanarCamera = false;

    m_firstCamera.setCameraPos(
        x,
        y,
        z
    );

    controllable = &m_firstCamera;
    m_usePlanarCamera = false;

    newScene = new Map(this, mapId, wdtFileId);
}


void WoWSceneImpl::setReplaceTextureArray(std::vector<int> &replaceTextureArray) {
    if (newScene != nullptr) {
        newScene->setReplaceTextureArray(replaceTextureArray);
    } else {
        currentScene->setReplaceTextureArray(replaceTextureArray);
    }
}

void WoWSceneImpl::setAnimationId(int animationId) {
    if (newScene != nullptr) {
        newScene->setAnimationId(animationId);
    } else {
        currentScene->setAnimationId(animationId);
    }
}

void WoWSceneImpl::setSceneWithFileDataId(int sceneType, int fileDataId, int cameraNum) {
    if (sceneType == 0) {
        m_usePlanarCamera = cameraNum == -1;
        if (m_usePlanarCamera) {
            controllable = &m_planarCamera;
        }
        newScene = new M2Scene(this, fileDataId , cameraNum);
    } else if (sceneType == 1) {
        controllable = &m_firstCamera;
        m_usePlanarCamera = false;
        newScene = new WmoScene(this, fileDataId);
    }
}

WoWSceneImpl::WoWSceneImpl(Config *config, IFileRequest * requestProcessor, IDevice * device, int canvWidth, int canvHeight)
        :
        wmoMainCache(requestProcessor, CacheHolderType::CACHE_MAIN_WMO),
        wmoGeomCache(requestProcessor, CacheHolderType::CACHE_GROUP_WMO),
        wdtCache(requestProcessor, CacheHolderType::CACHE_WDT),
        wdlCache(requestProcessor, CacheHolderType::CACHE_WDL),
        m2GeomCache(requestProcessor, CacheHolderType::CACHE_M2),
        skinGeomCache(requestProcessor, CacheHolderType::CACHE_SKIN),
        textureCache(requestProcessor, CacheHolderType::CACHE_BLP),
        adtObjectCache(requestProcessor, CacheHolderType::CACHE_ADT),
        db2Cache(requestProcessor, CacheHolderType::CACHE_DB2),
        animCache(requestProcessor, CacheHolderType::CACHE_ANIM),
        skelCache(requestProcessor, CacheHolderType::CACHE_SKEL)
{
    m_gdevice.reset(device);
//    m_gdevice.reset(IDeviceFactory::createDevice("ogl4"));

#ifdef __EMSCRIPTEN__
    m_supportThreads = false;
#endif

//    std::ofstream *out = new std::ofstream("log_output.txt");
//    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
//    std::cout.rdbuf(out->rdbuf()); //redirect std::cout to out.txt!
//
    m_sceneWideUniformBuffer = m_gdevice->createUniformBuffer(sizeof(sceneWideBlockVSPS));

    this->m_config = config;

    this->canvWidth = canvWidth;
    this->canvHeight = canvHeight;
    this->canvAspect = (float)canvWidth / (float)canvHeight;

    //Init caches

    //Test scene 1: Shattrath
//    m_firstCamera.setCameraPos(-1663, 5098, 27); //Shattrath
//    m_firstCamera.setCameraPos(-241, 1176, 256); //Dark Portal

//    currentScene = new Map(this, 530, "Expansion01");
//    m_firstCamera.setCameraPos(972, 2083, 0); //Lost isles template
//    m_firstCamera.setCameraPos(-834, 4500, 0); //Dalaran 2
//    m_firstCamera.setCameraPos(-719, 2772, 317); //Near the black tower
//    m_firstCamera.setCameraPos( 4054, 7370, 27); // Druid class hall
//    currentScene = new Map(this, 1220, "Troll Raid");
//    currentScene = new Map(this, 0, "BrokenShoreBattleshipFinale");

//    m_firstCamera.setCameraPos(-1663, 5098, 27);
//    m_firstCamera.setCameraPos( -7134, 931, 27); // THE WOUND
//    currentScene = new Map(this, 1817, "silithusphase01");
//
//    m_firstCamera.setCameraPos( -594, 4664, 200);
//    currentScene = new Map(this, 1513, "Artifact-MageOrderHall");

//    m_firstCamera.setCameraPos( 3733.33325, 2666.66675, 0);
//    currentScene = new Map(this, "BLTestMap");

//    currentScene = new Map(this, 1779, "argus_rifts");

//    m_secondCamera.setCameraPos(-1663, 5098, 27);
//
//    m_firstCamera.setCameraPos(5243.2461346537075f, 1938.6550422193939f, 717.0332923206179f); //HallsOfReflection
//    currentScene = new Map(this, 668, "HallsOfReflection");
//     .go 668 5243 1938 760
    // .go 668 0 0 0

//    m_firstCamera.setCameraPos( 2290,  -9.475f, 470); // Ulduar Raid
//    currentScene = new Map(this, 603, "UlduarRaid");
//
//   m_firstCamera.setCameraPos(  -8192, -4819, 200); // Ulduar Raid
//    currentScene = new Map(this, 1803, "Kalimdor");
//
//    m_firstCamera.setCameraPos(  2843, 847, 200); // Ulduar Raid
//    currentScene = new Map(this, "Islands_7VR_Swamp_Prototype2");

//    m_firstCamera.setCameraPos( -2925, 8997, 200); // Antorus
//    m_firstCamera.setCameraPos( 5333.3335, 10666.667, 1); //Strange WMO
//    currentScene = new Map(this, 1669, "argus 1");

//    m_firstCamera.setCameraPos( 4266.67, -2133.33, 200); // VoidElf
//    currentScene = new Map(this, "VoildElf");
//    currentScene = new Map(this, "silithusphase01");

//    m_firstCamera.setCameraPos(-3000, -3000, 0); //Broken shore
//    currentScene = new Map(this, "stormgarde keep");
//
//    m_firstCamera.setCameraPos(-963, 977, 80);
//    currentScene = new Map(this, "dungeonblockout");
//
//    m_firstCamera.setCameraPos(939, -4813 , 0); //Scholozar Basin
//    m_firstCamera.setCameraPos(5783, 850, 200); //Near Dalaran
//    m_firstCamera.setCameraPos(3608, 5961, 292); //Near Dalaran
//    currentScene = new Map(this, 571, "Northrend");
//
//    m_firstCamera.setCameraPos(-8517, 1104, 200); //Stormwind
//    m_firstCamera.setCameraPos(0, 0, 200); //Stormwind
//    currentScene = new Map(this, 0, "Azeroth");
//
//   m_firstCamera.setCameraPos(-5025, -807, 500); //Ironforge
//   m_firstCamera.setCameraPos(-921, 767, 200);
//    currentScene = new Map(this, 0, "Zandalar");
//
//    m_firstCamera.setCameraPos(0, 0, 200); //Zaldalar
//    currentScene = new Map(this, 1642, "test_01");
//
//
//    m_firstCamera.setCameraPos(570, 979, 200); //Maelstorm Shaman
//    currentScene = new Map(this, "MaelstromShaman");

//    m_firstCamera.setCameraPos(11733, 3200, 200); //Kalimdor 2
//    m_firstCamera.setCameraPos(6947, 408, 162); //Kalimdor 2
//    currentScene = new Map(this, 1, "Kalimdor 2");
//
//    m_firstCamera.setCameraPos(1825.32, 376.095, 70.0652); //LordaeronScenario
//    currentScene = new Map(this, 1, "LordaeronScenario");
//
//   m_firstCamera.setCameraPos( 2652, 1083, 200) ; //LordaeronScenario
//    currentScene = new Map(this, 1, "legionshiphorizontalstormheim");

//   m_firstCamera.setCameraPos( -9169.86, 1604.42, 26.84) ; //LordaeronScenario
//    currentScene = new Map(this, 1, "ahnqirajtemple");
//
//    m_firstCamera.setCameraPos(3062, 495, 200 ); //Valhalla
//    m_firstCamera.setCameraPos(2979, 3525, 200); //Field of the Eternal Hunt
//    currentScene = new Map(this, 1477, "Valhallas");
//
//    m_firstCamera.setCameraPos(2902, 2525, 200db); //Field of the Eternal Hunt
//    m_firstCamera.setCameraPos(3993, 2302, 1043); //Field of the Eternal Hunt
//    currentScene = new Map(this, "NagaDungeon");
//    m_firstCamera.setCameraPos(829, -296, 200 ); //Field of the Eternal Hunt
//    currentScene = new Map(this, "unused");

//    m_firstCamera.setCameraPos(-2825, -4546, 200 ); //Field of the Eternal Hunt
//    currentScene = new Map(this, "ScenarioAlcazIsland");

//    m_firstCamera.setCameraPos( 0,  0, 470); // Panda ria
//    currentScene = new Map(this, 0, "HawaiiMainLand");

//    m_firstCamera.setCameraPos(-12886, -165, 200); // Pandaria
//    currentScene = new Map(this, "Azeroth");
//
//   m_firstCamera.setCameraPos(-12017, 3100, 200); // Pandaria
//    currentScene = new Map(this, 0, "Kalimdor");
//
//    m_firstCamera.setCameraPos( -8517, 1104, 200);
//    currentScene = new Map(this, 0, "escapefromstockades");


    //Test scene 2: tree from shattrath
//    m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new M2Scene(this,
//        "WORLD\\EXPANSION01\\DOODADS\\TEROKKAR\\TREES\\TEROKKARTREEMEDIUMPINECONES.m2");
//    currentScene = new M2Scene(this,
//        "creature/akama/akama.m2");
//
//   m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new M2Scene(this,
//       "WORLD\\GENERIC\\PASSIVEDOODADS\\SHIPS\\SHIPANIMATION\\TRANSPORTSHIP_SAILS.m2");
//
//    m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new M2Scene(this,
//        "world\\generic\\human\\passive doodads\\gryphonroost\\gryphonroost01.m2");
//    currentScene = new M2Scene(this,
//        "world/expansion07/doodads/dungeon/doodads/8hu_waycrest_painting10.m2");
//
//    m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new M2Scene(this,
//        "creature/twilightascendantwater/twilightascendantwater.m2");
//
//    Test scene 2: tree from shattrath
//    m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new M2Scene(this,
//        "WORLD\\AZEROTH\\ELWYNN\\PASSIVEDOODADS\\WATERFALL\\ELWYNNTALLWATERFALL01.m2");

//    m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new M2Scene(this,
//        "creature/celestialdragonwyrm/celestialdragonwyrm.m2");

//   m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new M2Scene(this,
//        "creature/lorthemar/lorthemar.m2");
    m_usePlanarCamera = false;

//    m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new M2Scene(this,
//        "creature\\wingedhorse\\wingedhorse.m2");
//    currentScene = new M2Scene(this,
//        "environments/stars/hellfireskybox.m2");
//
//
//  m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new M2Scene(this,
//        "world/expansion06/doodads/brokenshore/7bs_tombofsargerasfx_01_reduced.m2");

//    currentScene = new M2Scene(this,
//        "world/expansion07/doodads/8xp_burningteldrassil01.m2");
//currentScene = new M2Scene(this,
//        "world/expansion06/doodads/brokenshore/7bs_tombofsargerasfxbeam_01_reduced.m2");
//
//   m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new M2Scene(this,
//                               "WORLD\\EXPANSION02\\DOODADS\\ULDUAR\\UL_SMALLSTATUE_DRUID.m2");
//   m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new M2Scene(this,
//        "interface/glues/models/ui_mainmenu_northrend/ui_mainmenu_northrend.m2", 0);
//    currentScene = new M2Scene(this,
//        "interface/glues/models/ui_mainmenu_legion/ui_mainmenu_legion.m2", 0);
//
//   currentScene = new M2Scene(this,
//        "interface/glues/models/ui_mainmenu_battleforazeroth/ui_mainmenu_battleforazeroth.m2", 0);
//    currentScene = new M2Scene(this,
//        "interface/glues/models/ui_mainmenu_warlords/ui_mainmenu_warlords.m2", 0);

//   currentScene = new M2Scene(this,
//        "interface/glues/models/ui_mainmenu_pandaria/ui_mainmenu_pandaria.m2", 0);
//   currentScene = new M2Scene(this,
//        "interface/glues/models/ui_mainmenu_cataclysm/ui_mainmenu_cataclysm.m2", 0);
//    currentScene = new M2Scene(this,
//        "interface/glues/models/ui_mainmenu_burningcrusade/ui_mainmenu_burningcrusade.m2", 0);
//    mathfu::vec4 ambientColorOver = mathfu::vec4(0.3929412066936493f, 0.26823532581329346f, 0.3082353174686432f, 0);
//    currentScene->setAmbientColorOverride(ambientColorOver, true);
//    config->setBCLightHack(true);

//    currentScene = new M2Scene(this,
//        "interface/glues/models/ui_mainmenu/ui_mainmenu.m2", 0);
//    config->setBCLightHack(true);

//    currentScene = new M2Scene(this,
//        "interface/glues/models/ui_worgen/ui_worgen.m2", 0);

//    currentScene = new M2Scene(this,
//        "interface/glues/models/ui_pandaren/ui_pandaren.m2", 0);
//
//    currentScene = new M2Scene(this,
//        "interface/glues/models/ui_nightelf/ui_nightelf.m2", 0);

//    currentScene = new M2Scene(this,
//        "world/khazmodan/ironforge/passivedoodads/throne/dwarventhrone01.m2");
//
//   currentScene = new M2Scene(this,
//        "creature/murloc/murloc.m2");
//
//
// currentScene = new M2Scene(this,
//        "WORLD\\EXPANSION02\\DOODADS\\GENERIC\\SCOURGE\\SC_EYEOFACHERUS_02.m2");

// currentScene = new M2Scene(this,
//        "world/lordaeron/alteracmountains/passivedoodads/dalaran/dalarandome.m2");

// currentScene = new M2Scene(this,
//        "WORLD/EXPANSION02/DOODADS/CRYSTALSONGFOREST/BUBBLE/CAMOUFLAGEBUBBLE_CRYSTALSONG.m2");

//    m_usePlanarCamera = true;
//    if (m_usePlanarCamera) {
//        controllable = &m_planarCamera;
//    }

//    currentScene = new M2Scene(this, 2200968, 0);

    //Test scene 3: Ironforge
//    m_firstCamera.setCameraPos(1.78252912f,  33.4062042f, -126.937592f); //Room under dalaran
//    m_firstCamera.setCameraPos(-32.1193314, 0.432947099, 9.5181284); //Room with transparent window
//    currentScene = new WmoScene(this,
//        "world\\wmo\\brokenisles\\dalaran2.wmo");
//    currentScene = new WmoScene(this,
//        "World\\wmo\\Dungeon\\test\\test.wmo");



//   m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new WmoScene(this,
//        "WORLD/WMO/NORTHREND/BUILDINGS/HUMAN/ND_HUMAN_INN/ND_HUMAN_INN.WMO");

//  m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new WmoScene(this,
//    "World\\wmo\\BrokenIsles\\Suramar\\7SR_SuramarCity_Single_B_Core_C.wmo");
//
//    m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new WmoScene(this,
//        "world\\wmo\\dungeon\\karazhanb\\7du_karazhanb_castle.wmo");
//    m_firstCamera.setCameraPos(-1161.35, -798.59, 835.05);
//    currentScene = new Map(this, "grimbatolraid");

//   currentScene = new WmoScene(this,
//        "world\\wmo\\dungeon\\karazhanb\\7du_karazhanb_tower.wmo");

//    currentScene = new WmoScene(this,
//        "world\\wmo\\dungeon\\karazhanb\\7du_karazhanb_nether.wmo");
//    currentScene = new WmoScene(this,
//        "world\\wmo\\dungeon\\mantidraid\\pa_mantid_raid.wmo");
//    currentScene = new WmoScene(this,
//        "world/wmo/dungeon/grimbatol/kz_grimbatol.wmo");
//    currentScene = new WmoScene(this,
//        "world/wmo/dungeon/grimbatol/kz_grimbatol_raid.wmo");//
// currentScene = new WmoScene(this,
//        "\tworld/wmo/dungeon/thunderkingraid/pa_thunderking_raid.wmo");


//    currentScene = new WmoScene(this,
 //       "World/wmo/Dungeon/AZ_Subway/Subway.wmo");
//    currentScene = new WmoScene(this,
//                                "world/wmo/azeroth/buildings/stranglethorn_bootybay/bootybay.wmo"); //bootybay
//                                2324175);
//
//   currentScene = new WmoScene(this,
//                               2198682);
//   currentScene = new WmoScene(this,
//                               "world/wmo/kultiras/nightelf/8ne_nightelf_dockbroken01.wmo");


//    m_firstCamera.setCameraPos(136.784775,-42.097565,33.5634689);
//    currentScene = new WmoScene(this,
//        "world\\wmo\\dungeon\\tombofsargerasraid\\7du_tombofsargeras_raid.wmo");
// currentScene = new WmoScene(this,
//        "world\\wmo\\khazmodan\\cities\\ironforge\\ironforge.wmo");

// currentScene = new WmoScene(this,
//        "WORLD\\WMO\\PANDARIA\\VALEOFETERNALBLOSSOMS\\TEMPLES\\MG_RAIDBUILDING_LD.WMO");
//
//    currentScene = new WmoScene(this,
//        "world\\wmo\\draenor\\tanaanjungle\\6tj_darkportal_antiportal.wmo");
//
//    currentScene = new WmoScene(this,
//        "world\\wmo\\azeroth\\buildings\\stormwind\\stormwind.WMO");

//    m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new WmoScene(this,
//        "world\\wmo\\dungeon\\argusraid\\7du_argusraid_shivantemple.wmo");

//    m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new WmoScene(this,
//        "world/wmo/brokenisles/valsharah/7vs_nightmare_worldtree.wmo");

//    m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new WmoScene(this,
//        "WORLD\\WMO\\TRANSPORTS\\TRANSPORT_SHIP\\TRANSPORTSHIP.WMO");
//
//   m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new WmoScene(this,
//        "world/wmo/brokenisles/brokenshore/7bs_tombofsargerasfx_01_wmo.wmo");
//
//  currentScene = new WmoScene(this,
//        "world/wmo/brokenisles/brokenshore/7bs_tombofsargeras.wmo");
//


//  m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new WmoScene(this,
//        "world/wmo/azeroth/buildings/worldtree/theworldtreehyjal.wmo");

//    m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new WmoScene(this,
//        "world/wmo/dungeon/argusraid/7du_argusraid_pantheon.wmo");
//
//   currentScene = new WmoScene(this,
//        "world/wmo/lorderon/undercity/8xp_undercity.wmo");

//   currentScene = new WmoScene(this,
//        "world/wmo/zuldazar/forsaken/8fk_forsaken_shiplarge01.wmo");

//   currentScene = new WmoScene(this,
//        "world/wmo/dungeon/ulduar/ulduar_raid.wmo");

    db2Light = nullptr;
    db2LightData = nullptr;
    db2WmoAreaTable = nullptr;

//    setScene(2, "world/maps/ahnqiraj/ahnqiraj_26_46.adt", -1);
//    setScene(2, "WORLD/MAPTEXTURES/MAELSTROMDEATHWINGFIGHT/MAELSTROMDEATHWINGFIGHT_32_32.adt", -1);
//    setScene(2, "WORLD/MAPTEXTURES/Expansion01/Expansion01_44_8.adt", -1);
//    setSceneWithFileDataId(1, 1846142, -1); // wmo with horde symbol
//    setSceneWithFileDataId(1, 324981, -1);
//    setSceneWithFileDataId(1, 1120838, -1);
//    setSceneWithFileDataId(1, 1699872, -1);
//    setScene(2, "world/maps/nzoth/nzoth_32_27.adt", -1);
//    setScene(2, "world/maps/Kalimdor/Kalimdor_40_47.adt", -1);
//    setScene(0, "interface/glues/models/ui_mainmenu_northrend/ui_mainmenu_northrend.m2", 0);
//    setMap(1, 782779, -8183, -4708, 200);
//    setScene(2, "world/maps/SilithusPhase01/SilithusPhase01_30_45.adt", -1);
//    setSceneWithFileDataId(1, 108803, -1);
    setSceneWithFileDataId(0, 125407, -1); // phoneix
//    setSceneWithFileDataId(0, 2500382, -1); // galliwix mount
    //setSceneWithFileDataId(0, 125995, -1); //portal
//    setSceneWithFileDataId(0, 1612576, -1); //portal
//    setSceneWithFileDataId(1, 108803, -1); //caverns of time in Tanaris

//    setSceneWithFileDataId(0, 1100087, -1); //bloodelfMale_hd
//    setSceneWithFileDataId(0, 1814471, -1); //nightbornemale
//    setSceneWithFileDataId(0, 1269330, -1); //nightbornemale creature


    if (m_supportThreads) {
        g_globalThreadsSingleton.loadingResourcesThread = std::thread([&]() {
            using namespace std::chrono_literals;

            while (!this->m_isTerminating) {
                std::this_thread::sleep_for(1ms);

                processCaches(1000);
            }
        });


        g_globalThreadsSingleton.cullingThread = std::thread(([&]() {
            using namespace std::chrono_literals;
            FrameCounter frameCounter;

            while (!this->m_isTerminating) {
                auto future = nextDeltaTime.get_future();
                future.wait();

//                std::cout << "update frame = " << getDevice()->getUpdateFrameNumber() << std::endl;

                int currentFrame = m_gdevice->getCullingFrameNumber();
                WoWFrameData *frameParam = &m_FrameParams[currentFrame];
                frameParam->deltaTime = future.get();
                nextDeltaTime = std::promise<float>();

                frameCounter.beginMeasurement();
                DoCulling();

                frameCounter.endMeasurement("Culling thread ");

                this->cullingFinished.set_value(true);
            }
        }));

        if (device->getIsAsynBuffUploadSupported()) {
            g_globalThreadsSingleton.updateThread = std::thread(([&]() {
                FrameCounter frameCounter;

                FrameCounter singleUpdateCNT;
                FrameCounter meshesCollectCNT;
                while (!this->m_isTerminating) {
                    auto future = nextDeltaTimeForUpdate.get_future();
                    future.wait();
                    nextDeltaTimeForUpdate = std::promise<float>();
                    frameCounter.beginMeasurement();

                    IDevice *device = getDevice();
                    int updateObjFrame = device->getUpdateFrameNumber();
                    WoWFrameData *objFrameParam = &m_FrameParams[updateObjFrame];

                    updateFrameIndex = updateObjFrame;

                    device->startUpdateForNextFrame();

                    singleUpdateCNT.beginMeasurement();
                    currentScene->update(objFrameParam);
                    singleUpdateCNT.endMeasurement("single update ");

                    meshesCollectCNT.beginMeasurement();
                    currentScene->collectMeshes(objFrameParam);
                    meshesCollectCNT.endMeasurement("collectMeshes ");

                    device->prepearMemoryForBuffers(objFrameParam->meshes);
                    sceneWideBlockVSPS *blockPSVS = &m_sceneWideUniformBuffer->getObject<sceneWideBlockVSPS>();
                    if (blockPSVS != nullptr) {
                        blockPSVS->uLookAtMat = objFrameParam->m_lookAtMat4;
                        blockPSVS->uPMatrix = objFrameParam->m_perspectiveMatrix;
                        m_sceneWideUniformBuffer->save();
                    }

                    device->updateBuffers(objFrameParam->renderedThisFrame);

                    currentScene->doPostLoad(objFrameParam); //Do post load after rendering is done!
                    device->uploadTextureForMeshes(objFrameParam->renderedThisFrame);

                    device->endUpdateForNextFrame();
                    frameCounter.endMeasurement("Update Thread");

                    updateFinished.set_value(true);
                }
            }));
        }
    }
}

void WoWSceneImpl::setScreenSize(int canvWidth, int canvHeight) {
    this->canvWidth = canvWidth;
    this->canvHeight = canvHeight;
    this->canvAspect = (float)canvWidth / (float)canvHeight;
}

/* Shaders stuff */
//
//void WoWSceneImpl::drawTexturedQuad(GLuint texture,
//                                    float x,
//                                    float y,
//                                    float width,
//                                    float height,
//                                    float canv_width,
//                                    float canv_height,
//                                    bool drawDepth) {
//    /*
//    glDisable(GL_DEPTH_TEST);
//    glBindBuffer(GL_ARRAY_BUFFER, this->vertBuffer);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//
//    glEnableVertexAttribArray(+drawDepthShader::Attribute::position);
//    glVertexAttribPointer(+drawDepthShader::Attribute::position, 2, GL_FLOAT, GL_FALSE, 0, 0);
//
//    glUniform1f(this->drawDepthBuffer->getUnf("uWidth"), width/canv_width);
//    glUniform1f(this->drawDepthBuffer->getUnf("uHeight"), height/canv_height);
//    glUniform1f(this->drawDepthBuffer->getUnf("uX"), x/canv_width);
//    glUniform1f(this->drawDepthBuffer->getUnf("uY"), y/canv_height);
//    glUniform1i(this->drawDepthBuffer->getUnf("drawDepth"), (drawDepth) ? 1 : 0);
//
//    glBindTexture(GL_TEXTURE_2D, texture);
//    glDrawArrays(GL_TRIANGLES, 0, 6);
//    glEnable(GL_DEPTH_TEST);
//    */
//}
/****************/

void WoWSceneImpl::drawCamera () {
    /*
    glDisable(GL_DEPTH_TEST);

    mathfu::mat4 invViewFrustum = this->m_viewCameraForRender.Inverse();

    glUniformMatrix4fv(drawFrustumShader->getUnf("uInverseViewProjection"), 1, GL_FALSE, &invViewFrustum[0]);

    glDrawElements(GL_LINES, 48, GL_UNSIGNED_SHORT, 0);
    glEnable(GL_DEPTH_TEST);
    */
}

void print_timediff(const char* prefix, const struct timespec& start, const
struct timespec& end)
{
    double milliseconds = (end.tv_nsec - start.tv_nsec) / 1e6 + (end.tv_sec - start.tv_sec) * 1e3;
    printf("%s: %lf milliseconds\n", prefix, milliseconds);
}

void WoWSceneImpl::draw(animTime_t deltaTime) {
    std::future<bool> cullingFuture;
    std::future<bool> updateFuture;
    if (m_supportThreads) {
        cullingFuture = cullingFinished.get_future();

        nextDeltaTime.set_value(deltaTime);
        if (getDevice()->getIsAsynBuffUploadSupported()) {
            nextDeltaTimeForUpdate.set_value(deltaTime);
            updateFuture = updateFinished.get_future();
        }
    }

//    std::cout << "draw frame = " << getDevice()->getDrawFrameNumber() << std::endl;

    //Replace the scene
    if (newScene != nullptr) {
        if (currentScene != nullptr) delete currentScene;
        currentScene = newScene;
        newScene = nullptr;
    }

    if (currentScene == nullptr) return;

    if (needToDropCache) {
        this->actuallDropCache();
        needToDropCache = false;
    }


    IDevice *device = getDevice();
    device->reset();
    int currentFrame = device->getDrawFrameNumber();
    WoWFrameData *frameParam = &m_FrameParams[currentFrame];

    if (!m_supportThreads) {
        processCaches(10);
        frameParam->deltaTime = deltaTime;
        DoCulling();
    }

    float clearColor[4];
    m_config->getClearColor(clearColor);
    device->setClearScreenColor(clearColor[0], clearColor[1], clearColor[2]);
    device->setViewPortDimensions(0,0,this->canvWidth, this->canvHeight);
    device->beginFrame();

    mathfu::mat4 mainLookAtMat4 = frameParam->m_lookAtMat4;

    if (this->m_config->getDoubleCameraDebug()) {
        //Draw static camera
        m_isDebugCamera = true;
        frameParam->m_lookAtMat4 = frameParam->m_secondLookAtMat;
        currentScene->draw(frameParam);
        m_isDebugCamera = false;

        if (this->m_config->getDrawDepthBuffer() /*&& this.depth_texture_ext*/) {
            /*
            //Draw real camera into square at bottom of screen
            this->activateRenderDepthShader();
            glEnableVertexAttribArray(0);
            glUniform1f(drawDepthBuffer->getUnf("uFarPlane"), farPlane);
            glUniform1f(drawDepthBuffer->getUnf("uNearPlane"), nearPlane);

            this->drawTexturedQuad(this->frameBufferDepthTexture,
                                   this->canvWidth * 0.60f,
                                   0,//this.canvas.height * 0.75,
                                   this->canvWidth * 0.40f,
                                   this->canvHeight * 0.40f,
                                   this->canvWidth,
                                   this->canvHeight, true);
           */
        } else {
            //Render real camera
//            glBindFramebuffer(GL_FRAMEBUFFER, this->frameBuffer);
//            glClearScreen(/*this.fogColor*/);
//            glDepthMask(GL_TRUE);
//
//            this->m_lookAtMat4 = lookAtMat4;
//            currentScene->draw();
//            glBindFramebuffer(GL_FRAMEBUFFER, GL_ZERO);
//
//            this->activateRenderDepthShader();
//            this->drawTexturedQuad(this->frameBufferColorTexture,
//                                   this->canvWidth * 0.60f,
//                                   0,//this.canvas.height * 0.75,
//                                   this->canvWidth * 0.40f,
//                                   this->canvHeight * 0.40f,
//                                   this->canvWidth,
//                                   this->canvHeight, false);


        }
    } else {
        //Render real camera
        frameParam->m_lookAtMat4 = mainLookAtMat4;
        currentScene->draw(frameParam);

        if (this->m_config->getDrawDepthBuffer() /*&& this.depth_texture_ext*/) {
            //Draw real camera into square at bottom of screen
            /*
            this->activateRenderDepthShader();
            glEnableVertexAttribArray(0);
            glUniform1f(drawDepthBuffer->getUnf("uFarPlane"), farPlane);
            glUniform1f(drawDepthBuffer->getUnf("uNearPlane"), nearPlane);

            this->drawTexturedQuad(this->frameBufferDepthTexture,
                                   this->canvWidth * 0.60f,
                                   0,//this.canvas.height * 0.75,
                                   this->canvWidth * 0.40f,
                                   this->canvHeight * 0.40f,
                                   this->canvWidth,
                                   this->canvHeight, true);
           */
        }
    }
//    clock_gettime(CLOCK_MONOTONIC, &renderingAndUpdateEnd);

//    print_timediff("rendering", renderingAndUpdateStart, renderingAndUpdateEnd);


//    nextDeltaTime = deltaTime;
    device->commitFrame();
    device->reset();

    if (!device->getIsAsynBuffUploadSupported()) {
        int updateObjFrame = device->getUpdateFrameNumber();

        WoWFrameData *objFrameParam = &m_FrameParams[updateObjFrame];
        updateFrameIndex = updateObjFrame;

        device->startUpdateForNextFrame();
        currentScene->update(objFrameParam);
        currentScene->collectMeshes(objFrameParam);

        sceneWideBlockVSPS &blockPSVS = m_sceneWideUniformBuffer->getObject<sceneWideBlockVSPS>();
        blockPSVS.uLookAtMat = objFrameParam->m_lookAtMat4;
        blockPSVS.uPMatrix = objFrameParam->m_perspectiveMatrix;
        m_sceneWideUniformBuffer->save();

        device->updateBuffers(objFrameParam->renderedThisFrame);

        currentScene->doPostLoad(objFrameParam); //Do post load after rendering is done!
        device->uploadTextureForMeshes(objFrameParam->renderedThisFrame);

        device->endUpdateForNextFrame();
    }


    if (m_supportThreads) {
        cullingFuture.wait();
        cullingFinished = std::promise<bool>();

        if (device->getIsAsynBuffUploadSupported()) {
            updateFuture.wait();
            updateFinished = std::promise<bool>();
        }
    }


    device->increaseFrameNumber();
}
mathfu::mat3 blizzTranspose(mathfu::mat4 &value) {

    return mathfu::mat3(
        value.GetColumn(0).x,value.GetColumn(1).x,value.GetColumn(2).x,
        value.GetColumn(0).y,value.GetColumn(1).y,value.GetColumn(2).y,
        value.GetColumn(0).z,value.GetColumn(1).z,value.GetColumn(2).z
    );
}

void WoWSceneImpl::SetDirection(WoWFrameData &frameParamHolder) {

    // Phi Table
    static const float phiTable[4][2] = {
            { 0.0,  2.2165682f },
            { 0.25, 1.9198623f },
            { 0.5,  2.2165682f },
            { 0.75, 1.9198623f }
    };

    // Theta Table


    static const float thetaTable[4][2] = {
        { 0.0,  3.926991f },
        { 0.25, 3.926991f },
        { 0.5,  3.926991f },
        { 0.75, 3.926991f }
    };

//    float phi = DayNight::InterpTable(&DayNight::phiTable, 4u, DayNight::g_dnInfo.dayProgression);
//    float theta = DayNight::InterpTable(&DayNight::thetaTable, 4u, DayNight::g_dnInfo.dayProgression);

    float phi = phiTable[0][1];
    float theta = thetaTable[0][1];

    // Convert from spherical coordinates to XYZ
    // x = rho * sin(phi) * cos(theta)
    // y = rho * sin(phi) * sin(theta)
    // z = rho * cos(phi)

    float sinPhi = (float) sin(phi);
    float cosPhi = (float) cos(phi);

    float sinTheta = (float) sin(theta);
    float cosTheta = (float) cos(theta);

    mathfu::mat3 lookAtRotation = mathfu::mat4::ToRotationMatrix(frameParamHolder.m_lookAtMat4);

    mathfu::vec4 sunDirWorld = mathfu::vec4(sinPhi * cosTheta, sinPhi * sinTheta, cosPhi, 0);
//    mathfu::vec4 sunDirWorld = mathfu::vec4(-0.30822, -0.30822, -0.89999998, 0);
    frameParamHolder.m_sunDir = (lookAtRotation * sunDirWorld.xyz());
//    frameParamHolder.m_sunDir = frameParamHolder.m_sunDir.Normalized();

    mathfu::vec4 upVector ( 0.0, 0.0 , 1.0 , 0.0);
//    frameParamHolder.m_upVector = (frameParamHolder.m_lookAtMat4.Transpose() * upVector).xyz();
    frameParamHolder.m_upVector = (lookAtRotation * upVector.xyz());;
}

WoWSceneImpl::~WoWSceneImpl() {
    m_isTerminating = true;
    if (g_globalThreadsSingleton.cullingThread.joinable()) {
        g_globalThreadsSingleton.cullingThread.join();
    }

    if (g_globalThreadsSingleton.updateThread.joinable()) {
        g_globalThreadsSingleton.updateThread.join();
    }

    if (g_globalThreadsSingleton.loadingResourcesThread.joinable()) {
        g_globalThreadsSingleton.loadingResourcesThread.join();
    }
}

WoWScene *createWoWScene(Config *config, IFileRequest *requestProcessor, IDevice *device, int canvWidth, int canvHeight) {
#ifdef __ANDROID_API__
    std::cout.rdbuf(new androidbuf());
#endif

    return new WoWSceneImpl(config, requestProcessor, device, canvWidth, canvHeight);
}

void WoWSceneImpl::clearCache() {
//    std::cout << "Called " << __PRETTY_FUNCTION__ << std::endl;
//    needToDropCache = true;
    actuallDropCache();
}
void WoWSceneImpl::actuallDropCache() {
//    std::cout << "Called " << __PRETTY_FUNCTION__ << std::endl;
    this->adtObjectCache.clear();
    this->wdtCache.clear();
    this->wdlCache.clear();
    this->wmoGeomCache.clear();
    this->wmoMainCache.clear();
    this->m2GeomCache.clear();
    this->skinGeomCache.clear();
    this->animCache.clear();
    this->textureCache.clear();
    this->db2Cache.clear();
}

void WoWSceneImpl::provideFile(CacheHolderType holderType, const char *fileName, const HFileContent &data) {
//    std::cout << "data.use_count() = " << data.use_count() << std::endl << std::flush;
    std::string s_fileName(fileName);

    switch (holderType) {
        case CacheHolderType::CACHE_M2:
            m2GeomCache.provideFile(s_fileName, data);
            break;
        case CacheHolderType::CACHE_SKIN:
            skinGeomCache.provideFile(s_fileName, data);
            break;
        case CacheHolderType::CACHE_MAIN_WMO:
            wmoMainCache.provideFile(s_fileName, data);
            break;
        case CacheHolderType::CACHE_GROUP_WMO:
            wmoGeomCache.provideFile(s_fileName, data);
            break;
        case CacheHolderType::CACHE_ADT:
            adtObjectCache.provideFile(s_fileName, data);
            break;
        case CacheHolderType::CACHE_WDT:
            wdtCache.provideFile(s_fileName, data);
            break;
        case CacheHolderType::CACHE_WDL:
            wdlCache.provideFile(s_fileName, data);
            break;
        case CacheHolderType::CACHE_BLP:
            textureCache.provideFile(s_fileName, data);
            break;
        case CacheHolderType::CACHE_DB2:
            db2Cache.provideFile(s_fileName, data);
            break;
        case CacheHolderType::CACHE_ANIM:
            animCache.provideFile(s_fileName, data);
            break;
        case CacheHolderType::CACHE_SKEL:
            skelCache.provideFile(s_fileName, data);
            break;

    }
}

