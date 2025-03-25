//
// Created by Deamon on 7/16/2017.
//
#include <algorithm>
#include <iostream>
#include <set>
#include <cmath>
#include "map.h"
#include "../../algorithms/mathHelper.h"
#include "../../algorithms/grahamScan.h"
#include "../../persistance/wdtFile.h"
#include "../../../gapi/interface/meshes/IM2Mesh.h"
#include "../../../gapi/interface/IDevice.h"
#include "../../algorithms/quick-sort-omp.h"
#include "../../../gapi/UniformBufferStructures.h"
#include <ShaderDefinitions.h>
#include "tbb/tbb.h"
#include "../../algorithms/FrameCounter.h"

#if (__AVX__ && __SSE2__)
#include "../../algorithms/mathHelper_culling_sse.h"
#endif
#include "../../algorithms/mathHelper_culling.h"
#include "../../../gapi/interface/materials/IMaterial.h"
#include "../../../renderer/frame/FrameProfile.h"
#include "map_load_max_contants.h"


std::array<mathfu::vec4, 122> skyConusVBO = {
    {
       {0, 0, 0.2928932309150696f, 0},
       {0, 0.4903479814529419f, 0.13914519548416138f, 1},
       {0.12691140174865723f, 0.4736397862434387f, 0.13914519548416138f, 1},
       {0.24517399072647095f, 0.4246537685394287f, 0.13914519548416138f, 1},
       {0.3467283844947815f, 0.3467283844947815f, 0.13914519548416138f, 1},
       {0.4246538281440735f, 0.24517402052879333f, 0.13914519548416138f, 1},
       {0.4736397862434387f, 0.1269114464521408f, 0.13914519548416138f, 1},
       {0.4903479516506195f, -4.286758326088602e-8f, 0.13914519548416138f, 1},
       {0.4736397862434387f, -0.12691152095794678f, 0.13914519548416138f, 1},
       {0.4246537685394287f, -0.2451740801334381f, 0.13914519548416138f, 1},
       {0.3467283844947815f, -0.34672844409942627f, 0.13914519548416138f, 1},
       {0.24517402052879333f, -0.4246538281440735f, 0.13914519548416138f, 1},
       {0.12691132724285126f, -0.4736397862434387f, 0.13914519548416138f, 1},
       {-4.286758326088602e-8f, -0.4903479516506195f, 0.13914519548416138f, 1},
       {-0.12691141664981842f, -0.4736397862434387f, 0.13914519548416138f, 1},
       {-0.2451740801334381f, -0.42465370893478394f, 0.13914519548416138f, 1},
       {-0.34672844409942627f, -0.34672826528549194f, 0.13914519548416138f, 1},
       {-0.4246538281440735f, -0.2451738715171814f, 0.13914519548416138f, 1},
       {-0.4736397862434387f, -0.12691129744052887f, 0.13914519548416138f, 1},
       {-0.4903479516506195f, 8.573516652177204e-8f, 0.13914519548416138f, 1},
       {-0.4736397862434387f, 0.126911461353302f, 0.13914519548416138f, 1},
       {-0.4246538281440735f, 0.24517402052879333f, 0.13914519548416138f, 1},
       {-0.34672826528549194f, 0.3467285633087158f, 0.13914519548416138f, 1},
       {-0.2451738715171814f, 0.42465388774871826f, 0.13914519548416138f, 1},
       {-0.12691129744052887f, 0.4736398458480835f, 0.13914519548416138f, 1},
       {0, 0.5679999589920044f, 0.08489322662353516f, 2},
       {0.1470092087984085f, 0.5486457943916321f, 0.08489322662353516f, 2},
       {0.2839999794960022f, 0.49190235137939453f, 0.08489322662353516f, 2},
       {0.40163663029670715f, 0.40163663029670715f, 0.08489322662353516f, 2},
       {0.4919024109840393f, 0.2840000092983246f, 0.08489322662353516f, 2},
       {0.5486457943916321f, 0.1470092535018921f, 0.08489322662353516f, 2},
       {0.5679998993873596f, -4.965613342733377e-8f, 0.08489322662353516f, 2},
       {0.5486457943916321f, -0.14700935781002045f, 0.08489322662353516f, 2},
       {0.49190235137939453f, -0.28400006890296936f, 0.08489322662353516f, 2},
       {0.40163663029670715f, -0.40163668990135193f, 0.08489322662353516f, 2},
       {0.2840000092983246f, -0.4919024109840393f, 0.08489322662353516f, 2},
       {0.14700911939144135f, -0.5486457943916321f, 0.08489322662353516f, 2},
       {-4.965613342733377e-8f, -0.5679998993873596f, 0.08489322662353516f, 2},
       {-0.1470092236995697f, -0.5486457943916321f, 0.08489322662353516f, 2},
       {-0.28400006890296936f, -0.49190229177474976f, 0.08489322662353516f, 2},
       {-0.40163668990135193f, -0.4016364812850952f, 0.08489322662353516f, 2},
       {-0.4919024109840393f, -0.28399983048439026f, 0.08489322662353516f, 2},
       {-0.5486457943916321f, -0.14700908958911896f, 0.08489322662353516f, 2},
       {-0.5679998993873596f, 9.931226685466754e-8f, 0.08489322662353516f, 2},
       {-0.5486457943916321f, 0.14700926840305328f, 0.08489322662353516f, 2},
       {-0.4919024109840393f, 0.2840000092983246f, 0.08489322662353516f, 2},
       {-0.4016364812850952f, 0.4016368091106415f, 0.08489322662353516f, 2},
       {-0.28399983048439026f, 0.49190250039100647f, 0.08489322662353516f, 2},
       {-0.14700908958911896f, 0.5486459136009216f, 0.08489322662353516f, 2},
       {0, 0.6413320302963257f, 0.024161219596862793f, 3},
       {0.16598893702030182f, 0.6194791793823242f, 0.024161219596862793f, 3},
       {0.32066601514816284f, 0.5554097890853882f, 0.024161219596862793f, 3},
       {0.4534902274608612f, 0.4534902274608612f, 0.024161219596862793f, 3},
       {0.555409848690033f, 0.32066604495048523f, 0.024161219596862793f, 3},
       {0.6194791793823242f, 0.1659889966249466f, 0.024161219596862793f, 3},
       {0.6413319706916809f, -5.606702657701135e-8f, 0.024161219596862793f, 3},
       {0.6194791793823242f, -0.16598911583423615f, 0.024161219596862793f, 3},
       {0.5554097890853882f, -0.3206661343574524f, 0.024161219596862793f, 3},
       {0.4534902274608612f, -0.453490287065506f, 0.024161219596862793f, 3},
       {0.32066604495048523f, -0.555409848690033f, 0.024161219596862793f, 3},
       {0.16598884761333466f, -0.6194791793823242f, 0.024161219596862793f, 3},
       {-5.606702657701135e-8f, -0.6413319706916809f, 0.024161219596862793f, 3},
       {-0.1659889668226242f, -0.6194791793823242f, 0.024161219596862793f, 3},
       {-0.3206661343574524f, -0.5554097294807434f, 0.024161219596862793f, 3},
       {-0.453490287065506f, -0.45349007844924927f, 0.024161219596862793f, 3},
       {-0.555409848690033f, -0.3206658661365509f, 0.024161219596862793f, 3},
       {-0.6194791793823242f, -0.16598880290985107f, 0.024161219596862793f, 3},
       {-0.6413319706916809f, 1.121340531540227e-7f, 0.024161219596862793f, 3},
       {-0.6194791793823242f, 0.16598902642726898f, 0.024161219596862793f, 3},
       {-0.555409848690033f, 0.32066604495048523f, 0.024161219596862793f, 3},
       {-0.45349007844924927f, 0.4534904360771179f, 0.024161219596862793f, 3},
       {-0.3206658661365509f, 0.5554099082946777f, 0.024161219596862793f, 3},
       {-0.16598880290985107f, 0.619479238986969f, 0.024161219596862793f, 3},
       {0, 0.6530885696411133f, 0.013454735279083252f, 4},
       {0.1690317541360855f, 0.6308351159095764f, 0.013454735279083252f, 4},
       {0.32654428482055664f, 0.5655912160873413f, 0.013454735279083252f, 4},
       {0.4618033468723297f, 0.4618033468723297f, 0.013454735279083252f, 4},
       {0.5655913352966309f, 0.32654431462287903f, 0.013454735279083252f, 4},
       {0.6308351159095764f, 0.16903181374073029f, 0.013454735279083252f, 4},
       {0.6530885100364685f, -5.7094815986147296e-8f, 0.013454735279083252f, 4},
       {0.6308351159095764f, -0.16903193295001984f, 0.013454735279083252f, 4},
       {0.5655912160873413f, -0.3265444040298462f, 0.013454735279083252f, 4},
       {0.4618033468723297f, -0.4618034362792969f, 0.013454735279083252f, 4},
       {0.32654431462287903f, -0.5655913352966309f, 0.013454735279083252f, 4},
       {0.16903166472911835f, -0.6308351159095764f, 0.013454735279083252f, 4},
       {-5.7094815986147296e-8f, -0.6530885100364685f, 0.013454735279083252f, 4},
       {-0.1690317839384079f, -0.6308351159095764f, 0.013454735279083252f, 4},
       {-0.3265444040298462f, -0.5655911564826965f, 0.013454735279083252f, 4},
       {-0.4618034362792969f, -0.4618031978607178f, 0.013454735279083252f, 4},
       {-0.5655913352966309f, -0.3265441358089447f, 0.013454735279083252f, 4},
       {-0.6308351159095764f, -0.16903162002563477f, 0.013454735279083252f, 4},
       {-0.6530885100364685f, 1.1418963197229459e-7f, 0.013454735279083252f, 4},
       {-0.6308351159095764f, 0.16903184354305267f, 0.013454735279083252f, 4},
       {-0.5655913352966309f, 0.32654431462287903f, 0.013454735279083252f, 4},
       {-0.4618031978607178f, 0.4618035852909088f, 0.013454735279083252f, 4},
       {-0.3265441358089447f, 0.5655913949012756f, 0.013454735279083252f, 4},
       {-0.16903162002563477f, 0.6308351755142212f, 0.013454735279083252f, 4},
       {0, 0.6875f, -0.004999999888241291f, 5},
       {0.1779380887746811f, 0.6640740036964417f, -0.004999999888241291f, 5},
       {0.34375f, 0.5953924059867859f, -0.004999999888241291f, 5},
       {0.48613590002059937f, 0.48613590002059937f, -0.004999999888241291f, 5},
       {0.5953924655914307f, 0.3437500298023224f, -0.004999999888241291f, 5},
       {0.6640740036964417f, 0.17793814837932587f, -0.004999999888241291f, 5},
       {0.6874999403953552f, -6.010316155879991e-8f, -0.004999999888241291f, 5},
       {0.6640740036964417f, -0.1779382824897766f, -0.004999999888241291f, 5},
       {0.5953924059867859f, -0.34375011920928955f, -0.004999999888241291f, 5},
       {0.48613590002059937f, -0.48613598942756653f, -0.004999999888241291f, 5},
       {0.3437500298023224f, -0.5953924655914307f, -0.004999999888241291f, 5},
       {0.17793798446655273f, -0.6640740036964417f, -0.004999999888241291f, 5},
       {-6.010316155879991e-8f, -0.6874999403953552f, -0.004999999888241291f, 5},
       {-0.17793811857700348f, -0.6640740036964417f, -0.004999999888241291f, 5},
       {-0.34375011920928955f, -0.5953923463821411f, -0.004999999888241291f, 5},
       {-0.48613598942756653f, -0.4861357510089874f, -0.004999999888241291f, 5},
       {-0.5953924655914307f, -0.3437498211860657f, -0.004999999888241291f, 5},
       {-0.6640740036964417f, -0.17793795466423035f, -0.004999999888241291f, 5},
       {-0.6874999403953552f, 1.2020632311759982e-7f, -0.004999999888241291f, 5},
       {-0.6640740036964417f, 0.17793817818164825f, -0.004999999888241291f, 5},
       {-0.5953924655914307f, 0.3437500298023224f, -0.004999999888241291f, 5},
       {-0.4861357510089874f, 0.48613613843917847f, -0.004999999888241291f, 5},
       {-0.3437498211860657f, 0.5953925848007202f, -0.004999999888241291f, 5},
       {-0.17793795466423035f, 0.6640740633010864f, -0.004999999888241291f, 5},
       {  0,  0,  -1.7071068286895752f, 5 }
   }
};

