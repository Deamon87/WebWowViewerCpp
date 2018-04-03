#include "opengl/header.h"
#include "wowScene.h"
#include "shaders.h"
#include "shader/ShaderRuntimeData.h"
#include "algorithms/mathHelper.h"
#include "objects/scenes/m2Scene.h"
#include "objects/scenes/wmoScene.h"
#include "androidLogSupport.h"

#include "mathfu/glsl_mappings.h"
#include "persistance/db2/DB2Light.h"
#include <iostream>
#include <cmath>

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
    this->m_config = config;

    this->canvWidth = canvWidth;
    this->canvHeight = canvHeight;
    this->canvAspect = (float)canvWidth / (float)canvHeight;

#ifndef WITH_GLESv2
    /* Allocate and assign a Vertex Array Object to our handle */
    glGenVertexArrays(1, &vao);

    /* Bind our Vertex Array Object as the current used object */
    glBindVertexArray(vao);
#endif

//    self.initGlContext(canvas);
    this->initArrayInstancedExt();
    this->initDepthTextureExt();
//    if (this->enableDeferred) {
//        this->initDeferredRendering();
//    }
    this->initVertBuffer();
    this->initRenderBuffers();
    this->initAnisotropicExt();
    this->initVertexArrayObjectExt();
    this->initCompressedTextureS3tcExt();

    this->initShaders();

    this->initSceneApi();
    this->initSceneGraph();
    this->createBlackPixelTexture();

    this->initBoxVBO();
    this->initTextureCompVBO();
    this->initCaches();
    this->initCamera();

    //Init caches

    //Test scene 1: Shattrath
//    m_firstCamera.setCameraPos(-1663, 5098, 27); //Shattrath
//    m_firstCamera.setCameraPos(-241, 1176, 256); //Dark Portal

//    currentScene = new Map(this, "Expansion01");
//    m_firstCamera.setCameraPos(972, 2083, 0); //Lost isles template
//    m_firstCamera.setCameraPos(-834, 4500, 0); //Dalaran 2
//    m_firstCamera.setCameraPos(-719, 2772, 317); //Near the black tower
//    m_firstCamera.setCameraPos( 4054, 7370, 27); // Druid class hall
//    currentScene = new Map(this, "Troll Raid");
//    currentScene = new Map(this, "BrokenShoreBattleshipFinale");

//    m_firstCamera.setCameraPos(-1663, 5098, 27);
    m_firstCamera.setCameraPos( -7134, 931, 27); // THE WOUND
    currentScene = new Map(this, "silithusphase01");
//
//    m_firstCamera.setCameraPos( -594, 4664, 200);
//    currentScene = new Map(this, "Artifact-MageOrderHall");

//    m_firstCamera.setCameraPos( 3733.33325, 2666.66675, 0);
//    currentScene = new Map(this, "BLTestMap");

//    currentScene = new Map(this, "argus_rifts");

//    m_secondCamera.setCameraPos(-1663, 5098, 27);
//
//    m_firstCamera.setCameraPos(5243.2461346537075f, 1938.6550422193939f, 717.0332923206179f); //HallsOfReflection
//    currentScene = new Map(this, "HallsOfReflection");
//     .go 668 5243 1938 760
    // .go 668 0 0 0

//    m_firstCamera.setCameraPos( 2290,  -9.475f, 470); // Ulduar Raid
//    currentScene = new Map(this, "UlduarRaid");
//
//   m_firstCamera.setCameraPos(  1252, 3095, 200); // Ulduar Raid
//    currentScene = new Map(this, "AzeriteBG1");
//
//    m_firstCamera.setCameraPos(  2843, 847, 200); // Ulduar Raid
//    currentScene = new Map(this, "Islands_7VR_Swamp_Prototype2");

//    m_firstCamera.setCameraPos( -2925, 8997, 200); // Antorus
//    m_firstCamera.setCameraPos( 5333.3335, 10666.667, 1); //Strange WMO
//    currentScene = new Map(this, "argus 1");

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
//    currentScene = new Map(this, "Northrend");
//
//    m_firstCamera.setCameraPos(-8517, 1104, 200); //Stormwind
//    currentScene = new Map(this, "Azeroth");
//
//    m_firstCamera.setCameraPos(-876, 775, 200); //Zaldalar
//    currentScene = new Map(this, "Zandalar");
//

//
//    m_firstCamera.setCameraPos(570, 979, 200); //Maelstorm Shaman
//    currentScene = new Map(this, "MaelstromShaman");
//
//    m_firstCamera.setCameraPos(3062, 495, 200 ); //Valhalla
//    m_firstCamera.setCameraPos(2979, 3525, 200); //Field of the Eternal Hunt
//    currentScene = new Map(this, "Valhallas");
//
//    m_firstCamera.setCameraPos(2902, 2525, 200); //Field of the Eternal Hunt
//    m_firstCamera.setCameraPos(3993, 2302, 1043); //Field of the Eternal Hunt
//    currentScene = new Map(this, "NagaDungeon");
//    m_firstCamera.setCameraPos(829, -296, 200 ); //Field of the Eternal Hunt
//    currentScene = new Map(this, "unused");w

//    m_firstCamera.setCameraPos(-2825, -4546, 200 ); //Field of the Eternal Hunt
//    currentScene = new Map(this, "ScenarioAlcazIsland");

//    m_firstCamera.setCameraPos( 0,  0, 470); // Pandaria
//    currentScene = new Map(this, "HawaiiMainLand");

//    m_firstCamera.setCameraPos(-12886, -165, 200); // Pandaria
//    currentScene = new Map(this, "Azeroth");
//
//   m_firstCamera.setCameraPos(0, 0, 0); // Pandaria
//    currentScene = new Map(this, "Ulduar80");
//
//    m_firstCamera.setCameraPos( -8517, 1104, 200);
//    currentScene = new Map(this, "escapefromstockades");


    //Test scene 2: tree from shattrath
