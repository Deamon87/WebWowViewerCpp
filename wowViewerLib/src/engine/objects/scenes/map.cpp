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
    skyVs->setUpdateHandler([config](IUniformBufferChunk *self, const HFrameDepedantData &frameDepedantData) -> void {
        auto &meshblockVS = self->getObject<DnSky::meshWideBlockVS>();
        meshblockVS.skyColor[0] = frameDepedantData->SkyTopColor;
        meshblockVS.skyColor[1] = frameDepedantData->SkyMiddleColor;
        meshblockVS.skyColor[2] = frameDepedantData->SkyBand1Color;
        meshblockVS.skyColor[3] = frameDepedantData->SkyBand2Color;
        meshblockVS.skyColor[4] = frameDepedantData->SkySmogColor;
        meshblockVS.skyColor[5] = frameDepedantData->SkyFogColor;
    });

    //TODO: Pass m_skyConeAlpha to fragment shader

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

void Map::checkCulling(HCullStage cullStage) {
//    std::cout << "Map::checkCulling finished called" << std::endl;
//    std::cout << "m_wdtfile->getIsLoaded() = " << m_wdtfile->getIsLoaded() << std::endl;
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

    getPotentialEntities(cameraPos, potentialM2, cullStage, lookAtMat4, camera4, frustumPlanes, frustumPoints,
                         potentialWmo);

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
    m_api->getConfig()->areaName = areaRecord.areaName;
    stateForConditions.currentAreaId = areaRecord.areaId;
    stateForConditions.currentParentAreaId = areaRecord.parentAreaId;

    updateLightAndSkyboxData(cullStage, cameraVec3, stateForConditions, areaRecord);

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
        if (m_wdlObject != nullptr) {
            m_wdlObject->checkSkyScenes(
                stateForConditions,
                cullStage->exteriorView.drawnM2s,
                cameraPos,
                frustumPlanes,
                frustumPoints);
        }

        if (config->renderSkyDom) {
            for (auto model : m_exteriorSkyBoxes) {
                if (model != nullptr) {
                    model->checkFrustumCulling(cameraPos,
                                               frustumPlanes,
                                               frustumPoints);
                    cullStage->exteriorView.drawnM2s.push_back(model);
                }
            }
        }
    }

    //Fill M2 objects for views from WmoGroups
    for (auto &view : cullStage->interiorViews) {
        view.addM2FromGroups(frustumMat, lookAtMat4, cameraPos);
    }
    cullStage->exteriorView.addM2FromGroups(frustumMat, lookAtMat4, cameraPos);
    for (auto &adtRes : cullStage->exteriorView.drawnADTs) {
        adtRes->adtObject->collectMeshes(*adtRes,  cullStage->exteriorView.m_opaqueMeshes, cullStage->exteriorView.m_transparentMeshes, cullStage->exteriorView.renderOrder);
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

void Map::updateLightAndSkyboxData(const HCullStage &cullStage, mathfu::vec3 &cameraVec3,
                                   StateForConditions &stateForConditions, const AreaRecord &areaRecord) {///-----------------------------------
    Config* config = this->m_api->getConfig();

    bool fogRecordWasFound = false;
    mathfu::vec3 endFogColor = mathfu::vec3(0.0, 0.0, 0.0);

    std::vector<LightResult> wmoFogs = {};
    if (cullStage->m_currentWMO != nullptr) {
        cullStage->m_currentWMO->checkFog(cameraVec3, wmoFogs);
    }
    std::vector<LightResult> lightResults;

    if ((m_api->databaseHandler != nullptr)) {
        //Check zoneLight
        LightResult zoneLightResult;
        bool zoneLightFound = false;
        int LightId;
        for (const auto &zoneLight : m_zoneLights) {
            if (MathHelper::isPointInsideNonConvex(cameraVec3, zoneLight.aabb, zoneLight.points)) {
                zoneLightFound = true;
                LightId = zoneLight.LightID;
                break;
            }
        }

        if (zoneLightFound) {
            m_api->databaseHandler->getLightById(LightId, config->currentTime, zoneLightResult);
        }

        m_api->databaseHandler->getEnvInfo(m_mapId,
                                           cameraVec3.x,
                                           cameraVec3.y,
                                           cameraVec3.z,
                                           config->currentTime,
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
            if ((_light.skyBoxFlags & 2) == 0) {
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
            auto fdd = cullStage->frameDepedantData;
            fdd->currentGlow = currentGlow;
        } else if (config->glowSource == EParameterSource::eConfig) {
            auto fdd = cullStage->frameDepedantData;
            fdd->currentGlow = config->currentGlow;
        }


        if (config->globalLighting == EParameterSource::eDatabase) {
            auto fdd = cullStage->frameDepedantData;

            fdd->exteriorAmbientColor = mathfu::vec4(ambientColor[2], ambientColor[1], ambientColor[0], 0);
            fdd->exteriorGroundAmbientColor = mathfu::vec4(groundAmbientColor[2], groundAmbientColor[1], groundAmbientColor[0],
                                                  0);
            fdd->exteriorHorizontAmbientColor = mathfu::vec4(horizontAmbientColor[2], horizontAmbientColor[1],
                                                    horizontAmbientColor[0], 0);
            fdd->exteriorDirectColor = mathfu::vec4(directColor[2], directColor[1], directColor[0], 0);
            auto extDir = calcExteriorColorDir(
                cullStage->matricesForCulling,
                m_api->getConfig()->currentTime
            );
            fdd->exteriorDirectColorDir = { extDir.x, extDir.y, extDir.z };
        } else if (config->globalLighting == EParameterSource::eConfig) {
            auto fdd = cullStage->frameDepedantData;

            fdd->exteriorAmbientColor = config->exteriorAmbientColor;
            fdd->exteriorGroundAmbientColor = config->exteriorGroundAmbientColor;
            fdd->exteriorHorizontAmbientColor = config->exteriorHorizontAmbientColor;
            fdd->exteriorDirectColor = config->exteriorDirectColor;
            auto extDir = calcExteriorColorDir(
                cullStage->matricesForCulling,
                m_api->getConfig()->currentTime
            );
            fdd->exteriorDirectColorDir = { extDir.x, extDir.y, extDir.z };
        }


        if (config->waterColorParams == EParameterSource::eDatabase)
        {
            auto fdd = cullStage->frameDepedantData;
            fdd->closeRiverColor = mathfu::vec4(closeRiverColor[2], closeRiverColor[1], closeRiverColor[0], 0);
            fdd->farRiverColor = mathfu::vec4(farRiverColor[2], farRiverColor[1], farRiverColor[0], 0);
            fdd->closeOceanColor = mathfu::vec4(closeOceanColor[2], closeOceanColor[1], closeOceanColor[0], 0);
            fdd->farOceanColor = mathfu::vec4(farOceanColor[2], farOceanColor[1], farOceanColor[0], 0);
        } else if (config->waterColorParams == EParameterSource::eConfig) {
            auto fdd = cullStage->frameDepedantData;
            fdd->closeRiverColor = config->closeRiverColor;
            fdd->farRiverColor = config->farRiverColor;
            fdd->closeOceanColor = config->closeOceanColor;
            fdd->farOceanColor = config->farOceanColor;
        }
        if (config->skyParams == EParameterSource::eDatabase) {
            auto fdd = cullStage->frameDepedantData;
            fdd->SkyTopColor =      mathfu::vec4(SkyTopColor[2], SkyTopColor[1], SkyTopColor[0], 0);
            fdd->SkyMiddleColor =   mathfu::vec4(SkyMiddleColor[2], SkyMiddleColor[1], SkyMiddleColor[0], 0);
            fdd->SkyBand1Color =    mathfu::vec4(SkyBand1Color[2], SkyBand1Color[1], SkyBand1Color[0], 0);
            fdd->SkyBand2Color =    mathfu::vec4(SkyBand2Color[2], SkyBand2Color[1], SkyBand2Color[0], 0);
            fdd->SkySmogColor =     mathfu::vec4(SkySmogColor[2], SkySmogColor[1], SkySmogColor[0], 0);
            fdd->SkyFogColor =      mathfu::vec4(SkyFogColor[2], SkyFogColor[1], SkyFogColor[0], 0);
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
        mathfu::vec3 FogColor = {0, 0, 0};
        mathfu::vec3 EndFogColor = {0, 0, 0};
        float EndFogColorDistance = 0;
        mathfu::vec3 SunFogColor = {0, 0, 0};
        float SunFogStrength = 0;
        mathfu::vec3 FogHeightColor = {0, 0, 0};
        mathfu::vec4 FogHeightCoefficients = {0, 0, 0, 0};

        std::vector<LightResult> combinedResults = {};
        std::sort(combinedResults.begin(), combinedResults.end(), [](const LightResult &a, const LightResult &b) -> bool {
            return a.blendCoef > b.blendCoef;
        });
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

                globalFog.FogColor = {config->FogColor.z, config->FogColor.y, config->FogColor.x};
                globalFog.EndFogColor = {config->EndFogColor.z, config->EndFogColor.y, config->EndFogColor.x};
                globalFog.SunFogColor = {config->SunFogColor.z, config->SunFogColor.y, config->SunFogColor.x};
                globalFog.FogHeightColor = {config->FogHeightColor.z, config->FogHeightColor.y, config->FogHeightColor.x};

                combinedResults.push_back(globalFog);
            }
        }
        //Rebalance blendCoefs
        if (totalSummator < 1.0f && totalSummator > 0.0f) {
            for (auto &_light : combinedResults) {
                _light.blendCoef = _light.blendCoef / totalSummator;
            }
        }

        for (auto &_light : combinedResults) {
            FogEnd += _light.FogEnd * _light.blendCoef;
            FogScaler += _light.FogScaler * _light.blendCoef;
            FogDensity += _light.FogDensity * _light.blendCoef;
            FogHeight += _light.FogHeight * _light.blendCoef;
            FogHeightScaler += _light.FogHeightScaler * _light.blendCoef;
            FogHeightDensity += _light.FogHeightScaler * _light.blendCoef;
            SunFogAngle += _light.SunFogAngle * _light.blendCoef;
            FogColor += mathfu::vec3(_light.FogColor.data()) * _light.blendCoef;
            EndFogColor += mathfu::vec3(_light.EndFogColor.data()) * _light.blendCoef;
            EndFogColorDistance += _light.EndFogColorDistance * _light.blendCoef;
            SunFogColor += mathfu::vec3(_light.SunFogColor.data()) * _light.blendCoef;
            SunFogStrength += _light.SunFogStrength * _light.blendCoef;
            FogHeightColor += mathfu::vec3(_light.FogHeightColor.data()) * _light.blendCoef;
            FogHeightCoefficients += mathfu::vec4(_light.FogHeightCoefficients) * _light.blendCoef;
        }

        //In case of no data -> disable the fog
        {
            auto fdd = cullStage->frameDepedantData;
            fdd->FogDataFound = combinedResults.size() > 0;
            fdd->FogEnd = FogEnd;
            fdd->FogScaler = FogScaler;
            fdd->FogDensity = FogDensity;
            fdd->FogHeight = FogHeight;
            fdd->FogHeightScaler = FogHeightScaler;
            fdd->FogHeightDensity = FogHeightDensity;
            fdd->SunFogAngle = SunFogAngle;
            fdd->FogColor = mathfu::vec3(FogColor[2], FogColor[1], FogColor[0]);
            fdd->EndFogColor = mathfu::vec3(EndFogColor[2], EndFogColor[1], EndFogColor[0]);
            fdd->EndFogColorDistance = EndFogColorDistance;
            fdd->SunFogColor = mathfu::vec3(SunFogColor[2], SunFogColor[1], SunFogColor[0]);
            fdd->SunFogStrength = SunFogStrength;
            fdd->FogHeightColor = mathfu::vec3(FogHeightColor[2], FogHeightColor[1], FogHeightColor[0]);
            fdd->FogHeightCoefficients = mathfu::vec4(FogHeightCoefficients[0], FogHeightCoefficients[1],
                                             FogHeightCoefficients[2], FogHeightCoefficients[3]);
        }
    }

//    this->m_api->getConfig()->setClearColor(0,0,0,0);
}

void Map::getPotentialEntities(const mathfu::vec4 &cameraPos, std::vector<std::shared_ptr<M2Object>> &potentialM2,
                               HCullStage &cullStage, mathfu::mat4 &lookAtMat4, mathfu::vec4 &camera4,
                               std::vector<mathfu::vec4> &frustumPlanes, std::vector<mathfu::vec3> &frustumPoints,
                               std::vector<std::shared_ptr<WmoObject>> &potentialWmo) {
    if (m_wdtfile->getStatus() == FileStatus::FSLoaded) {
        if (!m_wdtfile->mphd->flags.wdt_uses_global_map_obj) {
            int adt_x = worldCoordinateToAdtIndex(camera4.y);
            int adt_y = worldCoordinateToAdtIndex(camera4.x);
            if ((adt_x >= 64) || (adt_x < 0)) return;
            if ((adt_y >= 64) || (adt_y < 0)) return;

            cullStage->adtAreadId = -1;
            auto &adtObjectCameraAt = mapTiles[adt_x][adt_y];

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
    }
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

    if (m_wdlObject == nullptr && m_wdtfile != nullptr && m_wdtfile->getStatus() == FileStatus::FSLoaded) {
        if (m_wdtfile->mphd->flags.wdt_has_maid) {
            m_wdlObject = std::make_shared<WdlObject>(m_api, m_wdtfile->mphd->wdlFileDataID);
            m_wdlObject->setMapApi(this);
        }
    }

    std::vector<std::shared_ptr<M2Object>> m2ObjectsCandidates;
    std::vector<std::shared_ptr<WmoObject>> wmoCandidates;

    if (m_wdlObject != nullptr) {
        m_wdlObject->checkFrustumCulling(
            cameraPos, cullStage->exteriorView.frustumPlanes[0],
            frustumPoints,
            hullLines,
            lookAtMat4, m2ObjectsCandidates, wmoCandidates);
    }

    getCandidatesEntities(hullLines, lookAtMat4, cameraPos, frustumPoints, cullStage, m2ObjectsCandidates,
                          wmoCandidates);

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
            m_api->getConfig()->threadCount,
            m_api->getConfig()->quickSortCutoff,
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
        int numThreads = m_api->getConfig()->threadCount;

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

void Map::getCandidatesEntities(std::vector<mathfu::vec3> &hullLines, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos,
                                std::vector<mathfu::vec3> &frustumPoints, HCullStage &cullStage,
                                std::vector<std::shared_ptr<M2Object>> &m2ObjectsCandidates,
                                std::vector<std::shared_ptr<WmoObject>> &wmoCandidates)  {
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

        if (!m_wdtfile->mphd->flags.wdt_uses_global_map_obj) {
            for (int i = adt_x_min; i <= adt_x_max; i++) {
                for (int j = adt_y_min; j <= adt_y_max; j++) {
                    if ((i < 0) || (i > 64)) continue;
                    if ((j < 0) || (j > 64)) continue;

                    auto adtObject = mapTiles[i][j];
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
                            auto &mapFileIds = m_wdtfile->mapFileDataIDs[j * 64 + i];
                            if (mapFileIds.rootADT > 0) {
                                adtObject = std::make_shared<AdtObject>(m_api, i, j,
                                                                        m_wdtfile->mapFileDataIDs[j * 64 + i],
                                                                        m_wdtfile);
                            } else {
                                continue;
                            }
                        } else {
                            std::string adtFileTemplate =
                                "world/maps/" + mapName + "/" + mapName + "_" + std::to_string(i) + "_" +
                                std::to_string(j);
                            adtObject = std::make_shared<AdtObject>(m_api, adtFileTemplate, mapName, i, j, m_wdtfile);
                        }


                        adtObject->setMapApi(this);
                        mapTiles[i][j] = adtObject;
                    }
                }
            }
        } else {
            wmoCandidates.push_back(wmoMap);
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

//        std::cout << "indexBuffer.size = " << indexBuffer.size() << std::endl;

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



void Map::update(HUpdateStage updateStage) {
    mathfu::vec3 cameraVec3 = updateStage->cameraMatrices->cameraPos.xyz();
    mathfu::mat4 &frustumMat = updateStage->cameraMatrices->perspectiveMat;
    mathfu::mat4 &lookAtMat = updateStage->cameraMatrices->lookAtMat;
    animTime_t deltaTime = updateStage->delta;

    Config* config = this->m_api->getConfig();
//    if (config->getRenderM2()) {
//        std::for_each(frameData->m2Array.begin(), frameData->m2Array.end(), [deltaTime, &cameraVec3, &lookAtMat](M2Object *m2Object) {

//    #pragma
    int numThreads = m_api->getConfig()->threadCount;
    {
        #pragma omp parallel for num_threads(numThreads) schedule(dynamic, 4)
        for (const auto& m2Object : updateStage->cullResult->m2Array) {
            if (m2Object != nullptr) {
                m2Object->update(deltaTime, cameraVec3, lookAtMat);
            }
        }
    }

//        });
//    }

    for (const auto &wmoObject : updateStage->cullResult->wmoArray) {
        if (wmoObject == nullptr) continue;
        wmoObject->update();
    }

    for (const auto &adtObjectRes : updateStage->cullResult->adtArray) {
        adtObjectRes->adtObject->update(deltaTime);
    }

    //2. Calc distance every 100 ms
//    #pragma omp parallel for
    for (const auto& m2Object : updateStage->cullResult->m2Array) {
        if (m2Object == nullptr) continue;
        m2Object->calcDistance(cameraVec3);
    };

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

    //Collect meshes
}

void Map::updateBuffers(HUpdateStage updateStage) {
    auto cullStage = updateStage->cullResult;

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
void Map::produceUpdateStage(HUpdateStage updateStage) {
    this->update(updateStage);

    //Create meshes
    updateStage->opaqueMeshes = std::make_shared<MeshesToRender>();
    updateStage->transparentMeshes = std::make_shared<MeshesToRender>();

    auto &opaqueMeshes = updateStage->opaqueMeshes->meshes;
    auto transparentMeshes = std::vector<HGMesh>();

    auto cullStage = updateStage->cullResult;

    if ( !m_suppressDrawingSky && (m_skyConeAlpha > 0) && (cullStage->exteriorView.viewCreated || cullStage->currentWmoGroupIsExtLit)) {
        if (skyMesh != nullptr)
            opaqueMeshes.push_back(skyMesh);
    }



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
        view->collectMeshes(opaqueMeshes, transparentMeshes);
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
        m2Object->collectMeshes(opaqueMeshes, transparentMeshes, m_viewRenderOrder);
        m2Object->drawParticles(opaqueMeshes, transparentMeshes, m_viewRenderOrder);
    }
//    }

    //No need to sort array which has only one element
    if (transparentMeshes.size() > 1) {
        auto *sortedArrayPtr = &transparentMeshes[0];
        std::vector<int> indexArray = std::vector<int>(transparentMeshes.size());
        for (int i = 0; i < indexArray.size(); i++) {
            indexArray[i] = i;
        }

        auto *config = m_api->getConfig();

//        if (config->getMovementSpeed() > 2) {
        quickSort_parallel(
            indexArray.data(),
            indexArray.size(),
            config->threadCount,
            config->quickSortCutoff,
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

        auto &targetTranspMeshes = updateStage->transparentMeshes->meshes;
        targetTranspMeshes.reserve(indexArray.size());
        for (int i = 0; i < indexArray.size(); i++) {
            targetTranspMeshes.push_back(transparentMeshes[indexArray[i]]);
        }
    } else {
        auto &targetTranspMeshes = updateStage->transparentMeshes->meshes;
        for (int i = 0; i < transparentMeshes.size(); i++) {
            targetTranspMeshes.push_back(transparentMeshes[i]);
        }
    }

//    opaqueMeshes.erase(
//        std::remove_if(
//            opaqueMeshes.begin(),
//            opaqueMeshes.end(),
//            [](const HGMesh& item) { return item == nullptr;}),
//        opaqueMeshes.end());
//
//    updateStage->transparentMeshes->meshes.erase(
//        std::remove_if(
//            updateStage->transparentMeshes->meshes.begin(),
//            updateStage->transparentMeshes->meshes.end(),
//            [](const HGMesh& item) { return item == nullptr;}),
//        updateStage->transparentMeshes->meshes.end());

    //1. Collect buffers
    std::vector<HGUniformBufferChunk> &bufferChunks = updateStage->uniformBufferChunks;
    int renderIndex = 0;
    for (const auto &mesh : opaqueMeshes) {
        for (int i = 0; i < 5; i++ ) {
            auto bufferChunk = mesh->getUniformBuffer(i);

            if (bufferChunk != nullptr) {
                bufferChunks.push_back(bufferChunk);
            }
        }
    }
    for (const auto &mesh : updateStage->transparentMeshes->meshes) {
        for (int i = 0; i < 5; i++ ) {
            auto bufferChunk = mesh->getUniformBuffer(i);

            if (bufferChunk != nullptr) {
                bufferChunks.push_back(bufferChunk);
            }
        }
    }

    std::sort( bufferChunks.begin(), bufferChunks.end());
    bufferChunks.erase( unique( bufferChunks.begin(), bufferChunks.end() ), bufferChunks.end() );
}
void Map::produceDrawStage(HDrawStage resultDrawStage, HUpdateStage updateStage, std::vector<HGUniformBufferChunk> &additionalChunks) {
    auto cullStage = updateStage->cullResult;

    //Create scenewide uniform
    resultDrawStage->frameDepedantData = updateStage->cullResult->frameDepedantData;
    auto renderMats = resultDrawStage->matricesForRendering;

    resultDrawStage->opaqueMeshes = updateStage->opaqueMeshes;
    resultDrawStage->transparentMeshes = updateStage->transparentMeshes;

    HDrawStage origResultDrawStage = resultDrawStage;
    bool frameBufferSupported = m_api->hDevice->getIsRenderbufferSupported();
    frameBufferSupported = false;
    if (frameBufferSupported) {
        //Create new drawstage and draw everything there
        resultDrawStage = std::make_shared<DrawStage>();

        *resultDrawStage = *origResultDrawStage;

        origResultDrawStage->opaqueMeshes = nullptr;
        origResultDrawStage->transparentMeshes = nullptr;

        resultDrawStage->target = m_api->hDevice->createFrameBuffer(
            resultDrawStage->viewPortDimensions.maxs[0],
            resultDrawStage->viewPortDimensions.maxs[1],
            {ITextureFormat::itRGBA},
            ITextureFormat::itDepth32,
            2
        );
    }

    auto config = m_api->getConfig();
    resultDrawStage->sceneWideBlockVSPSChunk = m_api->hDevice->createUniformBufferChunk(sizeof(sceneWideBlockVSPS));
    resultDrawStage->sceneWideBlockVSPSChunk->setUpdateHandler([renderMats, config](IUniformBufferChunk *chunk, const HFrameDepedantData &fdd) -> void {
        auto *blockPSVS = &chunk->getObject<sceneWideBlockVSPS>();

        blockPSVS->uLookAtMat = renderMats->lookAtMat;
        blockPSVS->uPMatrix = renderMats->perspectiveMat;
        blockPSVS->uInteriorSunDir = renderMats->interiorDirectLightDir;
        blockPSVS->uViewUp = renderMats->viewUp;

        blockPSVS->extLight.uExteriorAmbientColor = fdd->exteriorAmbientColor;
        blockPSVS->extLight.uExteriorHorizontAmbientColor = fdd->exteriorHorizontAmbientColor;
        blockPSVS->extLight.uExteriorGroundAmbientColor = fdd->exteriorGroundAmbientColor;
        blockPSVS->extLight.uExteriorDirectColor = fdd->exteriorDirectColor;
        blockPSVS->extLight.uExteriorDirectColorDir = mathfu::vec4(fdd->exteriorDirectColorDir, 1.0);

//        float fogEnd = std::min(config->getFarPlane(), config->getFogEnd());
        float fogEnd = config->farPlane;
        if (config->disableFog || !config->fogDataFound) {
            fogEnd = 100000000.0f;
            fdd->FogScaler = 0;
            fdd->FogDensity = 0;
        }

        float fogStart = std::max<float>(config->farPlane - 250, 0);
        fogStart = std::max<float>(fogEnd - fdd->FogScaler * (fogEnd - fogStart), 0);


        blockPSVS->fogData.densityParams = mathfu::vec4(
            fogStart,
            fogEnd ,
            fdd->FogDensity / 1000,
            0);
        blockPSVS->fogData.heightPlane = mathfu::vec4(0,0,0,0);
        blockPSVS->fogData.color_and_heightRate = mathfu::vec4(fdd->FogColor,fdd->FogHeightScaler);
        blockPSVS->fogData.heightDensity_and_endColor = mathfu::vec4(
            fdd->FogHeightDensity,
            fdd->EndFogColor.x,
            fdd->EndFogColor.y,
            fdd->EndFogColor.z
        );
        blockPSVS->fogData.sunAngle_and_sunColor = mathfu::vec4(
            fdd->SunFogAngle,
            fdd->SunFogColor.x * fdd->SunFogStrength,
            fdd->SunFogColor.y * fdd->SunFogStrength,
            fdd->SunFogColor.z * fdd->SunFogStrength
        );
        blockPSVS->fogData.heightColor_and_endFogDistance = mathfu::vec4(
            fdd->FogHeightColor,
            (fdd->EndFogColorDistance > 0) ?
            fdd->EndFogColorDistance :
            1000.0f
        );
        blockPSVS->fogData.sunPercentage = mathfu::vec4(
            (fdd->SunFogColor.Length() > 0) ? 0.5f : 0.0f
            , 0, 0, 0);

    });

    updateStage->uniformBufferChunks.push_back(resultDrawStage->sceneWideBlockVSPSChunk);

    if (frameBufferSupported ) {
        doGaussBlur(resultDrawStage, origResultDrawStage, updateStage);
    }
}

void Map::doGaussBlur(const HDrawStage &resultDrawStage, HDrawStage &origResultDrawStage, HUpdateStage &updateStage) const {
    if (quadBindings == nullptr)
        return;

    ///2 Rounds of ffxgauss4 (Horizontal and Vertical blur)
    ///With two frameBuffers
    ///Size for buffers : is 4 times less than current canvas
    int targetWidth = resultDrawStage->viewPortDimensions.maxs[0] >> 2;
    int targetHeight = resultDrawStage->viewPortDimensions.maxs[1] >> 2;

    auto frameB1 = m_api->hDevice->createFrameBuffer(
        targetWidth,
        targetHeight,
        {ITextureFormat::itRGBA},
        ITextureFormat::itDepth32,
        2
    );
    auto frameB2 = m_api->hDevice->createFrameBuffer(
        targetWidth,
        targetHeight,
        {ITextureFormat::itRGBA},
        ITextureFormat::itDepth32,
        2
    );

    auto vertexChunk = m_api->hDevice->createUniformBufferChunk(sizeof(mathfu::vec4_packed));
    updateStage->uniformBufferChunks.push_back(vertexChunk);
    vertexChunk->setUpdateHandler([](IUniformBufferChunk *self, const HFrameDepedantData &frameDepedantData) -> void {
        auto &meshblockVS = self->getObject<mathfu::vec4_packed>();
        meshblockVS.x = 1;
        meshblockVS.y = 1;
        meshblockVS.z = 0;
        meshblockVS.w = 0;
    });


    auto ffxGaussFrag = m_api->hDevice->createUniformBufferChunk(sizeof(FXGauss::meshWideBlockPS));
    updateStage->uniformBufferChunks.push_back(ffxGaussFrag);
    ffxGaussFrag->setUpdateHandler([](IUniformBufferChunk *self, const HFrameDepedantData &frameDepedantData) -> void {
        auto &meshblockVS = self->getObject<FXGauss::meshWideBlockPS>();
        static const float s_texOffsetX[4] = {-1, 0, 0, -1};
        static const float s_texOffsetY[4] = {2, 2, -1, -1};;

        for (int i = 0; i < 4; i++) {
            meshblockVS.texOffsetX[i] = s_texOffsetX[i];
            meshblockVS.texOffsetY[i] = s_texOffsetY[i];
        }
    });


    auto ffxGaussFrag2 = m_api->hDevice->createUniformBufferChunk(sizeof(FXGauss::meshWideBlockPS));
    updateStage->uniformBufferChunks.push_back(ffxGaussFrag2);
    ffxGaussFrag2->setUpdateHandler([](IUniformBufferChunk *self, const HFrameDepedantData &frameDepedantData) -> void {
        auto &meshblockVS = self->getObject<FXGauss::meshWideBlockPS>();
        static const float s_texOffsetX[4] = {-6, -1, 1, 6};
        static const float s_texOffsetY[4] = {0, 0, 0, 0};;

        for (int i = 0; i < 4; i++) {
            meshblockVS.texOffsetX[i] = s_texOffsetX[i];
            meshblockVS.texOffsetY[i] = s_texOffsetY[i];
        }
    });
    auto ffxGaussFrag3 = m_api->hDevice->createUniformBufferChunk(sizeof(FXGauss::meshWideBlockPS));
    updateStage->uniformBufferChunks.push_back(ffxGaussFrag3);
    ffxGaussFrag3->setUpdateHandler([](IUniformBufferChunk *self, const HFrameDepedantData &frameDepedantData) -> void {
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
        drawStage->opaqueMeshes = std::make_shared<MeshesToRender>();
        drawStage->opaqueMeshes->meshes.push_back(hmesh);

        ///3. Reassign previous frame
        prevStage = drawStage;
    }

    //And the final is ffxglow to screen
    {
        auto config = m_api->getConfig();;

        auto glow = origResultDrawStage->frameDepedantData->currentGlow;
        auto ffxGlowfragmentChunk = m_api->hDevice->createUniformBufferChunk(sizeof(mathfu::vec4_packed));
        updateStage->uniformBufferChunks.push_back(ffxGlowfragmentChunk);
        ffxGlowfragmentChunk->setUpdateHandler([glow, config](IUniformBufferChunk *self, const HFrameDepedantData &frameDepedantData) -> void {
            auto &meshblockVS = self->getObject<mathfu::vec4_packed>();
            meshblockVS.x = 1;
            meshblockVS.y = 1;
            meshblockVS.z = 0; //mix_coeficient
            meshblockVS.w = glow; //glow multiplier
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
        origResultDrawStage->opaqueMeshes = std::make_shared<MeshesToRender>();
        origResultDrawStage->opaqueMeshes->meshes.push_back(hmesh);
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