std::array<uint16_t,300> skyConusIBO = {
    0 ,  1 ,  0  ,
    2 ,  0 ,  3  ,
    0 ,  4 ,  0  ,
    5 ,  0 ,  6  ,
    0 ,  7 ,  0  ,
    8 ,  0 ,  9  ,
    0 ,  10 ,  0  ,
    11 ,  0 ,  12  ,
    0 ,  13 ,  0  ,
    14 ,  0 ,  15  ,
    0 ,  16 ,  0  ,
    17 ,  0 ,  18  ,
    0 ,  19 ,  0  ,
    20 ,  0 ,  21  ,
    0 ,  22 ,  0  ,
    23 ,  0 ,  24  ,
    0 ,  1 ,  1  ,
    25 ,  2 ,  26  ,
    3 ,  27 ,  4  ,
    28 ,  5 ,  29  ,
    6 ,  30 ,  7  ,
    31 ,  8 ,  32  ,
    9 ,  33 ,  10  ,
    34 ,  11 ,  35  ,
    12 ,  36 ,  13  ,
    37 ,  14 ,  38  ,
    15 ,  39 ,  16  ,
    40 ,  17 ,  41  ,
    18 ,  42 ,  19  ,
    43 ,  20 ,  44  ,
    21 ,  45 ,  22  ,
    46 ,  23 ,  47  ,
    24 ,  48 ,  1  ,
    25 ,  25 ,  49  ,
    26 ,  50 ,  27  ,
    51 ,  28 ,  52  ,
    29 ,  53 ,  30  ,
    54 ,  31 ,  55  ,
    32 ,  56 ,  33  ,
    57 ,  34 ,  58  ,
    35 ,  59 ,  36  ,
    60 ,  37 ,  61  ,
    38 ,  62 ,  39  ,
    63 ,  40 ,  64  ,
    41 ,  65 ,  42  ,
    66 ,  43 ,  67  ,
    44 ,  68 ,  45  ,
    69 ,  46 ,  70  ,
    47 ,  71 ,  48  ,
    72 ,  25 ,  49  ,
    49 ,  73 ,  50  ,
    74 ,  51 ,  75  ,
    52 ,  76 ,  53  ,
    77 ,  54 ,  78  ,
    55 ,  79 ,  56  ,
    80 ,  57 ,  81  ,
    58 ,  82 ,  59  ,
    83 ,  60 ,  84  ,
    61 ,  85 ,  62  ,
    86 ,  63 ,  87  ,
    64 ,  88 ,  65  ,
    89 ,  66 ,  90  ,
    67 ,  91 ,  68  ,
    92 ,  69 ,  93  ,
    70 ,  94 ,  71  ,
    95 ,  72 ,  96  ,
    49 ,  73 ,  73  ,
    97 ,  74 ,  98  ,
    75 ,  99 ,  76  ,
    100 ,  77 ,  101  ,
    78 ,  102 ,  79  ,
    103 ,  80 ,  104  ,
    81 ,  105 ,  82  ,
    106 ,  83 ,  107  ,
    84 ,  108 ,  85  ,
    109 ,  86 ,  110  ,
    87 ,  111 ,  88  ,
    112 ,  89 ,  113  ,
    90 ,  114 ,  91  ,
    115 ,  92 ,  116  ,
    93 ,  117 ,  94  ,
    118 ,  95 ,  119  ,
    96 ,  120 ,  73  ,
    97 ,  97 ,  121  ,
    98 ,  121 ,  99  ,
    121 ,  100 ,  121  ,
    101 ,  121 ,  102  ,
    121 ,  103 ,  121  ,
    104 ,  121 ,  105  ,
    121 ,  106 ,  121  ,
    107 ,  121 ,  108  ,
    121 ,  109 ,  121  ,
    110 ,  121 ,  111  ,
    121 ,  112 ,  121  ,
    113 ,  121 ,  114  ,
    121 ,  115 ,  121  ,
    116 ,  121 ,  117  ,
    121 ,  118 ,  121  ,
    119 ,  121 ,  120  ,
    121 ,  97 ,  121  ,
};

