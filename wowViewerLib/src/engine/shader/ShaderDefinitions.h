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

struct waterShader {
    enum class Attribute {
        aPosition = 0, waterShaderAttributeEnd
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

struct adtShader {
    enum class Attribute {
        aHeight = 0, aColor = 1, aVertexLighting = 2, aNormal = 3, aIndex = 4, adtShaderAttributeEnd
    };
};

struct drawLinesShader {
    enum class Attribute {
        aPosition = 0, drawLinesShaderAttributeEnd
    };
};

struct adtWater {
    enum class Attribute {
        aPositionTransp = 0, aTexCoord = 1, adtWaterAttributeEnd
    };
};

struct drawFrustumShader {
    enum class Attribute {
        aPosition = 0, drawFrustumShaderAttributeEnd
    };
};

struct drawPortalShader {
    enum class Attribute {
        aPosition = 0, drawPortalShaderAttributeEnd
    };
};

struct adtLodShader {
    enum class Attribute {
        aHeight = 0, aIndex = 1, adtLodShaderAttributeEnd
    };
};

struct drawBBShader {
    enum class Attribute {
        aPosition = 0, drawBBShaderAttributeEnd
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

struct ribbonShader {
    enum class Attribute {
        aPosition = 0, aColor = 1, aTexcoord0 = 2, ribbonShaderAttributeEnd
    };
};

struct renderFrameBufferShader {
    enum class Attribute {
        a_position = 0, renderFrameBufferShaderAttributeEnd
    };
};

struct imguiShader {
    enum class Attribute {
        Position = 0, UV = 1, Color = 2, imguiShaderAttributeEnd
    };
};

struct m2ParticleShader {
    enum class Attribute {
        aPosition = 0, aColor = 1, aTexcoord0 = 2, aTexcoord1 = 3, aTexcoord2 = 4, m2ParticleShaderAttributeEnd
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
}},{"waterShader",  {
{"aPosition", 0},
}},{"skyConus",  {
{"aPosition", 0},
}},{"m2Shader",  {
{"aPosition", 0},
{"aNormal", 1},
{"bones", 2},
{"boneWeights", 3},
{"aTexCoord", 4},
{"aTexCoord2", 5},
}},{"adtShader",  {
{"aHeight", 0},
{"aColor", 1},
{"aVertexLighting", 2},
{"aNormal", 3},
{"aIndex", 4},
}},{"drawLinesShader",  {
{"aPosition", 0},
}},{"adtWater",  {
{"aPositionTransp", 0},
{"aTexCoord", 1},
}},{"drawFrustumShader",  {
{"aPosition", 0},
}},{"drawPortalShader",  {
{"aPosition", 0},
}},{"adtLodShader",  {
{"aHeight", 0},
{"aIndex", 1},
}},{"drawBBShader",  {
{"aPosition", 0},
}},{"drawPoints",  {
{"aPosition", 0},
}},{"drawQuad",  {
{"position", 0},
}},{"ribbonShader",  {
{"aPosition", 0},
{"aColor", 1},
{"aTexcoord0", 2},
}},{"renderFrameBufferShader",  {
{"a_position", 0},
}},{"imguiShader",  {
{"Position", 0},
{"UV", 1},
{"Color", 2},
}},{"m2ParticleShader",  {
{"aPosition", 0},
{"aColor", 1},
{"aTexcoord0", 2},
{"aTexcoord1", 3},
{"aTexcoord2", 4},
}},};

const std::unordered_map<std::string, shaderMetaData> shaderMetaInfo = {{ "wmoShader.vert.spv", {
{
{0,1,64},
{0,0,240},
{0,2,16},
}
}
},{ "waterShader.vert.spv", {
{
{0,0,128},
{0,1,64},
}
}
},{ "waterShader.frag.spv", {
{
{0,4,16},
}
}
},{ "skyConus.frag.spv", {
{
}
}
},{ "ribbonShader.frag.spv", {
{
}
}
},{ "renderFrameBufferShader.frag.spv", {
{
{0,2,168},
}
}
},{ "m2Shader.vert.spv", {
{
{0,1,14144},
{0,2,160},
{0,0,240},
}
}
},{ "m2Shader.frag.spv", {
{
{0,4,48},
{0,3,256},
{0,0,240},
{0,1,14144},
}
}
},{ "renderFrameBufferShader.vert.spv", {
{
}
}
},{ "m2ParticleShader.vert.spv", {
{
{0,0,128},
}
}
},{ "drawDepthShader.frag.spv", {
{
{0,2,12},
}
}
},{ "adtWater.vert.spv", {
{
{0,0,240},
}
}
},{ "drawQuad.vert.spv", {
{
{0,2,16},
}
}
},{ "m2ParticleShader.frag.spv", {
{
{0,4,32},
}
}
},{ "drawBBShader.vert.spv", {
{
{0,1,112},
{0,0,128},
}
}
},{ "drawFrustumShader.frag.spv", {
{
{0,2,12},
}
}
},{ "drawBBShader.frag.spv", {
{
{0,0,112},
}
}
},{ "adtLodShader.frag.spv", {
{
{0,0,84},
}
}
},{ "adtShader.frag.spv", {
{
{0,4,32},
{0,0,240},
{0,3,32},
}
}
},{ "adtLodShader.vert.spv", {
{
{0,0,144},
}
}
},{ "adtShader.vert.spv", {
{
{0,0,240},
{0,2,16},
}
}
},{ "ffxglow.frag.spv", {
{
{0,4,16},
}
}
},{ "imguiShader.frag.spv", {
{
}
}
},{ "adtWater.frag.spv", {
{
{0,4,16},
}
}
},{ "wmoShader.frag.spv", {
{
{0,4,32},
{0,0,240},
{0,3,32},
}
}
},{ "drawLinesShader.vert.spv", {
{
{0,0,128},
}
}
},{ "drawLinesShader.frag.spv", {
{
{0,1,12},
}
}
},{ "ribbonShader.vert.spv", {
{
{0,0,240},
}
}
},{ "ffxgauss4.frag.spv", {
{
{0,4,32},
}
}
},{ "skyConus.vert.spv", {
{
{0,0,240},
{0,2,96},
}
}
},{ "drawPoints.frag.spv", {
{
{0,1,12},
}
}
},{ "drawPoints.vert.spv", {
{
{0,0,128},
{0,1,64},
}
}
},{ "drawPortalShader.frag.spv", {
{
{0,1,16},
}
}
},{ "drawPortalShader.vert.spv", {
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
},{ "imguiShader.vert.spv", {
{
{0,1,64},
}
}
},};

const  std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderName = {
  {"wmoShader",  {
    {
      2, {
        {"_181.VertexShader_UseLitColor", false, 0, 1, 4, 0},
      }
    },
    {
      1, {
        {"_93.uPlacementMat", true, 0, 4, 4, 0},
      }
    },
    {
      3, {
        {"_693.intLight.uInteriorAmbientColorAndApplyInteriorLight", true, 0, 1, 4, 0},
        {"_693.intLight.uInteriorDirectColorAndApplyExteriorLight", true, 16, 1, 4, 0},
      }
    },
    {
      4, {
        {"_221.UseLitColor_EnableAlpha_PixelShader", false, 0, 1, 4, 0},
        {"_221.FogColor_AlphaTest", true, 16, 1, 4, 0},
      }
    },
    {
      0, {
        {"_110.scene.uLookAtMat", true, 0, 4, 4, 0},
        {"_110.scene.uPMatrix", true, 64, 4, 4, 0},
        {"_110.scene.uViewUp", true, 128, 1, 4, 0},
        {"_110.scene.uInteriorSunDir", true, 144, 1, 4, 0},
        {"_110.scene.extLight.uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_110.scene.extLight.uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_110.scene.extLight.uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_110.scene.extLight.uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_110.scene.extLight.uExteriorDirectColorDir", true, 224, 1, 4, 0},
      }
    },
  }},
  {"waterShader",  {
    {
      1, {
        {"_32.uPlacementMat", true, 0, 4, 4, 0},
      }
    },
    {
      4, {
        {"_12.waterTypeV", false, 0, 1, 4, 0},
      }
    },
    {
      0, {
        {"_24.uLookAtMat", true, 0, 4, 4, 0},
        {"_24.uPMatrix", true, 64, 4, 4, 0},
      }
    },
  }},
  {"skyConus",  {
    {
      2, {
        {"_92.skyColor[0]", true, 0, 1, 4, 6},
      }
    },
    {
      0, {
        {"_52.scene.uLookAtMat", true, 0, 4, 4, 0},
        {"_52.scene.uPMatrix", true, 64, 4, 4, 0},
        {"_52.scene.uViewUp", true, 128, 1, 4, 0},
        {"_52.scene.uInteriorSunDir", true, 144, 1, 4, 0},
        {"_52.scene.extLight.uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_52.scene.extLight.uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_52.scene.extLight.uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_52.scene.extLight.uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_52.scene.extLight.uExteriorDirectColorDir", true, 224, 1, 4, 0},
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
        {"_239.scene.uLookAtMat", true, 0, 4, 4, 0},
        {"_239.scene.uPMatrix", true, 64, 4, 4, 0},
        {"_239.scene.uViewUp", true, 128, 1, 4, 0},
        {"_239.scene.uInteriorSunDir", true, 144, 1, 4, 0},
        {"_239.scene.extLight.uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_239.scene.extLight.uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_239.scene.extLight.uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_239.scene.extLight.uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_239.scene.extLight.uExteriorDirectColorDir", true, 224, 1, 4, 0},
      }
    },
    {
      4, {
        {"_247.PixelShader_UnFogged_IsAffectedByLight", false, 0, 1, 4, 0},
        {"_247.uFogColorAndAlphaTest", true, 16, 1, 4, 0},
        {"_247.uPcColor", true, 32, 1, 4, 0},
      }
    },
    {
      3, {
        {"_272.intLight.uInteriorAmbientColorAndApplyInteriorLight", true, 0, 1, 4, 0},
        {"_272.intLight.uInteriorDirectColorAndApplyExteriorLight", true, 16, 1, 4, 0},
        {"_272.pc_lights[0].color", true, 32, 1, 4, 0},
        {"_272.pc_lights[0].position", true, 48, 1, 4, 0},
        {"_272.pc_lights[0].attenuation", true, 64, 1, 4, 0},
        {"_272.pc_lights[1].color", true, 80, 1, 4, 0},
        {"_272.pc_lights[1].position", true, 96, 1, 4, 0},
        {"_272.pc_lights[1].attenuation", true, 112, 1, 4, 0},
        {"_272.pc_lights[2].color", true, 128, 1, 4, 0},
        {"_272.pc_lights[2].position", true, 144, 1, 4, 0},
        {"_272.pc_lights[2].attenuation", true, 160, 1, 4, 0},
        {"_272.pc_lights[3].color", true, 176, 1, 4, 0},
        {"_272.pc_lights[3].position", true, 192, 1, 4, 0},
        {"_272.pc_lights[3].attenuation", true, 208, 1, 4, 0},
        {"_272.lightCount", false, 224, 1, 4, 0},
        {"_272.interiorExteriorBlend", true, 240, 1, 4, 0},
      }
    },
  }},
  {"m2ParticleShader",  {
    {
      0, {
        {"_47.uLookAtMat", true, 0, 4, 4, 0},
        {"_47.uPMatrix", true, 64, 4, 4, 0},
      }
    },
    {
      4, {
        {"_38.uAlphaTestv", true, 0, 1, 4, 0},
        {"_38.uPixelShaderv", false, 16, 1, 4, 0},
      }
    },
  }},
  {"ribbonShader",  {
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
  {"imguiShader",  {
    {
      1, {
        {"_30.ProjMtx", true, 0, 4, 4, 0},
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
  {"adtWater",  {
    {
      0, {
        {"_27.scene.uLookAtMat", true, 0, 4, 4, 0},
        {"_27.scene.uPMatrix", true, 64, 4, 4, 0},
        {"_27.scene.uViewUp", true, 128, 1, 4, 0},
        {"_27.scene.uInteriorSunDir", true, 144, 1, 4, 0},
        {"_27.scene.extLight.uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_27.scene.extLight.uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_27.scene.extLight.uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_27.scene.extLight.uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_27.scene.extLight.uExteriorDirectColorDir", true, 224, 1, 4, 0},
      }
    },
    {
      4, {
        {"_12.color", true, 0, 1, 4, 0},
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
  {"adtShader",  {
    {
      2, {
        {"_130.uPos", true, 0, 1, 4, 0},
      }
    },
    {
      3, {
        {"_515.uFogStartAndFogEnd", true, 0, 1, 4, 0},
        {"_515.uFogColor", true, 16, 1, 4, 0},
      }
    },
    {
      4, {
        {"_252.uHeightScale", true, 0, 1, 4, 0},
        {"_252.uHeightOffset", true, 16, 1, 4, 0},
      }
    },
    {
      0, {
        {"_90.scene.uLookAtMat", true, 0, 4, 4, 0},
        {"_90.scene.uPMatrix", true, 64, 4, 4, 0},
        {"_90.scene.uViewUp", true, 128, 1, 4, 0},
        {"_90.scene.uInteriorSunDir", true, 144, 1, 4, 0},
        {"_90.scene.extLight.uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_90.scene.extLight.uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_90.scene.extLight.uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_90.scene.extLight.uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_90.scene.extLight.uExteriorDirectColorDir", true, 224, 1, 4, 0},
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
};
#endif


#endif
