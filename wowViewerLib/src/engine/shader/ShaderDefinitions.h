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
struct renderFrameBufferShader {
    enum class Attribute {
        a_position = 0, renderFrameBufferShaderAttributeEnd
    };
};

struct m2Shader {
    enum class Attribute {
        aPosition = 0, aNormal = 1, bones = 2, boneWeights = 3, aTexCoord = 4, aTexCoord2 = 5, m2ShaderAttributeEnd
    };
};

struct wmoShader {
    enum class Attribute {
        aPosition = 0, aNormal = 1, aTexCoord = 2, aTexCoord2 = 3, aTexCoord3 = 4, aColor = 5, aColor2 = 6, wmoShaderAttributeEnd
    };
};

struct drawPortalShader {
    enum class Attribute {
        aPosition = 0, drawPortalShaderAttributeEnd
    };
};

struct m2ParticleShader {
    enum class Attribute {
        aPosition = 0, aColor = 1, aTexcoord0 = 2, aTexcoord1 = 3, aTexcoord2 = 4, m2ParticleShaderAttributeEnd
    };
};

struct adtShader {
    enum class Attribute {
        aHeight = 0, aColor = 1, aVertexLighting = 2, aNormal = 3, aIndex = 4, adtShaderAttributeEnd
    };
};

struct adtLodShader {
    enum class Attribute {
        aHeight = 0, aIndex = 1, adtLodShaderAttributeEnd
    };
};

struct ribbonShader {
    enum class Attribute {
        aPosition = 0, aColor = 1, aTexcoord0 = 2, ribbonShaderAttributeEnd
    };
};

struct drawDepthShader {
    enum class Attribute {
        position = 0, texture = 1, drawDepthShaderAttributeEnd
    };
};

struct drawLinesShader {
    enum class Attribute {
        aPosition = 0, drawLinesShaderAttributeEnd
    };
};

