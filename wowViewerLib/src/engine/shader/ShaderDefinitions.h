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
struct wmoShader {
    enum class Attribute {
        aPosition = 0, aNormal = 1, aTexCoord = 2, aTexCoord2 = 3, aTexCoord3 = 4, aColor = 5, aColor2 = 6, wmoShaderAttributeEnd
    };
};

struct waterfallShader {
    enum class Attribute {
        aPosition = 0, aNormal = 1, bones = 2, boneWeights = 3, aTexCoord = 4, aTexCoord2 = 5, waterfallShaderAttributeEnd
    };
};

struct ribbonShader {
    enum class Attribute {
        aPosition = 0, aColor = 1, aTexcoord0 = 2, ribbonShaderAttributeEnd
    };
};

struct drawQuad {
    enum class Attribute {
        position = 0, drawQuadAttributeEnd
    };
};

struct m2ParticleShader {
    enum class Attribute {
        aPosition = 0, aColor = 1, aTexcoord0 = 2, aTexcoord1 = 3, aTexcoord2 = 4, m2ParticleShaderAttributeEnd
    };
};

struct drawPortalShader {
    enum class Attribute {
        aPosition = 0, drawPortalShaderAttributeEnd
    };
};

struct waterShader {
    enum class Attribute {
        aPositionTransp = 0, aTexCoord = 1, waterShaderAttributeEnd
    };
};

struct renderFrameBufferShader {
    enum class Attribute {
        a_position = 0, renderFrameBufferShaderAttributeEnd
    };
};

struct drawLinesShader {
    enum class Attribute {
        aPosition = 0, drawLinesShaderAttributeEnd
    };
};

struct drawFrustumShader {
    enum class Attribute {
        aPosition = 0, drawFrustumShaderAttributeEnd
    };
};

struct skyConus {
    enum class Attribute {
        aPosition = 0, skyConusAttributeEnd
    };
};

struct adtShader {
    enum class Attribute {
        aHeight = 0, aColor = 1, aVertexLighting = 2, aNormal = 3, aIndex = 4, adtShaderAttributeEnd
    };
};

struct m2Shader {
    enum class Attribute {
        aPosition = 0, aNormal = 1, bones = 2, boneWeights = 3, aTexCoord = 4, aTexCoord2 = 5, m2ShaderAttributeEnd
    };
};

struct imguiShader {
    enum class Attribute {
        Position = 0, UV = 1, Color = 2, imguiShaderAttributeEnd
    };
};

struct drawPoints {
    enum class Attribute {
        aPosition = 0, drawPointsAttributeEnd
    };
};

struct drawBBShader {
    enum class Attribute {
        aPosition = 0, drawBBShaderAttributeEnd
    };
};

struct adtLodShader {
    enum class Attribute {
        aHeight = 0, aIndex = 1, adtLodShaderAttributeEnd
    };
};