HGVertexBufferBindings createSkyBindings(const HMapSceneBufferCreate &sceneRenderer) {
    auto skyIBO = sceneRenderer->createSkyIndexBuffer(skyConusIBO.size() * sizeof(uint16_t));
    skyIBO->uploadData(
        skyConusIBO.data(),
        skyConusIBO.size() * sizeof(uint16_t));

    auto skyVBO = sceneRenderer->createSkyVertexBuffer(skyConusVBO.size() * sizeof(mathfu::vec4_packed));
    skyVBO->uploadData(
        skyConusVBO.data(),
        skyConusVBO.size() * sizeof(mathfu::vec4_packed)
    );

    auto skyBindings = sceneRenderer->createSkyVAO(skyVBO, skyIBO);

    return skyBindings;
}

Map::Map(HApiContainer api, int mapId, const std::string &mapName) : m_dayNightLightHolder(api, mapId) {
    initMapTiles();

    m_mapId = mapId; m_api = api; this->mapName = mapName;
    m_sceneMode = SceneMode::smMap;
    createAdtFreeLamdas();

    MapRecord mapRecord;
    api->databaseHandler->getMapById(mapId, mapRecord);
    useWeightedBlend = (mapRecord.flags0 & 0x4) > 0;
    has0x200000Flag = (mapRecord.flags0 & 0x200000) > 0;


    std::string wdtFileName = "world/maps/"+mapName+"/"+mapName+".wdt";
    std::string wdlFileName = "world/maps/"+mapName+"/"+mapName+".wdl";
    std::string wdtLightFileName = "world/maps/"+mapName+"/"+mapName+"_lgt.wdt";

    m_wdtfile = api->cacheStorage->getWdtFileCache()->get(wdtFileName);
    m_wdtLightObject = std::make_shared<WdtLightsObject>(api, wdtLightFileName);

    m_wdlObject = std::make_shared<WdlObject>(api, wdlFileName);
    m_wdlObject->setMapApi(this);

    m_dayNightLightHolder.loadZoneLights();

    m_sceneWideBlockVSPSChunk = nullptr;
}

std::tuple<HGMesh, std::shared_ptr<ISkyMeshMaterial>> createSkyMesh(const HMapSceneBufferCreate &sceneRenderer,
                                                              const HGVertexBufferBindings &skyBindings, bool conusFor0x4Sky) {

    PipelineTemplate pipelineTemplate;
    pipelineTemplate.depthWrite = false;
    pipelineTemplate.depthCulling = true;
    pipelineTemplate.backFaceCulling = false;
    pipelineTemplate.blendMode = conusFor0x4Sky ? EGxBlendEnum::GxBlend_Alpha : EGxBlendEnum::GxBlend_Opaque;
    pipelineTemplate.element = DrawElementMode::TRIANGLE_STRIP;

    auto material = sceneRenderer->createSkyMeshMaterial(pipelineTemplate);

    gMeshTemplate meshTemplate(skyBindings);

    meshTemplate.meshType = MeshType::eGeneralMesh;

    if (conusFor0x4Sky) {
        meshTemplate.start = 198 * 2;
        meshTemplate.end = 102;
    } else {
        meshTemplate.start = 0;
        meshTemplate.end = 300;
    }

    //Make mesh
    HGMesh hmesh = sceneRenderer->createMesh(meshTemplate, material);
    return {hmesh, material};
}