//    m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new M2Scene(this,
//        "WORLD\\EXPANSION01\\DOODADS\\TEROKKAR\\TREES\\TEROKKARTREEMEDIUMPINECONES.m2");
//
//   m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new M2Scene(this,
//       "WORLD\\GENERIC\\HUMAN\\PASSIVE DOODADS\\BANNERS\\STORMWINDLIONBANNER.m2");
//
//    m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new M2Scene(this,
//        "world\\generic\\human\\passive doodads\\gryphonroost\\gryphonroost01.m2");

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
//        "world/expansion06/doodads/brokenshore/7bs_tombofsargerasfxbeam_01_reduced.m2");
//
//   m_firstCamera.setCameraPos(0, 0, 0);
//    currentScene = new M2Scene(this,
//                               "WORLD\\EXPANSION02\\DOODADS\\ULDUAR\\UL_SMALLSTATUE_DRUID.m2");
//   m_firstCamera.setCameraPos(0,  0, 0);
//    currentScene = new M2Scene(this,
//        "interface/glues/models/ui_mainmenu_northrend/ui_mainmenu_northrend.m2", 0);
//    currentScene = new M2Scene(this,
//        "interface/glues/models/ui_mainmenu_legion/ui_mainmenu_legion.m2", 0);
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

//    currentScene = new M2Scene(this,
//        "interface/glues/models/ui_worgen/ui_worgen.m2", 0);

//    currentScene = new M2Scene(this,
//        "\tinterface/glues/models/ui_pandaren/ui_pandaren.m2", 0);
//
//    currentScene = new M2Scene(this,
//        "interface/glues/models/ui_nightelf/ui_nightelf.m2", 0);

//    currentScene = new M2Scene(this,
//        "world/khazmodan/ironforge/passivedoodads/throne/dwarventhrone01.m2");

    //Test scene 3: Ironforge
//    m_firstCamera.setCameraPos(1.78252912,  33.4062042, -126.937592); //Room under dalaran
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
//        "world\\wmo\\dungeon\\mantidraid\\pa_mantid_raid.wmo");
//    currentScene = new WmoScene(this,
//        "world/wmo/dungeon/grimbatol/kz_grimbatol.wmo");
//    currentScene = new WmoScene(this,
//        "world/wmo/dungeon/grimbatol/kz_grimbatol_raid.wmo");


//    currentScene = new WmoScene(this,
//        "World/wmo/Dungeon/AZ_Subway/Subway.wmo");



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

    db2Light = new DB2Light(db2Cache.get("dbfilesclient/light.db2"));

#ifndef WITH_GLESv2
    glBindVertexArray(0);
#endif
}

ShaderRuntimeData * WoWSceneImpl::compileShader(std::string shaderName,
        std::string vertShaderString,
        std::string fragmentShaderString,
        std::string *vertExtraDefStringsExtern, std::string *fragExtraDefStringsExtern) {

    std::string vertExtraDefStrings = (vertExtraDefStringsExtern != nullptr) ? *vertExtraDefStringsExtern : "";
    std::string fragExtraDefStrings = (fragExtraDefStringsExtern != nullptr) ? *fragExtraDefStringsExtern : "";


    if (fragExtraDefStringsExtern == nullptr) {
        fragExtraDefStrings = "";
    }


    bool glsl330 = true;
#ifdef __ANDROID_API__
    glsl330 = false;
#endif
#ifdef __APPLE__
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
    glsl330 = false;
#elif TARGET_OS_IPHONE
    glsl330 = false;
#elif TARGET_OS_MAC
    glsl330 = true;
#else
#   error "Unknown Apple platform"
#endif
#endif

    if (glsl330) {
        vertExtraDefStrings = "#version 330\n" + vertExtraDefStrings;
        vertExtraDefStrings += "#define varying out\n";
        vertExtraDefStrings += "#define attribute in\n"
                "#define precision\n"
                "#define lowp\n"
                "#define mediump\n"
                "#define highp\n";

        fragExtraDefStrings = "#version 330\n" + fragExtraDefStrings;
        fragExtraDefStrings += "#define varying in\n"
                "#define precision\n"
                "#define lowp\n"
                "#define mediump\n"
                "#define highp\n";
//        fragExtraDefStrings += "#define gl_FragColorDef out vec4 gl_FragColor\n";

        //Insert gl_FragColor for glsl 330
        fragmentShaderString = trimmed(fragmentShaderString.insert(
                fragmentShaderString.find("void main(", fragmentShaderString.find("COMPILING_FS", 0)),
                "\n out vec4 gl_FragColor; \n"));
    } else {
        vertExtraDefStrings += "#version 100\n";

        fragExtraDefStrings += "#define gl_FragColorDef uniform vec4 notUsed\n";
    }

    if (m_enable) {
        vertExtraDefStrings = vertExtraDefStrings + "#define ENABLE_DEFERRED 1\r\n";
        fragExtraDefStrings = fragExtraDefStrings + "#define ENABLE_DEFERRED 1\r\n";
    }


    GLint maxVertexUniforms;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &maxVertexUniforms);
    int maxMatrixUniforms = (maxVertexUniforms / 4) - 9;

    vertExtraDefStrings = vertExtraDefStrings + "#define MAX_MATRIX_NUM "+std::to_string(maxMatrixUniforms)+"\r\n"+"#define COMPILING_VS 1\r\n ";
    fragExtraDefStrings = fragExtraDefStrings + "#define COMPILING_FS 1\r\n";

