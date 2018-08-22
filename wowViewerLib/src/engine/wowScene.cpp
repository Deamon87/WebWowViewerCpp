#include "opengl/header.h"
#include "wowScene.h"
#include "shader/ShaderRuntimeData.h"
#include "algorithms/mathHelper.h"
#include "objects/scenes/m2Scene.h"
#include "objects/scenes/wmoScene.h"
#include "androidLogSupport.h"

#include "mathfu/glsl_mappings.h"
#include "persistance/db2/DB2Light.h"
#include "persistance/db2/DB2WmoAreaTable.h"
#include "shader/ShaderDefinitions.h"
#include "../gapi/UniformBufferStructures.h"
#include "objects/GlobalThreads.h"
#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>

void WoWSceneImpl::DoCulling() {
    float farPlane = 3000;
    float nearPlane = 1;
    float fov = toRadian(45.0);

    static const mathfu::vec3 upVector(0,0,1);

    M2CameraResult cameraResult;
    mathfu::mat4 lookAtMat4;
    mathfu::vec4 cameraVec4;

    m_firstCamera.setMovementSpeed(m_config->getMovementSpeed());

    if (!m_config->getUseSecondCamera()){
        this->m_firstCamera.tick(nextDeltaTime);
    } else {
        this->m_secondCamera.tick(nextDeltaTime);
    }

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
        m_nextFrameParams.m_lookAtMat4 = lookAtMat4;

    } else {
        cameraVec4 = mathfu::vec4(m_firstCamera.getCameraPosition(), 1);
        lookAtMat4 = this->m_firstCamera.getLookatMat();
        m_nextFrameParams.m_lookAtMat4 = lookAtMat4;
    }

    m_nextFrameParams.m_perspectiveMatrixForCulling =
            mathfu::mat4::Perspective(
                    fov,
                    this->canvAspect,
                    nearPlane,
                    500);
    //Camera for rendering
    mathfu::mat4 perspectiveMatrixForCameraRender =
            mathfu::mat4::Perspective(fov,
                                      this->canvAspect,
                                      nearPlane,
                                      farPlane);
    mathfu::mat4 viewCameraForRender =
            perspectiveMatrixForCameraRender * lookAtMat4;


    m_nextFrameParams.m_secondLookAtMat =
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
    m_nextFrameParams.m_perspectiveMatrix = perspectiveMatrix;

    m_nextFrameParams.m_viewCameraForRender = viewCameraForRender;

    m_nextFrameParams.m_cameraVec3 = cameraVec4.xyz();

    float ambient[4];
    m_config->getAmbientColor(ambient);
    m_nextFrameParams.m_globalAmbientColor = mathfu::vec4(ambient[0],ambient[1],ambient[2],ambient[3]);

    float sunColor[4];
    m_config->getSunColor(sunColor);
    m_nextFrameParams.m_globalSunColor = mathfu::vec4(sunColor[0],sunColor[1],sunColor[2],sunColor[3]);

    float fogColor[4];
    m_config->getFogColor(fogColor);
    m_nextFrameParams.m_fogColor = mathfu::vec4(fogColor);

    if (m_nextFrameParams.uFogStart < 0) {
        m_nextFrameParams.uFogStart = 3.0 * farPlane;
    }
    if (m_nextFrameParams.uFogEnd < 0) {
        m_nextFrameParams.uFogEnd = 4.0 * farPlane;
    }

    this->SetDirection(m_nextFrameParams);

    currentScene->checkCulling(m_nextFrameParams.m_perspectiveMatrixForCulling, lookAtMat4, cameraVec4);
    currentScene->update(nextDeltaTime, m_nextFrameParams.m_cameraVec3, m_nextFrameParams.m_perspectiveMatrixForCulling, m_nextFrameParams.m_lookAtMat4);

}