struct waterShader {
    enum class Attribute {
        aPosition = 0, waterShaderAttributeEnd
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

struct drawFrustumShader {
    enum class Attribute {
        aPosition = 0, drawFrustumShaderAttributeEnd
    };
};

std::string loadShader(std::string shaderName);
#ifdef SHADERDATACPP
const std::unordered_map<std::string, std::vector<attributeDefine>> attributesPerShaderName = {
{"renderFrameBufferShader",  {
{"a_position", 0},
}},{"m2Shader",  {
{"aPosition", 0},
{"aNormal", 1},
{"bones", 2},
{"boneWeights", 3},
{"aTexCoord", 4},
{"aTexCoord2", 5},
}},{"wmoShader",  {
{"aPosition", 0},
{"aNormal", 1},
{"aTexCoord", 2},
{"aTexCoord2", 3},
{"aTexCoord3", 4},
{"aColor", 5},
{"aColor2", 6},
}},{"drawPortalShader",  {
{"aPosition", 0},
}},{"m2ParticleShader",  {
{"aPosition", 0},
{"aColor", 1},
{"aTexcoord0", 2},
{"aTexcoord1", 3},
{"aTexcoord2", 4},
}},{"adtShader",  {
{"aHeight", 0},
{"aColor", 1},
{"aVertexLighting", 2},
{"aNormal", 3},
{"aIndex", 4},
}},{"adtLodShader",  {
{"aHeight", 0},
{"aIndex", 1},
}},{"ribbonShader",  {
{"aPosition", 0},
{"aColor", 1},
{"aTexcoord0", 2},
}},{"drawDepthShader",  {
{"position", 0},
{"texture", 1},
}},{"drawLinesShader",  {
{"aPosition", 0},
}},{"waterShader",  {
{"aPosition", 0},
}},{"drawBBShader",  {
{"aPosition", 0},
}},{"drawPoints",  {
{"aPosition", 0},
}},{"drawFrustumShader",  {
{"aPosition", 0},
}},};

const std::unordered_map<std::string, shaderMetaData> shaderMetaInfo = {{ "wmoShader.vert.spv", {
{
{0,1,64},
{0,0,128},
{0,2,16},
}
}
},{ "waterShader.frag.spv", {
{
{0,4,4},
}
}
},{ "ribbonShader.vert.spv", {
{
{0,0,128},
}
}
},{ "wmoShader.frag.spv", {
{
{0,4,112},
{0,3,64},
}
}
},{ "ribbonShader.frag.spv", {
{
}
}
},{ "m2Shader.vert.spv", {
{
{0,1,14144},
{0,2,160},
{0,0,128},
}
}
},{ "m2ParticleShader.frag.spv", {
{
{0,4,32},
}
}
},{ "drawLinesShader.frag.spv", {
{
{0,1,12},
}
}
},{ "drawBBShader.frag.spv", {
{
{0,0,112},
}
}
},{ "drawDepthShader.frag.spv", {
{
{0,2,12},
}
}
},{ "drawLinesShader.vert.spv", {
{
{0,0,128},
}
}
},{ "adtShader.frag.spv", {
{
{0,3,80},
{0,4,32},
}
}
},{ "renderFrameBufferShader.frag.spv", {
{
{0,2,168},
}
}
},{ "m2ParticleShader.vert.spv", {
{
{0,0,128},
}
}
},{ "adtLodShader.vert.spv", {
{
{0,0,144},
}
}
},{ "renderFrameBufferShader.vert.spv", {
{
}
}
},{ "m2Shader.frag.spv", {
{
{0,4,240},
{0,3,64},
{0,0,128},
{0,1,14144},
}
}
},{ "drawBBShader.vert.spv", {
{
{0,1,112},
{0,0,128},
}
}
},{ "adtShader.vert.spv", {
{
{0,0,128},
{0,2,16},
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
},{ "drawDepthShader.vert.spv", {
{
{0,2,16},
}
}
},{ "drawPoints.vert.spv", {
{
{0,0,128},
{0,1,64},
}
}
},{ "drawFrustumShader.frag.spv", {
{
{0,2,12},
}
}
},{ "drawFrustumShader.vert.spv", {
{
{0,0,128},
}
}
},{ "drawPortalShader.frag.spv", {
{
{0,1,16},
}
}
},{ "waterShader.vert.spv", {
{
{0,0,128},
{0,1,64},
}
}
},{ "drawPortalShader.vert.spv", {
{
{0,0,128},
{0,1,64},
}
}
},};

const  std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderName = {
  {"renderFrameBufferShader",  {
    {
      2, {
        {"_34.gauss_offsets[0]", true, 0, 1, 1, 5},
        {"_34.gauss_weights[0]", true, 80, 1, 1, 5},
        {"_34.uResolution", true, 160, 1, 2, 0},
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
        {"_237.uLookAtMat", true, 0, 4, 4, 0},
        {"_237.uPMatrix", true, 64, 4, 4, 0},
      }
    },
    {
      4, {
        {"_29.PixelShader_UnFogged_IsAffectedByLight_LightCount", false, 0, 1, 4, 0},
        {"_29.uFogColorAndAlphaTest", true, 16, 1, 4, 0},
        {"_29.pc_lights[0].color", true, 32, 1, 4, 0},
        {"_29.pc_lights[0].position", true, 48, 1, 4, 0},
        {"_29.pc_lights[0].attenuation", true, 64, 1, 4, 0},
        {"_29.pc_lights[1].color", true, 80, 1, 4, 0},
        {"_29.pc_lights[1].position", true, 96, 1, 4, 0},
        {"_29.pc_lights[1].attenuation", true, 112, 1, 4, 0},
        {"_29.pc_lights[2].color", true, 128, 1, 4, 0},
        {"_29.pc_lights[2].position", true, 144, 1, 4, 0},
        {"_29.pc_lights[2].attenuation", true, 160, 1, 4, 0},
        {"_29.pc_lights[3].color", true, 176, 1, 4, 0},
        {"_29.pc_lights[3].position", true, 192, 1, 4, 0},
        {"_29.pc_lights[3].attenuation", true, 208, 1, 4, 0},
        {"_29.uPcColor", true, 224, 1, 4, 0},
      }
    },
    {
      3, {
        {"_49.uViewUp", true, 0, 1, 4, 0},
        {"_49.uSunDirAndFogStart", true, 16, 1, 4, 0},
        {"_49.uSunColorAndFogEnd", true, 32, 1, 4, 0},
        {"_49.uAmbientLight", true, 48, 1, 4, 0},
      }
    },
  }},
  {"wmoShader",  {
    {
      2, {
        {"_179.VertexShader_UseLitColor", false, 0, 1, 4, 0},
      }
    },
    {
      0, {
        {"_108.uLookAtMat", true, 0, 4, 4, 0},
        {"_108.uPMatrix", true, 64, 4, 4, 0},
      }
    },
    {
      1, {
        {"_93.uPlacementMat", true, 0, 4, 4, 0},
      }
    },
    {
      4, {
        {"_21.uViewUp", true, 0, 1, 4, 0},
        {"_21.uSunDir_FogStart", true, 16, 1, 4, 0},
        {"_21.uSunColor_uFogEnd", true, 32, 1, 4, 0},
        {"_21.uAmbientLight", true, 48, 1, 4, 0},
        {"_21.uAmbientLight2AndIsBatchA", true, 64, 1, 4, 0},
        {"_21.UseLitColor_EnableAlpha_PixelShader", false, 80, 1, 4, 0},
        {"_21.FogColor_AlphaTest", true, 96, 1, 4, 0},
      }
    },
    {
      3, {
        {"_902.uPlacementMat", true, 0, 4, 4, 0},
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
  {"adtShader",  {
    {
      2, {
        {"_100.uPos", true, 0, 1, 4, 0},
      }
    },
    {
      0, {
        {"_53.uLookAtMat", true, 0, 4, 4, 0},
        {"_53.uPMatrix", true, 64, 4, 4, 0},
      }
    },
    {
      3, {
        {"_33.uViewUp", true, 0, 1, 4, 0},
        {"_33.uSunDir_FogStart", true, 16, 1, 4, 0},
        {"_33.uSunColor_uFogEnd", true, 32, 1, 4, 0},
        {"_33.uAmbientLight", true, 48, 1, 4, 0},
        {"_33.FogColor", true, 64, 1, 4, 0},
      }
    },
    {
      4, {
        {"_184.uHeightScale", true, 0, 1, 4, 0},
        {"_184.uHeightOffset", true, 16, 1, 4, 0},
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
  {"ribbonShader",  {
    {
      0, {
        {"_41.uLookAtMat", true, 0, 4, 4, 0},
        {"_41.uPMatrix", true, 64, 4, 4, 0},
      }
    },
  }},
  {"drawDepthShader",  {
    {
      2, {
        {"_36.uWidth", true, 0, 1, 1, 0},
        {"_36.uHeight", true, 4, 1, 1, 0},
        {"_36.uX", true, 8, 1, 1, 0},
        {"_36.uY", true, 12, 1, 1, 0},
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
  {"waterShader",  {
    {
      1, {
        {"_32.uPlacementMat", true, 0, 4, 4, 0},
      }
    },
    {
      4, {
        {"_9.waterType", false, 0, 1, 1, 0},
      }
    },
    {
      0, {
        {"_24.uLookAtMat", true, 0, 4, 4, 0},
        {"_24.uPMatrix", true, 64, 4, 4, 0},
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
};
#endif


#endif