//    vertShaderString = trimmed(vertShaderString.insert(
//        vertShaderString.find("\n",vertShaderString.find("#version", 0)+1)+1,
//        vertExtraDefStrings));
//
//    fragmentShaderString = trimmed(fragmentShaderString.insert(
//            fragmentShaderString.find("\n",fragmentShaderString.find("#version", 0)+1)+1,
//            fragExtraDefStrings));

    vertShaderString = vertShaderString.insert(0, vertExtraDefStrings);
    fragmentShaderString = fragmentShaderString.insert(0, fragExtraDefStrings);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *vertexShaderConst = (const GLchar *)vertShaderString.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderConst, 0);
    glCompileShader(vertexShader);

    GLint success = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        // Something went wrong during compilation; get the error
        GLint maxLength = 0;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);
        std::cout << "\ncould not compile vertex shader "<<shaderName<<":" << std::endl
                  << vertexShaderConst << std::endl << std::endl
                  << "error: "<<std::string(infoLog.begin(),infoLog.end())<< std::endl <<std::flush;

        throw "" ;
    }

    /* 1.2 Compile fragment shader */
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *fragmentShaderConst = (const GLchar *) fragmentShaderString.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderConst, 0);
    glCompileShader(fragmentShader);

    // Check if it compiled
    success = 0;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        // Something went wrong during compilation; get the error
        GLint maxLength = 0;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);
        std::cout << "\ncould not compile fragment shader "<<shaderName<<":" << std::endl
            << fragmentShaderConst << std::endl << std::endl
            << "error: "<<std::string(infoLog.begin(),infoLog.end())<< std::endl <<std::flush;

        throw "" ;
    }

    /* 1.3 Link the program */
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    const shaderDefinition *shaderDefinition1 = getShaderDef(shaderName.c_str());
    for (int i = 0; i < shaderDefinition1->attributesNum; i++) {
        glBindAttribLocation(program, shaderDefinition1->attributes[i].number, shaderDefinition1->attributes[i].variableName);
    }

    // link the program.
    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status) {
        char logbuffer[1000];
        int loglen;
        glGetProgramInfoLog(program, sizeof(logbuffer), &loglen, logbuffer);
        std::cout << "OpenGL Program Linker Error: " << logbuffer << std::endl << std::flush;
        throw "could not compile shader:" ;
    }

    //Get Uniforms
    ShaderRuntimeData *data = new ShaderRuntimeData();
    data->setProgram(program);

    GLint count;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
//    printf("Active Uniforms: %d\n", count);
    for (GLint i = 0; i < count; i++)
    {
        const GLsizei bufSize = 32; // maximum name length
        GLchar name[bufSize]; // variable name in GLSL
        GLsizei length; // name length
        GLint size; // size of the variable
        GLenum type; // type of the variable (float, vec3 or mat4, etc)

        glGetActiveUniform(program, (GLuint)i, bufSize, &length, &size, &type, name);
        GLint location = glGetUniformLocation(program, name);

        data->setUnf(std::string(name), location);
//        printf("Uniform #%d Type: %u Name: %s Location: %d\n", i, type, name, location);
    }
//    if (!shaderName.compare("m2Shader")) {
//        std::cout << fragmentShaderString << std::endl << std::flush;
//    }


    return data;
}
void WoWSceneImpl::initShaders() {
    const std::string textureCompositionShader = getShaderDef("textureCompositionShader")->shaderString;
    this->textureCompositionShader = this->compileShader("textureCompositionShader", textureCompositionShader, textureCompositionShader);

    const  std::string renderFrameShader = getShaderDef("renderFrameBufferShader")->shaderString;
    this->renderFrameShader        = this->compileShader("renderFrameBufferShader", renderFrameShader, renderFrameShader);

    const  std::string drawDepthBuffer = getShaderDef("drawDepthShader")->shaderString;
    this->drawDepthBuffer          = this->compileShader("drawDepthShader", drawDepthBuffer, drawDepthBuffer);

    const  std::string readDepthBuffer = getShaderDef("readDepthBufferShader")->shaderString;
    this->readDepthBuffer          = this->compileShader("readDepthBufferShader", readDepthBuffer, readDepthBuffer);

    const  std::string wmoShader = getShaderDef("wmoShader")->shaderString;
    this->wmoShader                = this->compileShader("wmoShader", wmoShader, wmoShader);

    const  std::string m2Shader = getShaderDef("m2Shader")->shaderString;
    this->m2Shader                 = this->compileShader("m2Shader", m2Shader, m2Shader);
//    this->m2InstancingShader       = this->compileShader("m2Shader", m2Shader, m2Shader,
//                                                         new std::string("#define INSTANCED 1\r\n "),
//                                                         new std::string("#define INSTANCED 1\r\n "));

    const  std::string m2ParticleShader = getShaderDef("m2ParticleShader")->shaderString;
    this->m2ParticleShader         = this->compileShader("m2ParticleShader", m2ParticleShader, m2ParticleShader);

    const  std::string bbShader = getShaderDef("drawBBShader")->shaderString;
    this->bbShader                 = this->compileShader("drawBBShader", bbShader, bbShader);

    const  std::string adtShader = getShaderDef("adtShader")->shaderString;
    this->adtShader                = this->compileShader("adtShader", adtShader, adtShader);

    const  std::string drawPortalShader = getShaderDef("drawPortalShader")->shaderString;
    this->drawPortalShader         = this->compileShader("drawPortalShader", drawPortalShader, drawPortalShader);

    const  std::string drawFrustumShader = getShaderDef("drawFrustumShader")->shaderString;
    this->drawFrustumShader        = this->compileShader("drawFrustumShader", drawFrustumShader, drawFrustumShader);

    const  std::string drawLinesShader = getShaderDef("drawLinesShader")->shaderString;
    this->drawLinesShader          = this->compileShader("drawLinesShader", drawLinesShader, drawLinesShader);

    const  std::string drawPoints = getShaderDef("drawPoints")->shaderString;
    this->drawPoints          = this->compileShader("drawPoints", drawPoints, drawPoints);
}

void WoWSceneImpl::initGlContext() {

}
void WoWSceneImpl::createBlackPixelTexture() {
    unsigned int ff = 0;
    glGenTextures(1, &blackPixel);

    glBindTexture(GL_TEXTURE_2D, blackPixel);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &ff);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}