WoWSceneImpl::WoWSceneImpl(Config *config, IFileRequest * requestProcessor, int canvWidth, int canvHeight)
        :
        wmoMainCache(requestProcessor),
        wdtCache(requestProcessor),
        wdlCache(requestProcessor),
        wmoGeomCache(requestProcessor),
        m2GeomCache(requestProcessor),
        skinGeomCache(requestProcessor),
        textureCache(requestProcessor),
        adtObjectCache(requestProcessor),
        db2Cache(requestProcessor){
//    m_gdevice = GDevice();

//    std::ofstream *out = new std::ofstream("log_output.txt");
//    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
//    std::cout.rdbuf(out->rdbuf()); //redirect std::cout to out.txt!

    m_sceneWideUniformBuffer = m_gdevice.createUniformBuffer(sizeof(sceneWideBlockVSPS));
    m_sceneWideUniformBuffer->createBuffer();

    this->m_config = config;

    renderLockNextMeshes = std::unique_lock<std::mutex>(m_lockNextMeshes,std::defer_lock);

    this->canvWidth = canvWidth;
    this->canvHeight = canvHeight;
    this->canvAspect = (float)canvWidth / (float)canvHeight;

    //Init caches

    //Test scene 1: Shattrath
//    m_firstCamera.setCameraPos(-1663, 5098, 27); //Shattrath
//    m_firstCamera.setCameraPos(-241, 1176, 256); //Dark Portal
//
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
//   m_firstCamera.setCameraPos(  1252, 3095, 200); // Ulduar Raid
//    currentScene = new Map(this, 1803, "AzeriteBG1");
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
//    currentScene = new Map(this, 0, "Azeroth");
//
//   m_firstCamera.setCameraPos(-5025, -807, 500); //Ironforge
   m_firstCamera.setCameraPos(0, 0, 200);
    currentScene = new Map(this, 0, "Azeroth");
//
//    m_firstCamera.setCameraPos(-876, 775, 200); //Zaldalar
//    currentScene = new Map(this, 1642, "Zandalar");
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
//    m_firstCamera.setCameraPos(3062, 495, 200 ); //Valhalla
//    m_firstCamera.setCameraPos(2979, 3525, 200); //Field of the Eternal Hunt
//    currentScene = new Map(this, 1477, "Valhallas");
//
//    m_firstCamera.setCameraPos(2902, 2525, 200); //Field of the Eternal Hunt
//    m_firstCamera.setCameraPos(3993, 2302, 1043); //Field of the Eternal Hunt
//    currentScene = new Map(this, "NagaDungeon");
//    m_firstCamera.setCameraPos(829, -296, 200 ); //Field of the Eternal Hunt
//    currentScene = new Map(this, "unused");

//    m_firstCamera.setCameraPos(-2825, -4546, 200 ); //Field of the Eternal Hunt
//    currentScene = new Map(this, "ScenarioAlcazIsland");

//    m_firstCamera.setCameraPos( 0,  0, 470); // Panda ria
//    currentScene = new Map(this, "HawaiiMainLand");

//    m_firstCamera.setCameraPos(-12886, -165, 200); // Pandaria
//    currentScene = new Map(this, "Azeroth");
//
//   m_firstCamera.setCameraPos(0, 0, 0); // Pandaria
//    currentScene = new Map(this, "Ulduar80");
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
//        "WORLD\\EXPANSION02\\DOODADS\\CRYSTALSONGFOREST\\BUBBLE\\CAMOUFLAGEBUBBLE_CRYSTALSONG.m2");

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
//        "WORLD\\EXPANSION02\\DOODADS\\GENERIC\\SCOURGE\\SC_EYEOFACHERUS_02.m2");

//    currentScene = new M2Scene(this,
//        "character/bloodelf/female/bloodelffemale_hd.m2", 0);

    //Test scene 3: Ironforge
//    m_firstCamera.setCameraPos(1.78252912f,  33.4062042f, -126.937592f); //Room under dalaran
//    m_firstCamera.setCameraPos(-32.1193314, 0.432947099, 9.5181284); //Room with transparent window
//    currentScene = new WmoScene(this,
//        "world\\wmo\\brokenisles\\dalaran2.wmo");
//    currentScene = new WmoScene(this,
//        "world\\wmo\\northrend\\dalaran\\nd_dalaran.wmo");



//   m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new WmoScene(this,
//        "WORLD\\WMO\\NORTHREND\\BUILDINGS\\HUMAN\\ND_HUMAN_INN\\ND_HUMAN_INN.WMO");
//
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
//        "World/wmo/Dungeon/AZ_Subway/Subway.wmo");
//    currentScene = new WmoScene(this,
//        "world/wmo/dungeon/ulduar/ulduar_raid.wmo");


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
//        "world\\wmo\\azeroth\\buildings\\stormwind\\stormwind2.WMO");

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
//  m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new WmoScene(this,
//        "world/wmo/azeroth/buildings/worldtree/theworldtreehyjal.wmo");

//    m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new WmoScene(this,
//        "world/wmo/dungeon/argusraid/7du_argusraid_pantheon.wmo");
//
//   currentScene = new WmoScene(this,
//        "world/wmo/lorderon/undercity/8xp_undercity.wmo");

    db2Light = new DB2Light(db2Cache.get("dbfilesclient/light.db2"));
    db2LightData = new DB2LightData(db2Cache.get("dbfilesclient/LightData.db2"));
    db2WmoAreaTable = new DB2WmoAreaTable(db2Cache.get("dbfilesclient/WmoAreaTable.db2"));


    g_globalThreadsSingleton.loadingResourcesThread = std::thread([&]() {
        using namespace std::chrono_literals;

        while (!this->m_isTerminating) {
            std::this_thread::sleep_for(1ms);
            this->adtObjectCache.processCacheQueue(1000);
            this->wdtCache.processCacheQueue(1000);
            this->wdlCache.processCacheQueue(1000);
            this->wmoGeomCache.processCacheQueue(1000);
            this->wmoMainCache.processCacheQueue(100);
            this->m2GeomCache.processCacheQueue(1000);
            this->skinGeomCache.processCacheQueue(1000);
            this->textureCache.processCacheQueue(1000);
            this->db2Cache.processCacheQueue(1000);
        }
    });



    g_globalThreadsSingleton.cullingAndUpdateThread = std::thread(([&](){
        using namespace std::chrono_literals;
        std::unique_lock<std::mutex> localLockNextMeshes (m_lockNextMeshes,std::defer_lock);

        while (!this->m_isTerminating) {
            if (!deltaTimeUpdate) {
                std::this_thread::sleep_for(500us);
                continue;
            }

            localLockNextMeshes.lock();
            deltaTimeUpdate = false;
            DoCulling();
            localLockNextMeshes.unlock();
        }
    }));

}

