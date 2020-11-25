//
// Created by Deamon on 2/2/2020.
//

#ifndef AWEBWOWVIEWERCPP_DRAWSTAGE_H
#define AWEBWOWVIEWERCPP_DRAWSTAGE_H

struct DrawStage;
struct CameraMatrices;
struct ViewPortDimensions;

typedef std::shared_ptr<DrawStage> HDrawStage;

#include <memory>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <mathfu/glsl_mappings.h>
#include "../gapi/interface/IDevice.h"
#include "CameraMatrices.h"


struct ViewPortDimensions{
    std::array<int, 2> mins;
    std::array<int, 2> maxs;
};

struct MeshesToRender {std::vector<HGMesh> meshes;} ;
typedef std::shared_ptr<MeshesToRender> HMeshesToRender;

struct FrameDepedantData {
    //Glow
    float currentGlow;

    //Ambient
    mathfu::vec4 exteriorAmbientColor = {1, 1, 1, 1};
    mathfu::vec4 exteriorHorizontAmbientColor = {1, 1, 1, 1};
    mathfu::vec4 exteriorGroundAmbientColor = {1, 1, 1, 1};
    mathfu::vec4 exteriorDirectColor = {0.3,0.3,0.3, 0.3};
    mathfu::vec3 exteriorDirectColorDir;

    mathfu::vec4 interiorAmbientColor;
    mathfu::vec4 interiorSunColor;
    mathfu::vec3 interiorSunDir;

//Sky params
    mathfu::vec4 SkyTopColor;
    mathfu::vec4 SkyMiddleColor;
    mathfu::vec4 SkyBand1Color;
    mathfu::vec4 SkyBand2Color;
    mathfu::vec4 SkySmogColor;
    mathfu::vec4 SkyFogColor;

//Fog params
    bool FogDataFound = false;

    float FogEnd = 0;
    float FogScaler = 0;
    float FogDensity = 0;
    float FogHeight = 0;
    float FogHeightScaler = 0;
    float FogHeightDensity = 0;
    float SunFogAngle = 0;
    mathfu::vec3 FogColor = mathfu::vec3(0,0,0);
    mathfu::vec3 EndFogColor = mathfu::vec3(0,0,0);
    float EndFogColorDistance = 0;
    mathfu::vec3 SunFogColor = mathfu::vec3(0,0,0);
    float SunFogStrength = 0;
    mathfu::vec3 FogHeightColor = mathfu::vec3(0,0,0);
    mathfu::vec4 FogHeightCoefficients = mathfu::vec4(0,0,0,0);

//Water params
    bool useMinimapWaterColor;
    bool useCloseRiverColorForDB;
    mathfu::vec4 closeRiverColor = mathfu::vec4(0,0,0,0);
    mathfu::vec4 farRiverColor = mathfu::vec4(0,0,0,0);

    mathfu::vec4 closeOceanColor = mathfu::vec4(0,0,0,0);
    mathfu::vec4 farOceanColor = mathfu::vec4(0,0,0,0);
};
typedef std::shared_ptr<FrameDepedantData> HFrameDepedantData;

struct DrawStage {
    HCameraMatrices matricesForRendering;
    HGUniformBufferChunk sceneWideBlockVSPSChunk;

    HFrameDepedantData frameDepedantData;
    HMeshesToRender opaqueMeshes;
    HMeshesToRender transparentMeshes;
    std::vector<HDrawStage> drawStageDependencies;

    bool invertedZ = false;

    bool setViewPort = false;
    ViewPortDimensions viewPortDimensions;

    bool clearScreen = false;
    mathfu::vec4 clearColor;

    HFrameBuffer target;
};
typedef std::shared_ptr<DrawStage> HDrawStage;

#endif //AWEBWOWVIEWERCPP_DRAWSTAGE_H
