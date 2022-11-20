#ifndef WOWMAPVIEWERREVIVED_SHADERDEFINITIONS_H
#define WOWMAPVIEWERREVIVED_SHADERDEFINITIONS_H

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

template <typename T>
inline constexpr const uint32_t operator+ (T const val) { return static_cast<const uint32_t>(val); };
struct fieldDefine {
    std::string name;
    bool isFloat ;
    int offset;
    int columns;
    int vecSize;
    int arraySize;
};
struct attributeDefine {
    std::string name;
    int location;
};
struct uboBindingData {
    int set;
    int binding;
    int size;
};

struct shaderMetaData {
    std::vector<uboBindingData> uboBindings;
};

//Per file
extern const std::unordered_map<std::string, shaderMetaData> shaderMetaInfo;
extern const std::unordered_map<std::string, std::vector<attributeDefine>> attributesPerShaderName;
extern const std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameVert;
extern const std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameFrag;
struct waterfallShader {
    enum class Attribute {
        aPosition = 0, aNormal = 1, bones = 2, boneWeights = 3, aTexCoord = 4, aTexCoord2 = 5, waterfallShaderAttributeEnd
    };
};

struct adtLodShader {
    enum class Attribute {
        aHeight = 0, aIndex = 1, adtLodShaderAttributeEnd
    };
};

struct adtShader {
    enum class Attribute {
        aHeight = 0, aColor = 1, aVertexLighting = 2, aNormal = 3, aIndex = 4, adtShaderAttributeEnd
    };
};

struct drawBBShader {
    enum class Attribute {
        aPosition = 0, drawBBShaderAttributeEnd
    };
};

struct waterShader {
    enum class Attribute {
        aPositionTransp = 0, aTexCoord = 1, waterShaderAttributeEnd
    };
};

struct m2ParticleShader {
    enum class Attribute {
        aPosition = 0, aColor = 1, aTexcoord0 = 2, aTexcoord1 = 3, aTexcoord2 = 4, m2ParticleShaderAttributeEnd
    };
};

struct drawFrustumShader {
    enum class Attribute {
        aPosition = 0, drawFrustumShaderAttributeEnd
    };
};

struct drawPoints {
    enum class Attribute {
        aPosition = 0, drawPointsAttributeEnd
    };
};

struct drawQuad {
    enum class Attribute {
        position = 0, drawQuadAttributeEnd
    };
};

struct skyConus {
    enum class Attribute {
        aPosition = 0, skyConusAttributeEnd
    };
};

struct m2Shader {
    enum class Attribute {
        aPosition = 0, aNormal = 1, bones = 2, boneWeights = 3, aTexCoord = 4, aTexCoord2 = 5, m2ShaderAttributeEnd
    };
};

struct drawPortalShader {
    enum class Attribute {
        aPosition = 0, drawPortalShaderAttributeEnd
    };
};

struct renderFrameBufferShader {
    enum class Attribute {
        a_position = 0, renderFrameBufferShaderAttributeEnd
    };
};

struct wmoShader {
    enum class Attribute {
        aPosition = 0, aNormal = 1, aTexCoord = 2, aTexCoord2 = 3, aTexCoord3 = 4, aTexCoord4 = 5, aColor = 6, aColor2 = 7, aColorSecond = 8, wmoShaderAttributeEnd
    };
};

struct imguiShader {
    enum class Attribute {
        Position = 0, UV = 1, Color = 2, imguiShaderAttributeEnd
    };
};

struct drawLinesShader {
    enum class Attribute {
        aPosition = 0, drawLinesShaderAttributeEnd
    };
};

struct ribbonShader {
    enum class Attribute {
        aPosition = 0, aColor = 1, aTexcoord0 = 2, ribbonShaderAttributeEnd
    };
};