void WoWSceneImpl::initGlContext() {

}

void WoWSceneImpl::setScreenSize(int canvWidth, int canvHeight) {
    this->canvWidth = canvWidth;
    this->canvHeight = canvHeight;
    this->canvAspect = (float)canvWidth / (float)canvHeight;
}

/* Shaders stuff */

void WoWSceneImpl::activateRenderFrameShader () {
//    glUseProgram(this->renderFrameShader->getProgram());
//    glActiveTexture(GL_TEXTURE0);
//
//    //glDisableVertexAttribArray(1);
//
//    float uResolution[2] = {(float)this->canvWidth, (float)this->canvHeight };
//    glUniform2fv(this->renderFrameShader->getUnf("uResolution"), 2, uResolution);
//
//    glUniform1i(this->renderFrameShader->getUnf("u_sampler"), 0);
//    if (this->renderFrameShader->hasUnf("u_depth")) {
//        glUniform1i(this->renderFrameShader->getUnf("u_depth"), 1);
//    }
}
/*
void WoWSceneImpl::activateTextureCompositionShader(GLuint texture) {
    glUseProgram(this->textureCompositionShader->getProgram());

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    if (this.textureCompVars.depthTexture) {
        GL_framebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this.textureCompVars.depthTexture, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, this.textureCompVars.textureCoords);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this.textureCompVars.elements);


    glVertexAttribPointer(this.currentShaderProgram.shaderAttributes.aTextCoord, 2, GL_FLOAT, false, 0, 0);  // position

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(this.currentShaderProgram.shaderUniforms.uTexture, 0);

    glDepthMask(true);
    glDisable(GL_CULL_FACE);

    glClearColor(0,0,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);
    glViewport(0,0,1024,1024);

}*/
void WoWSceneImpl::activateRenderDepthShader () {
//    glUseProgram(this->drawDepthBuffer->getProgram());
//    glActiveTexture(GL_TEXTURE0);
}

