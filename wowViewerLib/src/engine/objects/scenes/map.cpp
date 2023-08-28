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
#include "../../shader/ShaderDefinitions.h"
#include "tbb/tbb.h"
#include "../../algorithms/FrameCounter.h"

#if (__AVX__ && __SSE2__)
#include "../../algorithms/mathHelper_culling_sse.h"
#endif
#include "../../algorithms/mathHelper_culling.h"
#include "../../../gapi/interface/materials/IMaterial.h"
#include "../../../renderer/frame/FrameProfile.h"


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

Map::Map(HApiContainer api, int mapId, const std::string &mapName) {
    initMapTiles();

    m_mapId = mapId; m_api = api; this->mapName = mapName;
    m_sceneMode = SceneMode::smMap;
    createAdtFreeLamdas();

    std::string wdtFileName = "world/maps/"+mapName+"/"+mapName+".wdt";
    std::string wdlFileName = "world/maps/"+mapName+"/"+mapName+".wdl";

    m_wdtfile = api->cacheStorage->getWdtFileCache()->get(wdtFileName);
    m_wdlObject = std::make_shared<WdlObject>(api, wdlFileName);
    m_wdlObject->setMapApi(this);

    loadZoneLights();

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
    //TODO: Pass m_skyConeAlpha to fragment shader

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

void Map::makeFramePlan(const FrameInputParams<MapSceneParams> &frameInputParams, HMapRenderPlan &mapRenderPlan) {
    ZoneScoped ;

    cullCreateVarsCounter.beginMeasurement();
    Config* config = this->m_api->getConfig();

    mathfu::vec4 cameraPos = frameInputParams.frameParameters->matricesForCulling->cameraPos;
    mathfu::vec3 cameraVec3 = cameraPos.xyz();
    mathfu::mat4 &frustumMat = frameInputParams.frameParameters->matricesForCulling->perspectiveMat;
    mathfu::mat4 &lookAtMat4 = frameInputParams.frameParameters->matricesForCulling->lookAtMat;

    mapRenderPlan->renderingMatrices = frameInputParams.frameParameters->cameraMatricesForRendering;
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
    mapRenderPlan->m_currentWMO = nullptr;

    int bspNodeId = -1;
    int interiorGroupNum = -1;
    mapRenderPlan->m_currentWmoGroup = -1;

    //Get potential WMO
    WMOListContainer potentialWmo;
    M2ObjectListContainer potentialM2;

    cullCreateVarsCounter.endMeasurement();

    {
        ZoneScopedN("cullGetCurrentWMOCounter");
        cullGetCurrentWMOCounter.beginMeasurement();
        //Hack that is needed to get the current WMO the camera is in. Basically it does frustum culling over current ADT
        getPotentialEntities(frustumData, cameraPos, mapRenderPlan, potentialM2, potentialWmo);

        for (auto &checkingWmoObj: potentialWmo.getCandidates()) {
            WmoGroupResult groupResult;
            bool result = checkingWmoObj->getGroupWmoThatCameraIsInside(camera4, groupResult);

            if (result) {
                mapRenderPlan->m_currentWMO = checkingWmoObj;
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
        cullGetCurrentWMOCounter.endMeasurement();
    }

    cullGetCurrentZoneCounter.beginMeasurement();

    //7. Get AreaId and Area Name
    StateForConditions stateForConditions;

    AreaRecord areaRecord;
    if (mapRenderPlan->m_currentWMO != nullptr) {
        auto nameId = mapRenderPlan->m_currentWMO->getNameSet();
        auto wmoId = mapRenderPlan->m_currentWMO->getWmoId();
        auto groupId =  mapRenderPlan->m_currentWMO->getWmoGroupId(mapRenderPlan->m_currentWmoGroup);

        if (m_api->databaseHandler != nullptr) {
            areaRecord = m_api->databaseHandler->getWmoArea(wmoId, nameId, groupId);
        }
    }

    if (areaRecord.areaId == 0) {
        if (mapRenderPlan->adtAreadId > 0 && (m_api->databaseHandler != nullptr)) {
            areaRecord = m_api->databaseHandler->getArea(mapRenderPlan->adtAreadId);
        }
    }


    m_api->getConfig()->areaName = areaRecord.areaName;
    stateForConditions.currentAreaId = areaRecord.areaId;
    stateForConditions.currentParentAreaId = areaRecord.parentAreaId;

    mapRenderPlan->areaId = areaRecord.areaId;
    mapRenderPlan->parentAreaId = areaRecord.parentAreaId;

    cullGetCurrentZoneCounter.endMeasurement();

    //Get lights from DB
    cullUpdateLightsFromDBCounter.beginMeasurement();
    updateLightAndSkyboxData(mapRenderPlan, frustumData, stateForConditions, areaRecord);
    cullUpdateLightsFromDBCounter.endMeasurement();

    ///-----------------------------------


    auto lcurrentWMO = mapRenderPlan->m_currentWMO;
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
            mapRenderPlan->wmoArray.addToDrawn(mapRenderPlan->m_currentWMO);
        }

        cullExterior.beginMeasurement();
        auto exterior = mapRenderPlan->viewsHolder.getExterior();
        if ( exterior != nullptr ) {
            //Fix FrustumData for exterior was created after WMO traversal. So we need to fix it
            exterior->frustumData.perspectiveMat = frustumData.perspectiveMat;
            exterior->frustumData.viewMat = frustumData.viewMat;
            exterior->frustumData.farPlane = frustumData.farPlane;

            checkExterior(cameraPos, exterior->frustumData, m_viewRenderOrder, mapRenderPlan);
        }
        cullExterior.endMeasurement();
    } else {
        //Cull exterior
        cullExterior.beginMeasurement();
        auto exteriorView = mapRenderPlan->viewsHolder.getOrCreateExterior(frustumData);
        checkExterior(cameraPos, exteriorView->frustumData, m_viewRenderOrder, mapRenderPlan);
        cullExterior.endMeasurement();
    }

    cullSkyDoms.beginMeasurement();


    if ((mapRenderPlan->viewsHolder.getExterior() != nullptr || mapRenderPlan->currentWmoGroupIsExtLit || mapRenderPlan->currentWmoGroupShowExtSkybox) && (!m_exteriorSkyBoxes.empty())) {
        auto exteriorView = mapRenderPlan->viewsHolder.getOrCreateExterior(frustumData);
        auto skyBoxView = mapRenderPlan->viewsHolder.getSkybox();
        if (m_wdlObject != nullptr) {
            m_wdlObject->checkSkyScenes(
                stateForConditions,
                exteriorView->m2List,
                cameraPos,
                frustumData);
        }

        if (config->renderSkyDom) {
            for (auto &model : m_exteriorSkyBoxes) {
                if (model != nullptr) {
                    skyBoxView->m2List.addToDraw(model);
                }
            }
        }
    }
    cullSkyDoms.endMeasurement();

    cullCombineAllObjects.beginMeasurement();
    {
        auto exteriorView = mapRenderPlan->viewsHolder.getExterior();
        if (exteriorView != nullptr) {
            exteriorView->addM2FromGroups(frustumData, cameraPos);
            for (auto &adtRes: exteriorView->drawnADTs) {
                adtRes->adtObject->collectMeshes(*adtRes, exteriorView->m_adtOpaqueMeshes,
                                                 exteriorView->m_adtLiquidTransparentMeshes,
                                                 exteriorView->renderOrder);
            }
            mapRenderPlan->m2Array.addDrawnAndToLoad(exteriorView->m2List);
            mapRenderPlan->wmoGroupArray.addToLoadAndDraw(exteriorView->wmoGroupArray);
        }
    }

    //Fill and collect M2 objects for views from WmoGroups
    {
        auto &interiorViews = mapRenderPlan->viewsHolder.getInteriorViews();

        for (auto &interiorView: interiorViews) {
            interiorView->addM2FromGroups(frustumData, cameraPos);
            mapRenderPlan->m2Array.addDrawnAndToLoad(interiorView->m2List);
            mapRenderPlan->wmoGroupArray.addToLoadAndDraw(interiorView->wmoGroupArray);
        }
    }

    cullCombineAllObjects.endMeasurement();

    mapRenderPlan->renderSky = m_api->getConfig()->renderSkyDom &&
        (!m_suppressDrawingSky && (mapRenderPlan->viewsHolder.getExterior() || mapRenderPlan->currentWmoGroupIsExtLit));

    if (m_skyConeAlpha > 0) {
        mapRenderPlan->skyMesh = skyMesh;
    }
    if (mapRenderPlan->frameDependentData->overrideValuesWithFinalFog) {
        mapRenderPlan->skyMesh0x4 = skyMesh0x4Sky;
    }

    m_api->getConfig()->cullCreateVarsCounter           = cullCreateVarsCounter.getTimePerFrame();
    m_api->getConfig()->cullGetCurrentWMOCounter        = cullGetCurrentWMOCounter.getTimePerFrame();
    m_api->getConfig()->cullGetCurrentZoneCounter       = cullGetCurrentZoneCounter.getTimePerFrame();
    m_api->getConfig()->cullUpdateLightsFromDBCounter   = cullUpdateLightsFromDBCounter.getTimePerFrame();
    m_api->getConfig()->cullExterior                    = cullExterior.getTimePerFrame();
    m_api->getConfig()->cullExteriorSetDecl             = cullExteriorSetDecl.getTimePerFrame();
    m_api->getConfig()->cullExteriorWDLCull             = cullExteriorWDLCull.getTimePerFrame();
    m_api->getConfig()->cullExteriorGetCands            = cullExteriorGetCands.getTimePerFrame();
    m_api->getConfig()->cullExterioFrustumWMO           = cullExterioFrustumWMO.getTimePerFrame();
    m_api->getConfig()->cullExterioFrustumM2            = cullExterioFrustumM2.getTimePerFrame();
    m_api->getConfig()->cullSkyDoms                     = cullSkyDoms.getTimePerFrame();
    m_api->getConfig()->cullCombineAllObjects           = cullCombineAllObjects.getTimePerFrame();
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

mathfu::vec3 blendV3(mathfu::vec3 a, mathfu::vec3 b, float alpha) {
    return (a - b) * alpha + a;
}

void Map::updateLightAndSkyboxData(const HMapRenderPlan &mapRenderPlan, MathHelper::FrustumCullingData &frustumData,
                                   StateForConditions &stateForConditions, const AreaRecord &areaRecord) {
    ZoneScoped ;

    Config* config = this->m_api->getConfig();

    bool fogRecordWasFound = false;
    mathfu::vec3 endFogColor = mathfu::vec3(0.0, 0.0, 0.0);

    std::vector<LightResult> wmoFogs = {};
    if (mapRenderPlan->m_currentWMO != nullptr) {
        mapRenderPlan->m_currentWMO->checkFog(frustumData.cameraPos, wmoFogs);
    }
    std::vector<LightResult> lightResults;

    if ((m_api->databaseHandler != nullptr)) {
        //Check zoneLight
        getLightResultsFromDB(frustumData.cameraPos, config, lightResults, &stateForConditions);

        //Delete skyboxes that are not in light array
        std::unordered_map<int, std::shared_ptr<M2Object>> perFdidMap;
        auto modelIt = m_exteriorSkyBoxes.begin();
        while (modelIt != m_exteriorSkyBoxes.end()) {
            bool found = false;
            for (auto &_light : lightResults) {
                if (_light.skyBoxFdid == (*modelIt)->getModelFileId()) {
                    found = true;
                    break;
                }
            }

            if (found) {
                perFdidMap[(*modelIt)->getModelFileId()] = *modelIt;
                modelIt++;
            } else {
                modelIt = m_exteriorSkyBoxes.erase(modelIt);
            }
        }

        m_skyConeAlpha = 1.0;
        for (auto &_light : lightResults) {
            if (_light.skyBoxFdid == 0 || _light.lightSkyboxId == 0) continue;

            stateForConditions.currentSkyboxIds.push_back(_light.lightSkyboxId);
            std::shared_ptr<M2Object> skyBox = nullptr;
            if (perFdidMap[_light.skyBoxFdid] == nullptr) {
                skyBox = std::make_shared<M2Object>(m_api, true);
                skyBox->setLoadParams(0, {}, {});

                skyBox->setModelFileId(_light.skyBoxFdid);

                skyBox->createPlacementMatrix(mathfu::vec3(0, 0, 0), 0, mathfu::vec3(1, 1, 1), nullptr);
                skyBox->calcWorldPosition();
                m_exteriorSkyBoxes.push_back(skyBox);
            } else {
                skyBox = perFdidMap[_light.skyBoxFdid];
            }

            skyBox->setAlpha(_light.blendCoef);
            if ((_light.skyBoxFlags & 4) > 0 ) {
                //In this case conus is still rendered been, but all values are final fog values.
                auto fdd = mapRenderPlan->frameDependentData;
                fdd->overrideValuesWithFinalFog = true;
            }

            if ((_light.skyBoxFlags & 2) == 0) {
//                m_skyConeAlpha -= _light.blendCoef;
                m_skyConeAlpha -= _light.blendCoef;
            }

            if (_light.skyBoxFlags & 1) {
                skyBox->setOverrideAnimationPerc(config->currentTime / 2880.0, true);
            }
        }

        //Blend glow and ambient
        mathfu::vec3 ambientColor = {0, 0, 0};
        mathfu::vec3 horizontAmbientColor = {0, 0, 0};
        mathfu::vec3 groundAmbientColor = {0, 0, 0};
        mathfu::vec3 directColor = {0, 0, 0};
        mathfu::vec3 closeRiverColor = {0, 0, 0};
        mathfu::vec3 farRiverColor = {0, 0, 0};
        mathfu::vec3 closeOceanColor = {0, 0, 0};
        mathfu::vec3 farOceanColor = {0, 0, 0};

        mathfu::vec3 SkyTopColor = {0, 0, 0};
        mathfu::vec3 SkyMiddleColor = {0, 0, 0};
        mathfu::vec3 SkyBand1Color = {0, 0, 0};
        mathfu::vec3 SkyBand2Color = {0, 0, 0};
        mathfu::vec3 SkySmogColor = {0, 0, 0};
        mathfu::vec3 SkyFogColor = {0, 0, 0};
        float currentGlow = 0;

        for (auto &_light : lightResults) {
            currentGlow += _light.glow * _light.blendCoef;

            ambientColor += mathfu::vec3(_light.ambientColor) * _light.blendCoef;
            horizontAmbientColor += mathfu::vec3(_light.horizontAmbientColor) * _light.blendCoef;
            groundAmbientColor += mathfu::vec3(_light.groundAmbientColor) * _light.blendCoef;
            directColor += mathfu::vec3(_light.directColor) * _light.blendCoef;

            closeRiverColor += mathfu::vec3(_light.closeRiverColor) * _light.blendCoef;
            farRiverColor += mathfu::vec3(_light.farRiverColor) * _light.blendCoef;
            closeOceanColor += mathfu::vec3(_light.closeOceanColor) * _light.blendCoef;
            farOceanColor += mathfu::vec3(_light.farOceanColor) * _light.blendCoef;

            SkyTopColor += mathfu::vec3(_light.SkyTopColor.data()) * _light.blendCoef;
            SkyMiddleColor += mathfu::vec3(_light.SkyMiddleColor) * _light.blendCoef;
            SkyBand1Color += mathfu::vec3(_light.SkyBand1Color) * _light.blendCoef;
            SkyBand2Color += mathfu::vec3(_light.SkyBand2Color) * _light.blendCoef;
            SkySmogColor += mathfu::vec3(_light.SkySmogColor) * _light.blendCoef;
            SkyFogColor += mathfu::vec3(_light.SkyFogColor.data()) * _light.blendCoef;
        }

        //Database is in BGRA
        float ambientMult = areaRecord.ambientMultiplier * 2.0f + 1;
//        ambientColor *= ambientMult;
//        groundAmbientColor *= ambientMult;
//        horizontAmbientColor *= ambientMult;

        if (config->glowSource == EParameterSource::eDatabase) {
            auto fdd = mapRenderPlan->frameDependentData;
            fdd->currentGlow = currentGlow;
        } else if (config->glowSource == EParameterSource::eConfig) {
            auto fdd = mapRenderPlan->frameDependentData;
            fdd->currentGlow = config->currentGlow;
        }


        if (config->globalLighting == EParameterSource::eDatabase) {
            auto fdd = mapRenderPlan->frameDependentData;

            fdd->exteriorAmbientColor = mathfu::vec4(ambientColor[2], ambientColor[1], ambientColor[0], 0);
            fdd->exteriorGroundAmbientColor = mathfu::vec4(groundAmbientColor[2], groundAmbientColor[1], groundAmbientColor[0],
                                                  0);
            fdd->exteriorHorizontAmbientColor = mathfu::vec4(horizontAmbientColor[2], horizontAmbientColor[1],
                                                    horizontAmbientColor[0], 0);
            fdd->exteriorDirectColor = mathfu::vec4(directColor[2], directColor[1], directColor[0], 0);
            auto extDir = MathHelper::calcExteriorColorDir(
                frustumData.viewMat,
                m_api->getConfig()->currentTime
            );
            fdd->exteriorDirectColorDir = { extDir.x, extDir.y, extDir.z };
        } else if (config->globalLighting == EParameterSource::eConfig) {
            auto fdd = mapRenderPlan->frameDependentData;

            fdd->exteriorAmbientColor = config->exteriorAmbientColor;
            fdd->exteriorGroundAmbientColor = config->exteriorGroundAmbientColor;
            fdd->exteriorHorizontAmbientColor = config->exteriorHorizontAmbientColor;
            fdd->exteriorDirectColor = config->exteriorDirectColor;
            auto extDir = MathHelper::calcExteriorColorDir(
                frustumData.viewMat,
                m_api->getConfig()->currentTime
            );
            fdd->exteriorDirectColorDir = { extDir.x, extDir.y, extDir.z };
        }

        {
            auto fdd = mapRenderPlan->frameDependentData;
            fdd->useMinimapWaterColor = config->useMinimapWaterColor;
            fdd->useCloseRiverColorForDB = config->useCloseRiverColorForDB;
        }
        if (config->waterColorParams == EParameterSource::eDatabase)
        {
            auto fdd = mapRenderPlan->frameDependentData;
            fdd->closeRiverColor = mathfu::vec4(closeRiverColor[2], closeRiverColor[1], closeRiverColor[0], 0);
            fdd->farRiverColor = mathfu::vec4(farRiverColor[2], farRiverColor[1], farRiverColor[0], 0);
            fdd->closeOceanColor = mathfu::vec4(closeOceanColor[2], closeOceanColor[1], closeOceanColor[0], 0);
            fdd->farOceanColor = mathfu::vec4(farOceanColor[2], farOceanColor[1], farOceanColor[0], 0);
        } else if (config->waterColorParams == EParameterSource::eConfig) {
            auto fdd = mapRenderPlan->frameDependentData;
            fdd->closeRiverColor = config->closeRiverColor;
            fdd->farRiverColor = config->farRiverColor;
            fdd->closeOceanColor = config->closeOceanColor;
            fdd->farOceanColor = config->farOceanColor;
        }
        if (config->skyParams == EParameterSource::eDatabase) {
            auto fdd = mapRenderPlan->frameDependentData;
            fdd->SkyTopColor =      mathfu::vec4(SkyTopColor[2], SkyTopColor[1], SkyTopColor[0], 1.0);
            fdd->SkyMiddleColor =   mathfu::vec4(SkyMiddleColor[2], SkyMiddleColor[1], SkyMiddleColor[0], 1.0);
            fdd->SkyBand1Color =    mathfu::vec4(SkyBand1Color[2], SkyBand1Color[1], SkyBand1Color[0], 1.0);
            fdd->SkyBand2Color =    mathfu::vec4(SkyBand2Color[2], SkyBand2Color[1], SkyBand2Color[0], 1.0);
            fdd->SkySmogColor =     mathfu::vec4(SkySmogColor[2], SkySmogColor[1], SkySmogColor[0], 1.0);
            fdd->SkyFogColor =      mathfu::vec4(SkyFogColor[2], SkyFogColor[1], SkyFogColor[0], 1.0);
        }
    }

    //Handle fog
    {
        float FogEnd = 0;
        float FogScaler = 0;
        float FogDensity = 0;
        float FogHeight = 0;
        float FogHeightScaler = 0;
        float FogHeightDensity = 0;
        float SunFogAngle = 0;
        mathfu::vec3 EndFogColor = mathfu::vec3(0,0,0);
        float EndFogColorDistance = 0;
        mathfu::vec3 SunFogColor = mathfu::vec3(0,0,0);
        float SunFogStrength = 0;
        mathfu::vec3 FogHeightColor = mathfu::vec3(0,0,0);
        mathfu::vec4 FogHeightCoefficients = mathfu::vec4(0,0,0,0);
        mathfu::vec4 MainFogCoefficients = mathfu::vec4(0,0,0,0);
        mathfu::vec4 HeightDensityFogCoefficients = mathfu::vec4(0,0,0,0);
        float FogZScalar = 0;
        float LegacyFogScalar = 0;
        float MainFogStartDist = 0;
        float MainFogEndDist = 0;
        float FogBlendAlpha = 0;
        mathfu::vec3 HeightEndFogColor = mathfu::vec3(0,0,0);
        float FogStartOffset = 0;

        std::vector<LightResult> combinedResults = {};
        float totalSummator = 0.0;

        //Apply fog from WMO
        {
            bool fogDefaultExist = false;
            int fogDefaultIndex = -1;
            for (int i = 0; i < wmoFogs.size() && totalSummator < 1.0f; i++) {
                auto &fogRec = wmoFogs[i];
                if (fogRec.isDefault) {
                    fogDefaultExist = true;
                    fogDefaultIndex = i;
                    continue;
                }
                if (totalSummator + fogRec.blendCoef > 1.0f) {
                    fogRec.blendCoef = 1.0f - totalSummator;
                    totalSummator = 1.0f;
                } else {
                    totalSummator += fogRec.blendCoef;
                }
                combinedResults.push_back(fogRec);
            }

            if (fogDefaultExist && totalSummator < 1.0f) {
                wmoFogs[fogDefaultIndex].blendCoef = 1.0f - totalSummator;
                totalSummator = 1.0f;
                combinedResults.push_back(wmoFogs[fogDefaultIndex]);
            }
        }

        //Apply fogs from lights
        if (totalSummator < 1.0) {
            if (config->globalFog == EParameterSource::eDatabase) {
                bool fogDefaultExist = false;
                int fogDefaultIndex = -1;

                for (int i = 0; i < lightResults.size() && totalSummator < 1.0f; i++) {
                    auto &fogRec = lightResults[i];
                    if (fogRec.isDefault) {
                        fogDefaultExist = true;
                        fogDefaultIndex = i;
                        continue;
                    }
                    if (totalSummator + fogRec.blendCoef > 1.0f) {
                        fogRec.blendCoef = 1.0f - totalSummator;
                        totalSummator = 1.0f;
                    } else {
                        totalSummator += fogRec.blendCoef;
                    }
                    combinedResults.push_back(fogRec);
                }
                if (fogDefaultExist && totalSummator < 1.0f) {
                    lightResults[fogDefaultIndex].blendCoef = 1.0f - totalSummator;
                    totalSummator = 1.0f;
                    combinedResults.push_back(lightResults[fogDefaultIndex]);
                }
            } else if (config->globalFog == EParameterSource::eConfig) {
                LightResult globalFog;
                globalFog.FogScaler = config->FogScaler;
                globalFog.FogEnd = config->FogEnd;
                globalFog.FogDensity = config->FogDensity;

                globalFog.FogHeightScaler = config->FogHeightScaler;
                globalFog.FogHeightDensity = config->FogHeightDensity;
                globalFog.SunFogAngle = config->SunFogAngle;
                globalFog.EndFogColorDistance = config->EndFogColorDistance;
                globalFog.SunFogStrength = config->SunFogStrength;

                globalFog.blendCoef = 1.0 - totalSummator;
                globalFog.isDefault = true;

                globalFog.EndFogColor = {config->EndFogColor.z, config->EndFogColor.y, config->EndFogColor.x};
                globalFog.SunFogColor = {config->SunFogColor.z, config->SunFogColor.y, config->SunFogColor.x};
                globalFog.FogHeightColor = {config->FogHeightColor.z, config->FogHeightColor.y, config->FogHeightColor.x};

                combinedResults.push_back(globalFog);
            }
        }
        std::sort(combinedResults.begin(), combinedResults.end(), [](const LightResult &a, const LightResult &b) -> bool {
            return a.blendCoef > b.blendCoef;
        });

        //Rebalance blendCoefs
        if (totalSummator < 1.0f && totalSummator > 0.0f) {
            for (auto &_light : combinedResults) {
                _light.blendCoef = _light.blendCoef / totalSummator;
            }
        }

//        for (auto &_light : combinedResults) {
//            FogEnd += _light.FogEnd * _light.blendCoef;
//            FogScaler += _light.FogScaler * _light.blendCoef;
//            FogDensity += _light.FogDensity * _light.blendCoef;
//            FogHeight += _light.FogHeight * _light.blendCoef;
//            FogHeightScaler += _light.FogHeightScaler * _light.blendCoef;
//            FogHeightDensity += _light.FogHeightDensity * _light.blendCoef;
//            SunFogAngle += _light.SunFogAngle * _light.blendCoef;
//
//            EndFogColor += mathfu::vec3(_light.EndFogColor.data()) * _light.blendCoef;
//            EndFogColorDistance += _light.EndFogColorDistance * _light.blendCoef;
//            SunFogColor += mathfu::vec3(_light.SunFogColor.data()) * _light.blendCoef;
//            SunFogStrength += _light.SunFogStrength * _light.blendCoef;
//            FogHeightColor += mathfu::vec3(_light.FogHeightColor.data()) * _light.blendCoef;
//            FogHeightCoefficients += mathfu::vec4(_light.FogHeightCoefficients.data()) * _light.blendCoef;
//            MainFogCoefficients += mathfu::vec4(_light.MainFogCoefficients.data()) * _light.blendCoef;
//            HeightDensityFogCoefficients += mathfu::vec4(_light.HeightDensityFogCoefficients.data()) * _light.blendCoef;
//            FogZScalar += _light.FogZScalar * _light.blendCoef;
//            LegacyFogScalar += _light.LegacyFogScalar * _light.blendCoef;
//            MainFogStartDist += _light.MainFogStartDist * _light.blendCoef;
//            MainFogEndDist += _light.MainFogEndDist * _light.blendCoef;
//            FogBlendAlpha += _light.FogBlendAlpha * _light.blendCoef;
//            HeightEndFogColor += mathfu::vec3(_light.HeightEndFogColor.data()) * _light.blendCoef;
//            FogStartOffset += _light.FogStartOffset * _light.blendCoef;
//        }
//        FogBlendAlpha = 1.0f;

        //In case of no data -> disable the fog
        {
            auto fdd = mapRenderPlan->frameDependentData;
            fdd->FogDataFound = !combinedResults.empty();
//            std::cout << "combinedResults.empty() = " << combinedResults.empty() << std::endl;
//            std::cout << "combinedResults.size() = " << combinedResults.size() << std::endl;
            fdd->EndFogColor = mathfu::vec3(0.0f, 0, 0);
            for (auto &_light : lightResults) {
                auto &fogResult = fdd->fogResults.emplace_back();
                fogResult.FogEnd = _light.FogEnd;
                fogResult.FogScaler = _light.FogScaler;
                fogResult.FogDensity = _light.FogDensity;
                fogResult.FogHeight = _light.FogHeight;
                fogResult.FogHeightScaler = _light.FogHeightScaler;
                fogResult.FogHeightDensity = _light.FogHeightDensity;
                fogResult.SunFogAngle = _light.SunFogAngle;
                if (fdd->overrideValuesWithFinalFog) {
                    fogResult.FogColor = mathfu::vec3(_light.EndFogColor[2], _light.EndFogColor[1], _light.EndFogColor[0]);
                } else {
                    fogResult.FogColor = mathfu::vec3(_light.SkyFogColor[2], _light.SkyFogColor[1], _light.SkyFogColor[0]);
                }
                fogResult.EndFogColor = mathfu::vec3(_light.EndFogColor[2], _light.EndFogColor[1], _light.EndFogColor[0]);
                fdd->EndFogColor += fogResult.EndFogColor * _light.blendCoef;
                fogResult.EndFogColorDistance = _light.EndFogColorDistance;
                fogResult.SunFogColor = mathfu::vec3(_light.SunFogColor[2], _light.SunFogColor[1], _light.SunFogColor[0]);
                fogResult.SunFogStrength = _light.SunFogStrength;
                fogResult.FogHeightColor = mathfu::vec3(_light.FogHeightColor[2], _light.FogHeightColor[1], _light.FogHeightColor[0]);
                fogResult.FogHeightCoefficients = mathfu::vec4(_light.FogHeightCoefficients[0], _light.FogHeightCoefficients[1],
                                                          _light.FogHeightCoefficients[2], _light.FogHeightCoefficients[3]);
                fogResult.MainFogCoefficients = mathfu::vec4(_light.MainFogCoefficients[0], _light.MainFogCoefficients[1],
                                                        _light.MainFogCoefficients[2], _light.MainFogCoefficients[3]);
                fogResult.HeightDensityFogCoefficients = mathfu::vec4(_light.HeightDensityFogCoefficients[0],
                                                                 _light.HeightDensityFogCoefficients[1],
                                                                 _light.HeightDensityFogCoefficients[2],
                                                                 _light.HeightDensityFogCoefficients[3]);

                fogResult.FogZScalar = _light.FogZScalar;
                fogResult.LegacyFogScalar = _light.LegacyFogScalar;
                fogResult.MainFogStartDist = _light.MainFogStartDist;
                fogResult.MainFogEndDist = _light.MainFogEndDist;
                fogResult.FogBlendAlpha = _light.blendCoef;
                fogResult.HeightEndFogColor = mathfu::vec3(_light.HeightEndFogColor[2], _light.HeightEndFogColor[1], _light.HeightEndFogColor[0]);
                fogResult.FogStartOffset = _light.FogStartOffset;
            }
        }
    }

//    this->m_api->getConfig()->setClearColor(0,0,0,0);
}

void Map::getLightResultsFromDB(mathfu::vec3 &cameraVec3, const Config *config, std::vector<LightResult> &lightResults, StateForConditions *stateForConditions) {
    if (m_api->databaseHandler == nullptr)
        return ;

    LightResult zoneLightResult;

    bool zoneLightFound = false;
    int LightId;
    for (const auto &zoneLight : m_zoneLights) {
        CAaBox laabb = zoneLight.aabb;
        auto const vec50 = mathfu::vec3(50.0f,50.0f,0);
        laabb.min = (mathfu::vec3(laabb.min) - vec50);
        laabb.max = (mathfu::vec3(laabb.max) + vec50);
        if (MathHelper::isPointInsideNonConvex(cameraVec3, zoneLight.aabb, zoneLight.points)) {
            zoneLightFound = true;

            if (stateForConditions != nullptr) {
                stateForConditions->currentZoneLights.push_back(zoneLight.ID);
            }
            LightId = zoneLight.LightID;
            break;
        }
    }

    if (zoneLightFound) {
        m_api->databaseHandler->getLightById(LightId, config->currentTime, zoneLightResult, config->farPlane);
        if (stateForConditions != nullptr) {
            stateForConditions->currentZoneLights.push_back(zoneLightResult.lightParamId);
        }
    }

    m_api->databaseHandler->getEnvInfo(m_mapId,
                                       cameraVec3.x,
                                       cameraVec3.y,
                                       cameraVec3.z,
                                       config->currentTime,
                                       lightResults,
                                       config->farPlane
    );


    if (stateForConditions != nullptr) {
        for (auto &_light : lightResults) {
            stateForConditions->currentZoneLights.push_back(_light.lightParamId);
        }
    }

    //Calc final blendcoef for zoneLight;
    if (zoneLightFound) {
        float blendCoef = 1.0;

        for (auto &_light : lightResults) {
            if (!_light.isDefault) {
                blendCoef -= _light.blendCoef;
            }
        }
        if (blendCoef > 0) {
            zoneLightResult.blendCoef = blendCoef;
            lightResults.push_back(zoneLightResult);
            //Delete default from results;
            auto it = lightResults.begin();
            while (it != lightResults.end()) {
                if (it->isDefault) {
                    lightResults.erase(it);
                    break;
                } else
                    it++;
            }
        }
    }
}
void Map::getPotentialEntities(const MathHelper::FrustumCullingData &frustumData,
                               const mathfu::vec4 &cameraPos,
                               HMapRenderPlan &mapRenderPlan,
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
                wmoMap = std::make_shared<WmoObject>(m_api);
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
                        HMapRenderPlan &mapRenderPlan
) {
    ZoneScoped ;
//    std::cout << "Map::checkExterior finished called" << std::endl;
    if (m_wdlObject == nullptr && m_wdtfile != nullptr && m_wdtfile->getStatus() == FileStatus::FSLoaded) {
        if (m_wdtfile->mphd->flags.wdt_has_maid) {
            m_wdlObject = std::make_shared<WdlObject>(m_api, m_wdtfile->mphd->wdlFileDataID);
            m_wdlObject->setMapApi(this);
        }
    }

    auto exteriorView = mapRenderPlan->viewsHolder.getExterior(); //Should not be null, since we called checkExterior

    cullExteriorWDLCull.beginMeasurement();
    if (m_wdlObject != nullptr) {
        m_wdlObject->checkFrustumCulling(frustumData, cameraPos, exteriorView->m2List, mapRenderPlan->wmoArray);
    }
    cullExteriorWDLCull.endMeasurement();

    cullExteriorGetCands.beginMeasurement();
    getCandidatesEntities(frustumData, cameraPos, mapRenderPlan, exteriorView->m2List, mapRenderPlan->wmoArray);
    cullExteriorGetCands.endMeasurement();

    cullExterioFrustumWMO.beginMeasurement();
    //Frustum cull
    for (auto &wmoCandidate : mapRenderPlan->wmoArray.getCandidates()) {
        if (!wmoCandidate->isLoaded()) continue;

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
    cullExterioFrustumWMO.endMeasurement();

    cullExterioFrustumM2.beginMeasurement();
    //3.2 Iterate over all global WMOs and M2s (they have uniqueIds)
    {
        int numThreads = m_api->getConfig()->threadCount;
        auto results = std::vector<uint32_t>();

        auto &candidates = exteriorView->m2List.getCandidates();
        if (candidates.size() > 0) {
            results = std::vector<uint32_t>(candidates.size(), 0xFFFFFFFF);

            oneapi::tbb::parallel_for(tbb::blocked_range<size_t>(0, candidates.size(), 2000),
                                      [&](tbb::blocked_range<size_t> &r) {
//            for (size_t i = r.begin(); i != r.end(); ++i) {
//                auto &m2ObjectCandidate = tmpVector[i];
//                if(m2ObjectCandidate->checkFrustumCulling(cameraPos, frustumData)) {
//                    exteriorView->drawnM2s.insert(m2ObjectCandidate);
//                    cullStage->m2Array.insert(m2ObjectCandidate);
//                }
#if (__AVX__ && __SSE2__)
                  ObjectCullingSEE<std::shared_ptr<M2Object>>::cull(frustumData,
                                                                    r.begin(), r.end(), candidates,
                                                                    results);
#else
                  ObjectCulling<std::shared_ptr<M2Object>>::cull(frustumData,
                                                                 r.begin(), r.end(), candidates,
                                                                 results);
#endif
//            }
            }, tbb::auto_partitioner());
        }

        for (int i = 0; i < candidates.size(); i++) {
            if (!results[i]) {
                auto &m2ObjectCandidate = candidates[i];
                exteriorView->m2List.addToDraw(m2ObjectCandidate);
            }
        }
        cullExterioFrustumM2.endMeasurement();
    }
}

void Map::getCandidatesEntities(const MathHelper::FrustumCullingData &frustumData,
                                const mathfu::vec4 &cameraPos,
                                HMapRenderPlan &mapRenderPlan,
                                M2ObjectListContainer &m2ObjectsCandidates,
                                WMOListContainer &wmoCandidates) {
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

        } else {
            wmoCandidates.addCand(wmoMap);
        }
    }
}

void Map::checkADTCulling(int i, int j,
                          const MathHelper::FrustumCullingData &frustumData,
                          const mathfu::vec4 &cameraPos,
                          HMapRenderPlan &mapRenderPlan,
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

    auto adtObject = mapTiles[i][j];
    if (adtObject != nullptr) {

        std::shared_ptr<ADTObjRenderRes> adtFrustRes = std::make_shared<ADTObjRenderRes>();
        adtFrustRes->adtObject = adtObject;

        bool result = adtObject->checkFrustumCulling(
            *adtFrustRes.get(),
            cameraPos,
            frustumData, m2ObjectsCandidates, wmoCandidates);

//        if (this->m_adtBBHolder != nullptr) {
//            //When adt passes BBHolder test, consider it influences the picture even if checkFrustumCulling
//            //is false. So do forceful update for freeStrategy
//            adtObject->getFreeStrategy()(false, true, this->getCurrentSceneTime());
//        }

        if (result) {
            mapRenderPlan->viewsHolder.getExterior()->drawnADTs.push_back(adtFrustRes);
            mapRenderPlan->adtArray.push_back(adtFrustRes);
        }
    } else if (!m_lockedMap && true) { //(m_wdtfile->mapTileTable->mainInfo[j][i].Flag_HasADT > 0) {
        if (m_wdtfile->mphd->flags.wdt_has_maid) {
            auto &mapFileIds = m_wdtfile->mapFileDataIDs[j * 64 + i];
            if (mapFileIds.rootADT > 0) {
                adtObject = std::make_shared<AdtObject>(m_api, i, j,
                                                        m_wdtfile->mapFileDataIDs[j * 64 + i],
                                                        m_wdtfile);
            } else {
                return;
            }
        } else {
            std::string adtFileTemplate =
                "world/maps/" + mapName + "/" + mapName + "_" + std::to_string(i) + "_" +
                std::to_string(j);
            adtObject = std::make_shared<AdtObject>(m_api, adtFileTemplate, mapName, i, j, m_wdtfile);
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

    for (auto &m2Object : renderPlan->m2Array.getToLoadMain()) {
        if (m2Object == nullptr) continue;
        m2Object->doLoadMainFile();
    }
    for (auto &m2Object : renderPlan->m2Array.getToLoadGeom()) {
        if (m2Object == nullptr) continue;
        m2Object->doLoadGeom(sceneRenderer);
        m2ProcessedThisFrame++;
//        if (m2ProcessedThisFrame > 100) break;
    }

    if (auto skyboxView = renderPlan->viewsHolder.getSkybox()) {
        for (auto &m2Object : skyboxView->m2List.getToLoadMain()) {
            if (m2Object == nullptr) continue;
            m2Object->doLoadMainFile();
        }
        for (auto &m2Object: skyboxView->m2List.getToLoadGeom()) {
            if (m2Object == nullptr) continue;
            m2Object->doLoadGeom(sceneRenderer);
        }
    }
//    }

    for (auto &wmoObject : renderPlan->wmoArray.getToLoad()) {
        if (wmoObject == nullptr) continue;
        wmoObject->doPostLoad(sceneRenderer);
    }
    for (auto &wmoGroupObject : renderPlan->wmoGroupArray.getToLoad()) {
        if (wmoGroupObject == nullptr) continue;
        wmoGroupObject->doPostLoad(sceneRenderer);
        wmoGroupsProcessedThisFrame++;
        if (wmoGroupsProcessedThisFrame > 20) break;
    }

    for (auto &adtObject : renderPlan->adtArray) {
        adtObject->adtObject->doPostLoad(sceneRenderer);
    }

    if (quadBindings == nullptr)
    {
        const float epsilon = 0.f;

        std::array<mathfu::vec2_packed, 4> vertexBuffer = {
            mathfu::vec2_packed(mathfu::vec2(-1.0f + epsilon, -1.0f + epsilon)),
            mathfu::vec2_packed(mathfu::vec2(-1.0f + epsilon,  1.0f - epsilon)),
            mathfu::vec2_packed(mathfu::vec2(1.0f  - epsilon,  -1.0f+ epsilon)),
            mathfu::vec2_packed(mathfu::vec2(1.0f  - epsilon,  1.f  - epsilon))
        };
        std::vector<uint16_t > indexBuffer = {
            0, 1, 2,
            2, 1, 3
        };

//        std::cout << "indexBuffer.size = " << indexBuffer.size() << std::endl;

//TODO:
/*
        auto quadIBO = m_api->hDevice->createIndexBuffer();
        quadIBO->uploadData(
            indexBuffer.data(),
            indexBuffer.size() * sizeof(uint16_t));

        auto quadVBO = m_api->hDevice->createVertexBuffer();
        quadVBO->uploadData(
            vertexBuffer.data(),
            vertexBuffer.size() * sizeof(mathfu::vec2_packed)
        );

        quadBindings = m_api->hDevice->createVertexBufferBindings();
        quadBindings->setIndexBuffer(quadIBO);

        GVertexBufferBinding vertexBinding;
        vertexBinding.vertexBuffer = quadVBO;

        vertexBinding.bindings = std::vector<GBufferBinding>(fullScreen.begin(), fullScreen.end());

        quadBindings->addVertexBufferBinding(vertexBinding);
        quadBindings->save();
        */
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
        m2UpdateframeCounter.beginMeasurement();

        auto threadsAvailable = m_api->getConfig()->hardwareThreadCount();

        int granSize = m2ToDraw.size() / (2 * threadsAvailable);
        if (granSize == 0) granSize = m2ToDraw.size();

        if (granSize > 0) {
            tbb::parallel_for(tbb::blocked_range<size_t>(0, m2ToDraw.size(), granSize),
                              [&](tbb::blocked_range<size_t> r) {
                                  for (size_t i = r.begin(); i != r.end(); ++i) {
                                      auto &m2Object = m2ToDraw[i];
                                      m2Object->update(deltaTime, cameraVec3, lookAtMat);
                                  }
                              }, tbb::simple_partitioner());
        }

        if (auto skyBoxView = renderPlan->viewsHolder.getSkybox()) {
            for (auto &m2Object : skyBoxView->m2List.getDrawn()) {
                m2Object->update(deltaTime, cameraVec3, lookAtMat);
            }
        }

        m2UpdateframeCounter.endMeasurement();
    }

    {
        ZoneScopedN("wmoUpdate");
        wmoUpdate.beginMeasurement();
        for (const auto &wmoObject: renderPlan->wmoArray.getToDrawn()) {
            if (wmoObject == nullptr) continue;
            wmoObject->update();
        }
        wmoUpdate.endMeasurement();
    }

    {
        ZoneScopedN("wmoGroupUpdate");
        wmoGroupUpdate.beginMeasurement();
        for (const auto &wmoGroupObject: renderPlan->wmoGroupArray.getToDraw()) {
            if (wmoGroupObject == nullptr) continue;
            wmoGroupObject->update();
        }
        wmoGroupUpdate.endMeasurement();
    }

    {
        ZoneScopedN("adtUpdate");
        adtUpdate.beginMeasurement();
        {
            std::unordered_set<std::shared_ptr<AdtObject>> processedADT;
            for (const auto &adtObjectRes: renderPlan->adtArray) {
                if (processedADT.count(adtObjectRes->adtObject) == 0) {
                    adtObjectRes->adtObject->update(deltaTime);
                    processedADT.insert(adtObjectRes->adtObject);
                }
            }
        }
        adtUpdate.endMeasurement();
    }


    //2. Calc distance every 100 ms
    m2calcDistanceCounter.beginMeasurement();
    tbb::parallel_for(tbb::blocked_range<size_t>(0, m2ToDraw.size(), 500),
          [&](tbb::blocked_range<size_t> r) {
              for (size_t i = r.begin(); i != r.end(); ++i) {
                  auto &m2Object = m2ToDraw[i];
                  if (m2Object == nullptr) continue;
                  m2Object->calcDistance(cameraVec3);
              }
          }, tbb::auto_partitioner()
    );
    m2calcDistanceCounter.endMeasurement();

    //Cleanup ADT every 10 seconds
    adtCleanupCounter.beginMeasurement();
    if (adtFreeLambda!= nullptr && adtFreeLambda(true, false, this->m_currentTime)) {
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
    adtCleanupCounter.endMeasurement();
    this->m_currentTime += deltaTime;

    m_api->getConfig()->m2UpdateTime = m2UpdateframeCounter.getTimePerFrame();
    m_api->getConfig()->wmoGroupUpdateTime = wmoGroupUpdate.getTimePerFrame();
    m_api->getConfig()->adtUpdateTime = adtUpdate.getTimePerFrame();
    m_api->getConfig()->m2calcDistanceTime = m2calcDistanceCounter.getTimePerFrame();
    m_api->getConfig()->adtCleanupTime = adtCleanupCounter.getTimePerFrame();
    //Collect meshes
}

void Map::updateBuffers(const HMapRenderPlan &renderPlan) {
    ZoneScoped;
    {
        auto &meshblockVS = skyMeshMat0x4->m_skyColors->getObject();
        auto EndFogColorV4_1 = mathfu::vec4(renderPlan->frameDependentData->EndFogColor, 0.0);
        auto EndFogColorV4_2 = mathfu::vec4(renderPlan->frameDependentData->EndFogColor, 1.0);
        meshblockVS.skyColor[0] = EndFogColorV4_1;
        meshblockVS.skyColor[1] = EndFogColorV4_1;
        meshblockVS.skyColor[2] = EndFogColorV4_1;
        meshblockVS.skyColor[3] = EndFogColorV4_1;
        meshblockVS.skyColor[4] = EndFogColorV4_1;
        meshblockVS.skyColor[5] = EndFogColorV4_2;
        skyMeshMat0x4->m_skyColors->save();
    }
    {
        auto &meshblockVS = skyMeshMat->m_skyColors->getObject();
        meshblockVS.skyColor[0] = renderPlan->frameDependentData->SkyTopColor;
        meshblockVS.skyColor[1] = renderPlan->frameDependentData->SkyMiddleColor;
        meshblockVS.skyColor[2] = renderPlan->frameDependentData->SkyBand1Color;
        meshblockVS.skyColor[3] = renderPlan->frameDependentData->SkyBand2Color;
        meshblockVS.skyColor[4] = renderPlan->frameDependentData->SkySmogColor;
        meshblockVS.skyColor[5] = renderPlan->frameDependentData->SkyFogColor;
        skyMeshMat->m_skyColors->save();
    }

    {
        ZoneScopedN("m2BuffersUpdate");
        auto threadsAvailable = m_api->getConfig()->hardwareThreadCount();
        auto &m2ToDraw = renderPlan->m2Array.getDrawn();
        int granSize = m2ToDraw.size() / (2 * threadsAvailable);
        if (granSize == 0) granSize = m2ToDraw.size();

        //Can't be paralleled?
        for (auto &m2Object: renderPlan->m2Array.getDrawn()) {
            if (m2Object != nullptr) {
                m2Object->fitParticleAndRibbonBuffersToSize();
            }
        }

        if (granSize > 0) {
            tbb::parallel_for(tbb::blocked_range<size_t>(0, m2ToDraw.size(), granSize),
            [&](tbb::blocked_range<size_t> r) {
                for (size_t i = r.begin(); i != r.end(); ++i) {
                    auto &m2Object = m2ToDraw[i];
                    if (m2Object != nullptr) {
                        m2Object->uploadGeneratorBuffers(renderPlan->renderingMatrices->lookAtMat,
                                                       renderPlan->frameDependentData);
                    }
                }
            }, tbb::simple_partitioner());
        }

//        for (auto &m2Object: renderPlan->m2Array.getDrawn()) {
//            if (m2Object != nullptr) {
//                m2Object->uploadGeneratorBuffers(renderPlan->renderingMatrices->lookAtMat,
//                                                 renderPlan->frameDependentData);
//            }
//        }
    }
    {
        ZoneScopedN("m2SkyboxBuffersUpdate");
        if (auto skyBoxView = renderPlan->viewsHolder.getSkybox()) {
            for (auto &m2Object: skyBoxView->m2List.getDrawn()) {
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
            if (processedADT.count(adtObjectRes->adtObject) == 0) {
                adtObjectRes->adtObject->uploadGeneratorBuffers(renderPlan->frameDependentData);
                processedADT.insert(adtObjectRes->adtObject);
            }
        }
    }
}

std::shared_ptr<M2Object> Map::getM2Object(std::string fileName, SMDoodadDef &doodadDef) {
    auto it = m_m2MapObjects[doodadDef.uniqueId];
    if (!it.expired()) {
        return it.lock();
    } else {
        auto m2Object = std::make_shared<M2Object>(m_api);
        m2Object->setLoadParams(0, {}, {});
        m2Object->setModelFileName(fileName);
        m2Object->createPlacementMatrix(doodadDef);
        m2Object->calcWorldPosition();

        m_m2MapObjects[doodadDef.uniqueId] = std::weak_ptr<M2Object>(m2Object);
        return m2Object;
    }
    return nullptr;
}


std::shared_ptr<M2Object> Map::getM2Object(int fileDataId, SMDoodadDef &doodadDef) {
    auto it = m_m2MapObjects[doodadDef.uniqueId];
    if (!it.expired()) {
        return it.lock();
    } else {
        auto m2Object = std::make_shared<M2Object>(m_api);
        m2Object->setLoadParams(0, {}, {});
        m2Object->setModelFileId(fileDataId);
        m2Object->createPlacementMatrix(doodadDef);
        m2Object->calcWorldPosition();

        m_m2MapObjects[doodadDef.uniqueId] = std::weak_ptr<M2Object>(m2Object);
        return m2Object;
    }
    return nullptr;
}
std::shared_ptr<WmoObject> Map::getWmoObject(std::string fileName, SMMapObjDef &mapObjDef) {
    auto it = m_wmoMapObjects[mapObjDef.uniqueId];
    if (!it.expired()) {
        return it.lock();
    } else {
        auto wmoObject = std::make_shared<WmoObject>(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileName(fileName);

        m_wmoMapObjects[mapObjDef.uniqueId] = std::weak_ptr<WmoObject>(wmoObject);
        return wmoObject;
    }
    return nullptr;
}

std::shared_ptr<WmoObject> Map::getWmoObject(int fileDataId, SMMapObjDef &mapObjDef) {
    auto it = m_wmoMapObjects[mapObjDef.uniqueId];
    if (!it.expired()) {
        return it.lock();
    } else {
        auto wmoObject = std::make_shared<WmoObject>(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileId(fileDataId);

        m_wmoMapObjects[mapObjDef.uniqueId] = std::weak_ptr<WmoObject>(wmoObject);
        return wmoObject;
    }
    return nullptr;
}

std::shared_ptr<WmoObject> Map::getWmoObject(std::string fileName, SMMapObjDefObj1 &mapObjDef) {
    auto it = m_wmoMapObjects[mapObjDef.uniqueId];
    if (!it.expired()) {
        return it.lock();
    } else {
        auto wmoObject = std::make_shared<WmoObject>(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileName(fileName);

        m_wmoMapObjects[mapObjDef.uniqueId] = std::weak_ptr<WmoObject>(wmoObject);
        return wmoObject;
    }
    return nullptr;
}

std::shared_ptr<WmoObject> Map::getWmoObject(int fileDataId, SMMapObjDefObj1 &mapObjDef) {
    auto it = m_wmoMapObjects[mapObjDef.uniqueId];
    if (!it.expired()) {
        return it.lock();
    } else {
        auto wmoObject = std::make_shared<WmoObject>(m_api);
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

void Map::loadZoneLights() {
    if (m_api->databaseHandler != nullptr) {
        std::vector<ZoneLight> zoneLights;
        m_api->databaseHandler->getZoneLightsForMap(m_mapId, zoneLights);

        for (const auto &zoneLight : zoneLights) {
            mapInnerZoneLightRecord innerZoneLightRecord;
            innerZoneLightRecord.ID = zoneLight.ID;
            innerZoneLightRecord.name = zoneLight.name;
            innerZoneLightRecord.LightID = zoneLight.LightID;
//            innerZoneLightRecord.Zmin = zoneLight.Zmin;
//            innerZoneLightRecord.Zmax = zoneLight.Zmax;

            float minX = 9999; float maxX = -9999;
            float minY = 9999; float maxY = -9999;

            auto &points = innerZoneLightRecord.points;
            for (auto &zonePoint : zoneLight.points) {
                minX = std::min<float>(zonePoint.x, minX); minY = std::min<float>(zonePoint.y, minY);
                maxX = std::max<float>(zonePoint.x, maxX); maxY = std::max<float>(zonePoint.y, maxY);

                points.push_back(mathfu::vec2(zonePoint.x, zonePoint.y));
            }

            innerZoneLightRecord.aabb = CAaBox(
                C3Vector(mathfu::vec3(minX, minY, zoneLight.Zmin)),
                C3Vector(mathfu::vec3(maxX, maxY, zoneLight.Zmax))
            );

            auto &lines = innerZoneLightRecord.lines;
            for (int i = 0; i < (points.size() - 1); i++) {
                lines.push_back(points[i + 1] - points[i]);
            }
            lines.push_back( points[0] - points[points.size() - 1]);

            m_zoneLights.push_back(innerZoneLightRecord);
        }

    }
}

