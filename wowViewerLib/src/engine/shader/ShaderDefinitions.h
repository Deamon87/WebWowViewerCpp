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
extern const std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderName;
struct wmoShader {
    enum class Attribute {
        aPosition = 0, aNormal = 1, aTexCoord = 2, aTexCoord2 = 3, aTexCoord3 = 4, aColor = 5, aColor2 = 6, wmoShaderAttributeEnd
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
{0,0,352},
{0,2,16},
}
}
},{ "wmoShader.frag.spv", {
{
{0,4,32},
{0,0,352},
{0,3,32},
}
}
},{ "skyConus.frag.spv", {
{
}
}
},{ "ribbonShader.frag.spv", {
{
{0,0,352},
{0,4,32},
}
}
},{ "renderFrameBufferShader.vert.spv", {
{
}
}
},{ "waterShader.frag.spv", {
{
{0,4,16},
{0,0,352},
}
}
},{ "m2Shader.vert.spv", {
{
{0,1,14144},
{0,2,160},
{0,0,352},
}
}
},{ "drawLinesShader.frag.spv", {
{
{0,1,12},
}
}
},{ "m2Shader.frag.spv", {
{
{0,4,48},
{0,3,256},
{0,0,352},
{0,1,14144},
}
}
},{ "adtShader.vert.spv", {
{
{0,0,352},
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
{0,0,352},
}
}
},{ "skyConus.vert.spv", {
{
{0,0,352},
{0,2,96},
}
}
},{ "m2ParticleShader.frag.spv", {
{
{0,4,32},
{0,0,352},
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
{0,0,352},
{0,3,32},
}
}
},{ "drawFrustumShader.vert.spv", {
{
{0,0,128},
}
}
},{ "ribbonShader.vert.spv", {
{
{0,0,352},
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
{0,1,64},
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
{0,0,352},
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

const  std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderName = {
  {"wmoShader",  {
    {
      2, {
        {"_182.VertexShader_UseLitColor", false, 0, 1, 4, 0},
      }
    },
    {
      1, {
        {"_93.uPlacementMat", true, 0, 4, 4, 0},
      }
    },
    {
      3, {
        {"_902.intLight.uInteriorAmbientColorAndApplyInteriorLight", true, 0, 1, 4, 0},
        {"_902.intLight.uInteriorDirectColorAndApplyExteriorLight", true, 16, 1, 4, 0},
      }
    },
    {
      0, {
        {"_111.scene.uLookAtMat", true, 0, 4, 4, 0},
        {"_111.scene.uPMatrix", true, 64, 4, 4, 0},
        {"_111.scene.uViewUp", true, 128, 1, 4, 0},
        {"_111.scene.uInteriorSunDir", true, 144, 1, 4, 0},
        {"_111.scene.extLight.uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_111.scene.extLight.uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_111.scene.extLight.uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_111.scene.extLight.uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_111.scene.extLight.uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_111.fogData.densityParams", true, 240, 1, 4, 0},
        {"_111.fogData.heightPlane", true, 256, 1, 4, 0},
        {"_111.fogData.color_and_heightRate", true, 272, 1, 4, 0},
        {"_111.fogData.heightDensity_and_endColor", true, 288, 1, 4, 0},
        {"_111.fogData.sunAngle_and_sunColor", true, 304, 1, 4, 0},
        {"_111.fogData.heightColor_and_endFogDistance", true, 320, 1, 4, 0},
        {"_111.fogData.sunPercentage", true, 336, 1, 4, 0},
      }
    },
    {
      4, {
        {"_435.UseLitColor_EnableAlpha_PixelShader_BlendMode", false, 0, 1, 4, 0},
        {"_435.FogColor_AlphaTest", true, 16, 1, 4, 0},
      }
    },
  }},
  {"ribbonShader",  {
    {
      4, {
        {"_288.uAlphaTestv", true, 0, 1, 4, 0},
        {"_288.uPixelShaderv", false, 16, 1, 4, 0},
      }
    },
    {
      0, {
        {"_37.scene.uLookAtMat", true, 0, 4, 4, 0},
        {"_37.scene.uPMatrix", true, 64, 4, 4, 0},
        {"_37.scene.uViewUp", true, 128, 1, 4, 0},
        {"_37.scene.uInteriorSunDir", true, 144, 1, 4, 0},
        {"_37.scene.extLight.uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_37.scene.extLight.uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_37.scene.extLight.uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_37.scene.extLight.uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_37.scene.extLight.uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_37.fogData.densityParams", true, 240, 1, 4, 0},
        {"_37.fogData.heightPlane", true, 256, 1, 4, 0},
        {"_37.fogData.color_and_heightRate", true, 272, 1, 4, 0},
        {"_37.fogData.heightDensity_and_endColor", true, 288, 1, 4, 0},
        {"_37.fogData.sunAngle_and_sunColor", true, 304, 1, 4, 0},
        {"_37.fogData.heightColor_and_endFogDistance", true, 320, 1, 4, 0},
        {"_37.fogData.sunPercentage", true, 336, 1, 4, 0},
      }
    },
  }},
  {"imguiShader",  {
    {
      1, {
        {"_30.ProjMtx", true, 0, 4, 4, 0},
      }
    },
  }},
  {"drawPortalShader",  {
    {
      0, {
        {"_30.uLookAtMat", true, 0, 4, 4, 0},
        {"_30.uPMatrix", true, 64, 4, 4, 0},
      }
    },
    {
      1, {
        {"_40.uPlacementMat", true, 0, 4, 4, 0},
      }
    },
  }},
  {"m2ParticleShader",  {
    {
      0, {
        {"_43.scene.uLookAtMat", true, 0, 4, 4, 0},
        {"_43.scene.uPMatrix", true, 64, 4, 4, 0},
        {"_43.scene.uViewUp", true, 128, 1, 4, 0},
        {"_43.scene.uInteriorSunDir", true, 144, 1, 4, 0},
        {"_43.scene.extLight.uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_43.scene.extLight.uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_43.scene.extLight.uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_43.scene.extLight.uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_43.scene.extLight.uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_43.fogData.densityParams", true, 240, 1, 4, 0},
        {"_43.fogData.heightPlane", true, 256, 1, 4, 0},
        {"_43.fogData.color_and_heightRate", true, 272, 1, 4, 0},
        {"_43.fogData.heightDensity_and_endColor", true, 288, 1, 4, 0},
        {"_43.fogData.sunAngle_and_sunColor", true, 304, 1, 4, 0},
        {"_43.fogData.heightColor_and_endFogDistance", true, 320, 1, 4, 0},
        {"_43.fogData.sunPercentage", true, 336, 1, 4, 0},
      }
    },
    {
      4, {
        {"_266.uAlphaTestv", true, 0, 1, 4, 0},
        {"_266.uPixelShaderv", false, 16, 1, 4, 0},
      }
    },
  }},
  {"drawQuad",  {
    {
      2, {
        {"_36.uWidth", true, 0, 1, 1, 0},
        {"_36.uHeight", true, 4, 1, 1, 0},
        {"_36.uX", true, 8, 1, 1, 0},
        {"_36.uY", true, 12, 1, 1, 0},
      }
    },
  }},
  {"waterShader",  {
    {
      1, {
        {"_36.uPlacementMat", true, 0, 4, 4, 0},
      }
    },
    {
      0, {
        {"_28.scene.uLookAtMat", true, 0, 4, 4, 0},
        {"_28.scene.uPMatrix", true, 64, 4, 4, 0},
        {"_28.scene.uViewUp", true, 128, 1, 4, 0},
        {"_28.scene.uInteriorSunDir", true, 144, 1, 4, 0},
        {"_28.scene.extLight.uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_28.scene.extLight.uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_28.scene.extLight.uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_28.scene.extLight.uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_28.scene.extLight.uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_28.fogData.densityParams", true, 240, 1, 4, 0},
        {"_28.fogData.heightPlane", true, 256, 1, 4, 0},
        {"_28.fogData.color_and_heightRate", true, 272, 1, 4, 0},
        {"_28.fogData.heightDensity_and_endColor", true, 288, 1, 4, 0},
        {"_28.fogData.sunAngle_and_sunColor", true, 304, 1, 4, 0},
        {"_28.fogData.heightColor_and_endFogDistance", true, 320, 1, 4, 0},
        {"_28.fogData.sunPercentage", true, 336, 1, 4, 0},
      }
    },
    {
      4, {
        {"_242.color", true, 0, 1, 4, 0},
      }
    },
  }},
  {"renderFrameBufferShader",  {
    {
      2, {
        {"_34.gauss_offsets[0]", true, 0, 1, 1, 5},
        {"_34.gauss_weights[0]", true, 80, 1, 1, 5},
        {"_34.uResolution", true, 160, 1, 2, 0},
      }
    },
  }},
  {"drawLinesShader",  {
    {
      0, {
        {"_19.uLookAtMat", true, 0, 4, 4, 0},
        {"_19.uPMatrix", true, 64, 4, 4, 0},
      }
    },
    {
      1, {
        {"_19.uColor", true, 0, 1, 3, 0},
      }
    },
  }},
  {"drawFrustumShader",  {
    {
      0, {
        {"_13.uLookAtMat", true, 0, 4, 4, 0},
        {"_13.uPMatrix", true, 64, 4, 4, 0},
      }
    },
    {
      2, {
        {"_22.uColor", true, 0, 1, 3, 0},
      }
    },
  }},
  {"drawPoints",  {
    {
      0, {
        {"_19.uLookAtMat", true, 0, 4, 4, 0},
        {"_19.uPMatrix", true, 64, 4, 4, 0},
      }
    },
    {
      1, {
        {"_29.uPlacementMat", true, 0, 4, 4, 0},
      }
    },
  }},
  {"drawBBShader",  {
    {
      1, {
        {"_21.uPlacementMat", true, 0, 4, 4, 0},
        {"_21.uBBScale", true, 64, 1, 4, 0},
        {"_21.uBBCenter", true, 80, 1, 4, 0},
        {"_21.uColor", true, 96, 1, 4, 0},
      }
    },
    {
      0, {
        {"_59.uLookAtMat", true, 0, 4, 4, 0},
        {"_59.uPMatrix", true, 64, 4, 4, 0},
      }
    },
  }},
  {"drawDepthShader",  {
    {
      2, {
        {"_10.drawDepth", false, 0, 1, 1, 0},
        {"_10.uFarPlane", true, 4, 1, 1, 0},
        {"_10.uNearPlane", true, 8, 1, 1, 0},
      }
    },
  }},
  {"ffxgauss4",  {
    {
      4, {
        {"_33.texOffsetX", true, 0, 1, 4, 0},
        {"_33.texOffsetY", true, 16, 1, 4, 0},
      }
    },
  }},
  {"skyConus",  {
    {
      2, {
        {"_83.skyColor[0]", true, 0, 1, 4, 6},
      }
    },
    {
      0, {
        {"_51.scene.uLookAtMat", true, 0, 4, 4, 0},
        {"_51.scene.uPMatrix", true, 64, 4, 4, 0},
        {"_51.scene.uViewUp", true, 128, 1, 4, 0},
        {"_51.scene.uInteriorSunDir", true, 144, 1, 4, 0},
        {"_51.scene.extLight.uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_51.scene.extLight.uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_51.scene.extLight.uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_51.scene.extLight.uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_51.scene.extLight.uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_51.fogData.densityParams", true, 240, 1, 4, 0},
        {"_51.fogData.heightPlane", true, 256, 1, 4, 0},
        {"_51.fogData.color_and_heightRate", true, 272, 1, 4, 0},
        {"_51.fogData.heightDensity_and_endColor", true, 288, 1, 4, 0},
        {"_51.fogData.sunAngle_and_sunColor", true, 304, 1, 4, 0},
        {"_51.fogData.heightColor_and_endFogDistance", true, 320, 1, 4, 0},
        {"_51.fogData.sunPercentage", true, 336, 1, 4, 0},
      }
    },
  }},
  {"m2Shader",  {
    {
      2, {
        {"_230.vertexShader_IsAffectedByLight", false, 0, 1, 4, 0},
        {"_230.color_Transparency", true, 16, 1, 4, 0},
        {"_230.uTextMat[0]", true, 32, 4, 4, 2},
      }
    },
    {
      1, {
        {"_133.uPlacementMat", true, 0, 4, 4, 0},
        {"_133.uBoneMatrixes[0]", true, 64, 4, 4, 220},
      }
    },
    {
      0, {
        {"_240.scene.uLookAtMat", true, 0, 4, 4, 0},
        {"_240.scene.uPMatrix", true, 64, 4, 4, 0},
        {"_240.scene.uViewUp", true, 128, 1, 4, 0},
        {"_240.scene.uInteriorSunDir", true, 144, 1, 4, 0},
        {"_240.scene.extLight.uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_240.scene.extLight.uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_240.scene.extLight.uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_240.scene.extLight.uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_240.scene.extLight.uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_240.fogData.densityParams", true, 240, 1, 4, 0},
        {"_240.fogData.heightPlane", true, 256, 1, 4, 0},
        {"_240.fogData.color_and_heightRate", true, 272, 1, 4, 0},
        {"_240.fogData.heightDensity_and_endColor", true, 288, 1, 4, 0},
        {"_240.fogData.sunAngle_and_sunColor", true, 304, 1, 4, 0},
        {"_240.fogData.heightColor_and_endFogDistance", true, 320, 1, 4, 0},
        {"_240.fogData.sunPercentage", true, 336, 1, 4, 0},
      }
    },
    {
      3, {
        {"_484.intLight.uInteriorAmbientColorAndApplyInteriorLight", true, 0, 1, 4, 0},
        {"_484.intLight.uInteriorDirectColorAndApplyExteriorLight", true, 16, 1, 4, 0},
        {"_484.pc_lights[0].color", true, 32, 1, 4, 0},
        {"_484.pc_lights[0].position", true, 48, 1, 4, 0},
        {"_484.pc_lights[0].attenuation", true, 64, 1, 4, 0},
        {"_484.pc_lights[1].color", true, 80, 1, 4, 0},
        {"_484.pc_lights[1].position", true, 96, 1, 4, 0},
        {"_484.pc_lights[1].attenuation", true, 112, 1, 4, 0},
        {"_484.pc_lights[2].color", true, 128, 1, 4, 0},
        {"_484.pc_lights[2].position", true, 144, 1, 4, 0},
        {"_484.pc_lights[2].attenuation", true, 160, 1, 4, 0},
        {"_484.pc_lights[3].color", true, 176, 1, 4, 0},
        {"_484.pc_lights[3].position", true, 192, 1, 4, 0},
        {"_484.pc_lights[3].attenuation", true, 208, 1, 4, 0},
        {"_484.lightCountAndBcHack", false, 224, 1, 4, 0},
        {"_484.interiorExteriorBlend", true, 240, 1, 4, 0},
      }
    },
    {
      4, {
        {"_461.PixelShader_UnFogged_IsAffectedByLight_blendMode", false, 0, 1, 4, 0},
        {"_461.uFogColorAndAlphaTest", true, 16, 1, 4, 0},
        {"_461.uPcColor", true, 32, 1, 4, 0},
      }
    },
  }},
  {"adtShader",  {
    {
      2, {
        {"_139.uPos", true, 0, 1, 4, 0},
      }
    },
    {
      3, {
        {"_793.uFogStartAndFogEnd", true, 0, 1, 4, 0},
        {"_793.uFogColor", true, 16, 1, 4, 0},
      }
    },
    {
      0, {
        {"_91.scene.uLookAtMat", true, 0, 4, 4, 0},
        {"_91.scene.uPMatrix", true, 64, 4, 4, 0},
        {"_91.scene.uViewUp", true, 128, 1, 4, 0},
        {"_91.scene.uInteriorSunDir", true, 144, 1, 4, 0},
        {"_91.scene.extLight.uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_91.scene.extLight.uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_91.scene.extLight.uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_91.scene.extLight.uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_91.scene.extLight.uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_91.fogData.densityParams", true, 240, 1, 4, 0},
        {"_91.fogData.heightPlane", true, 256, 1, 4, 0},
        {"_91.fogData.color_and_heightRate", true, 272, 1, 4, 0},
        {"_91.fogData.heightDensity_and_endColor", true, 288, 1, 4, 0},
        {"_91.fogData.sunAngle_and_sunColor", true, 304, 1, 4, 0},
        {"_91.fogData.heightColor_and_endFogDistance", true, 320, 1, 4, 0},
        {"_91.fogData.sunPercentage", true, 336, 1, 4, 0},
      }
    },
    {
      4, {
        {"_439.uHeightScale", true, 0, 1, 4, 0},
        {"_439.uHeightOffset", true, 16, 1, 4, 0},
        {"_439.animationMat[0]", true, 32, 4, 4, 4},
      }
    },
  }},
  {"ffxglow",  {
    {
      4, {
        {"_34.blurAmount", true, 0, 1, 4, 0},
      }
    },
  }},
  {"adtLodShader",  {
    {
      0, {
        {"_55.uPos", true, 0, 1, 3, 0},
        {"_55.uLookAtMat", true, 16, 4, 4, 0},
        {"_55.uPMatrix", true, 80, 4, 4, 0},
      }
    },
  }},
};
#endif


#endif