void Map::makeFramePlan(const FrameInputParams<MapSceneParams> &frameInputParams, const HMapRenderPlan &mapRenderPlan) {
    ZoneScoped ;

    Config* config = this->m_api->getConfig();

    mathfu::vec4 cameraPos = frameInputParams.frameParameters->matricesForCulling->cameraPos;
    mathfu::vec3 cameraVec3 = cameraPos.xyz();
    mathfu::mat4 &frustumMat = frameInputParams.frameParameters->matricesForCulling->perspectiveMat;
    mathfu::mat4 &lookAtMat4 = frameInputParams.frameParameters->matricesForCulling->lookAtMat;

    mapRenderPlan->renderingMatrices = frameInputParams.frameParameters->renderTargets[0].cameraMatricesForRendering;
    mapRenderPlan->deltaTime = frameInputParams.delta;

    size_t adtRenderedThisFramePrev = mapRenderPlan->adtArray.size();
    mapRenderPlan->adtArray = {};
    mapRenderPlan->adtArray.reserve(adtRenderedThisFramePrev);

    mathfu::mat4 viewPerspectiveMat = frustumMat*lookAtMat4;
    mathfu::vec4 &camera4 = cameraPos;

//    auto oldPlanes = MathHelper::getFrustumClipsFromMatrix(viewPerspectiveMat);
//    auto newPlanes = MathHelper::getFrustumClipsFromMatrix(frustumMat);
//    for (int i = 0; i < newPlanes.size(); i++) {
//        newPlanes[i] = (lookAtMat4.Transpose().Inverse()) * newPlanes[i];
//    }

    MathHelper::PlanesUndPoints planesUndPoints;
    planesUndPoints.planes = MathHelper::getFrustumClipsFromMatrix(viewPerspectiveMat);
    planesUndPoints.points = MathHelper::calculateFrustumPointsFromMat(viewPerspectiveMat);
    auto testPoints = MathHelper::getIntersectionPointsFromPlanes(planesUndPoints.planes);
    planesUndPoints.hullLines = MathHelper::getHullLines(planesUndPoints.points);

    MathHelper::FrustumCullingData frustumData;
    frustumData.frustums = {planesUndPoints};
    frustumData.perspectiveMat = frustumMat;
    frustumData.viewMat = lookAtMat4;
    frustumData.cameraPos = cameraVec3;
    frustumData.farPlane = planesUndPoints.planes[planesUndPoints.planes.size() - 2]; //farPlane is always one before last

    m_viewRenderOrder = 0;

    mapRenderPlan->m_currentInteriorGroups = {};
    mapRenderPlan->m_currentWMO = emptyWMO;

    int bspNodeId = -1;
    int interiorGroupNum = -1;
    mapRenderPlan->m_currentWmoGroup = -1;

    //Get potential WMO
    WMOListContainer potentialWmo;
    M2ObjectListContainer potentialM2;

    {
        ZoneScopedN("cullGetCurrentWMOCounter");
        //Hack that is needed to get the current WMO the camera is in. Basically it does frustum culling over current ADT
        getPotentialEntities(frustumData, cameraPos, mapRenderPlan, potentialM2, potentialWmo);

        for (auto &wmoId: potentialWmo.getCandidates()) {
            WmoGroupResult groupResult;
            auto checkingWmoObj = wmoFactory->getObjectById<0>(wmoId);
            if (checkingWmoObj == nullptr) continue;

            bool result = checkingWmoObj->getGroupWmoThatCameraIsInside(camera4, groupResult);

            if (result) {
                mapRenderPlan->m_currentWMO = wmoId;
                mapRenderPlan->m_currentWmoGroup = groupResult.groupIndex;
                if (checkingWmoObj->isGroupWmoInterior(groupResult.groupIndex)) {
                    mapRenderPlan->m_currentInteriorGroups.push_back(groupResult);
                    interiorGroupNum = groupResult.groupIndex;
                    mapRenderPlan->currentWmoGroupIsExtLit = checkingWmoObj->isGroupWmoExteriorLit(
                        groupResult.groupIndex);
                    mapRenderPlan->currentWmoGroupShowExtSkybox = checkingWmoObj->isGroupWmoExtSkybox(
                        groupResult.groupIndex);
                } else {
                }
                bspNodeId = groupResult.nodeId;
                break;
            }
        }
    }


    //7. Get AreaId and Area Name
    AreaRecord wmoAreaRecord;
    bool wmoAreaFound = false;
    if (mapRenderPlan->m_currentWMO != emptyWMO) {
        auto l_currentWmoObject = wmoFactory->getObjectById<0>(mapRenderPlan->m_currentWMO);
        if (l_currentWmoObject != nullptr) {
            auto nameId = l_currentWmoObject->getNameSet();
            auto wmoId = l_currentWmoObject->getWmoId();
            auto groupId = l_currentWmoObject->getWmoGroupId(mapRenderPlan->m_currentWmoGroup);

            if (m_api->databaseHandler != nullptr) {
                wmoAreaFound = m_api->databaseHandler->getWmoArea(wmoId, nameId, groupId, wmoAreaRecord);
            }
        }
    }

    AreaRecord areaRecord;
    if ((m_api->databaseHandler != nullptr)) {
        if (wmoAreaRecord.areaId == 0) {
            if (mapRenderPlan->adtAreadId > 0) {
                areaRecord = m_api->databaseHandler->getArea(mapRenderPlan->adtAreadId);
            }
        } else {
            areaRecord = m_api->databaseHandler->getArea(wmoAreaRecord.areaId);
        }
    }


    mapRenderPlan->wmoAreaName = wmoAreaRecord.areaName;
    mapRenderPlan->areaName = areaRecord.areaName;

    auto &stateForConditions = mapRenderPlan->frameDependentData->stateForConditions;

    stateForConditions.currentAreaId = areaRecord.areaId;
    stateForConditions.currentParentAreaId = areaRecord.parentAreaId;

    mapRenderPlan->areaId = areaRecord.areaId;
    mapRenderPlan->parentAreaId = areaRecord.parentAreaId;


    //Get lights from DB
    updateLightAndSkyboxData(mapRenderPlan, frustumData, stateForConditions, areaRecord);

    ///-----------------------------------


    auto lcurrentWMO = wmoFactory->getObjectById<0>(mapRenderPlan->m_currentWMO);
    auto currentWmoGroup = mapRenderPlan->m_currentWmoGroup;

    if ((lcurrentWMO != nullptr) && (!mapRenderPlan->m_currentInteriorGroups.empty()) && (lcurrentWMO->isLoaded())) {
        if (lcurrentWMO->startTraversingWMOGroup(
            cameraPos,
            frustumData,
            mapRenderPlan->m_currentInteriorGroups[0].groupIndex,
            0,
            m_viewRenderOrder,
            true,
            mapRenderPlan->viewsHolder)) {
            mapRenderPlan->wmoArray.addToDrawn(lcurrentWMO);
        }

        auto exterior = mapRenderPlan->viewsHolder.getExterior();
        if ( exterior != nullptr ) {
            //Fix FrustumData for exterior was created after WMO traversal. So we need to fix it
            exterior->frustumData.perspectiveMat = frustumData.perspectiveMat;
            exterior->frustumData.viewMat = frustumData.viewMat;
            exterior->frustumData.farPlane = frustumData.farPlane;

            checkExterior(cameraPos, exterior->frustumData, m_viewRenderOrder, mapRenderPlan);
        }
    } else {
        //Cull exterior
        auto exteriorView = mapRenderPlan->viewsHolder.getOrCreateExterior(frustumData);
        checkExterior(cameraPos, exteriorView->frustumData, m_viewRenderOrder, mapRenderPlan);
    }

    if ((mapRenderPlan->viewsHolder.getExterior() != nullptr || mapRenderPlan->currentWmoGroupIsExtLit || mapRenderPlan->currentWmoGroupShowExtSkybox)) {
        ZoneScopedN("Skybox");
        auto exteriorView = mapRenderPlan->viewsHolder.getOrCreateExterior(frustumData);

        if (m_wdlObject != nullptr && config->renderSkyScene) {
            m_wdlObject->checkSkyScenes(
                stateForConditions,
                exteriorView->m2List,
                cameraPos,
                frustumData);
        }

        auto &exteriorSkyBoxes = m_dayNightLightHolder.getExteriorSkyBoxes();
        if (!exteriorSkyBoxes.empty() && config->renderSkyDom) {
            auto skyBoxView = mapRenderPlan->viewsHolder.getSkybox();

            for (auto &model: exteriorSkyBoxes) {
                if (model != nullptr) {
                    skyBoxView->m2List.addToDraw(model);
                }
            }
        }
    }

    std::vector<std::shared_ptr<CWmoNewLight>> newWmoLights = {};
    {

        auto exteriorView = mapRenderPlan->viewsHolder.getExterior();
        if (exteriorView != nullptr) {
            {
                ZoneScopedN("collect m2 from groups");
                exteriorView->addM2FromGroups(frustumData, cameraPos);
            }
            {
                ZoneScopedN("m2AndWMOArr merge");
                mapRenderPlan->m2Array.addDrawnAndToLoad(exteriorView->m2List);
                mapRenderPlan->wmoGroupArray.addToLoadAndDraw(exteriorView->wmoGroupArray);
            }

            exteriorView->collectLights(mapRenderPlan->pointLights, mapRenderPlan->spotLights, newWmoLights);
        }
    }

    //Fill and collect M2 objects for views from WmoGroups
    {
        ZoneScopedN("collect from interiors");
        auto &interiorViews = mapRenderPlan->viewsHolder.getInteriorViews();

        for (auto &interiorView: interiorViews) {
            interiorView->addM2FromGroups(frustumData, cameraPos);
            mapRenderPlan->m2Array.addDrawnAndToLoad(interiorView->m2List);
            mapRenderPlan->wmoGroupArray.addToLoadAndDraw(interiorView->wmoGroupArray);

            interiorView->collectLights(mapRenderPlan->pointLights, mapRenderPlan->spotLights, newWmoLights);
        }
    }
    {
        ZoneScopedN("process new lights");
        //Delete duplicates for new wmo lights
        std::sort(newWmoLights.begin(), newWmoLights.end());
        newWmoLights.erase(std::unique(newWmoLights.begin(), newWmoLights.end()), newWmoLights.end());

        //Collect spotLights and point lights from new WMO lights
        for (auto &newWmoLight: newWmoLights) {
            newWmoLight->collectLight(mapRenderPlan->pointLights, mapRenderPlan->spotLights);
        }
    }

    mapRenderPlan->renderSky = m_api->getConfig()->renderSkyDom &&
        (!m_suppressDrawingSky && (mapRenderPlan->viewsHolder.getExterior() || mapRenderPlan->currentWmoGroupIsExtLit));

//    if (m_skyConeAlpha > 0) {
        mapRenderPlan->skyMesh = skyMesh;
//    }
    if (mapRenderPlan->frameDependentData->overrideValuesWithFinalFog) {
        mapRenderPlan->skyMesh0x4 = skyMesh0x4Sky;
    }

//    //Limit M2 count based on distance/m2 height
//    for (auto it = this->m2RenderedThisFrameArr.begin();
//         it != this->m2RenderedThisFrameArr.end();) {
//        if ((*it)->getCurrentDistance() > ((*it)->getHeight() * 5)) {
//            it = this->m2RenderedThisFrameArr.erase(it);
//        } else {
//            ++it;
//        }
//    }
}

void Map::updateLightAndSkyboxData(const HMapRenderPlan &mapRenderPlan, MathHelper::FrustumCullingData &frustumData,
                                   StateForConditions &stateForConditions, const AreaRecord &areaRecord) {

    m_dayNightLightHolder.updateLightAndSkyboxData(mapRenderPlan, frustumData, stateForConditions, areaRecord);
}

void Map::getPotentialEntities(const MathHelper::FrustumCullingData &frustumData,
                               const mathfu::vec4 &cameraPos,
                               const HMapRenderPlan &mapRenderPlan,
                               M2ObjectListContainer &potentialM2,
                               WMOListContainer &potentialWmo) {

    if (m_wdtfile->getStatus() == FileStatus::FSLoaded) {
        if (!m_wdtfile->mphd->flags.wdt_uses_global_map_obj) {
            int adt_x = worldCoordinateToAdtIndex(cameraPos.y);
            int adt_y = worldCoordinateToAdtIndex(cameraPos.x);
            if ((adt_x >= 64) || (adt_x < 0)) return;
            if ((adt_y >= 64) || (adt_y < 0)) return;

            mapRenderPlan->adtAreadId = -1;
            auto &adtObjectCameraAt = mapTiles[adt_x][adt_y];

            if (adtObjectCameraAt != nullptr) {
                ADTObjRenderRes tempRes;
                tempRes.adtObject = adtObjectCameraAt;
                adtObjectCameraAt->checkReferences(
                    tempRes,
                    cameraPos,
                    frustumData,
                    5,
                    potentialM2,
                    potentialWmo,
                    0,
                    0,
                    16,
                    16
                );

                int adt_global_x = worldCoordinateToGlobalAdtChunk(cameraPos.y) % 16;
                int adt_global_y = worldCoordinateToGlobalAdtChunk(cameraPos.x) % 16;

                mapRenderPlan->adtAreadId = adtObjectCameraAt->getAreaId(adt_global_x, adt_global_y);
            }
        } else {
            if (wmoMap == nullptr) {
                wmoMap = wmoFactory->createObject(m_api);
                wmoMap->setLoadingParam(*m_wdtfile->wmoDef);
                wmoMap->setModelFileId(m_wdtfile->wmoDef->nameId);
            }

            potentialWmo.addCand(wmoMap);
        }
    }
}