std::string loadShader(std::string shaderName);
#ifdef SHADERDATACPP
const std::unordered_map<std::string, std::vector<attributeDefine>> attributesPerShaderName = {
{"waterfallShader",  {
{"aPosition", 0},
{"aNormal", 1},
{"bones", 2},
{"boneWeights", 3},
{"aTexCoord", 4},
{"aTexCoord2", 5},
}},{"adtLodShader",  {
{"aHeight", 0},
{"aIndex", 1},
}},{"adtShader",  {
{"aHeight", 0},
{"aColor", 1},
{"aVertexLighting", 2},
{"aNormal", 3},
{"aIndex", 4},
}},{"drawBBShader",  {
{"aPosition", 0},
}},{"waterShader",  {
{"aPositionTransp", 0},
{"aTexCoord", 1},
}},{"m2ParticleShader",  {
{"aPosition", 0},
{"aColor", 1},
{"aTexcoord0", 2},
{"aTexcoord1", 3},
{"aTexcoord2", 4},
}},{"drawFrustumShader",  {
{"aPosition", 0},
}},{"drawPoints",  {
{"aPosition", 0},
}},{"drawQuad",  {
{"position", 0},
}},{"skyConus",  {
{"aPosition", 0},
}},{"m2Shader",  {
{"aPosition", 0},
{"aNormal", 1},
{"bones", 2},
{"boneWeights", 3},
{"aTexCoord", 4},
{"aTexCoord2", 5},
}},{"drawPortalShader",  {
{"aPosition", 0},
}},{"renderFrameBufferShader",  {
{"a_position", 0},
}},{"wmoShader",  {
{"aPosition", 0},
{"aNormal", 1},
{"aTexCoord", 2},
{"aTexCoord2", 3},
{"aTexCoord3", 4},
{"aTexCoord4", 5},
{"aColor", 6},
{"aColor2", 7},
{"aColorSecond", 8},
}},{"imguiShader",  {
{"Position", 0},
{"UV", 1},
{"Color", 2},
}},{"drawLinesShader",  {
{"aPosition", 0},
}},{"ribbonShader",  {
{"aPosition", 0},
{"aColor", 1},
{"aTexcoord0", 2},
}},};

const std::unordered_map<std::string, shaderMetaData> shaderMetaInfo = {{ "wmoShader.vert.spv", {
{
{0,1,64},
{0,0,368},
{0,2,16},
}
}
},{ "wmoShader.frag.spv", {
{
{0,4,32},
{0,0,368},
{0,3,32},
}
}
},{ "waterShader.frag.spv", {
{
{0,4,16},
{0,0,368},
}
}
},{ "waterfallShader.frag.spv", {
{
{0,4,96},
{0,0,368},
}
}
},{ "adtShader.vert.spv", {
{
{0,0,368},
}
}
},{ "adtLodShader.vert.spv", {
{
{0,0,144},
}
}
},{ "waterfallShader.vert.spv", {
{
{0,2,144},
{0,1,14144},
{0,0,368},
}
}
},{ "drawPoints.vert.spv", {
{
{0,0,128},
{0,1,64},
}
}
},{ "drawFrustumShader.vert.spv", {
{
{0,0,128},
}
}
},{ "ffxglow.frag.spv", {
{
{0,4,16},
}
}
},{ "adtShader.frag.spv", {
{
{0,4,288},
{0,3,16},
{0,0,368},
}
}
},{ "drawBBShader.vert.spv", {
{
{0,1,112},
{0,0,368},
}
}
},{ "drawFrustumShader.frag.spv", {
{
}
}
},{ "drawDepthShader.frag.spv", {
{
{0,2,12},
}
}
},{ "adtLodShader.frag.spv", {
{
{0,0,84},
}
}
},{ "drawPoints.frag.spv", {
{
{0,1,12},
}
}
},{ "drawBBShader.frag.spv", {
{
{0,1,112},
}
}
},{ "drawLinesShader.frag.spv", {
{
}
}
},{ "skyConus.frag.spv", {
{
}
}
},{ "drawPortalShader.frag.spv", {
{
{0,4,16},
}
}
},{ "drawLinesShader.vert.spv", {
{
{0,0,128},
}
}
},{ "ffxgauss4.frag.spv", {
{
{0,4,32},
}
}
},{ "imguiShader.frag.spv", {
{
}
}
},{ "m2ParticleShader.vert.spv", {
{
{0,0,368},
}
}
},{ "drawQuad.vert.spv", {
{
{0,2,16},
}
}
},{ "imguiShader.vert.spv", {
{
{0,1,80},
}
}
},{ "m2Shader.frag.spv", {
{
{0,4,64},
{0,3,256},
{0,0,368},
{0,1,14144},
}
}
},{ "waterShader.vert.spv", {
{
{0,0,368},
{0,1,64},
}
}
},{ "drawPortalShader.vert.spv", {
{
{0,0,128},
}
}
},{ "ribbonShader.vert.spv", {
{
{0,0,368},
}
}
},{ "skyConus.vert.spv", {
{
{0,0,368},
{0,2,96},
}
}
},{ "renderFrameBufferShader.frag.spv", {
{
{0,2,168},
}
}
},{ "renderFrameBufferShader.vert.spv", {
{
}
}
},{ "m2ParticleShader.frag.spv", {
{
{0,4,32},
{0,0,368},
}
}
},{ "m2Shader.vert.spv", {
{
{0,1,14144},
{0,0,368},
{0,2,160},
}
}
},{ "ribbonShader.frag.spv", {
{
{0,4,48},
{0,0,368},
}
}
},};