void WoWSceneImpl::initAnisotropicExt() {

}
void WoWSceneImpl::initArrayInstancedExt() {

}
void WoWSceneImpl::initBoxVBO() {
    //From https://en.wikibooks.org/wiki/OpenGL_Programming/Bounding_box
    static const float vertices[] = {
        -1, -1, -1, //0
        1, -1, -1,  //1
        1, -1, 1,   //2
        -1, -1, 1,  //3
        -1, 1, 1,   //4
        1, 1, 1,    //5
        1, 1, -1,   //6
        -1, 1, -1  //7
    };

    GLuint vbo_vertices;
    glGenBuffers(1, &vbo_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, GL_ZERO);

    static const uint16_t elements[] = {
            0, 1, 1, 2, 2, 3, 3, 0,
            4, 5, 5, 6, 6, 7, 7, 4,
            7, 6, 6, 1, 1, 0, 0, 7,
            3, 2, 2, 5, 5, 4, 4, 3,
            6, 5, 5, 2, 2, 1, 1, 6,
            0, 3, 3, 4, 4, 7, 7, 0
    };

    GLuint ibo_elements;
    glGenBuffers(1, &ibo_elements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_elements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_ZERO);

    this->vbo_vertices = vbo_vertices;
    this->ibo_elements = ibo_elements;
}
void WoWSceneImpl::initCaches() {

}
void WoWSceneImpl::initCamera() {

}
void WoWSceneImpl::initCompressedTextureS3tcExt() {

}
void WoWSceneImpl::initDeferredRendering() {

}
void WoWSceneImpl::initDepthTextureExt() {

}
void WoWSceneImpl::initDrawBuffers(int frameBuffer) {

}

void WoWSceneImpl::initVertexArrayObjectExt() {

}