void Map::getAdtAreaId(const mathfu::vec4 &cameraPos, int &areaId, int &parentAreaId) {
    areaId = 0;
    parentAreaId = 0;

    if (m_wdtfile->getStatus() == FileStatus::FSLoaded) {
        if (!m_wdtfile->mphd->flags.wdt_uses_global_map_obj) {
            int adt_x = worldCoordinateToAdtIndex(cameraPos.y);
            int adt_y = worldCoordinateToAdtIndex(cameraPos.x);
            if ((adt_x >= 64) || (adt_x < 0)) return;
            if ((adt_y >= 64) || (adt_y < 0)) return;

            auto &adtObjectCameraAt = mapTiles[adt_x][adt_y];

            int adt_global_x = worldCoordinateToGlobalAdtChunk(cameraPos.y) % 16;
            int adt_global_y = worldCoordinateToGlobalAdtChunk(cameraPos.x) % 16;

            areaId = adtObjectCameraAt->getAreaId(adt_global_x, adt_global_y);

            if (areaId > 0 && (m_api->databaseHandler != nullptr)) {
                auto areaRecord = m_api->databaseHandler->getArea(areaId);
                parentAreaId = areaRecord.parentAreaId;
            }
        }
    }
}

void Map::checkExterior(mathfu::vec4 &cameraPos,
                        const MathHelper::FrustumCullingData &frustumData,
                        int viewRenderOrder,
                        const HMapRenderPlan &mapRenderPlan
) {
    ZoneScoped ;
//    std::cout << "Map::checkExterior finished called" << std::endl;
    if (m_wdlObject == nullptr && m_wdtfile != nullptr && m_wdtfile->getStatus() == FileStatus::FSLoaded) {
        if (m_wdtfile->mphd->flags.wdt_has_maid) {
            m_wdlObject = std::make_shared<WdlObject>(m_api, m_wdtfile->mphd->wdlFileDataID);
            m_wdlObject->setMapApi(this);
        }
    }

    if (m_wdtLightObject == nullptr && m_wdtfile != nullptr && m_wdtfile->getStatus() == FileStatus::FSLoaded) {
        if (m_wdtfile->mphd->flags.wdt_has_maid) {
            m_wdtLightObject = std::make_shared<WdtLightsObject>(m_api, m_wdtfile->mphd->lgtFileDataID);
        }
    }

    auto exteriorView = mapRenderPlan->viewsHolder.getExterior(); //Should not be null, since we called checkExterior

    if (m_wdlObject != nullptr) {
        m_wdlObject->checkFrustumCulling(frustumData, cameraPos, exteriorView->m2List, mapRenderPlan->wmoArray);
    }

    getCandidatesEntities(frustumData, cameraPos, mapRenderPlan, exteriorView->m2List, mapRenderPlan->wmoArray);

    //Frustum cull
    for (auto &wmoId : mapRenderPlan->wmoArray.getCandidates()) {
        auto wmoCandidate = wmoFactory->getObjectById<0>(wmoId);
        if (wmoCandidate!= nullptr && !wmoCandidate->isLoaded()) continue;

        if (wmoCandidate->startTraversingWMOGroup(
            cameraPos,
            frustumData,
            -1,
            0,
            viewRenderOrder,
            false,
            mapRenderPlan->viewsHolder)) {

            mapRenderPlan->wmoArray.addToDrawn(wmoCandidate);
        }
    }

    //3.2 Iterate over all global WMOs and M2s (they have uniqueIds)
    {
        ZoneScopedN("Cull M2");
        auto results = std::vector<uint32_t>();

        auto &candidates = exteriorView->m2List.getCandidates();
        if (candidates.size() > 0) {
            results = std::vector<uint32_t>(candidates.size(), 0xFFFFFFFF);

            oneapi::tbb::task_arena arena(m_api->getConfig()->hardwareThreadCount(), 1);
            arena.execute([&] {

                oneapi::tbb::parallel_for(tbb::blocked_range<size_t>(0, candidates.size(), 2000),
                                          [&](tbb::blocked_range<size_t> &r) {
    //            for (size_t i = r.begin(); i != r.end(); ++i) {
    //                auto &m2ObjectCandidate = tmpVector[i];
    //                if(m2ObjectCandidate->checkFrustumCulling(cameraPos, frustumData)) {
    //                    exteriorView->drawnM2s.insert(m2ObjectCandidate);
    //                    cullStage->m2Array.insert(m2ObjectCandidate);
    //                }
#if (__AVX__ && __SSE2__)
                      ObjectCullingSEE<M2ObjId>::cull(frustumData,
                                                                        r.begin(), r.end(), candidates,
                                                                        results);
#else
                      ObjectCulling<M2ObjId>::cull(frustumData,
                                                                     r.begin(), r.end(), candidates,
                                                                     results);
#endif
    //            }
                }, tbb::auto_partitioner());
            });
        }

        for (int i = 0; i < candidates.size(); i++) {
            if (!results[i]) {
//                auto m2ObjectCandidate = m2Factory->getObjectById<0>(candidates[i]);
                exteriorView->m2List.addToDraw(candidates[i]);
            }
        }
    }
}

void Map::getCandidatesEntities(const MathHelper::FrustumCullingData &frustumData,
                                const mathfu::vec4 &cameraPos,
                                const HMapRenderPlan &mapRenderPlan,
                                M2ObjectListContainer &m2ObjectsCandidates,
                                WMOListContainer &wmoCandidates) {
    ZoneScoped;
    if (m_wdtfile != nullptr && m_wdtfile->getStatus() == FileStatus::FSLoaded) {
        //Get visible area that should be checked
        float minx = 99999, maxx = -99999;
        float miny = 99999, maxy = -99999;

        //    float adt_x = floor((32 - (cameraPos[1] / 533.33333)));
//    float adt_y = floor((32 - (cameraPos[0] / 533.33333)));

//    //FOR DEBUG
//    adt_x_min = worldCoordinateToAdtIndex(cameraPos.y) - 2;
//    adt_x_max = adt_x_min + 4;
//    adt_y_min = worldCoordinateToAdtIndex(cameraPos.x) - 2;
//    adt_y_max = adt_y_min + 4;

//    for (int i = 0; i < 64; i++)
//        for (int j = 0; j < 64; j++) {
//            if (this->m_wdtfile->mapTileTable->mainInfo[i][j].Flag_AllWater) {
//                std::cout << AdtIndexToWorldCoordinate(j) <<" "<<  AdtIndexToWorldCoordinate(i) << std::endl;
//            }
//        }

//    std::cout << AdtIndexToWorldCoordinate(adt_y_min) <<" "<<  AdtIndexToWorldCoordinate(adt_x_min) << std::endl;

        for (auto &frustums : frustumData.frustums) {
            for (int i = 0; i < frustums.points.size(); i++) {
                auto &frustumPoint = frustums.points[i];

                minx = std::min<float>(frustumPoint.x, minx);
                maxx = std::max<float>(frustumPoint.x, maxx);
                miny = std::min<float>(frustumPoint.y, miny);
                maxy = std::max<float>(frustumPoint.y, maxy);
            }
        }

        int adt_x_min = std::max<int>(std::floor(worldCoordinateToAdtIndexF(maxy)), 0);
        int adt_x_max = std::min<int>(std::ceil(worldCoordinateToAdtIndexF(miny)), 63);

        int adt_y_min = std::max<int>(std::floor(worldCoordinateToAdtIndexF(maxx)), 0);
        int adt_y_max = std::min<int>(std::ceil(worldCoordinateToAdtIndexF(minx)), 63);

        if (!m_wdtfile->mphd->flags.wdt_uses_global_map_obj) {
            for (int i = adt_x_min; i <= adt_x_max; i++) {
                for (int j = adt_y_min; j <= adt_y_max; j++) {
                    checkADTCulling(i, j, frustumData, cameraPos, mapRenderPlan, m2ObjectsCandidates, wmoCandidates);
                }
            }
            for (auto &mandatoryAdt : m_mandatoryADT) {
                checkADTCulling(mandatoryAdt[0], mandatoryAdt[1], frustumData, cameraPos, mapRenderPlan, m2ObjectsCandidates, wmoCandidates);
            }

        } else if (wmoMap) {
            wmoCandidates.addCand(wmoMap);
        }
    }
}