void WoWSceneImpl::activateDrawPortalShader () {
 /*
    glUseProgram(drawPortalShader->getProgram());

    glUniformMatrix4fv(drawPortalShader->getUnf("uLookAtMat"), 1, GL_FALSE, &this->m_lookAtMat4[0]);
    glUniformMatrix4fv(drawPortalShader->getUnf("uPMatrix"), 1, GL_FALSE, &this->m_perspectiveMatrix[0]);
 */
}


void WoWSceneImpl::drawTexturedQuad(GLuint texture,
                                    float x,
                                    float y,
                                    float width,
                                    float height,
                                    float canv_width,
                                    float canv_height,
                                    bool drawDepth) {
    /*
    glDisable(GL_DEPTH_TEST);
    glBindBuffer(GL_ARRAY_BUFFER, this->vertBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray(+drawDepthShader::Attribute::position);
    glVertexAttribPointer(+drawDepthShader::Attribute::position, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glUniform1f(this->drawDepthBuffer->getUnf("uWidth"), width/canv_width);
    glUniform1f(this->drawDepthBuffer->getUnf("uHeight"), height/canv_height);
    glUniform1f(this->drawDepthBuffer->getUnf("uX"), x/canv_width);
    glUniform1f(this->drawDepthBuffer->getUnf("uY"), y/canv_height);
    glUniform1i(this->drawDepthBuffer->getUnf("drawDepth"), (drawDepth) ? 1 : 0);

    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);
    */
}


void WoWSceneImpl::activateBoundingBoxShader() {
    /*
    glUseProgram(this->bbShader->getProgram());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo_elements);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo_vertices);

    //gl.enableVertexAttribArray(this.currentShaderProgram.shaderAttributes.aPosition);
    glVertexAttribPointer(+drawBBShader::Attribute::aPosition, 3, GL_FLOAT, GL_FALSE, 0, nullptr);  // position

    glUniformMatrix4fv(this->bbShader->getUnf("uLookAtMat"), 1, GL_FALSE, &this->m_lookAtMat4[0]);
    glUniformMatrix4fv(this->bbShader->getUnf("uPMatrix"), 1, GL_FALSE, &this->m_perspectiveMatrix[0]);
    */
}
void WoWSceneImpl::deactivateBoundingBoxShader() {

}

void WoWSceneImpl::activateFrustumBoxShader() {
    /*
    glUseProgram(this->drawFrustumShader->getProgram());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo_elements);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo_vertices);

    glEnableVertexAttribArray(+drawFrustumShader::Attribute::aPosition);
    glVertexAttribPointer(+drawFrustumShader::Attribute::aPosition, 3, GL_FLOAT, GL_FALSE, 0, nullptr);  // position

    glUniformMatrix4fv(this->drawFrustumShader->getUnf("uLookAtMat"), 1, GL_FALSE, &this->m_lookAtMat4[0]);
    glUniformMatrix4fv(this->drawFrustumShader->getUnf("uPMatrix"), 1, GL_FALSE, &this->m_perspectiveMatrix[0]);
    */

}

void WoWSceneImpl::activateDrawPointShader() {
    /*
    glUseProgram(drawPoints->getProgram());

    glEnableVertexAttribArray(+drawPoints::Attribute::aPosition);

    glUniformMatrix4fv(drawPoints->getUnf("uLookAtMat"), 1, GL_FALSE, &this->m_lookAtMat4[0]);
    glUniformMatrix4fv(drawPoints->getUnf("uPMatrix"), 1, GL_FALSE, &this->m_perspectiveMatrix[0]);
     */

}
void WoWSceneImpl::deactivateDrawPointShader() {

}

