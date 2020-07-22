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
#include "../wowFrameData.h"
#include "../../algorithms/quick-sort-omp.h"
#include "../../../gapi/UniformBufferStructures.h"
#include "../../shader/ShaderDefinitions.h"

//#include "../../algorithms/quicksort-dualpivot.h"

static GBufferBinding fullScreen[1] = {
    {+drawQuad::Attribute::position, 2, GBindingType::GFLOAT, false, 0, 0},
};

static GBufferBinding skyConusBinding[1] = {
    {+drawQuad::Attribute::position, 4, GBindingType::GFLOAT, false, 0, 0},
};

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

HGMesh createSkyMesh(IDevice *device, Config *config) {
    auto skyIBO = device->createIndexBuffer();
    skyIBO->uploadData(
        skyConusIBO.data(),
        skyConusIBO.size() * sizeof(uint16_t));

    auto skyVBO = device->createVertexBuffer();
    skyVBO->uploadData(
        skyConusVBO.data(),
        skyConusVBO.size() * sizeof(mathfu::vec4_packed)
    );

    auto skyBindings = device->createVertexBufferBindings();
    skyBindings->setIndexBuffer(skyIBO);

    GVertexBufferBinding vertexBinding;
    vertexBinding.vertexBuffer = skyVBO;

    vertexBinding.bindings = std::vector<GBufferBinding>(&skyConusBinding[0], &skyConusBinding[1]);

    skyBindings->addVertexBufferBinding(vertexBinding);
    skyBindings->save();

    auto skyVs = device->createUniformBufferChunk(sizeof(DnSky::meshWideBlockVS));
    skyVs->setUpdateHandler([config](IUniformBufferChunk *self) -> void {
        auto &meshblockVS = self->getObject<DnSky::meshWideBlockVS>();
        meshblockVS.skyColor[0] = config->getSkyTopColor();
        meshblockVS.skyColor[1] = config->getSkyMiddleColor();
        meshblockVS.skyColor[2] = config->getSkyBand1Color();
        meshblockVS.skyColor[3] = config->getSkyBand2Color();
        meshblockVS.skyColor[4] = config->getSkySmogColor();
        meshblockVS.skyColor[5] = config->getSkyFogColor();


    });

    ///2. Create mesh
    auto shader = device->getShader("skyConus", nullptr);
    gMeshTemplate meshTemplate(skyBindings, shader);
    meshTemplate.meshType = MeshType::eGeneralMesh;
    meshTemplate.depthWrite = false;
    meshTemplate.depthCulling = true;
    meshTemplate.backFaceCulling = false;
    meshTemplate.skybox = true;
    meshTemplate.blendMode = EGxBlendEnum::GxBlend_Opaque;

    meshTemplate.texture.resize(0);

    meshTemplate.textureCount = 0;

    meshTemplate.ubo[0] = nullptr;
    meshTemplate.ubo[1] = nullptr;
    meshTemplate.ubo[2] = skyVs;

    meshTemplate.ubo[3] = nullptr;
    meshTemplate.ubo[4] = nullptr;

    meshTemplate.element = DrawElementMode::TRIANGLE_STRIP;
    meshTemplate.start = 0;
    meshTemplate.end = 300;

    //Make mesh
    HGMesh hmesh =  device->createMesh(meshTemplate);
    return hmesh;
}