std::string loadShader(std::string shaderName);
#ifdef SHADERDATACPP
const std::unordered_map<std::string, std::vector<attributeDefine>> attributesPerShaderName = {
{"wmoShader",  {
{"aPosition", 0},
{"aNormal", 1},
{"aTexCoord", 2},
{"aTexCoord2", 3},
{"aTexCoord3", 4},
{"aColor", 5},
{"aColor2", 6},
}},{"waterfallShader",  {
{"aPosition", 0},
{"aNormal", 1},
{"bones", 2},
{"boneWeights", 3},
{"aTexCoord", 4},
{"aTexCoord2", 5},
}},{"ribbonShader",  {
{"aPosition", 0},
{"aColor", 1},
{"aTexcoord0", 2},
}},{"drawQuad",  {
{"position", 0},
}},{"m2ParticleShader",  {
{"aPosition", 0},
{"aColor", 1},
{"aTexcoord0", 2},
{"aTexcoord1", 3},
{"aTexcoord2", 4},
}},{"drawPortalShader",  {
{"aPosition", 0},
}},{"waterShader",  {
{"aPositionTransp", 0},
{"aTexCoord", 1},
}},{"renderFrameBufferShader",  {
{"a_position", 0},
}},{"drawLinesShader",  {
{"aPosition", 0},
}},{"drawFrustumShader",  {
{"aPosition", 0},
}},{"skyConus",  {
{"aPosition", 0},
}},{"adtShader",  {
{"aHeight", 0},
{"aColor", 1},
{"aVertexLighting", 2},
{"aNormal", 3},
{"aIndex", 4},
}},{"m2Shader",  {
{"aPosition", 0},
{"aNormal", 1},
{"bones", 2},
{"boneWeights", 3},
{"aTexCoord", 4},
{"aTexCoord2", 5},
}},{"imguiShader",  {
{"Position", 0},
{"UV", 1},
{"Color", 2},
}},{"drawPoints",  {
{"aPosition", 0},
}},{"drawBBShader",  {
{"aPosition", 0},
}},{"adtLodShader",  {
{"aHeight", 0},
{"aIndex", 1},
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
},{ "waterfallShader.vert.spv", {
{
{0,2,144},
{0,1,14144},
{0,0,368},
}
}
},{ "waterfallShader.frag.spv", {
{
{0,4,96},
{0,0,368},
{0,1,14144},
}
}
},{ "skyConus.frag.spv", {
{
}
}
},{ "ribbonShader.frag.spv", {
{
{0,4,48},
{0,0,368},
}
}
},{ "renderFrameBufferShader.vert.spv", {
{
}
}
},{ "waterShader.frag.spv", {
{
{0,4,16},
{0,0,368},
}
}
},{ "m2Shader.vert.spv", {
{
{0,1,14144},
{0,2,160},
{0,0,368},
}
}
},{ "drawLinesShader.frag.spv", {
{
{0,1,12},
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
},{ "adtShader.vert.spv", {
{
{0,0,368},
{0,2,16},
}
}
},{ "drawLinesShader.vert.spv", {
{
{0,0,128},
}
}
},{ "m2ParticleShader.vert.spv", {
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
},{ "m2ParticleShader.frag.spv", {
{
{0,4,32},
{0,0,368},
}
}
},{ "imguiShader.frag.spv", {
{
}
}
},{ "ffxglow.frag.spv", {
{
{0,4,16},
}
}
},{ "ffxgauss4.frag.spv", {
{
{0,4,32},
}
}
},{ "drawFrustumShader.frag.spv", {
{
{0,2,12},
}
}
},{ "drawPoints.vert.spv", {
{
{0,0,128},
{0,1,64},
}
}
},{ "drawPoints.frag.spv", {
{
{0,1,12},
}
}
},{ "adtShader.frag.spv", {
{
{0,4,288},
{0,3,16},
{0,0,368},
}
}
},{ "drawFrustumShader.vert.spv", {
{
{0,0,128},
}
}
},{ "ribbonShader.vert.spv", {
{
{0,0,368},
}
}
},{ "drawBBShader.frag.spv", {
{
{0,0,112},
}
}
},{ "drawPortalShader.frag.spv", {
{
{0,1,16},
}
}
},{ "drawQuad.vert.spv", {
{
{0,2,16},
}
}
},{ "adtLodShader.frag.spv", {
{
{0,0,84},
}
}
},{ "drawBBShader.vert.spv", {
{
{0,1,112},
{0,0,128},
}
}
},{ "renderFrameBufferShader.frag.spv", {
{
{0,2,168},
}
}
},{ "imguiShader.vert.spv", {
{
{0,1,80},
}
}
},{ "drawPortalShader.vert.spv", {
{
{0,0,128},
{0,1,64},
}
}
},{ "waterShader.vert.spv", {
{
{0,0,368},
{0,1,64},
}
}
},{ "drawDepthShader.frag.spv", {
{
{0,2,12},
}
}
},{ "adtLodShader.vert.spv", {
{
{0,0,144},
}
}
},};

const  std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameVert = {
  {"adtLodShader",  {
    {
      0, {
        {"_55_uPos", true, 0, 1, 3, 0},
        {"_55_uLookAtMat", true, 16, 4, 4, 0},
        {"_55_uPMatrix", true, 80, 4, 4, 0},
      }
    },
  }},
  {"drawBBShader",  {
    {
      1, {
        {"_21_uPlacementMat", true, 0, 4, 4, 0},
        {"_21_uBBScale", true, 64, 1, 4, 0},
        {"_21_uBBCenter", true, 80, 1, 4, 0},
        {"_21_uColor", true, 96, 1, 4, 0},
      }
    },
    {
      0, {
        {"_59_uLookAtMat", true, 0, 4, 4, 0},
        {"_59_uPMatrix", true, 64, 4, 4, 0},
      }
    },
  }},
  {"drawPoints",  {
    {
      0, {
        {"_19_uLookAtMat", true, 0, 4, 4, 0},
        {"_19_uPMatrix", true, 64, 4, 4, 0},
      }
    },
    {
      1, {
        {"_29_uPlacementMat", true, 0, 4, 4, 0},
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
  {"m2Shader",  {
    {
      1, {
        {"_133_uPlacementMat", true, 0, 4, 4, 0},
        {"_133_uBoneMatrixes[0]", true, 64, 4, 4, 220},
      }
    },
    {
      2, {
        {"_230_vertexShader_IsAffectedByLight", false, 0, 1, 4, 0},
        {"_230_color_Transparency", true, 16, 1, 4, 0},
        {"_230_uTextMat[0]", true, 32, 4, 4, 2},
      }
    },
    {
      0, {
        {"_240_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_240_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_240_scene_uViewUp", true, 128, 1, 4, 0},
        {"_240_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_240_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_240_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_240_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_240_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_240_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_240_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_240_fogData_densityParams", true, 256, 1, 4, 0},
        {"_240_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_240_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_240_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_240_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_240_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_240_fogData_sunPercentage", true, 352, 1, 4, 0},
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
    {
      2, {
        {"_139_uPos", true, 0, 1, 4, 0},
      }
    },
  }},
  {"skyConus",  {
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
    {
      2, {
        {"_67_skyColor[0]", true, 0, 1, 4, 6},
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
  {"drawLinesShader",  {
    {
      0, {
        {"_19_uLookAtMat", true, 0, 4, 4, 0},
        {"_19_uPMatrix", true, 64, 4, 4, 0},
      }
    },
  }},
  {"renderFrameBufferShader",  {
  }},
  {"waterShader",  {
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
    {
      1, {
        {"_36_uPlacementMat", true, 0, 4, 4, 0},
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
    {
      1, {
        {"_40_uPlacementMat", true, 0, 4, 4, 0},
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
  {"drawQuad",  {
    {
      2, {
        {"_12_uWidth_uHeight_uX_uY", true, 0, 1, 4, 0},
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
  {"waterfallShader",  {
    {
      2, {
        {"_55_bumpScale", true, 0, 1, 4, 0},
        {"_55_uTextMat[0]", true, 16, 4, 4, 2},
      }
    },
    {
      1, {
        {"_104_uPlacementMat", true, 0, 4, 4, 0},
        {"_104_uBoneMatrixes[0]", true, 64, 4, 4, 220},
      }
    },
    {
      0, {
        {"_199_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_199_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_199_scene_uViewUp", true, 128, 1, 4, 0},
        {"_199_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_199_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_199_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_199_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_199_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_199_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_199_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_199_fogData_densityParams", true, 256, 1, 4, 0},
        {"_199_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_199_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_199_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_199_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_199_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_199_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
  }},
  {"wmoShader",  {
    {
      1, {
        {"_93_uPlacementMat", true, 0, 4, 4, 0},
      }
    },
    {
      0, {
        {"_111_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_111_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_111_scene_uViewUp", true, 128, 1, 4, 0},
        {"_111_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_111_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_111_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_111_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_111_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_111_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_111_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_111_fogData_densityParams", true, 256, 1, 4, 0},
        {"_111_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_111_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_111_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_111_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_111_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_111_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      2, {
        {"_182_VertexShader_UseLitColor", false, 0, 1, 4, 0},
      }
    },
  }},
};
const  std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameFrag = {
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
  {"ffxglow",  {
    {
      4, {
        {"_34_blurAmount", true, 0, 1, 4, 0},
      }
    },
  }},
  {"adtShader",  {
    {
      4, {
        {"_465_uHeightScale", true, 0, 1, 4, 0},
        {"_465_uHeightOffset", true, 16, 1, 4, 0},
        {"_465_animationMat[0]", true, 32, 4, 4, 4},
      }
    },
    {
      3, {
        {"_505_uUseHeightMixFormula", false, 0, 1, 4, 0},
      }
    },
    {
      0, {
        {"_747_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_747_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_747_scene_uViewUp", true, 128, 1, 4, 0},
        {"_747_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_747_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_747_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_747_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_747_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_747_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_747_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_747_fogData_densityParams", true, 256, 1, 4, 0},
        {"_747_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_747_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_747_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_747_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_747_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_747_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
  }},
  {"m2Shader",  {
    {
      4, {
        {"_472_PixelShader_UnFogged_IsAffectedByLight_blendMode", false, 0, 1, 4, 0},
        {"_472_uFogColorAndAlphaTest", true, 16, 1, 4, 0},
        {"_472_uTexSampleAlpha", true, 32, 1, 4, 0},
        {"_472_uPcColor", true, 48, 1, 4, 0},
      }
    },
    {
      3, {
        {"_495_intLight_uInteriorAmbientColorAndApplyInteriorLight", true, 0, 1, 4, 0},
        {"_495_intLight_uInteriorDirectColorAndApplyExteriorLight", true, 16, 1, 4, 0},
        {"_495_pc_lights[0].color", true, 32, 1, 4, 0},
        {"_495_pc_lights[0].position", true, 48, 1, 4, 0},
        {"_495_pc_lights[0].attenuation", true, 64, 1, 4, 0},
        {"_495_pc_lights[1].color", true, 80, 1, 4, 0},
        {"_495_pc_lights[1].position", true, 96, 1, 4, 0},
        {"_495_pc_lights[1].attenuation", true, 112, 1, 4, 0},
        {"_495_pc_lights[2].color", true, 128, 1, 4, 0},
        {"_495_pc_lights[2].position", true, 144, 1, 4, 0},
        {"_495_pc_lights[2].attenuation", true, 160, 1, 4, 0},
        {"_495_pc_lights[3].color", true, 176, 1, 4, 0},
        {"_495_pc_lights[3].position", true, 192, 1, 4, 0},
        {"_495_pc_lights[3].attenuation", true, 208, 1, 4, 0},
        {"_495_lightCountAndBcHack", false, 224, 1, 4, 0},
        {"_495_interiorExteriorBlend", true, 240, 1, 4, 0},
      }
    },
    {
      0, {
        {"_534_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_534_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_534_scene_uViewUp", true, 128, 1, 4, 0},
        {"_534_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_534_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_534_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_534_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_534_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_534_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_534_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_534_fogData_densityParams", true, 256, 1, 4, 0},
        {"_534_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_534_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_534_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_534_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_534_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_534_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      1, {
        {"_542_uPlacementMat", true, 0, 4, 4, 0},
        {"_542_uBoneMatrixes[0]", true, 64, 4, 4, 220},
      }
    },
  }},
  {"skyConus",  {
  }},
  {"ffxgauss4",  {
    {
      4, {
        {"_33_texOffsetX", true, 0, 1, 4, 0},
        {"_33_texOffsetY", true, 16, 1, 4, 0},
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
  {"waterfallShader",  {
    {
      4, {
        {"_219_values0", true, 0, 1, 4, 0},
        {"_219_values1", true, 16, 1, 4, 0},
        {"_219_values2", true, 32, 1, 4, 0},
        {"_219_values3", true, 48, 1, 4, 0},
        {"_219_values4", true, 64, 1, 4, 0},
        {"_219_baseColor", true, 80, 1, 4, 0},
      }
    },
    {
      0, {
        {"_487_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_487_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_487_scene_uViewUp", true, 128, 1, 4, 0},
        {"_487_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_487_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_487_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_487_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_487_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_487_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_487_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_487_fogData_densityParams", true, 256, 1, 4, 0},
        {"_487_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_487_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_487_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_487_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_487_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_487_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      1, {
        {"_570_uPlacementMat", true, 0, 4, 4, 0},
        {"_570_uBoneMatrixes[0]", true, 64, 4, 4, 220},
      }
    },
  }},
  {"drawBBShader",  {
    {
      0, {
        {"_13_uPlacementMat", true, 0, 4, 4, 0},
        {"_13_uBBScale", true, 64, 1, 4, 0},
        {"_13_uBBCenter", true, 80, 1, 4, 0},
        {"_13_uColor", true, 96, 1, 4, 0},
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
  {"imguiShader",  {
  }},
  {"drawFrustumShader",  {
    {
      2, {
        {"_22_uColor", true, 0, 1, 3, 0},
      }
    },
  }},
  {"drawLinesShader",  {
    {
      1, {
        {"_19_uColor", true, 0, 1, 3, 0},
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
  {"waterShader",  {
    {
      4, {
        {"_253_color", true, 0, 1, 4, 0},
      }
    },
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
  }},
  {"drawPortalShader",  {
    {
      1, {
        {"_12_uColor", true, 0, 1, 4, 0},
      }
    },
  }},
  {"m2ParticleShader",  {
    {
      4, {
        {"_277_uAlphaTestv", true, 0, 1, 4, 0},
        {"_277_uPixelShaderBlendModev", false, 16, 1, 4, 0},
      }
    },
    {
      0, {
        {"_485_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_485_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_485_scene_uViewUp", true, 128, 1, 4, 0},
        {"_485_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_485_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_485_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_485_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_485_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_485_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_485_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_485_fogData_densityParams", true, 256, 1, 4, 0},
        {"_485_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_485_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_485_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_485_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_485_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_485_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
  }},
  {"ribbonShader",  {
    {
      4, {
        {"_256_uAlphaTestScalev", true, 0, 1, 4, 0},
        {"_256_uPixelShaderv", false, 16, 1, 4, 0},
        {"_256_uTextureTranslate", true, 32, 1, 4, 0},
      }
    },
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
  }},
  {"wmoShader",  {
    {
      4, {
        {"_446_UseLitColor_EnableAlpha_PixelShader_BlendMode", false, 0, 1, 4, 0},
        {"_446_FogColor_AlphaTest", true, 16, 1, 4, 0},
      }
    },
    {
      0, {
        {"_909_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_909_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_909_scene_uViewUp", true, 128, 1, 4, 0},
        {"_909_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_909_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_909_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_909_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_909_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_909_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_909_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_909_fogData_densityParams", true, 256, 1, 4, 0},
        {"_909_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_909_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_909_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_909_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_909_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_909_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      3, {
        {"_913_intLight_uInteriorAmbientColorAndApplyInteriorLight", true, 0, 1, 4, 0},
        {"_913_intLight_uInteriorDirectColorAndApplyExteriorLight", true, 16, 1, 4, 0},
      }
    },
  }},
};
#endif


#endif