void Map::checkADTCulling(int i, int j,
                          const MathHelper::FrustumCullingData &frustumData,
                          const mathfu::vec4 &cameraPos,
                          const HMapRenderPlan &mapRenderPlan,
                          M2ObjectListContainer &m2ObjectsCandidates,
                          WMOListContainer &wmoCandidates) {
    if ((i < 0) || (i > 64)) return;
    if ((j < 0) || (j > 64)) return;

    if (this->m_adtConfigHolder != nullptr) {
        mathfu::vec3 min = m_adtConfigHolder->adtMin[i][j];
        mathfu::vec3 max = m_adtConfigHolder->adtMax[i][j];

        CAaBox box = {
            C3Vector(min),
            C3Vector(max)

//            C3Vector({AdtIndexToWorldCoordinate(j + 1) , AdtIndexToWorldCoordinate(i + 1), minZ}),
//            C3Vector({AdtIndexToWorldCoordinate(j) , AdtIndexToWorldCoordinate(i), maxZ})
        };

        bool bbCheck = MathHelper::checkFrustum( frustumData, box);

        if (!bbCheck)
            return;
    }

    auto &adtObject = mapTiles[i][j];
    auto &adtArray = mapRenderPlan->adtArray;
    if (adtObject != nullptr) {

        auto &adtFrustRes = adtArray.emplace_back();
        adtFrustRes.adtObject = adtObject;

        bool result = adtObject->checkFrustumCulling(
            adtFrustRes,
            cameraPos,
            frustumData, m2ObjectsCandidates, wmoCandidates);

//        if (this->m_adtBBHolder != nullptr) {
//            //When adt passes BBHolder test, consider it influences the picture even if checkFrustumCulling
//            //is false. So do forceful update for freeStrategy
//            adtObject->getFreeStrategy()(false, true, this->getCurrentSceneTime());
//        }

        if (!result) {
            adtArray.pop_back();
        } else {
            //Add lights from WDTLightObject
            if (m_wdtLightObject) {
                auto pointLightsOfAdt = m_wdtLightObject->getPointLights(i, j);
                auto &pointLights = mapRenderPlan->pointLights;
                pointLights.reserve(pointLights.size() + pointLightsOfAdt.size());
                for (auto &pointLight : pointLightsOfAdt) pointLights.push_back(pointLight.getLightRec());

                //Get spotLights
                m_wdtLightObject->collectSpotLights(i, j, mapRenderPlan->spotLights);
            }
        }
    } else if (!m_lockedMap && true) { //(m_wdtfile->mapTileTable->mainInfo[j][i].Flag_HasADT > 0) {
        if (m_wdtfile->mphd->flags.wdt_has_maid) {
            auto &mapFileIds = m_wdtfile->mapFileDataIDs[j * 64 + i];
            if (mapFileIds.rootADT > 0) {
                adtObject = adtObjectFactory->createObject(m_api, i, j,
                                                        m_wdtfile->mapFileDataIDs[j * 64 + i],
                                                        useWeightedBlend,
                                                        m_wdtfile);
            } else {
                return;
            }
        } else {
            std::string adtFileTemplate =
                "world/maps/" + mapName + "/" + mapName + "_" + std::to_string(i) + "_" +
                std::to_string(j);
            adtObject = adtObjectFactory->createObject(m_api, adtFileTemplate, mapName, i, j,
                                                      useWeightedBlend,
                                                      m_wdtfile);
        }

        adtObject->setMapApi(this);
        adtObject->setFreeStrategy(zeroStateLambda);

        mapTiles[i][j] = adtObject;
    }
}

void Map::createAdtFreeLamdas() {
    FreeStrategy lamda;
    auto *config = m_api->getConfig();
    if (m_api->getConfig()->adtFreeStrategy == EFreeStrategy::eTimeBased) {
        animTime_t l_currentTime = 0;
        lamda = [l_currentTime, config](bool doCheck, bool doUpdate, animTime_t currentTime) mutable -> bool {
            if (doCheck) {
                return (currentTime - l_currentTime) > config->adtTTLWithoutUpdate;
            }
            if (doUpdate) {
                l_currentTime = currentTime;
            }
            return false;
        };
    } else if (m_api->getConfig()->adtFreeStrategy == EFreeStrategy::eFrameBase) {
        int l_currentFrame = 0;
        auto l_hDevice = m_api->hDevice;
        lamda = [l_currentFrame, config, l_hDevice](bool doCheck, bool doUpdate, animTime_t currentTime) mutable -> bool {
            if (doCheck) {
//                return (l_hDevice->getFrameNumber() - l_currentFrame) > config->adtFTLWithoutUpdate;
            }
            if (doUpdate) {
//                l_currentFrame = l_hDevice->getFrameNumber();
            }
            return false;
        };
    }
    adtFreeLambda = lamda;
    zeroStateLambda = lamda;

}

void Map::doPostLoad(const HMapSceneBufferCreate &sceneRenderer, const HMapRenderPlan &renderPlan) {
    ZoneScoped;
    int processedThisFrame = 0;
    int m2ProcessedThisFrame = 0;
    int wmoProcessedThisFrame = 0;
    int wmoGroupsProcessedThisFrame = 0;

    {
        ZoneScopedN("Load m2 main");
        if (m_api->getConfig()->renderM2) {
            for (auto &m2ObjectId: renderPlan->m2Array.getToLoadMain()) {
                auto m2Object = m2Factory->getObjectById<0>(m2ObjectId);
                if (m2Object == nullptr) continue;
                m2Object->doLoadMainFile();
            }
        }
    }
    {
        ZoneScopedN("Load m2 geom");
        if (m_api->getConfig()->renderM2) {
            for (auto &m2ObjectId: renderPlan->m2Array.getToLoadGeom()) {
                auto m2Object = m2Factory->getObjectById<0>(m2ObjectId);
                if (m2Object == nullptr) continue;
                m2Object->doLoadGeom(sceneRenderer);
                m2ProcessedThisFrame++;
                if (m2ProcessedThisFrame > MAX_LOAD_M2_PER_FRAME) break;
            }
        }
    }

    if (m_api->getConfig()->renderSkyDom) {
        if (auto skyboxView = renderPlan->viewsHolder.getSkybox()) {
            for (auto &m2ObjectId: skyboxView->m2List.getToLoadMain()) {
                auto m2Object = m2Factory->getObjectById<0>(m2ObjectId);
                if (m2Object == nullptr) continue;
                m2Object->doLoadMainFile();
            }
            for (auto &m2ObjectId: skyboxView->m2List.getToLoadGeom()) {
                auto m2Object = m2Factory->getObjectById<0>(m2ObjectId);
                if (m2Object == nullptr) continue;
                m2Object->doLoadGeom(sceneRenderer);
            }
        }
    }
//    }

    {
        ZoneScopedN("Load wmoObject");
        if (m_api->getConfig()->renderWMO) {
            for (auto wmoId: renderPlan->wmoArray.getToLoad()) {
                auto wmoObject = wmoFactory->getObjectById<0>(wmoId);
                if (wmoObject != nullptr) {
                    wmoObject->doPostLoad(sceneRenderer);
                }
            }
        }
    }
    {
        ZoneScopedN("Load wmo group");
        if (m_api->getConfig()->renderWMO) {
            for (auto &wmoGroupObject: renderPlan->wmoGroupArray.getToLoad()) {
                if (wmoGroupObject == nullptr) continue;
                wmoGroupObject->doPostLoad(sceneRenderer);
                wmoGroupsProcessedThisFrame++;
                if (wmoGroupsProcessedThisFrame > MAX_LOAD_WMOGROUP_PER_FRAME) break;
            }
        }
    }

    {
        ZoneScopedN("Load adt");
        int adtProcessed = 0;
        for (auto &adtObject: renderPlan->adtArray) {
            adtProcessed += (adtObject.adtObject->doPostLoad(sceneRenderer)) ? 1 : 0;
            if (adtProcessed >= MAX_LOAD_ADT_PER_FRAME) break;
        }
    }

    if (skyMesh == nullptr) {
        auto skyMeshBinding = createSkyBindings(sceneRenderer);
        std::tie(skyMesh, skyMeshMat) = createSkyMesh(sceneRenderer, skyMeshBinding, false);
        std::tie(skyMesh0x4Sky, skyMeshMat0x4) = createSkyMesh(sceneRenderer, skyMeshBinding, true);
    }
    bool renderPortals = m_api->getConfig()->renderPortals;
    bool renderAntiPortals = m_api->getConfig()->renderAntiPortals;

    if (renderPortals) {
        for (auto &view : renderPlan->viewsHolder.getInteriorViews()) {
                view->produceTransformedPortalMeshes(sceneRenderer, m_api, view->worldPortalVertices);
        }
    }


    if (renderAntiPortals) {
        if(auto const &exterior = renderPlan->viewsHolder.getExterior()) {
            exterior->produceTransformedPortalMeshes(sceneRenderer, m_api,
                                                     exterior->worldAntiPortalVertices, true);
        }
    }

};