void Map::checkCulling(HCullStage cullStage) {
//    std::cout << "Map::checkCulling finished called" << std::endl;
//    std::cout << "m_wdtfile->getIsLoaded() = " << m_wdtfile->getIsLoaded() << std::endl;
    if (m_wdtfile->getStatus() != FileStatus::FSLoaded) return;

    Config* config = this->m_api->getConfig();

    mathfu::vec4 cameraPos = cullStage->matricesForCulling->cameraPos;
    mathfu::vec3 cameraVec3 = cameraPos.xyz();
    mathfu::mat4 &frustumMat = cullStage->matricesForCulling->perspectiveMat;
    mathfu::mat4 &lookAtMat4 = cullStage->matricesForCulling->lookAtMat;

    size_t adtRenderedThisFramePrev = cullStage->adtArray.size();
    cullStage->adtArray = {};
    cullStage->adtArray.reserve(adtRenderedThisFramePrev);

    size_t m2RenderedThisFramePrev = cullStage->m2Array.size();
    cullStage->m2Array = {};
    cullStage->m2Array.reserve(m2RenderedThisFramePrev);

    size_t wmoRenderedThisFramePrev = cullStage->wmoArray.size();
    cullStage->wmoArray = {};
    cullStage->wmoArray.reserve(wmoRenderedThisFramePrev);


    mathfu::mat4 viewPerspectiveMat = frustumMat*lookAtMat4;

    mathfu::vec4 &camera4 = cameraPos;
    std::vector<mathfu::vec4> frustumPlanes = MathHelper::getFrustumClipsFromMatrix(viewPerspectiveMat);
    std::vector<mathfu::vec3> frustumPoints = MathHelper::calculateFrustumPointsFromMat(viewPerspectiveMat);
    std::vector<mathfu::vec3> hullines = MathHelper::getHullLines(frustumPoints);

    cullStage->exteriorView = ExteriorView();
    cullStage->interiorViews = std::vector<InteriorView>();
    m_viewRenderOrder = 0;

    cullStage->m_currentInteriorGroups = {};
    cullStage->m_currentWMO = nullptr;

    int bspNodeId = -1;
    int interiorGroupNum = -1;
    cullStage->m_currentWmoGroup = -1;

    //Get center of near plane


    //Get potential WMO
    std::vector<std::shared_ptr<WmoObject>> potentialWmo;
    std::vector<std::shared_ptr<M2Object>> potentialM2;

    if (!m_wdtfile->mphd->flags.wdt_uses_global_map_obj) {
        int adt_x = worldCoordinateToAdtIndex(camera4.y);
        int adt_y = worldCoordinateToAdtIndex(camera4.x);
        cullStage->adtAreadId = -1;
        std::shared_ptr<AdtObject> adtObjectCameraAt = this->mapTiles[adt_x][adt_y];
        if (adtObjectCameraAt != nullptr) {
            ADTObjRenderRes tempRes;
            tempRes.adtObject = adtObjectCameraAt;
            adtObjectCameraAt->checkReferences(
                tempRes,
                camera4,
                frustumPlanes,
                frustumPoints,
                lookAtMat4,
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

            cullStage->adtAreadId = adtObjectCameraAt->getAreaId(adt_global_x, adt_global_y);
        }
    } else {
        if (wmoMap == nullptr) {
            wmoMap = std::make_shared<WmoObject>(m_api);
            wmoMap->setLoadingParam(*m_wdtfile->wmoDef);
            wmoMap->setModelFileId(m_wdtfile->wmoDef->nameId);
        }

        potentialWmo.push_back(wmoMap);
    }


    for (auto &checkingWmoObj : potentialWmo) {
        WmoGroupResult groupResult;
        bool result = checkingWmoObj->getGroupWmoThatCameraIsInside(camera4, groupResult);

        if (result) {
            cullStage->m_currentWMO = checkingWmoObj;
            cullStage->m_currentWmoGroup = groupResult.groupIndex;
            if (checkingWmoObj->isGroupWmoInterior(groupResult.groupIndex)) {
                cullStage->m_currentInteriorGroups.push_back(groupResult);
                interiorGroupNum = groupResult.groupIndex;
                cullStage->currentWmoGroupIsExtLit = checkingWmoObj->isGroupWmoExteriorLit(groupResult.groupIndex);
                cullStage->currentWmoGroupShowExtSkybox = checkingWmoObj->isGroupWmoExtSkybox(groupResult.groupIndex);
            } else {
            }
            bspNodeId = groupResult.nodeId;
            break;
        }
    }

    //7. Get AreaId and Area Name
    StateForConditions stateForConditions;

    AreaRecord areaRecord;
    if (cullStage->m_currentWMO != nullptr) {
        auto nameId = cullStage->m_currentWMO->getNameSet();
        auto wmoId = cullStage->m_currentWMO->getWmoId();
        auto groupId = cullStage->m_currentWMO->getWmoGroupId(cullStage->m_currentWmoGroup);

        if (m_api->databaseHandler != nullptr) {
            areaRecord = m_api->databaseHandler->getWmoArea(wmoId, nameId, groupId);
        }
    }

    if (areaRecord.areaId == 0) {
        if (cullStage->adtAreadId > 0 && (m_api->databaseHandler != nullptr)) {
            areaRecord = m_api->databaseHandler->getArea(cullStage->adtAreadId);
        }
    }
    m_api->getConfig()->setAreaName(areaRecord.areaName);
    stateForConditions.currentAreaId = areaRecord.areaId;
    stateForConditions.currentParentAreaId = areaRecord.parentAreaId;

    ///-----------------------------------
    bool fogRecordWasFound = false;
    mathfu::vec3 ambientColor = mathfu::vec3(0.0,0.0,0.0);
    mathfu::vec3 directColor = mathfu::vec3(0.0,0.0,0.0);
    mathfu::vec3 endFogColor = mathfu::vec3(0.0,0.0,0.0);

    if (cullStage->m_currentWMO != nullptr) {
        CImVector sunFogColor;
        fogRecordWasFound = cullStage->m_currentWMO->checkFog(cameraVec3, sunFogColor);
        if (fogRecordWasFound) {
            endFogColor =
                mathfu::vec3((sunFogColor.r & 0xFF) / 255.0f,
                             ((sunFogColor.g ) & 0xFF) / 255.0f,
                             ((sunFogColor.b ) & 0xFF) / 255.0f);
        }
    }

    if ((m_api->databaseHandler != nullptr)) {
        //Check zoneLight
        LightResult zoneLightResult;
        std::vector<LightResult> lightResults;
        bool zoneLightFound = false;
        int LightId;
        for (const auto& zoneLight : m_zoneLights) {
            if (MathHelper::isPointInsideNonConvex(cameraVec3, zoneLight.aabb, zoneLight.points)) {
                zoneLightFound = true;
                LightId = zoneLight.LightID;
                break;
            }
        }

        if (zoneLightFound) {
            m_api->databaseHandler->getLightById(LightId, config->getCurrentTime(), zoneLightResult );
        }

        m_api->databaseHandler->getEnvInfo(m_mapId,
                                           cameraVec3.x,
                                           cameraVec3.y,
                                           cameraVec3.z,
                                           config->getCurrentTime(),
                                           lightResults
        );

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
        for (auto &_light : lightResults) {
            stateForConditions.currentSkyboxIds.push_back(_light.lightSkyboxId);
            std::shared_ptr<M2Object> skyBox = nullptr;
            if (perFdidMap[_light.skyBoxFdid] == nullptr) {
                skyBox = std::make_shared<M2Object>(m_api, true);
                skyBox->setLoadParams(0, {},{});

                skyBox->setModelFileId(_light.skyBoxFdid);

                skyBox->createPlacementMatrix(mathfu::vec3(0,0,0), 0, mathfu::vec3(1,1,1), nullptr);
                skyBox->calcWorldPosition();
                m_exteriorSkyBoxes.push_back(skyBox);
            } else {
                skyBox = perFdidMap[_light.skyBoxFdid];
            }

            skyBox->setAlpha(_light.blendCoef);
        }
        //Blend glow and ambient
        mathfu::vec3 ambientColor = {0,0,0};
        mathfu::vec3 horizontAmbientColor = {0,0,0};
        mathfu::vec3 groundAmbientColor = {0,0,0};
        mathfu::vec3 directColor = {0,0,0};
        mathfu::vec3 closeRiverColor = {0,0,0};

        mathfu::vec3 SkyTopColor= {0,0,0};
        mathfu::vec3 SkyMiddleColor= {0,0,0};
        mathfu::vec3 SkyBand1Color= {0,0,0};
        mathfu::vec3 SkyBand2Color= {0,0,0};
        mathfu::vec3 SkySmogColor= {0,0,0};
        mathfu::vec3 SkyFogColor= {0,0,0};

        m_currentGlow = 0;
        for (auto &_light : lightResults) {
            m_currentGlow += _light.glow * _light.blendCoef;
            ambientColor += mathfu::vec3(_light.ambientColor) * _light.blendCoef;
            horizontAmbientColor += mathfu::vec3(_light.horizontAmbientColor) * _light.blendCoef;
            groundAmbientColor += mathfu::vec3(_light.groundAmbientColor) * _light.blendCoef;

            directColor += mathfu::vec3(_light.directColor) * _light.blendCoef;
            closeRiverColor += mathfu::vec3(_light.closeRiverColor) * _light.blendCoef;

            SkyTopColor += mathfu::vec3(_light.SkyTopColor) * _light.blendCoef;
            SkyMiddleColor += mathfu::vec3(_light.SkyMiddleColor) * _light.blendCoef;
            SkyBand1Color += mathfu::vec3(_light.SkyBand1Color) * _light.blendCoef;
            SkyBand2Color += mathfu::vec3(_light.SkyBand2Color) * _light.blendCoef;
            SkySmogColor += mathfu::vec3(_light.SkySmogColor) * _light.blendCoef;
            SkyFogColor += mathfu::vec3(_light.SkyFogColor) * _light.blendCoef;
        }

        //Database is in BGRA
        float ambientMult = areaRecord.ambientMultiplier*2.0f + 1;
//        ambientColor *= ambientMult;
//        groundAmbientColor *= ambientMult;
//        horizontAmbientColor *= ambientMult;

        config->setExteriorAmbientColor(ambientColor[2], ambientColor[1], ambientColor[0], 0);
        config->setExteriorGroundAmbientColor(groundAmbientColor[2], groundAmbientColor[1], groundAmbientColor[0], 0);
        config->setExteriorHorizontAmbientColor(horizontAmbientColor[2], horizontAmbientColor[1], horizontAmbientColor[0], 0);
        config->setExteriorDirectColor(directColor[2], directColor[1], directColor[0], 0);
        config->setCloseRiverColor(closeRiverColor[2], closeRiverColor[1], closeRiverColor[0], 0);


        config->setSkyTopColor(SkyTopColor[2], SkyTopColor[1], SkyTopColor[0]);
        config->setSkyMiddleColor(SkyMiddleColor[2], SkyMiddleColor[1], SkyMiddleColor[0]);
        config->setSkyBand1Color(SkyBand1Color[2], SkyBand1Color[1], SkyBand1Color[0]);
        config->setSkyBand2Color(SkyBand2Color[2], SkyBand2Color[1], SkyBand2Color[0]);
        config->setSkySmogColor(SkySmogColor[2], SkySmogColor[1], SkySmogColor[0]);
        config->setSkyFogColor(SkyFogColor[2], SkyFogColor[1], SkyFogColor[0]);
    }

    config->setFogColor(
        endFogColor.x,
        endFogColor.y,
        endFogColor.z,
        1.0
    );
    ///-----------------------------------


    auto lcurrentWMO = cullStage->m_currentWMO;
    auto currentWmoGroup = cullStage->m_currentWmoGroup;

    if ((lcurrentWMO != nullptr) && (!cullStage->m_currentInteriorGroups.empty()) && (lcurrentWMO->isLoaded())) {
        lcurrentWMO->resetTraversedWmoGroups();
        if (lcurrentWMO->startTraversingWMOGroup(
            cameraPos,
            viewPerspectiveMat,
            cullStage->m_currentInteriorGroups[0].groupIndex,
            0,
            m_viewRenderOrder,
            true,
            cullStage->interiorViews,
            cullStage->exteriorView)) {

            cullStage->wmoArray.push_back(cullStage->m_currentWMO);
        }

        if (cullStage->exteriorView.viewCreated) {
            checkExterior(cameraPos, frustumPoints, hullines, lookAtMat4, viewPerspectiveMat, m_viewRenderOrder, cullStage);
        }
    } else {
        //Cull exterior
        cullStage->exteriorView.viewCreated = true;
        cullStage->exteriorView.frustumPlanes.push_back(frustumPlanes);
        checkExterior(cameraPos, frustumPoints, hullines, lookAtMat4, viewPerspectiveMat, m_viewRenderOrder, cullStage);
    }
    if (  (cullStage->exteriorView.viewCreated || cullStage->currentWmoGroupIsExtLit || cullStage->currentWmoGroupShowExtSkybox) && (!m_exteriorSkyBoxes.empty())) {
        cullStage->exteriorView.viewCreated = true;
        m_wdlObject->checkSkyScenes(
            stateForConditions,
            cullStage->exteriorView.drawnM2s,
            cameraPos,
            frustumPlanes,
            frustumPoints);

        for (auto model : m_exteriorSkyBoxes) {
            if (model != nullptr) {
                model->checkFrustumCulling(cameraPos,
                                           frustumPlanes,
                                           frustumPoints);
                cullStage->exteriorView.drawnM2s.push_back(model);
            }
        }
    }

    //Fill M2 objects for views from WmoGroups
    for (auto &view : cullStage->interiorViews) {
        view.addM2FromGroups(frustumMat, lookAtMat4, cameraPos);
    }
    cullStage->exteriorView.addM2FromGroups(frustumMat, lookAtMat4, cameraPos);
    for (auto &adtRes : cullStage->exteriorView.drawnADTs) {
        adtRes->adtObject->collectMeshes(*adtRes.get(),  cullStage->exteriorView.drawnChunks, cullStage->exteriorView.renderOrder);
    }

    //Collect M2s for update
    cullStage->m2Array.clear();
    auto inserter = std::back_inserter(cullStage->m2Array);
    for (auto &view : cullStage->interiorViews) {
        std::copy(view.drawnM2s.begin(), view.drawnM2s.end(), inserter);
    }
    std::copy(cullStage->exteriorView.drawnM2s.begin(), cullStage->exteriorView.drawnM2s.end(), inserter);

    //Sort and delete duplicates
    std::sort( cullStage->m2Array.begin(), cullStage->m2Array.end() );
    cullStage->m2Array.erase( unique( cullStage->m2Array.begin(), cullStage->m2Array.end() ), cullStage->m2Array.end() );
    cullStage->m2Array = std::vector<std::shared_ptr<M2Object>>(cullStage->m2Array.begin(), cullStage->m2Array.end());

    std::sort( cullStage->wmoArray.begin(), cullStage->wmoArray.end() );
    cullStage->wmoArray.erase( unique( cullStage->wmoArray.begin(), cullStage->wmoArray.end() ), cullStage->wmoArray.end() );
    cullStage->wmoArray = std::vector<std::shared_ptr<WmoObject>>(cullStage->wmoArray.begin(), cullStage->wmoArray.end());

    cullStage->adtArray = std::vector<std::shared_ptr<ADTObjRenderRes>>(cullStage->adtArray.begin(), cullStage->adtArray.end());

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

void Map::checkExterior(mathfu::vec4 &cameraPos,
                        std::vector<mathfu::vec3> &frustumPoints,
                        std::vector<mathfu::vec3> &hullLines,
                        mathfu::mat4 &lookAtMat4,
                        mathfu::mat4 &viewPerspectiveMat,
                        int viewRenderOrder,
                        HCullStage cullStage
) {
//    std::cout << "Map::checkExterior finished called" << std::endl;
    if (m_wdlObject == nullptr) {
        if (m_wdtfile->mphd->flags.wdt_has_maid) {
            m_wdlObject = std::make_shared<WdlObject>(m_api, m_wdtfile->mphd->wdlFileDataID);
            m_wdlObject->setMapApi(this);
        }

        return;
    }


    std::vector<std::shared_ptr<M2Object>> m2ObjectsCandidates;
    std::vector<std::shared_ptr<WmoObject>> wmoCandidates;

//    float adt_x = floor((32 - (cameraPos[1] / 533.33333)));
//    float adt_y = floor((32 - (cameraPos[0] / 533.33333)));

    //Get visible area that should be checked
    float minx = 99999, maxx = -99999;
    float miny = 99999, maxy = -99999;

    for (int i = 0; i < frustumPoints.size(); i++) {
        mathfu::vec3 &frustumPoint = frustumPoints[i];

        minx = std::min(frustumPoint.x, minx);
        maxx = std::max(frustumPoint.x, maxx);
        miny = std::min(frustumPoint.y, miny);
        maxy = std::max(frustumPoint.y, maxy);
    }
    int adt_x_min = std::max(worldCoordinateToAdtIndex(maxy), 0);
    int adt_x_max = std::min(worldCoordinateToAdtIndex(miny), 63);

    int adt_y_min = std::max(worldCoordinateToAdtIndex(maxx), 0);
    int adt_y_max = std::min(worldCoordinateToAdtIndex(minx), 63);

    int adt_global_x = worldCoordinateToGlobalAdtChunk(cameraPos.y);
    int adt_global_y = worldCoordinateToGlobalAdtChunk(cameraPos.x);

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

    m_wdlObject->checkFrustumCulling(
        cameraPos, cullStage->exteriorView.frustumPlanes[0],
        frustumPoints,
        hullLines,
        lookAtMat4, m2ObjectsCandidates, wmoCandidates);

    if (!m_wdtfile->mphd->flags.wdt_uses_global_map_obj) {
        for (int i = adt_x_min; i <= adt_x_max; i++) {
            for (int j = adt_y_min; j <= adt_y_max; j++) {
                if ((i < 0) || (i > 64)) continue;
                if ((j < 0) || (j > 64)) continue;

                auto adtObject = this->mapTiles[i][j];
                if (adtObject != nullptr) {

                    std::shared_ptr<ADTObjRenderRes> adtFrustRes = std::make_shared<ADTObjRenderRes>();
                    adtFrustRes->adtObject = adtObject;


                    bool result = adtObject->checkFrustumCulling(
                        *adtFrustRes.get(),
                        cameraPos,
                        adt_global_x,
                        adt_global_y,
                        cullStage->exteriorView.frustumPlanes[0], //TODO:!
                        frustumPoints,
                        hullLines,
                        lookAtMat4, m2ObjectsCandidates, wmoCandidates);
                    if (result) {
                        cullStage->exteriorView.drawnADTs.push_back(adtFrustRes);
                        cullStage->adtArray.push_back(adtFrustRes);
                    }
                } else if (!m_lockedMap && true) { //(m_wdtfile->mapTileTable->mainInfo[j][i].Flag_HasADT > 0) {
                    if (m_wdtfile->mphd->flags.wdt_has_maid) {
                        adtObject = std::make_shared<AdtObject>(m_api, i, j, m_wdtfile->mapFileDataIDs[j * 64 + i],
                                                                m_wdtfile);
                    } else {
                        std::string adtFileTemplate =
                            "world/maps/" + mapName + "/" + mapName + "_" + std::to_string(i) + "_" + std::to_string(j);
                        adtObject = std::make_shared<AdtObject>(m_api, adtFileTemplate, mapName, i, j, m_wdtfile);
                    }


                    adtObject->setMapApi(this);
                    this->mapTiles[i][j] = adtObject;
                }
            }
        }
    } else {
        wmoCandidates.push_back(wmoMap);
    }

    //Sort and delete duplicates
    std::sort( wmoCandidates.begin(), wmoCandidates.end() );
    wmoCandidates.erase( unique( wmoCandidates.begin(), wmoCandidates.end() ), wmoCandidates.end() );

    //Frustum cull
    for (auto it = wmoCandidates.begin(); it != wmoCandidates.end(); ++it) {
        auto wmoCandidate = *it;

        if (!wmoCandidate->isLoaded()) {
            cullStage->wmoArray.push_back(wmoCandidate);
            continue;
        }
        wmoCandidate->resetTraversedWmoGroups();
        if (wmoCandidate->startTraversingWMOGroup(
            cameraPos,
            viewPerspectiveMat,
            -1,
            0,
            viewRenderOrder,
            false,
            cullStage->interiorViews,
            cullStage->exteriorView)) {

            cullStage->wmoArray.push_back(wmoCandidate);
        }
    }

    //Sort and delete duplicates
    if (m2ObjectsCandidates.size() > 0)
    {
        auto *sortedArrayPtr = &m2ObjectsCandidates[0];
        std::vector<int> indexArray = std::vector<int>(m2ObjectsCandidates.size());
        for (int i = 0; i < indexArray.size(); i++) {
            indexArray[i] = i;
        }
        quickSort_parallel(
            indexArray.data(),
            indexArray.size(),
            m_api->getConfig()->getThreadCount(),
            m_api->getConfig()->getQuickSortCutoff(),
            [sortedArrayPtr](int indexA, int indexB) {
                auto *pA = sortedArrayPtr[indexA].get();
                auto *pB = sortedArrayPtr[indexB].get();

                return pA < pB;
            });

        if (m2ObjectsCandidates.size() > 1) {
            std::shared_ptr<M2Object> prevElem = nullptr;
            std::vector<std::shared_ptr<M2Object>> m2Unique;
            m2Unique.reserve(indexArray.size());
            for (int i = 0; i < indexArray.size(); i++) {
                if (prevElem != m2ObjectsCandidates[indexArray[i]]) {
                    m2Unique.push_back(m2ObjectsCandidates[indexArray[i]]);
                    prevElem = m2ObjectsCandidates[indexArray[i]];
                }
            }
            m2ObjectsCandidates = m2Unique;
        }
    }

//    std::sort( m2ObjectsCandidates.begin(), m2ObjectsCandidates.end() );
//    m2ObjectsCandidates.erase( unique( m2ObjectsCandidates.begin(), m2ObjectsCandidates.end() ), m2ObjectsCandidates.end() );

    //3.2 Iterate over all global WMOs and M2s (they have uniqueIds)
    {
        int numThreads = m_api->getConfig()->getThreadCount();

        #pragma omp parallel for num_threads(numThreads)
        for (size_t i = 0; i < m2ObjectsCandidates.size(); i++) {
            auto m2ObjectCandidate = m2ObjectsCandidates[i];
            bool frustumResult = m2ObjectCandidate->checkFrustumCulling(
                cameraPos,
                cullStage->exteriorView.frustumPlanes[0], //TODO:!
                frustumPoints);
        }
    }
//        bool frustumResult = true;
    {
        for (size_t i = 0; i < m2ObjectsCandidates.size(); i++) {
            auto m2ObjectCandidate = m2ObjectsCandidates[i];
            if (m2ObjectCandidate->m_cullResult) {
                cullStage->exteriorView.drawnM2s.push_back(m2ObjectCandidate);
                cullStage->m2Array.push_back(m2ObjectCandidate);
            }
        }
    }
}
void Map::doPostLoad(HCullStage cullStage) {
    int processedThisFrame = 0;
    int groupsProcessedThisFrame = 0;
//    if (m_api->getConfig()->getRenderM2()) {
    for (int i = 0; i < cullStage->m2Array.size(); i++) {
        auto m2Object = cullStage->m2Array[i];
        if (m2Object == nullptr) continue;
        m2Object->doPostLoad();
    }
//    }

    for (auto &wmoObject : cullStage->wmoArray) {
        if (wmoObject == nullptr) continue;
        wmoObject->doPostLoad(groupsProcessedThisFrame);
    }

    for (auto &adtObject : cullStage->adtArray) {
        adtObject->adtObject->doPostLoad();
    }

    if (quadBindings == nullptr)
    {
        std::array<mathfu::vec2_packed, 4> vertexBuffer = {
            mathfu::vec2_packed(mathfu::vec2(-1.0f + 0.001, -1.0f+ 0.001)),
            mathfu::vec2_packed(mathfu::vec2(-1.0f+ 0.001,  1.0f- 0.001)),
            mathfu::vec2_packed(mathfu::vec2(1.0f- 0.001,  -1.0f+ 0.001)),
            mathfu::vec2_packed(mathfu::vec2(1.0f- 0.001,  1.f- 0.001))
        };
        std::vector<uint16_t > indexBuffer = {
            0, 1, 2,
            2, 1, 3
        };

        std::cout << "indexBuffer.size = " << indexBuffer.size() << std::endl;

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

        vertexBinding.bindings = std::vector<GBufferBinding>(&fullScreen[0], &fullScreen[1]);

        quadBindings->addVertexBufferBinding(vertexBinding);
        quadBindings->save();
    }
    if (skyMesh == nullptr) {
        skyMesh = createSkyMesh(m_api->hDevice.get(), m_api->getConfig());
    }
};

mathfu::vec3 calcExteriorColorDir(HCameraMatrices cameraMatrices, int time) {
    // Phi Table
    static const std::array<std::array<float, 2>, 4> phiTable = {
        {
            { 0.0f,  2.2165682f },
            { 0.25f, 1.9198623f },
            { 0.5f,  2.2165682f },
            { 0.75f, 1.9198623f }
        }
    };

    // Theta Table


    static const std::array<std::array<float, 2>, 4> thetaTable = {
        {
            {0.0f, 3.926991f},
            {0.25f, 3.926991f},
            { 0.5f,  3.926991f },
            { 0.75f, 3.926991f }
        }
    };

//    float phi = DayNight::InterpTable(&DayNight::phiTable, 4u, DayNight::g_dnInfo.dayProgression);
//    float theta = DayNight::InterpTable(&DayNight::thetaTable, 4u, DayNight::g_dnInfo.dayProgression);

    float phi = phiTable[0][1];
    float theta = thetaTable[0][1];

    //Find Index
    float timeF = time / 2880.0f;
    int firstIndex = -1;
    for (int i = 0; i < 4; i++) {
        if (timeF < phiTable[i][0]) {
            firstIndex = i;
            break;
        }
    }
    if (firstIndex == -1) {
        firstIndex = 3;
    }
    {
        float alpha =  (phiTable[firstIndex][0] -  timeF) / (thetaTable[firstIndex][0] - thetaTable[firstIndex-1][0]);
        phi = phiTable[firstIndex][1]*(1.0 - alpha) + phiTable[firstIndex - 1][1]*alpha;
    }


    // Convert from spherical coordinates to XYZ
    // x = rho * sin(phi) * cos(theta)
    // y = rho * sin(phi) * sin(theta)
    // z = rho * cos(phi)

    float sinPhi = (float) sin(phi);
    float cosPhi = (float) cos(phi);

    float sinTheta = (float) sin(theta);
    float cosTheta = (float) cos(theta);

    mathfu::mat3 lookAtRotation = mathfu::mat4::ToRotationMatrix(cameraMatrices->lookAtMat);

    mathfu::vec4 sunDirWorld = mathfu::vec4(sinPhi * cosTheta, sinPhi * sinTheta, cosPhi, 0);
//    mathfu::vec4 sunDirWorld = mathfu::vec4(-0.30822, -0.30822, -0.89999998, 0);
    return (lookAtRotation * sunDirWorld.xyz());
}

void Map::update(HUpdateStage updateStage) {
    if (m_wdtfile->getStatus() != FileStatus::FSLoaded) return;

    mathfu::vec3 cameraVec3 = updateStage->cameraMatrices->cameraPos.xyz();
    mathfu::mat4 &frustumMat = updateStage->cameraMatrices->perspectiveMat;
    mathfu::mat4 &lookAtMat = updateStage->cameraMatrices->lookAtMat;
    animTime_t deltaTime = updateStage->delta;

    Config* config = this->m_api->getConfig();
//    if (config->getRenderM2()) {
//        std::for_each(frameData->m2Array.begin(), frameData->m2Array.end(), [deltaTime, &cameraVec3, &lookAtMat](M2Object *m2Object) {

//    #pragma
    int numThreads = m_api->getConfig()->getThreadCount();
    {
        #pragma omp parallel for num_threads(numThreads) schedule(dynamic, 4)
        for (int i = 0; i < updateStage->cullResult->m2Array.size(); i++) {
            auto m2Object = updateStage->cullResult->m2Array[i];
            if (m2Object != nullptr) {
                m2Object->update(deltaTime, cameraVec3, lookAtMat);
            }
        }
    }

//        });
//    }

    for (auto &wmoObject : updateStage->cullResult->wmoArray) {
        if (wmoObject == nullptr) continue;
        wmoObject->update();
    }

    for (auto &adtObjectRes : updateStage->cullResult->adtArray) {
        adtObjectRes->adtObject->update(deltaTime);
    }

    //2. Calc distance every 100 ms
//    #pragma omp parallel for
    for (int i = 0; i < updateStage->cullResult->m2Array.size(); i++) {
        auto m2Object = updateStage->cullResult->m2Array[i];
        if (m2Object == nullptr) continue;
        m2Object->calcDistance(cameraVec3);
    };




    //Calc exteriorDirectColorDir
    auto extDir = calcExteriorColorDir(
        updateStage->cameraMatrices,
        m_api->getConfig()->getCurrentTime()
    );
    m_api->getConfig()->setExteriorDirectColorDir(
        extDir.x, extDir.y, extDir.z
    );

    //8. Check fog color every 2 seconds
    if (this->m_currentTime + updateStage->delta - this->m_lastTimeLightCheck > 30) {


        this->m_lastTimeLightCheck = this->m_currentTime;
    }

    //Cleanup ADT every 10 seconds
    if (this->m_currentTime + updateStage->delta- this->m_lastTimeAdtCleanup > 10000) {
        for (int i = 0; i < 64; i++) {
            for (int j = 0; j < 64; j++) {
                auto adtObj = mapTiles[i][j];
                //Free obj, if it was unused for 10 secs
                if (adtObj != nullptr && ((this->m_currentTime - adtObj->getLastTimeOfUpdate()) > 10000)) {
                    mapTiles[i][j] = nullptr;
                }
            }
        }

        this->m_lastTimeAdtCleanup = this->m_currentTime;
    }
    this->m_currentTime += updateStage->delta;
}

void Map::updateBuffers(HCullStage cullStage) {
    for (int i = 0; i < cullStage->m2Array.size(); i++) {
        auto m2Object = cullStage->m2Array[i];
        if (m2Object != nullptr) {
            m2Object->uploadGeneratorBuffers(cullStage->matricesForCulling->lookAtMat);
        }
    }

    for (auto &wmoObject : cullStage->wmoArray) {
        if (wmoObject == nullptr) continue;
        wmoObject->uploadGeneratorBuffers();
    }

    for (auto &adtRes: cullStage->adtArray) {
        if (adtRes == nullptr) continue;
        adtRes->adtObject->uploadGeneratorBuffers(*adtRes.get());
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

void Map::produceDrawStage(HDrawStage resultDrawStage, HUpdateStage updateStage, std::vector<HGUniformBufferChunk> &additionalChunks) {
    auto cullStage = updateStage->cullResult;
    auto renderedThisFramePreSort = std::vector<HGMesh>();

    bool frameBufferSupported = true;

    if (quadBindings == nullptr)
        return;

    if (  cullStage->exteriorView.viewCreated || cullStage->currentWmoGroupIsExtLit) {
        renderedThisFramePreSort.push_back(skyMesh);
    }

    HDrawStage origResultDrawStage = resultDrawStage;
    if (frameBufferSupported) {
        //Create new drawstage and draw everything there
        resultDrawStage = std::make_shared<DrawStage>();

        resultDrawStage->drawStageDependencies = origResultDrawStage->drawStageDependencies;
        resultDrawStage->matricesForRendering = origResultDrawStage->matricesForRendering;
        resultDrawStage->setViewPort = origResultDrawStage->setViewPort;
        resultDrawStage->viewPortDimensions = origResultDrawStage->viewPortDimensions;;
        resultDrawStage->clearScreen = origResultDrawStage->clearScreen;;
        resultDrawStage->clearColor = origResultDrawStage->clearColor;;

        resultDrawStage->target = m_api->hDevice->createFrameBuffer(
            resultDrawStage->viewPortDimensions.maxs[0],
            resultDrawStage->viewPortDimensions.maxs[1],
            {ITextureFormat::itRGBA},
            ITextureFormat::itDepth32
        );
    }
    //Create scenewide uniform
    auto renderMats = resultDrawStage->matricesForRendering;

    auto config = m_api->getConfig();
    resultDrawStage->sceneWideBlockVSPSChunk = m_api->hDevice->createUniformBufferChunk(sizeof(sceneWideBlockVSPS));
    resultDrawStage->sceneWideBlockVSPSChunk->setUpdateHandler([renderMats, config](IUniformBufferChunk *chunk) -> void {
        auto *blockPSVS = &chunk->getObject<sceneWideBlockVSPS>();

        blockPSVS->uLookAtMat = renderMats->lookAtMat;
        blockPSVS->uPMatrix = renderMats->perspectiveMat;
        blockPSVS->uInteriorSunDir = renderMats->interiorDirectLightDir;
        blockPSVS->uViewUp = renderMats->viewUp;

        blockPSVS->extLight.uExteriorAmbientColor = config->getExteriorAmbientColor();
        blockPSVS->extLight.uExteriorHorizontAmbientColor = config->getExteriorHorizontAmbientColor();
        blockPSVS->extLight.uExteriorGroundAmbientColor = config->getExteriorGroundAmbientColor();
        blockPSVS->extLight.uExteriorDirectColor = config->getExteriorDirectColor();
        blockPSVS->extLight.uExteriorDirectColorDir = mathfu::vec4(config->getExteriorDirectColorDir(), 1.0);
    });


    additionalChunks.push_back(resultDrawStage->sceneWideBlockVSPSChunk);

    //Create meshes
    resultDrawStage->meshesToRender = std::make_shared<MeshesToRender>();


    // Put everything into one array and sort
    std::vector<GeneralView *> vector;
    for (auto & interiorView : updateStage->cullResult->interiorViews) {
        if (interiorView.viewCreated) {
            vector.push_back(&interiorView);
        }
    }
    if (updateStage->cullResult->exteriorView.viewCreated) {
        vector.push_back(&updateStage->cullResult->exteriorView);
    }

//    if (m_api->getConfig()->getRenderWMO()) {
    for (auto &view : vector) {
        view->collectMeshes(renderedThisFramePreSort);
    }
//    }

    std::vector<std::shared_ptr<M2Object>> m2ObjectsRendered;
    for (auto &view : vector) {
        std::copy(view->drawnM2s.begin(),view->drawnM2s.end(), std::back_inserter(m2ObjectsRendered));
    }

//    std::unordered_set<std::shared_ptr<M2Object>> s;
//    for (auto i : m2ObjectsRendered)
//        s.insert(i);
//    m2ObjectsRendered.assign( s.begin(), s.end() );

    std::sort( m2ObjectsRendered.begin(), m2ObjectsRendered.end() );
    m2ObjectsRendered.erase( unique( m2ObjectsRendered.begin(), m2ObjectsRendered.end() ), m2ObjectsRendered.end() );

//    if (m_api->getConfig()->getRenderM2()) {
    for (auto &m2Object : m2ObjectsRendered) {
        if (m2Object == nullptr) continue;
        m2Object->collectMeshes(renderedThisFramePreSort, m_viewRenderOrder);
        m2Object->drawParticles(renderedThisFramePreSort, m_viewRenderOrder);
    }
//    }

    //No need to sort array which has only one element
    if (renderedThisFramePreSort.size() > 1) {
        auto *sortedArrayPtr = &renderedThisFramePreSort[0];
        std::vector<int> indexArray = std::vector<int>(renderedThisFramePreSort.size());
        for (int i = 0; i < indexArray.size(); i++) {
            indexArray[i] = i;
        }

        auto *config = m_api->getConfig();

//        if (config->getMovementSpeed() > 2) {
        quickSort_parallel(
            indexArray.data(),
            indexArray.size(),
            config->getThreadCount(),
            config->getQuickSortCutoff(),
#include "../../../gapi/interface/sortLambda.h"
        );
//        } else {
//            quickSort_dualPoint(
//                indexArray.data(),
//                indexArray.size(),
//                config->getThreadCount(),
//                config->getQuickSortCutoff(),
//                #include "../../../gapi/interface/sortLambda.h"
//            );
//        }

        resultDrawStage->meshesToRender->meshes.reserve(indexArray.size());
        for (int i = 0; i < indexArray.size(); i++) {
            resultDrawStage->meshesToRender->meshes.push_back(renderedThisFramePreSort[indexArray[i]]);
        }
    } else {
        for (int i = 0; i < renderedThisFramePreSort.size(); i++) {
            resultDrawStage->meshesToRender->meshes.push_back(renderedThisFramePreSort[i]);
        }
    }

    if (frameBufferSupported) {
        doGaussBlur(resultDrawStage, origResultDrawStage);
    }
}

void Map::doGaussBlur(const HDrawStage &resultDrawStage, HDrawStage &origResultDrawStage) const {///2 Rounds of ffxgauss4 (Horizontal and Vertical blur)
    ///With two frameBuffers
    ///Size for buffers : is 4 times less than current canvas
    int targetWidth = resultDrawStage->viewPortDimensions.maxs[0] >> 2;
    int targetHeight = resultDrawStage->viewPortDimensions.maxs[1] >> 2;

    auto frameB1 = m_api->hDevice->createFrameBuffer(
        targetWidth,
        targetHeight,
        {ITextureFormat::itRGBA},
        ITextureFormat::itDepth32
    );
    auto frameB2 = m_api->hDevice->createFrameBuffer(
        targetWidth,
        targetHeight,
        {ITextureFormat::itRGBA},
        ITextureFormat::itDepth32
    );

    auto vertexChunk = m_api->hDevice->createUniformBufferChunk(sizeof(mathfu::vec4_packed));
    vertexChunk->setUpdateHandler([](IUniformBufferChunk *self) -> void {
        auto &meshblockVS = self->getObject<mathfu::vec4_packed>();
        meshblockVS.x = 1;
        meshblockVS.y = 1;
        meshblockVS.z = 0;
        meshblockVS.w = 0;
    });


    auto ffxGaussFrag = m_api->hDevice->createUniformBufferChunk(sizeof(FXGauss::meshWideBlockPS));
    ffxGaussFrag->setUpdateHandler([](IUniformBufferChunk *self) -> void {
        auto &meshblockVS = self->getObject<FXGauss::meshWideBlockPS>();
        static const float s_texOffsetX[4] = {-1, 0, 0, -1};
        static const float s_texOffsetY[4] = {2, 2, -1, -1};;

        for (int i = 0; i < 4; i++) {
            meshblockVS.texOffsetX[i] = s_texOffsetX[i];
            meshblockVS.texOffsetY[i] = s_texOffsetY[i];
        }
    });


    auto ffxGaussFrag2 = m_api->hDevice->createUniformBufferChunk(sizeof(FXGauss::meshWideBlockPS));
    ffxGaussFrag2->setUpdateHandler([](IUniformBufferChunk *self) -> void {
        auto &meshblockVS = self->getObject<FXGauss::meshWideBlockPS>();
        static const float s_texOffsetX[4] = {-6, -1, 1, 6};
        static const float s_texOffsetY[4] = {0, 0, 0, 0};;

        for (int i = 0; i < 4; i++) {
            meshblockVS.texOffsetX[i] = s_texOffsetX[i];
            meshblockVS.texOffsetY[i] = s_texOffsetY[i];
        }
    });
    auto ffxGaussFrag3 = m_api->hDevice->createUniformBufferChunk(sizeof(FXGauss::meshWideBlockPS));
    ffxGaussFrag3->setUpdateHandler([](IUniformBufferChunk *self) -> void {
        auto &meshblockVS = self->getObject<FXGauss::meshWideBlockPS>();
        static const float s_texOffsetX[4] = {0, 0, 0, 0};
        static const float s_texOffsetY[4] = {10, 2, -2, -10};;

        for (int i = 0; i < 4; i++) {
            meshblockVS.texOffsetX[i] = s_texOffsetX[i];
            meshblockVS.texOffsetY[i] = s_texOffsetY[i];
        }
    });
    HGUniformBufferChunk frags[3] = {ffxGaussFrag, ffxGaussFrag2, ffxGaussFrag3};

    HDrawStage prevStage = resultDrawStage;
    for (int i = 0; i < 3; i++) {
        ///1. Create draw stage
        HDrawStage drawStage = std::make_shared<DrawStage>();

        drawStage->drawStageDependencies = {prevStage};
        drawStage->matricesForRendering = nullptr;
        drawStage->setViewPort = true;
        drawStage->viewPortDimensions = {{0, 0},
                                         {resultDrawStage->viewPortDimensions.maxs[0] >> 2,
                                             resultDrawStage->viewPortDimensions.maxs[1] >> 2}};
        drawStage->clearScreen = false;
        drawStage->target = ((i & 1) > 0) ? frameB1 : frameB2;

        ///2. Create mesh
        auto shader = m_api->hDevice->getShader("fullScreen_ffxgauss4", nullptr);
        gMeshTemplate meshTemplate(quadBindings, shader);
        meshTemplate.meshType = MeshType::eGeneralMesh;
        meshTemplate.depthWrite = false;
        meshTemplate.depthCulling = false;
        meshTemplate.backFaceCulling = false;
        meshTemplate.blendMode = EGxBlendEnum::GxBlend_Opaque;

        meshTemplate.texture.resize(1);
        meshTemplate.texture[0] = prevStage->target->getAttachment(0);

        meshTemplate.textureCount = 1;

        meshTemplate.ubo[0] = nullptr;
        meshTemplate.ubo[1] = nullptr;
        meshTemplate.ubo[2] = vertexChunk;

        meshTemplate.ubo[3] = nullptr;
        meshTemplate.ubo[4] = frags[i];

        meshTemplate.element = DrawElementMode::TRIANGLES;
        meshTemplate.start = 0;
        meshTemplate.end = 6;

        //Make mesh
        HGMesh hmesh = m_api->hDevice->createMesh(meshTemplate);
        drawStage->meshesToRender = std::make_shared<MeshesToRender>();
        drawStage->meshesToRender->meshes.push_back(hmesh);

        ///3. Reassign previous frame
        prevStage = drawStage;
    }

    //And the final is ffxglow to screen
    {
        auto config = m_api->getConfig();
        auto glow = m_currentGlow;
        auto ffxGlowfragmentChunk = m_api->hDevice->createUniformBufferChunk(sizeof(mathfu::vec4_packed));
        ffxGlowfragmentChunk->setUpdateHandler([glow, config](IUniformBufferChunk *self) -> void {
            auto &meshblockVS = self->getObject<mathfu::vec4_packed>();
            meshblockVS.x = 1;
            meshblockVS.y = 1;
            meshblockVS.z = 0; //mix_coeficient
            meshblockVS.w = config->getUseGaussBlur() ? fminf(0.5f, glow) : 0; //glow multiplier
        });

        auto shader = m_api->hDevice->getShader("fullScreen_quad", nullptr);
        gMeshTemplate meshTemplate(quadBindings, shader);
        meshTemplate.meshType = MeshType::eGeneralMesh;
        meshTemplate.depthWrite = false;
        meshTemplate.depthCulling = false;
        meshTemplate.backFaceCulling = false;
        meshTemplate.blendMode = EGxBlendEnum::GxBlend_Opaque;

        meshTemplate.texture.resize(2);
        meshTemplate.texture[0] = resultDrawStage->target->getAttachment(0);
        meshTemplate.texture[1] = prevStage->target->getAttachment(0);

        meshTemplate.textureCount = 2;


        meshTemplate.ubo[0] = nullptr;
        meshTemplate.ubo[1] = nullptr;
        meshTemplate.ubo[2] = vertexChunk;

        meshTemplate.ubo[3] = nullptr;
        meshTemplate.ubo[4] = ffxGlowfragmentChunk;

        meshTemplate.element = DrawElementMode::TRIANGLES;
        meshTemplate.start = 0;
        meshTemplate.end = 6;

        //Make mesh
        HGMesh hmesh = m_api->hDevice->createMesh(meshTemplate);
        origResultDrawStage->drawStageDependencies = {prevStage};
        origResultDrawStage->meshesToRender = std::make_shared<MeshesToRender>();
        origResultDrawStage->meshesToRender->meshes.push_back(hmesh);
    }
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
                minX = std::min(zonePoint.x, minX); minY = std::min(zonePoint.y, minY);
                maxX = std::max(zonePoint.x, maxX); maxY = std::max(zonePoint.y, maxY);

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