void WoWSceneImpl::initVertBuffer(){
    static const float verts[] = {
            1,  1,
            -1,  1,
            -1, -1,
            1,  1,
            -1, -1,
            1,  -1,
    };
    const int vertsLength = sizeof(verts) ;/// sizeof(verts[0]);
//    std::cout << "vertsLength = " << vertsLength << std::endl;
    GLuint vertBuffer = 0;
    glGenBuffers(1, &vertBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
    glBufferData(GL_ARRAY_BUFFER, 48, &verts[0], GL_STATIC_DRAW);

    this->vertBuffer = vertBuffer;

}
void WoWSceneImpl::setScreenSize(int canvWidth, int canvHeight) {
    this->canvWidth = canvWidth;
    this->canvHeight = canvHeight;
    this->canvAspect = (float)canvWidth / (float)canvHeight;

    this->initRenderBuffers();
}

void WoWSceneImpl::initRenderBuffers() {
    //1. Free previous buffers
    if(this->frameBuffer != -1) {
        glDeleteFramebuffers(1, &this->frameBuffer);
    }
    if(this->frameBufferColorTexture != -1) {
        glDeleteTextures(1, &this->frameBufferColorTexture);
    }
    if(this->frameBufferDepthTexture != -1) {
        glDeleteTextures(1, &this->frameBufferDepthTexture);
    }

    GLuint framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLuint colorTexture = 0;
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->canvWidth, this->canvHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
//    // Create the depth texture
        GLuint depthTexture = 0;
//    if (this.depth_texture_ext) {
        glGenTextures(1, &depthTexture);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, this->canvWidth, this->canvHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,
                     nullptr);
//    }
//
//    if (!this.enableDeferred) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
//    }
//    if (this.depth_texture_ext) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
//    }

    glBindTexture(GL_TEXTURE_2D, 0);
    this->frameBuffer = framebuffer;
    this->frameBufferColorTexture = colorTexture;
    this->frameBufferDepthTexture = depthTexture;
//
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void WoWSceneImpl::initSceneApi() {

}

void WoWSceneImpl::initSceneGraph() {

}

void WoWSceneImpl::initTextureCompVBO() {

}

/* Shaders stuff */

void WoWSceneImpl::activateRenderFrameShader () {
    glUseProgram(this->renderFrameShader->getProgram());
    glActiveTexture(GL_TEXTURE0);

    //glDisableVertexAttribArray(1);

    float uResolution[2] = {(float)this->canvWidth, (float)this->canvHeight };
    glUniform2fv(this->renderFrameShader->getUnf("uResolution"), 2, uResolution);

    glUniform1i(this->renderFrameShader->getUnf("u_sampler"), 0);
    if (this->renderFrameShader->hasUnf("u_depth")) {
        glUniform1i(this->renderFrameShader->getUnf("u_depth"), 1);
    }
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
    glUseProgram(this->drawDepthBuffer->getProgram());
    glActiveTexture(GL_TEXTURE0);
}

void WoWSceneImpl::activateDrawPortalShader () {
    glUseProgram(drawPortalShader->getProgram());

    glUniformMatrix4fv(drawPortalShader->getUnf("uLookAtMat"), 1, GL_FALSE, &this->m_lookAtMat4[0]);
    glUniformMatrix4fv(drawPortalShader->getUnf("uPMatrix"), 1, GL_FALSE, &this->m_perspectiveMatrix[0]);
}


void WoWSceneImpl::drawTexturedQuad(GLuint texture,
                                    float x,
                                    float y,
                                    float width,
                                    float height,
                                    float canv_width,
                                    float canv_height,
                                    bool drawDepth) {
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
}


void WoWSceneImpl::activateM2Shader() {
    glUseProgram(this->m2Shader->getProgram());
//    glEnableVertexAttribArray(0);
//    if (!this.vao_ext) {
    this->activateM2ShaderAttribs();
//    }

    glUniformMatrix4fv(this->m2Shader->getUnf("uLookAtMat"), 1, GL_FALSE, &this->m_lookAtMat4[0]);
    glUniformMatrix4fv(this->m2Shader->getUnf("uPMatrix"), 1, GL_FALSE, &this->m_perspectiveMatrix[0]);

    glUniform1i(this->m2Shader->getUnf("uTexture"), 0);
    glUniform1i(this->m2Shader->getUnf("uTexture2"), 1);

    glUniform1f(this->m2Shader->getUnf("uFogStart"), this->uFogStart);
    glUniform1f(this->m2Shader->getUnf("uFogEnd"), this->uFogEnd);

    glUniform3fv(this->m2Shader->getUnf("uFogColor"), 1, &this->m_fogColor[0]);


    mathfu::vec4 ambient = this->currentScene->getAmbientColor();
    glUniform4fv(this->m2Shader->getUnf("uAmbientLight"), 1, &ambient[0]);

    mathfu::vec4 upVector ( 0.0, 0.0 , 1.0 , 0.0);
    upVector = (this->m_lookAtMat4.Transpose() * upVector);
    glUniform3fv(this->m2Shader->getUnf("uViewUp"), 1, &upVector[0]);
    glUniform3fv(this->m2Shader->getUnf("uSunDir"), 1, &m_sunDir[0]);

    mathfu::vec4 &sunColor = m_globalSunColor;
    glUniform3fv(this->m2Shader->getUnf("uSunColor"), 1, &sunColor[0]);

    glActiveTexture(GL_TEXTURE0);
}

void WoWSceneImpl::activateM2ParticleShader() {
    glUseProgram(this->m2ParticleShader->getProgram());
//    glEnableVertexAttribArray(0);
//    if (!this.vao_ext) {


    glEnableVertexAttribArray(+m2ParticleShader::Attribute::aPosition);
    glEnableVertexAttribArray(+m2ParticleShader::Attribute::aColor);
//    glEnableVertexAttribArray(+m2ParticleShader::Attribute::alpha);
    glEnableVertexAttribArray(+m2ParticleShader::Attribute::aTexcoord0);
    glEnableVertexAttribArray(+m2ParticleShader::Attribute::aTexcoord1);
    glEnableVertexAttribArray(+m2ParticleShader::Attribute::aTexcoord2);
//    }

    glUniformMatrix4fv(this->m2ParticleShader->getUnf("uPMatrix"), 1, GL_FALSE, &this->m_perspectiveMatrix[0]);
    glUniformMatrix4fv(this->m2ParticleShader->getUnf("uViewMatrix"), 1, GL_FALSE, &this->m_lookAtMat4[0]);
//    glUniformMatrix4fv(this->m2ParticleShader->getUnf("uViewMatrix"), 1, GL_FALSE, &mathfu::mat4::Identity()[0]);

    glUniform1i(this->m2ParticleShader->getUnf("uTexture"), 0);
    glUniform1i(this->m2ParticleShader->getUnf("uTexture2"), 1);
    glUniform1i(this->m2ParticleShader->getUnf("uTexture3"), 2);

    glActiveTexture(GL_TEXTURE0);
}
void WoWSceneImpl::deactivateM2ParticleShader() {
    glDisableVertexAttribArray(+m2ParticleShader::Attribute::aColor);
    glDisableVertexAttribArray(+m2ParticleShader::Attribute::aTexcoord0);
    glDisableVertexAttribArray(+m2ParticleShader::Attribute::aTexcoord1);
    glDisableVertexAttribArray(+m2ParticleShader::Attribute::aTexcoord2);
}


void WoWSceneImpl::deactivateM2Shader() {
    //glDisableVertexAttribArray(+m2Shader::Attribute::aPosition);
//    if (shaderAttributes.aNormal) {
    glDisableVertexAttribArray(+m2Shader::Attribute::aNormal);
//    }
    glDisableVertexAttribArray(+m2Shader::Attribute::boneWeights);
    glDisableVertexAttribArray(+m2Shader::Attribute::bones);
    glDisableVertexAttribArray(+m2Shader::Attribute::aTexCoord);
    glDisableVertexAttribArray(+m2Shader::Attribute::aTexCoord2);
}

void WoWSceneImpl::activateM2InstancingShader() {
    glUseProgram(this->m2InstancingShader->getProgram());

    glUniformMatrix4fv(this->m2InstancingShader->getUnf("uLookAtMat"), 1, GL_FALSE, &this->m_lookAtMat4[0]);
    glUniformMatrix4fv(this->m2InstancingShader->getUnf("uPMatrix"), 1, GL_FALSE, &this->m_perspectiveMatrix[0]);

    glUniform1i(this->m2InstancingShader->getUnf("uTexture"), 0);
    glUniform1i(this->m2InstancingShader->getUnf("uTexture2"), 1);

    glUniform1f(this->m2InstancingShader->getUnf("uFogStart"), this->uFogStart);
    glUniform1f(this->m2InstancingShader->getUnf("uFogEnd"), this->uFogEnd);

    glActiveTexture(GL_TEXTURE0);
    glEnableVertexAttribArray(+m2Shader::Attribute::aPosition);
//    if (shaderAttributes.aNormal) {
//        glEnableVertexAttribArray(+m2Shader::Attribute::aNormal);
//    }
    glEnableVertexAttribArray(+m2Shader::Attribute::boneWeights);
    glEnableVertexAttribArray(+m2Shader::Attribute::bones);
    glEnableVertexAttribArray(+m2Shader::Attribute::aTexCoord);
    glEnableVertexAttribArray(+m2Shader::Attribute::aTexCoord2);

    glEnableVertexAttribArray(+m2Shader::Attribute::aPlacementMat + 0);
    glEnableVertexAttribArray(+m2Shader::Attribute::aPlacementMat + 1);
    glEnableVertexAttribArray(+m2Shader::Attribute::aPlacementMat + 2);
    glEnableVertexAttribArray(+m2Shader::Attribute::aPlacementMat + 3);
//    glEnableVertexAttribArray(+m2Shader::Attribute::aAmbientColor);

#ifndef WITH_GLESv2
    if ( true/*instExt != null */) {
        glVertexAttribDivisor(+m2Shader::Attribute::aPlacementMat + 0, 1);
        glVertexAttribDivisor(+m2Shader::Attribute::aPlacementMat + 1, 1);
        glVertexAttribDivisor(+m2Shader::Attribute::aPlacementMat + 2, 1);
        glVertexAttribDivisor(+m2Shader::Attribute::aPlacementMat + 3, 1);
//        glVertexAttribDivisor(+m2Shader::Attribute::aAmbientColor, 1);
    }
#endif

    glUniform3fv(this->m2InstancingShader->getUnf("uFogColor"), 1, &this->m_fogColor[0]);
}

void WoWSceneImpl::deactivateM2InstancingShader() {

//    glDisableVertexAttribArray(+m2Shader::Attribute::aPosition);
//    if (shaderAttributes.aNormal) {
    glDisableVertexAttribArray(+m2Shader::Attribute::aNormal);
//    }
    glDisableVertexAttribArray(+m2Shader::Attribute::boneWeights);
    glDisableVertexAttribArray(+m2Shader::Attribute::bones);
    glDisableVertexAttribArray(+m2Shader::Attribute::aTexCoord);
    glDisableVertexAttribArray(+m2Shader::Attribute::aTexCoord2);
#ifndef WITH_GLESv2
    if (true /*instExt*/) {
        glVertexAttribDivisor(+m2Shader::Attribute::aPlacementMat + 0, 0);
        glVertexAttribDivisor(+m2Shader::Attribute::aPlacementMat + 1, 0);
        glVertexAttribDivisor(+m2Shader::Attribute::aPlacementMat + 2, 0);
        glVertexAttribDivisor(+m2Shader::Attribute::aPlacementMat + 3, 0);
    }
#endif
    glDisableVertexAttribArray(+m2Shader::Attribute::aPlacementMat + 0);
    glDisableVertexAttribArray(+m2Shader::Attribute::aPlacementMat + 1);
    glDisableVertexAttribArray(+m2Shader::Attribute::aPlacementMat + 2);
    glDisableVertexAttribArray(+m2Shader::Attribute::aPlacementMat + 3);

//    glEnableVertexAttribArray(0);
}

void WoWSceneImpl::activateBoundingBoxShader() {
    glUseProgram(this->bbShader->getProgram());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo_elements);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo_vertices);

    //gl.enableVertexAttribArray(this.currentShaderProgram.shaderAttributes.aPosition);
    glVertexAttribPointer(+drawBBShader::Attribute::aPosition, 3, GL_FLOAT, GL_FALSE, 0, nullptr);  // position

    glUniformMatrix4fv(this->bbShader->getUnf("uLookAtMat"), 1, GL_FALSE, &this->m_lookAtMat4[0]);
    glUniformMatrix4fv(this->bbShader->getUnf("uPMatrix"), 1, GL_FALSE, &this->m_perspectiveMatrix[0]);
}
void WoWSceneImpl::deactivateBoundingBoxShader() {

}