void Map::update(const HMapRenderPlan &renderPlan) {
    ZoneScoped;

    mathfu::vec3 cameraVec3   = renderPlan->renderingMatrices->cameraPos.xyz();
    mathfu::mat4 &frustumMat  = renderPlan->renderingMatrices->perspectiveMat;
    mathfu::mat4 &lookAtMat   = renderPlan->renderingMatrices->lookAtMat;
    animTime_t deltaTime      = renderPlan->deltaTime;

    Config* config = this->m_api->getConfig();

    auto &m2ToDraw = renderPlan->m2Array.getDrawn();
    {
        ZoneScopedN("m2UpdateframeCounter");

        auto threadsAvailable = m_api->getConfig()->hardwareThreadCount();

        int granSize = m2ToDraw.size() / (2 * threadsAvailable);
        if (granSize == 0) granSize = m2ToDraw.size();

        std::mutex fillLights;
        if (granSize > 0) {
            oneapi::tbb::task_arena arena(m_api->getConfig()->hardwareThreadCount(), 1);
            arena.execute([&] {
                tbb::affinity_partitioner ap;
                tbb::parallel_for(tbb::blocked_range<size_t>(0, m2ToDraw.size(), granSize),
                    [&](tbb::blocked_range<size_t> r) {
                        std::vector<LocalLight> localLights;

                        for (size_t i = r.begin(); i != r.end(); ++i) {
                            auto m2Object = m2Factory->getObjectById<0>(m2ToDraw[i]);
                            if (m2Object == nullptr) continue;
                            m2Object->update(deltaTime, cameraVec3, lookAtMat);\

                            m2Object->collectLights(localLights);
                        }
                        if (!localLights.empty()) {
                            std::unique_lock<std::mutex> lock(fillLights);
                            renderPlan->pointLights.insert(
                                renderPlan->pointLights.end(),
                                localLights.begin(),localLights.end());
                        }
                    }, ap);
            });
        }

        if (auto skyBoxView = renderPlan->viewsHolder.getSkybox()) {
            for (auto &m2ObjectId : skyBoxView->m2List.getDrawn()) {
                auto m2Object = m2Factory->getObjectById<0>(m2ObjectId);
                if (m2Object == nullptr) continue;
                m2Object->update(deltaTime, cameraVec3, lookAtMat);
            }
        }
    }

    {
        ZoneScopedN("wmoUpdate");

        for (const auto &wmoId: renderPlan->wmoArray.getToDrawn()) {
            auto wmoObject = wmoFactory->getObjectById<0>(wmoId);
            if (wmoObject == nullptr) continue;
            wmoObject->update();
        }
    }

    {
        ZoneScopedN("wmoGroupUpdate");
        for (const auto &wmoGroupObject: renderPlan->wmoGroupArray.getToDraw()) {
            if (wmoGroupObject == nullptr) continue;
            wmoGroupObject->update();
        }
    }

    {
        ZoneScopedN("adtUpdate");
        {
            std::unordered_set<std::shared_ptr<AdtObject>> processedADT;
            for (const auto &adtObjectRes: renderPlan->adtArray) {
                if (processedADT.find(adtObjectRes.adtObject) != processedADT.end()) {
                    adtObjectRes.adtObject->update(deltaTime);
                    processedADT.insert(adtObjectRes.adtObject);
                }
            }
        }
    }

    //2. Calc distance every 100 ms
    {
        ZoneScopedN("Calc m2 distance");
        oneapi::tbb::task_arena arena(m_api->getConfig()->hardwareThreadCount(), 1);
        arena.execute([&] {
            tbb::parallel_for(tbb::blocked_range<size_t>(0, m2ToDraw.size(), 500),
                              [&](tbb::blocked_range<size_t> r) {
                                  for (size_t i = r.begin(); i != r.end(); ++i) {
                                      auto m2Object = m2Factory->getObjectById<0>(m2ToDraw[i]);
                                      if (m2Object == nullptr) continue;
                                      m2Object->calcDistance(cameraVec3);
                                  }
                              }, tbb::auto_partitioner()
            );
        });
    }

    //Cleanup ADT every 10 seconds
    {
        ZoneScopedN("Cleanup ADT");
        if (adtFreeLambda != nullptr && adtFreeLambda(true, false, this->m_currentTime)) {
            for (int i = 0; i < 64; i++) {
                for (int j = 0; j < 64; j++) {
                    auto adtObj = mapTiles[i][j];
                    //Free obj, if it was unused for 10 secs
                    if (adtObj != nullptr && adtObj->getFreeStrategy()(true, false, this->m_currentTime)) {
//                    std::cout << "try to free adtObj" << std::endl;

                        mapTiles[i][j] = nullptr;
                    }
                }
            }

            adtFreeLambda(false, true, this->m_currentTime + deltaTime);
        }
    }

    this->m_currentTime += deltaTime;
}