/****************/



void glClearScreen() {
#ifndef WITH_GLESv2
    glClearDepthf(1.0f);
#else
    glClearDepthf(1.0f);
#endif
    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
//    glClearColor(0.0, 0.0, 0.0, 0.0);
//    glClearColor(0.25, 0.06, 0.015, 0.0);
    glClearColor(0.117647, 0.207843, 0.392157, 1);
    //glClearColor(fogColor[0], fogColor[1], fogColor[2], 1);
//    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glDisable(GL_SCISSOR_TEST);
}

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
    struct timespec renderingAndUpdateStart, renderingAndUpdateEnd;
//    clock_gettime(CLOCK_MONOTONIC, &renderingAndUpdateStart);

    getDevice()->reset();
    glClearScreen();

    glViewport(0,0,this->canvWidth, this->canvHeight);

    sceneWideBlockVSPS &blockPSVS = m_sceneWideUniformBuffer->getObject<sceneWideBlockVSPS>();
    blockPSVS.uLookAtMat = m_currentFrameParams.m_lookAtMat4;
    blockPSVS.uPMatrix = m_currentFrameParams.m_perspectiveMatrix;

    m_sceneWideUniformBuffer->save();

    mathfu::mat4 mainLookAtMat4 = m_currentFrameParams.m_lookAtMat4;

    if (this->m_config->getDoubleCameraDebug()) {
        //Draw static camera
        m_isDebugCamera = true;
        m_currentFrameParams.m_lookAtMat4 = m_currentFrameParams.m_secondLookAtMat;
        currentScene->draw();
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
        m_currentFrameParams.m_lookAtMat4 = mainLookAtMat4;
        currentScene->draw();

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

    currentScene->doPostLoad(); //Do post load after rendering is done!

    struct timespec cullingAndUpdateStart, cullingAndUpdateEnd;
//    clock_gettime(CLOCK_MONOTONIC, &cullingAndUpdateStart);
//    DoCulling();
//    m_currentFrameParams = m_nextFrameParams;
//    clock_gettime(CLOCK_MONOTONIC, &cullingAndUpdateEnd);
//
//    print_timediff("DoCulling", cullingAndUpdateStart, cullingAndUpdateEnd);
    renderLockNextMeshes.lock();
    m_currentFrameParams = m_nextFrameParams;

    nextDeltaTime = deltaTime;
    deltaTimeUpdate = true;
    currentScene->copyToCurrentFrame();
    m_gdevice.toogleEvenFrame();
    renderLockNextMeshes.unlock();

    glFlush();

}
mathfu::mat3 blizzTranspose(mathfu::mat4 &value) {

    return mathfu::mat3(
        value.GetColumn(0).x,value.GetColumn(1).x,value.GetColumn(2).x,
        value.GetColumn(0).y,value.GetColumn(1).y,value.GetColumn(2).y,
        value.GetColumn(0).z,value.GetColumn(1).z,value.GetColumn(2).z
    );
}

void WoWSceneImpl::SetDirection(WoWFrameParamHolder &frameParamHolder) {

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
    if (g_globalThreadsSingleton.cullingAndUpdateThread.joinable()) {
        g_globalThreadsSingleton.cullingAndUpdateThread.join();
    }

    if (g_globalThreadsSingleton.loadingResourcesThread.joinable()) {
        g_globalThreadsSingleton.loadingResourcesThread.join();
    }
}

WoWScene * createWoWScene(Config *config, IFileRequest * requestProcessor, int canvWidth, int canvHeight){
#ifdef _WIN32
    glewExperimental = true; // Needed in core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return nullptr;
	}
#endif
#ifdef __ANDROID_API__
     std::cout.rdbuf(new androidbuf());
#endif

    return new WoWSceneImpl(config, requestProcessor, canvWidth, canvHeight);
}