void WoWSceneImpl::activateM2ShaderAttribs() {
    glEnableVertexAttribArray(+m2Shader::Attribute::aPosition);
    glEnableVertexAttribArray(+m2Shader::Attribute::aNormal);
    glEnableVertexAttribArray(+m2Shader::Attribute::boneWeights);
    glEnableVertexAttribArray(+m2Shader::Attribute::bones);
    glEnableVertexAttribArray(+m2Shader::Attribute::aTexCoord);
    glEnableVertexAttribArray(+m2Shader::Attribute::aTexCoord2);
}

void WoWSceneImpl::activateWMOShader() {
    glUseProgram(this->wmoShader->getProgram());

    glEnableVertexAttribArray(+wmoShader::Attribute::aPosition);
//    if (shaderAttributes.aNormal) {
    glEnableVertexAttribArray(+wmoShader::Attribute::aNormal);
//    }
    glEnableVertexAttribArray(+wmoShader::Attribute::aTexCoord);
    glEnableVertexAttribArray(+wmoShader::Attribute::aTexCoord2);
    glEnableVertexAttribArray(+wmoShader::Attribute::aColor);
    glEnableVertexAttribArray(+wmoShader::Attribute::aColor2);

    glUniformMatrix4fv(this->wmoShader->getUnf("uLookAtMat"), 1, GL_FALSE, &this->m_lookAtMat4[0]);
    glUniformMatrix4fv(this->wmoShader->getUnf("uPMatrix"), 1, GL_FALSE, &this->m_perspectiveMatrix[0]);

    glUniform1i(this->wmoShader->getUnf("uTexture"), 0);
    glUniform1i(this->wmoShader->getUnf("uTexture2"), 1);

    glUniform1f(this->wmoShader->getUnf("uFogStart"), this->uFogStart);
    glUniform1f(this->wmoShader->getUnf("uFogEnd"), this->uFogEnd);

    glUniform3fv(this->wmoShader->getUnf("uFogColor"), 1, &this->m_fogColor[0]);

    mathfu::vec4 upVector ( 0.0, 0.0 , 1.0 , 0.0);
    upVector = (this->m_lookAtMat4.Transpose() * upVector);
    glUniform3fv(this->wmoShader->getUnf("uViewUp"), 1, &upVector[0]);
    glUniform3fv(this->wmoShader->getUnf("uSunDir"), 1, &m_sunDir[0]);

    mathfu::vec4 &sunColor = m_globalSunColor;
    glUniform3fv(this->wmoShader->getUnf("uSunColor"), 1, &sunColor[0]);


    glActiveTexture(GL_TEXTURE0);
}