void Map::updateBuffers(const HMapSceneBufferCreate &sceneRenderer, const HMapRenderPlan &renderPlan) {
    ZoneScoped;
    if (skyMeshMat0x4)
    {
        auto &meshblockVS = skyMeshMat0x4->m_skyColors->getObject();
        auto EndFogColor = !renderPlan->frameDependentData->fogResults.empty() ?
                           renderPlan->frameDependentData->fogResults[0].EndFogColor:
                           mathfu::vec3(0,0,0);

        if (EndFogColor.Length() < 0.0001) {
            EndFogColor = renderPlan->frameDependentData->skyColors.SkyFogColor.xyz();
        }


        auto EndFogColorV4_1 = mathfu::vec4(EndFogColor, 0.0);
        auto EndFogColorV4_2 = mathfu::vec4(EndFogColor, 1.0);
        meshblockVS.skyColor[0] = EndFogColorV4_1;
        meshblockVS.skyColor[1] = EndFogColorV4_1;
        meshblockVS.skyColor[2] = EndFogColorV4_1;
        meshblockVS.skyColor[3] = EndFogColorV4_1;
        meshblockVS.skyColor[4] = EndFogColorV4_1;
        meshblockVS.skyColor[5] = EndFogColorV4_2;
        skyMeshMat0x4->m_skyColors->save();
    }
    if (skyMeshMat)
    {
        auto &meshblockVS = skyMeshMat->m_skyColors->getObject();
        meshblockVS.skyColor[0] = renderPlan->frameDependentData->skyColors.SkyTopColor;
        meshblockVS.skyColor[1] = renderPlan->frameDependentData->skyColors.SkyMiddleColor;
        meshblockVS.skyColor[2] = renderPlan->frameDependentData->skyColors.SkyBand1Color;
        meshblockVS.skyColor[3] = renderPlan->frameDependentData->skyColors.SkyBand2Color;
        meshblockVS.skyColor[4] = renderPlan->frameDependentData->skyColors.SkySmogColor;
        meshblockVS.skyColor[5] = renderPlan->frameDependentData->skyColors.SkyFogColor;
        skyMeshMat->m_skyColors->save();
    }

    {
        ZoneScopedN("m2BuffersUpdate");
        auto threadsAvailable = m_api->getConfig()->hardwareThreadCount();
        auto &m2ToDraw = renderPlan->m2Array.getDrawn();
        int granSize = m2ToDraw.size() / (2 * threadsAvailable);
        if (granSize == 0) granSize = m2ToDraw.size();

        //Can't be paralleled?
        {
            ZoneScopedN("fitParticleAndRibbonBuffersToSize");
            auto &drawnM2s = renderPlan->m2Array.getDrawn();
            for (auto &m2ObjectId: drawnM2s) {
                auto m2Object = m2Factory->getObjectById<0>(m2ObjectId);
                if (m2Object != nullptr) {
                    m2Object->fitParticleAndRibbonBuffersToSize(sceneRenderer);
                }
            }
        }

        auto updateLambda = [&](const std::function<void(M2Object *)> &callback) {
            if (granSize > 0) {
                auto l_device = m_api->hDevice;
                auto oldProcessingFrame = m_api->hDevice->getCurrentProcessingFrameNumber();
                auto processingFrame = oldProcessingFrame;
                    tbb::affinity_partitioner ap;
                    tbb::parallel_for(tbb::blocked_range<size_t>(0, m2ToDraw.size(), granSize),
                        [&](tbb::blocked_range<size_t> r) {
                            l_device->setCurrentProcessingFrameNumber(processingFrame);
                            for (size_t i = r.begin(); i != r.end(); ++i) {
                                auto m2Object = m2Factory->getObjectById<0>(m2ToDraw[i]);
                                if (m2Object != nullptr) {
                                    callback(m2Object);
                                }
                            }
                        }, ap);
                //Restore processing frame
                l_device->setCurrentProcessingFrameNumber(oldProcessingFrame);
            }
        };

        {
            ZoneScopedN("upload Model buffers");

            updateLambda([&renderPlan](M2Object *m2Object) {
                m2Object->uploadBuffers(renderPlan->renderingMatrices->lookAtMat,
                                                 renderPlan->frameDependentData);
            });
        }

        {
            ZoneScopedN("upload generators buffers");

            updateLambda([&renderPlan](M2Object *m2Object) {
                m2Object->uploadGeneratorBuffers(renderPlan->renderingMatrices->lookAtMat,
                                                 renderPlan->frameDependentData);
            });
        }



//        for (auto &m2ObjectId: renderPlan->m2Array.getDrawn()) {
//            auto m2Object = m2Factory->getObjectById<0>(m2ObjectId);
//            if (m2Object != nullptr) {
//                m2Object->uploadGeneratorBuffers(renderPlan->renderingMatrices->lookAtMat,
//                                                 renderPlan->frameDependentData);
//            }
//        }
    }
    {
        ZoneScopedN("m2SkyboxBuffersUpdate");
        if (auto skyBoxView = renderPlan->viewsHolder.getSkybox()) {
            for (auto &m2ObjectId: skyBoxView->m2List.getDrawn()) {
                auto m2Object = m2Factory->getObjectById<0>(m2ObjectId);
                m2Object->uploadBuffers(renderPlan->renderingMatrices->lookAtMat,
                                                 renderPlan->frameDependentData);
            }
        }
    }
    {
        ZoneScopedN("m2SkyboxGenerateBuffersUpdate");
        if (auto skyBoxView = renderPlan->viewsHolder.getSkybox()) {
            for (auto &m2ObjectId: skyBoxView->m2List.getDrawn()) {
                auto m2Object = m2Factory->getObjectById<0>(m2ObjectId);
                m2Object->uploadGeneratorBuffers(renderPlan->renderingMatrices->lookAtMat,
                                                 renderPlan->frameDependentData);
            }
        }
    }

    {
        ZoneScopedN("wmoBuffersUpdate");
        for (auto &wmoGroupObject: renderPlan->wmoGroupArray.getToDraw()) {
            if (wmoGroupObject == nullptr) continue;
            wmoGroupObject->uploadGeneratorBuffers(renderPlan->frameDependentData, getCurrentSceneTime());
        }
    }

    {
        ZoneScopedN("adtBuffersUpdate");
        std::unordered_set<std::shared_ptr<AdtObject>> processedADT;
        for (const auto &adtObjectRes: renderPlan->adtArray) {
            if (processedADT.find(adtObjectRes.adtObject) != processedADT.end()) {
                adtObjectRes.adtObject->uploadGeneratorBuffers(renderPlan->frameDependentData);
                processedADT.insert(adtObjectRes.adtObject);
            }
        }
    }
}

std::shared_ptr<M2Object> Map::getM2Object(std::string fileName, const SMDoodadDef &doodadDef) {
    auto it = m_m2MapObjects[doodadDef.uniqueId];
    if (!it.expired()) {
        return it.lock();
    } else {
        auto m2Object = m2Factory->createObject(m_api);
        m2Object->setLoadParams(0, {}, {});
        m2Object->setModelFileName(fileName);
        m2Object->createPlacementMatrix(doodadDef);
        m2Object->calcWorldPosition();

        m_m2MapObjects[doodadDef.uniqueId] = std::weak_ptr<M2Object>(m2Object);
        return m2Object;
    }
    return nullptr;
}


std::shared_ptr<M2Object> Map::getM2Object(int fileDataId, const SMDoodadDef &doodadDef) {
    auto it = m_m2MapObjects[doodadDef.uniqueId];
    if (!it.expired()) {
        return it.lock();
    } else {
        auto m2Object = m2Factory->createObject(m_api);
        m2Object->setLoadParams(0, {}, {});
        m2Object->setModelFileId(fileDataId);
        m2Object->createPlacementMatrix(doodadDef);
        m2Object->calcWorldPosition();

        m_m2MapObjects[doodadDef.uniqueId] = std::weak_ptr<M2Object>(m2Object);
        return m2Object;
    }
    return nullptr;
}
std::shared_ptr<WmoObject> Map::getWmoObject(std::string fileName, const SMMapObjDef &mapObjDef) {
    auto it = m_wmoMapObjects[mapObjDef.uniqueId];
    if (!it.expired()) {
        return it.lock();
    } else {
        auto wmoObject = wmoFactory->createObject(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileName(fileName);

        m_wmoMapObjects[mapObjDef.uniqueId] = std::weak_ptr<WmoObject>(wmoObject);
        return wmoObject;
    }
    return nullptr;
}

std::shared_ptr<WmoObject> Map::getWmoObject(int fileDataId, const SMMapObjDef &mapObjDef) {
    auto it = m_wmoMapObjects[mapObjDef.uniqueId];
    if (!it.expired()) {
        return it.lock();
    } else {
        auto wmoObject = wmoFactory->createObject(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileId(fileDataId);

        m_wmoMapObjects[mapObjDef.uniqueId] = std::weak_ptr<WmoObject>(wmoObject);
        return wmoObject;
    }
    return nullptr;
}

std::shared_ptr<WmoObject> Map::getWmoObject(std::string fileName, const SMMapObjDefObj1 &mapObjDef) {
    auto it = m_wmoMapObjects[mapObjDef.uniqueId];
    if (!it.expired()) {
        return it.lock();
    } else {
        auto wmoObject = wmoFactory->createObject(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileName(fileName);

        m_wmoMapObjects[mapObjDef.uniqueId] = std::weak_ptr<WmoObject>(wmoObject);
        return wmoObject;
    }
    return nullptr;
}

std::shared_ptr<WmoObject> Map::getWmoObject(int fileDataId, const SMMapObjDefObj1 &mapObjDef) {
    auto it = m_wmoMapObjects[mapObjDef.uniqueId];
    if (!it.expired()) {
        return it.lock();
    } else {
        auto wmoObject = wmoFactory->createObject(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileId(fileDataId);

        m_wmoMapObjects[mapObjDef.uniqueId] = std::weak_ptr<WmoObject>(wmoObject);
        return wmoObject;
    }
    return nullptr;
}
animTime_t Map::getCurrentSceneTime() {
    return m_currentTime;
}