const  std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameVert = {
  {"waterfallShader",  {
    {
      0, {
        {"_196_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_196_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_196_scene_uViewUp", true, 128, 1, 4, 0},
        {"_196_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_196_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_196_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_196_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_196_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_196_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_196_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_196_fogData_densityParams", true, 256, 1, 4, 0},
        {"_196_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_196_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_196_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_196_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_196_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_196_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      1, {
        {"_101_uPlacementMat", true, 0, 4, 4, 0},
        {"_101_uBoneMatrixes[0]", true, 64, 4, 4, 220},
      }
    },
    {
      2, {
        {"_50_bumpScale", true, 0, 1, 4, 0},
        {"_50_uTextMat[0]", true, 16, 4, 4, 2},
      }
    },
  }},
  {"adtLodShader",  {
    {
      0, {
        {"_55_uPos", true, 0, 1, 3, 0},
        {"_55_uLookAtMat", true, 16, 4, 4, 0},
        {"_55_uPMatrix", true, 80, 4, 4, 0},
      }
    },
  }},
  {"adtShader",  {
    {
      0, {
        {"_91_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_91_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_91_scene_uViewUp", true, 128, 1, 4, 0},
        {"_91_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_91_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_91_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_91_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_91_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_91_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_91_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_91_fogData_densityParams", true, 256, 1, 4, 0},
        {"_91_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_91_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_91_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_91_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_91_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_91_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
  }},
  {"drawBBShader",  {
    {
      0, {
        {"_62_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_62_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_62_scene_uViewUp", true, 128, 1, 4, 0},
        {"_62_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_62_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_62_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_62_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_62_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_62_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_62_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_62_fogData_densityParams", true, 256, 1, 4, 0},
        {"_62_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_62_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_62_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_62_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_62_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_62_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      1, {
        {"_21_uPlacementMat", true, 0, 4, 4, 0},
        {"_21_uBBScale", true, 64, 1, 4, 0},
        {"_21_uBBCenter", true, 80, 1, 4, 0},
        {"_21_uColor", true, 96, 1, 4, 0},
      }
    },
  }},
  {"waterShader",  {
    {
      1, {
        {"_36_uPlacementMat", true, 0, 4, 4, 0},
      }
    },
    {
      0, {
        {"_28_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_28_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_28_scene_uViewUp", true, 128, 1, 4, 0},
        {"_28_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_28_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_28_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_28_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_28_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_28_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_28_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_28_fogData_densityParams", true, 256, 1, 4, 0},
        {"_28_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_28_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_28_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_28_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_28_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_28_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
  }},
  {"m2ParticleShader",  {
    {
      0, {
        {"_43_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_43_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_43_scene_uViewUp", true, 128, 1, 4, 0},
        {"_43_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_43_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_43_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_43_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_43_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_43_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_43_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_43_fogData_densityParams", true, 256, 1, 4, 0},
        {"_43_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_43_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_43_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_43_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_43_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_43_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
  }},
  {"drawFrustumShader",  {
    {
      0, {
        {"_13_uLookAtMat", true, 0, 4, 4, 0},
        {"_13_uPMatrix", true, 64, 4, 4, 0},
      }
    },
  }},
  {"drawPoints",  {
    {
      1, {
        {"_29_uPlacementMat", true, 0, 4, 4, 0},
      }
    },
    {
      0, {
        {"_19_uLookAtMat", true, 0, 4, 4, 0},
        {"_19_uPMatrix", true, 64, 4, 4, 0},
      }
    },
  }},
  {"drawQuad",  {
    {
      2, {
        {"_12_uWidth_uHeight_uX_uY", true, 0, 1, 4, 0},
      }
    },
  }},
  {"skyConus",  {
    {
      2, {
        {"_73_skyColor[0]", true, 0, 1, 4, 6},
      }
    },
    {
      0, {
        {"_26_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_26_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_26_scene_uViewUp", true, 128, 1, 4, 0},
        {"_26_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_26_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_26_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_26_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_26_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_26_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_26_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_26_fogData_densityParams", true, 256, 1, 4, 0},
        {"_26_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_26_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_26_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_26_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_26_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_26_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
  }},
  {"m2Shader",  {
    {
      2, {
        {"_612_vertexShader_IsAffectedByLight", false, 0, 1, 4, 0},
        {"_612_color_Transparency", true, 16, 1, 4, 0},
        {"_612_uTextMat[0]", true, 32, 4, 4, 2},
      }
    },
    {
      0, {
        {"_581_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_581_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_581_scene_uViewUp", true, 128, 1, 4, 0},
        {"_581_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_581_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_581_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_581_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_581_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_581_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_581_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_581_fogData_densityParams", true, 256, 1, 4, 0},
        {"_581_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_581_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_581_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_581_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_581_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_581_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      1, {
        {"_484_uPlacementMat", true, 0, 4, 4, 0},
        {"_484_uBoneMatrixes[0]", true, 64, 4, 4, 220},
      }
    },
  }},
  {"drawPortalShader",  {
    {
      0, {
        {"_30_uLookAtMat", true, 0, 4, 4, 0},
        {"_30_uPMatrix", true, 64, 4, 4, 0},
      }
    },
  }},
  {"renderFrameBufferShader",  {
  }},
  {"wmoShader",  {
    {
      2, {
        {"_253_VertexShader_UseLitColor", false, 0, 1, 4, 0},
      }
    },
    {
      0, {
        {"_187_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_187_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_187_scene_uViewUp", true, 128, 1, 4, 0},
        {"_187_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_187_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_187_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_187_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_187_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_187_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_187_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_187_fogData_densityParams", true, 256, 1, 4, 0},
        {"_187_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_187_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_187_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_187_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_187_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_187_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      1, {
        {"_169_uPlacementMat", true, 0, 4, 4, 0},
      }
    },
  }},
  {"imguiShader",  {
    {
      1, {
        {"_30_ProjMtx", true, 0, 4, 4, 0},
        {"_30_uiScale", true, 64, 1, 4, 0},
      }
    },
  }},
  {"drawLinesShader",  {
    {
      0, {
        {"_19_uLookAtMat", true, 0, 4, 4, 0},
        {"_19_uPMatrix", true, 64, 4, 4, 0},
      }
    },
  }},
  {"ribbonShader",  {
    {
      0, {
        {"_37_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_37_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_37_scene_uViewUp", true, 128, 1, 4, 0},
        {"_37_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_37_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_37_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_37_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_37_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_37_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_37_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_37_fogData_densityParams", true, 256, 1, 4, 0},
        {"_37_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_37_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_37_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_37_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_37_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_37_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
  }},
};
const  std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameFrag = {
  {"waterfallShader",  {
    {
      0, {
        {"_686_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_686_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_686_scene_uViewUp", true, 128, 1, 4, 0},
        {"_686_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_686_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_686_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_686_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_686_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_686_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_686_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_686_fogData_densityParams", true, 256, 1, 4, 0},
        {"_686_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_686_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_686_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_686_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_686_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_686_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      4, {
        {"_419_values0", true, 0, 1, 4, 0},
        {"_419_values1", true, 16, 1, 4, 0},
        {"_419_values2", true, 32, 1, 4, 0},
        {"_419_values3", true, 48, 1, 4, 0},
        {"_419_values4", true, 64, 1, 4, 0},
        {"_419_baseColor", true, 80, 1, 4, 0},
      }
    },
  }},
  {"adtLodShader",  {
    {
      0, {
        {"_65_uViewUp", true, 0, 1, 4, 0},
        {"_65_uSunDir_FogStart", true, 16, 1, 4, 0},
        {"_65_uSunColor_uFogEnd", true, 32, 1, 4, 0},
        {"_65_uAmbientLight", true, 48, 1, 4, 0},
        {"_65_FogColor", true, 64, 1, 4, 0},
        {"_65_uNewFormula", false, 80, 1, 1, 0},
      }
    },
  }},
  {"renderFrameBufferShader",  {
    {
      2, {
        {"_34_gauss_offsets[0]", true, 0, 1, 1, 5},
        {"_34_gauss_weights[0]", true, 80, 1, 1, 5},
        {"_34_uResolution", true, 160, 1, 2, 0},
      }
    },
  }},
  {"drawPortalShader",  {
    {
      4, {
        {"_12_uColor", true, 0, 1, 4, 0},
      }
    },
  }},
  {"adtShader",  {
    {
      0, {
        {"_724_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_724_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_724_scene_uViewUp", true, 128, 1, 4, 0},
        {"_724_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_724_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_724_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_724_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_724_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_724_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_724_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_724_fogData_densityParams", true, 256, 1, 4, 0},
        {"_724_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_724_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_724_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_724_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_724_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_724_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      3, {
        {"_480_uUseHeightMixFormula", false, 0, 1, 4, 0},
      }
    },
    {
      4, {
        {"_440_uHeightScale", true, 0, 1, 4, 0},
        {"_440_uHeightOffset", true, 16, 1, 4, 0},
        {"_440_animationMat[0]", true, 32, 4, 4, 4},
      }
    },
  }},
  {"drawBBShader",  {
    {
      1, {
        {"_13_uPlacementMat", true, 0, 4, 4, 0},
        {"_13_uBBScale", true, 64, 1, 4, 0},
        {"_13_uBBCenter", true, 80, 1, 4, 0},
        {"_13_uColor", true, 96, 1, 4, 0},
      }
    },
  }},
  {"ribbonShader",  {
    {
      0, {
        {"_304_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_304_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_304_scene_uViewUp", true, 128, 1, 4, 0},
        {"_304_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_304_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_304_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_304_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_304_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_304_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_304_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_304_fogData_densityParams", true, 256, 1, 4, 0},
        {"_304_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_304_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_304_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_304_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_304_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_304_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      4, {
        {"_256_uAlphaTestScalev", true, 0, 1, 4, 0},
        {"_256_uPixelShaderv", false, 16, 1, 4, 0},
        {"_256_uTextureTranslate", true, 32, 1, 4, 0},
      }
    },
  }},
  {"drawLinesShader",  {
  }},
  {"waterShader",  {
    {
      0, {
        {"_277_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_277_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_277_scene_uViewUp", true, 128, 1, 4, 0},
        {"_277_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_277_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_277_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_277_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_277_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_277_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_277_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_277_fogData_densityParams", true, 256, 1, 4, 0},
        {"_277_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_277_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_277_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_277_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_277_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_277_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      4, {
        {"_253_color", true, 0, 1, 4, 0},
      }
    },
  }},
  {"m2ParticleShader",  {
    {
      0, {
        {"_486_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_486_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_486_scene_uViewUp", true, 128, 1, 4, 0},
        {"_486_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_486_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_486_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_486_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_486_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_486_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_486_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_486_fogData_densityParams", true, 256, 1, 4, 0},
        {"_486_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_486_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_486_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_486_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_486_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_486_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      4, {
        {"_277_uAlphaTestv", true, 0, 1, 4, 0},
        {"_277_uPixelShaderBlendModev", false, 16, 1, 4, 0},
      }
    },
  }},
  {"drawDepthShader",  {
    {
      2, {
        {"_10_drawDepth", false, 0, 1, 1, 0},
        {"_10_uFarPlane", true, 4, 1, 1, 0},
        {"_10_uNearPlane", true, 8, 1, 1, 0},
      }
    },
  }},
  {"drawPoints",  {
    {
      1, {
        {"_13_uColor", true, 0, 1, 3, 0},
      }
    },
  }},
  {"drawFrustumShader",  {
  }},
  {"ffxgauss4",  {
    {
      4, {
        {"_33_texOffsetX", true, 0, 1, 4, 0},
        {"_33_texOffsetY", true, 16, 1, 4, 0},
      }
    },
  }},
  {"skyConus",  {
  }},
  {"m2Shader",  {
    {
      1, {
        {"_1157_uPlacementMat", true, 0, 4, 4, 0},
        {"_1157_uBoneMatrixes[0]", true, 64, 4, 4, 220},
      }
    },
    {
      0, {
        {"_1149_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_1149_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_1149_scene_uViewUp", true, 128, 1, 4, 0},
        {"_1149_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_1149_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_1149_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_1149_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_1149_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_1149_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_1149_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_1149_fogData_densityParams", true, 256, 1, 4, 0},
        {"_1149_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_1149_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_1149_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_1149_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_1149_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_1149_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      3, {
        {"_1110_intLight_uInteriorAmbientColorAndApplyInteriorLight", true, 0, 1, 4, 0},
        {"_1110_intLight_uInteriorDirectColorAndApplyExteriorLight", true, 16, 1, 4, 0},
        {"_1110_pc_lights[0].color", true, 32, 1, 4, 0},
        {"_1110_pc_lights[0].position", true, 48, 1, 4, 0},
        {"_1110_pc_lights[0].attenuation", true, 64, 1, 4, 0},
        {"_1110_pc_lights[1].color", true, 80, 1, 4, 0},
        {"_1110_pc_lights[1].position", true, 96, 1, 4, 0},
        {"_1110_pc_lights[1].attenuation", true, 112, 1, 4, 0},
        {"_1110_pc_lights[2].color", true, 128, 1, 4, 0},
        {"_1110_pc_lights[2].position", true, 144, 1, 4, 0},
        {"_1110_pc_lights[2].attenuation", true, 160, 1, 4, 0},
        {"_1110_pc_lights[3].color", true, 176, 1, 4, 0},
        {"_1110_pc_lights[3].position", true, 192, 1, 4, 0},
        {"_1110_pc_lights[3].attenuation", true, 208, 1, 4, 0},
        {"_1110_lightCountAndBcHack", false, 224, 1, 4, 0},
        {"_1110_interiorExteriorBlend", true, 240, 1, 4, 0},
      }
    },
    {
      4, {
        {"_1087_PixelShader_UnFogged_IsAffectedByLight_blendMode", false, 0, 1, 4, 0},
        {"_1087_uFogColorAndAlphaTest", true, 16, 1, 4, 0},
        {"_1087_uTexSampleAlpha", true, 32, 1, 4, 0},
        {"_1087_uPcColor", true, 48, 1, 4, 0},
      }
    },
  }},
  {"ffxglow",  {
    {
      4, {
        {"_34_blurAmount", true, 0, 1, 4, 0},
      }
    },
  }},
  {"wmoShader",  {
    {
      3, {
        {"_1355_intLight_uInteriorAmbientColorAndApplyInteriorLight", true, 0, 1, 4, 0},
        {"_1355_intLight_uInteriorDirectColorAndApplyExteriorLight", true, 16, 1, 4, 0},
      }
    },
    {
      0, {
        {"_1348_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_1348_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_1348_scene_uViewUp", true, 128, 1, 4, 0},
        {"_1348_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_1348_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_1348_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_1348_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_1348_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_1348_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_1348_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_1348_fogData_densityParams", true, 256, 1, 4, 0},
        {"_1348_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_1348_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_1348_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_1348_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_1348_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_1348_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      4, {
        {"_1268_UseLitColor_EnableAlpha_PixelShader_BlendMode", false, 0, 1, 4, 0},
        {"_1268_FogColor_AlphaTest", true, 16, 1, 4, 0},
      }
    },
  }},
  {"imguiShader",  {
  }},
};
#endif


#endif