void WoWSceneImpl::deactivateWMOShader() {
    glDisableVertexAttribArray(+wmoShader::Attribute::aNormal);
//    }
    glDisableVertexAttribArray(+wmoShader::Attribute::aTexCoord);
    glDisableVertexAttribArray(+wmoShader::Attribute::aTexCoord2);
    glDisableVertexAttribArray(+wmoShader::Attribute::aColor);
    glDisableVertexAttribArray(+wmoShader::Attribute::aColor2);
}
void WoWSceneImpl::activateAdtShader (){
    glUseProgram(adtShader->getProgram());

    glEnableVertexAttribArray(+adtShader::Attribute::aHeight);
    glEnableVertexAttribArray(+adtShader::Attribute::aIndex);
    glEnableVertexAttribArray(+adtShader::Attribute::aNormal);
    glEnableVertexAttribArray(+adtShader::Attribute::aColor);
    glEnableVertexAttribArray(+adtShader::Attribute::aVertexLighting);

    glUniformMatrix4fv(adtShader->getUnf("uLookAtMat"), 1, GL_FALSE, &this->m_lookAtMat4[0]);
    glUniformMatrix4fv(adtShader->getUnf("uPMatrix"), 1, GL_FALSE, &this->m_perspectiveMatrix[0]);

    glUniform1i(adtShader->getUnf("uAlphaTexture"), 0);
    glUniform1i(adtShader->getUnf("uLayer0"), 1);
    glUniform1i(adtShader->getUnf("uLayer1"), 2);
    glUniform1i(adtShader->getUnf("uLayer2"), 3);
    glUniform1i(adtShader->getUnf("uLayer3"), 4);
    glUniform1i(adtShader->getUnf("uLayerHeight0"), 5);
    glUniform1i(adtShader->getUnf("uLayerHeight1"), 6);
    glUniform1i(adtShader->getUnf("uLayerHeight2"), 7);
    glUniform1i(adtShader->getUnf("uLayerHeight3"), 8);

    glUniform1f(adtShader->getUnf("uFogStart"), this->uFogStart);
    glUniform1f(adtShader->getUnf("uFogEnd"), this->uFogEnd);

    glUniform3fv(adtShader->getUnf("uFogColor"), 1, &this->m_fogColor[0]);

    mathfu::vec4 ambient = this->currentScene->getAmbientColor();
    glUniform4fv(this->adtShader->getUnf("uAmbientLight"), 1, &ambient[0]);

    mathfu::vec4 upVector ( 0.0, 0.0 , 1.0 , 0.0);

    upVector = (this->m_lookAtMat4.Transpose() * upVector);
    glUniform3fv(this->adtShader->getUnf("uViewUp"), 1, &upVector[0]);
    glUniform3fv(this->adtShader->getUnf("uSunDir"), 1, &m_sunDir[0]);

    mathfu::vec4 &sunColor = m_globalSunColor;
    glUniform3fv(this->adtShader->getUnf("uSunColor"), 1, &sunColor[0]);
}
void WoWSceneImpl::deactivateAdtShader() {
    glDisableVertexAttribArray(+adtShader::Attribute::aIndex);
    glDisableVertexAttribArray(+adtShader::Attribute::aColor);
    glDisableVertexAttribArray(+adtShader::Attribute::aNormal);
    glDisableVertexAttribArray(+adtShader::Attribute::aVertexLighting);
    glUseProgram(0);
}

void WoWSceneImpl::activateFrustumBoxShader() {
    glUseProgram(drawFrustumShader->getProgram());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo_elements);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo_vertices);

    glEnableVertexAttribArray(+drawFrustumShader::Attribute::aPosition);
    glVertexAttribPointer(+drawFrustumShader::Attribute::aPosition, 3, GL_FLOAT, GL_FALSE, 0, nullptr);  // position

    glUniformMatrix4fv(drawFrustumShader->getUnf("uLookAtMat"), 1, GL_FALSE, &this->m_lookAtMat4[0]);
    glUniformMatrix4fv(drawFrustumShader->getUnf("uPMatrix"), 1, GL_FALSE, &this->m_perspectiveMatrix[0]);

}

void WoWSceneImpl::activateDrawPointShader() {
    glUseProgram(drawPoints->getProgram());

    glEnableVertexAttribArray(+drawPoints::Attribute::aPosition);

    glUniformMatrix4fv(drawPoints->getUnf("uLookAtMat"), 1, GL_FALSE, &this->m_lookAtMat4[0]);
    glUniformMatrix4fv(drawPoints->getUnf("uPMatrix"), 1, GL_FALSE, &this->m_perspectiveMatrix[0]);

}
void WoWSceneImpl::deactivateDrawPointShader() {

}

/****************/

GLuint WoWSceneImpl::getBlackPixelTexture(){
    return blackPixel;
}

void glClearScreen() {
#ifndef WITH_GLESv2
    glClearDepth(1.0);
#else
    glClearDepthf(1.0f);
#endif
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glDisable(GL_BLEND);
    glClearColor(0.0, 0.0, 0.0, 1);
//    glClearColor(0.117647, 0.207843, 0.392157, 1);
    //glClearColor(fogColor[0], fogColor[1], fogColor[2], 1);
//    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDisable(GL_CULL_FACE);

    glDisable(GL_SCISSOR_TEST);
}

void WoWSceneImpl::drawCamera () {
    glDisable(GL_DEPTH_TEST);

    mathfu::mat4 invViewFrustum = this->m_viewCameraForRender.Inverse();

    glUniformMatrix4fv(drawFrustumShader->getUnf("uInverseViewProjection"), 1, GL_FALSE, &invViewFrustum[0]);

    glDrawElements(GL_LINES, 48, GL_UNSIGNED_SHORT, 0);
    glEnable(GL_DEPTH_TEST);
}

void WoWSceneImpl::draw(animTime_t deltaTime) {
#ifndef WITH_GLESv2
    glBindVertexArray(vao);
#endif

    glClearScreen();
    mathfu::vec3 *cameraVector;
    float ambient[4];
    m_config->getAmbientColor(ambient);
    m_globalAmbientColor = mathfu::vec4(ambient[0],ambient[1],ambient[2],ambient[3]) * 2.0f;

    float sunColor[4];
    m_config->getSunColor(sunColor);
    m_globalSunColor = mathfu::vec4(sunColor[0],sunColor[1],sunColor[2],sunColor[3]) * 2.0f;

    static const mathfu::vec3 upVector(0,0,1);

    float farPlane = 300;
    float nearPlane = 1;
    float fov = toRadian(45.0);

//    float diagFov = 2.0944;
//    float fov = diagFov / sqrt(1 + canvAspect*canvAspect);

    //If use camera settings
    //Figure out way to assign the object with camera
    if (!m_config->getUseSecondCamera()){
        this->m_firstCamera.tick(deltaTime);
    } else {
        this->m_secondCamera.tick(deltaTime);
    }

    M2CameraResult cameraResult;
    mathfu::mat4 lookAtMat4;
    mathfu::vec4 cameraVec4;
    if ( currentScene->getCameraSettings(cameraResult)) {
        farPlane = cameraResult.far_clip;
        nearPlane = cameraResult.near_clip;
        fov = cameraResult.diagFov/ sqrt(1 + canvAspect*canvAspect);

        lookAtMat4 =
            mathfu::mat4::LookAt(
                -cameraResult.target_position.xyz()+cameraResult.position.xyz(),
                mathfu::vec3(0,0,0),
                upVector) * mathfu::mat4::FromTranslationVector(-cameraResult.position.xyz());
        cameraVec4 = cameraResult.position;
        m_lookAtMat4 = lookAtMat4;

    } else {
        cameraVec4 = mathfu::vec4(m_firstCamera.getCameraPosition(), 1);
        lookAtMat4 = this->m_firstCamera.getLookatMat();
        m_lookAtMat4 = lookAtMat4;
    }

    if (this->uFogStart < 0) {
        this->uFogStart = farPlane+8000;
    }
    if (this->uFogEnd < 0) {
        this->uFogEnd = farPlane+12000;
    }
//    mathfu::mat4 lookAtMat4 =
//        mathfu::mat4::LookAt(
//                this->m_firstCamera.getCameraPosition(),
//                this->m_firstCamera.getCameraLookAt(),
//                upVector);

// Second camera for debug
    mathfu::mat4 secondLookAtMat =
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
//    float o_height = (this->canvHeight * (533.333/256/* zoom 5 in Alram viewer */))/ 8 ;
//    float o_width = o_height * this->canvWidth / this->canvHeight ;

//    mathfu::mat4 perspectiveMatrix =
//        mathfu::mat4::Ortho(
//                -o_width,
//                o_width,
//                -o_height,
//                o_height,
//                1,
//                4000);
//
    m_perspectiveMatrix = perspectiveMatrix;

    mathfu::mat4 perspectiveMatrixForCulling =
            mathfu::mat4::Perspective(
                    fov,
                    this->canvAspect,
                    nearPlane,
                    farPlane-50);
    //Camera for rendering
    mathfu::mat4 perspectiveMatrixForCameraRender =
            mathfu::mat4::Perspective(fov,
                                        this->canvAspect,
                                        nearPlane,
                                        farPlane);
    mathfu::mat4 viewCameraForRender =
            perspectiveMatrixForCameraRender * lookAtMat4;

    this->m_viewCameraForRender = viewCameraForRender;
    this->SetDirection();

    if (db2Light->getIsLoaded()) {
        db2Light->getRecord(1);
    }

    this->adtObjectCache.processCacheQueue(10);
    this->wdtCache.processCacheQueue(10);
    this->wdlCache.processCacheQueue(10);
    this->wmoGeomCache.processCacheQueue(10);
    this->wmoMainCache.processCacheQueue(10);
    this->m2GeomCache.processCacheQueue(10);
    this->skinGeomCache.processCacheQueue(10);
    this->textureCache.processCacheQueue(10);
    this->db2Cache.processCacheQueue(10);

    mathfu::vec3 cameraVec3 = cameraVec4.xyz();
    currentScene->update(deltaTime, cameraVec3, perspectiveMatrixForCulling, lookAtMat4);
//    this.worldObjectManager.update(deltaTime, cameraPos, lookAtMat4);
//
    currentScene->checkCulling(perspectiveMatrixForCulling, lookAtMat4, cameraVec4);
//

    glViewport(0,0,this->canvWidth, this->canvHeight);

    if (this->m_config->getDoubleCameraDebug()) {
        //Draw static camera
        m_isDebugCamera = true;
        this->m_lookAtMat4 = secondLookAtMat;
        currentScene->draw();
        m_isDebugCamera = false;

        if (this->m_config->getDrawDepthBuffer() /*&& this.depth_texture_ext*/) {
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
        } else {
            //Render real camera
            glBindFramebuffer(GL_FRAMEBUFFER, this->frameBuffer);
            glClearScreen(/*this.fogColor*/);
            glDepthMask(GL_TRUE);

            this->m_lookAtMat4 = lookAtMat4;
            currentScene->draw();
            glBindFramebuffer(GL_FRAMEBUFFER, GL_ZERO);

            this->activateRenderDepthShader();
            this->drawTexturedQuad(this->frameBufferColorTexture,
                                   this->canvWidth * 0.60f,
                                   0,//this.canvas.height * 0.75,
                                   this->canvWidth * 0.40f,
                                   this->canvHeight * 0.40f,
                                   this->canvWidth,
                                   this->canvHeight, false);


        }
    } else {
        //Render real camera
        this->m_lookAtMat4 = lookAtMat4;
        currentScene->draw();

        if (this->m_config->getDrawDepthBuffer() /*&& this.depth_texture_ext*/) {
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
        }
    }

#ifndef WITH_GLESv2
    glBindVertexArray(0);
#endif
}

void WoWSceneImpl::SetDirection() {

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

    mathfu::vec4 sunDirWorld = mathfu::vec4(sinPhi * cosTheta, sinPhi * sinTheta, cosPhi, 0);
    m_sunDir = (m_lookAtMat4 * sunDirWorld).xyz();
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
     std::cout.rdbuf(new androidbuf);
#endif

    return new WoWSceneImpl(config, requestProcessor, canvWidth, canvHeight);
}