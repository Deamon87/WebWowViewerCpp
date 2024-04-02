#ifndef WOWMAPVIEWERREVIVED_SHADERDEFINITIONS_H
#define WOWMAPVIEWERREVIVED_SHADERDEFINITIONS_H

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>

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

    constexpr const int MAX_SHADER_DESC_SETS = 8;

    enum class ShaderStage {
        Unk, Vertex, Fragment, RayGenerate, RayAnyHit, RayClosestHit, RayMiss
    };


    struct uboBindingData {
        unsigned int set;
        unsigned int binding;
        unsigned long long size;
    };
    struct ssboBindingData {
        unsigned int set;
        unsigned int binding;
        unsigned long long size;
    };
    struct imageBindingData {
        unsigned int set;
        unsigned int binding;
        std::string imageName;
    };

    struct bindingAmountData {
        unsigned int start = 999;
        unsigned int end = 0;
        unsigned int length = 0;
    };

    struct shaderMetaData {
        ShaderStage stage;

        std::vector<uboBindingData> uboBindings;
        std::array<bindingAmountData, MAX_SHADER_DESC_SETS> uboBindingAmountsPerSet;

        std::vector<ssboBindingData> ssboBindingData;

        std::vector<imageBindingData> imageBindings;
        std::array<bindingAmountData, MAX_SHADER_DESC_SETS> imageBindingAmountsPerSet;
    };

    //Per file
    extern const std::unordered_map<std::string, shaderMetaData> shaderMetaInfo;
    extern const std::unordered_map<std::string, std::vector<attributeDefine>> attributesPerShaderName;

    extern const std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameVert;

    extern const std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameFrag;

struct drawQuad {
    enum class Attribute {
        position = 0, drawQuadAttributeEnd
    };
};

struct renderFrameBufferShader {
    enum class Attribute {
        a_position = 0, renderFrameBufferShaderAttributeEnd
    };
};

struct drawPortalShader {
    enum class Attribute {
        aPosition = 0, drawPortalShaderAttributeEnd
    };
};

struct adtShader {
    enum class Attribute {
        aPos = 0, aColor = 1, aVertexLighting = 2, aNormal = 3, adtShaderAttributeEnd
    };
};

struct pointLight {
    enum class Attribute {
        position = 0, pointLightAttributeEnd
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

struct imguiShader {
    enum class Attribute {
        Position = 0, UV = 1, Color = 2, imguiShaderAttributeEnd
    };
};

struct wmoShader {
    enum class Attribute {
        aPosition = 0, aNormal = 1, aTexCoord = 2, aTexCoord2 = 3, aTexCoord3 = 4, aTexCoord4 = 5, aColor = 6, aColor2 = 7, aColorSecond = 8, wmoAmbient = 9, wmoShaderAttributeEnd
    };
};

struct drawBBShader {
    enum class Attribute {
        aPosition = 0, drawBBShaderAttributeEnd
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

struct m2ParticleShader {
    enum class Attribute {
        aPosition = 0, aColor = 1, aTexcoord0 = 2, aTexcoord1 = 3, aTexcoord2 = 4, aAlphaCutoff = 5, m2ParticleShaderAttributeEnd
    };
};

struct waterShader {
    enum class Attribute {
        aPositionTransp = 0, aTexCoord = 1, waterShaderAttributeEnd
    };
};

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

std::string loadShader(std::string shaderName);
#ifdef SHADERDATACPP
const std::unordered_map<std::string, std::vector<attributeDefine>> attributesPerShaderName = {
{ "drawQuad",
  {
    { "position", 0},
  }
},
{ "renderFrameBufferShader",
  {
    { "a_position", 0},
  }
},
{ "drawPortalShader",
  {
    { "aPosition", 0},
  }
},
{ "adtShader",
  {
    { "aPos", 0},
    { "aColor", 1},
    { "aVertexLighting", 2},
    { "aNormal", 3},
  }
},
{ "pointLight",
  {
    { "position", 0},
  }
},
{ "skyConus",
  {
    { "aPosition", 0},
  }
},
{ "m2Shader",
  {
    { "aPosition", 0},
    { "aNormal", 1},
    { "bones", 2},
    { "boneWeights", 3},
    { "aTexCoord", 4},
    { "aTexCoord2", 5},
  }
},
{ "imguiShader",
  {
    { "Position", 0},
    { "UV", 1},
    { "Color", 2},
  }
},
{ "wmoShader",
  {
    { "aPosition", 0},
    { "aNormal", 1},
    { "aTexCoord", 2},
    { "aTexCoord2", 3},
    { "aTexCoord3", 4},
    { "aTexCoord4", 5},
    { "aColor", 6},
    { "aColor2", 7},
    { "aColorSecond", 8},
    { "wmoAmbient", 9},
  }
},
{ "drawBBShader",
  {
    { "aPosition", 0},
  }
},
{ "drawLinesShader",
  {
    { "aPosition", 0},
  }
},
{ "ribbonShader",
  {
    { "aPosition", 0},
    { "aColor", 1},
    { "aTexcoord0", 2},
  }
},
{ "m2ParticleShader",
  {
    { "aPosition", 0},
    { "aColor", 1},
    { "aTexcoord0", 2},
    { "aTexcoord1", 3},
    { "aTexcoord2", 4},
    { "aAlphaCutoff", 5},
  }
},
{ "waterShader",
  {
    { "aPositionTransp", 0},
    { "aTexCoord", 1},
  }
},
{ "waterfallShader",
  {
    { "aPosition", 0},
    { "aNormal", 1},
    { "bones", 2},
    { "boneWeights", 3},
    { "aTexCoord", 4},
    { "aTexCoord2", 5},
  }
},
{ "adtLodShader",
  {
    { "aHeight", 0},
    { "aIndex", 1},
  }
},
{ "drawFrustumShader",
  {
    { "aPosition", 0},
  }
},
{ "drawPoints",
  {
    { "aPosition", 0},
  }
},
};

const std::unordered_map<std::string, shaderMetaData> shaderMetaInfo = { 
{ "./forwardRendering/wmoShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {1,0,64},
      {0,0,560},
      {1,1,16},
    },
    {
      {
        {0,0,1},
        {0,1,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/wmoShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {1,2,32},
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {2,2,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {2,0, "uTexture"},
      {2,1, "uTexture2"},
      {2,2, "uTexture3"},
      {2,3, "uTexture4"},
      {2,4, "uTexture5"},
      {2,5, "uTexture6"},
      {2,6, "uTexture7"},
      {2,7, "uTexture8"},
      {2,8, "uTexture9"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,8,9},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/waterfallShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {2,0,112},
      {1,5,4096},
      {1,2,16384},
      {0,0,560},
      {1,0,64},
      {1,1,384},
      {1,3,4096},
      {1,4,256},
    },
    {
      {
        {0,0,1},
        {0,5,6},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {3,3, "uBumpTexture"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {3,3,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/waterShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,560},
      {1,0,64},
    },
    {
      {
        {0,0,1},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/waterShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {1,1,48},
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {1,1,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {2,0, "uTexture"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/adtLodShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,144},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/m2Shader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {1,2,16384},
      {1,0,64},
      {0,0,560},
      {1,1,384},
      {1,3,4096},
      {1,4,256},
      {1,5,4096},
      {2,0,64},
    },
    {
      {
        {0,0,1},
        {0,5,6},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/wmo/forward/wmoShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {1,6,0},
      {1,4,0},
      {1,3,0},
      {1,5,0},
      {1,1,0},
      {1,2,0},
    },
    {
      {2,0, "s_Textures"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/m2ParticleShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/wmo/deferred/wmoShader_opaq.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {1,6,0},
      {1,4,0},
      {1,3,0},
      {1,5,0},
      {1,1,0},
      {1,2,0},
    },
    {
      {2,0, "s_Textures"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/adt/forward/adtShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {1,3,0},
      {1,1,0},
      {1,2,0},
    },
    {
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/wmo/deferred/wmoShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {1,6,0},
      {1,4,0},
      {1,3,0},
      {1,5,0},
      {1,1,0},
      {1,2,0},
    },
    {
      {2,0, "s_Textures"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/drawQuad.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,16},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/wmo/forward/wmoShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {1,6,0},
      {1,4,0},
      {1,1,0},
      {1,2,0},
      {1,3,0},
      {1,5,0},
    },
    {
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/waterfall/forward/waterfallShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {2,1,0},
      {2,0,0},
      {1,1,0},
      {1,2,0},
      {1,3,0},
      {1,4,0},
      {1,5,0},
      {1,6,0},
      {1,7,0},
      {1,8,0},
      {1,9,0},
    },
    {
      {3,0, "s_Textures"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/waterfall/deferred/waterFallShader_opaq.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {2,1,0},
      {2,0,0},
      {1,1,0},
      {1,2,0},
      {1,3,0},
      {1,4,0},
      {1,5,0},
      {1,6,0},
      {1,7,0},
      {1,8,0},
      {1,9,0},
    },
    {
      {3,0, "s_Textures"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/m2Ribbon/forward/ribbonShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/m2Ribbon/forward/ribbonShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {1,1,16},
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {1,1,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {1,0,4096},
    },
    {
      {2,0, "uTexture"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/adt/deferred/adtShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {1,3,0},
      {1,1,0},
      {1,2,0},
    },
    {
      {2,0, "s_LayerTextures"},
      {4,0, "s_LayerHeightTextures"},
      {3,0, "s_AlphaTextures"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,1},
        {0,0,1},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/ribbonShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/wmo/forward/wmoShader_opaq.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {1,6,0},
      {1,4,0},
      {1,3,0},
      {1,5,0},
      {1,1,0},
      {1,2,0},
    },
    {
      {2,0, "s_Textures"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/m2/deferred/m2Shader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {1,9,0},
      {1,8,0},
      {1,7,0},
      {1,2,0},
      {1,5,0},
      {1,4,0},
      {1,6,0},
      {1,1,0},
      {1,3,0},
    },
    {
      {2,0, "s_Textures"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/adtShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,0,560},
      {1,1,48},
      {1,0,64},
    },
    {
      {
        {0,0,1},
        {0,1,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {2,0, "uLayer0"},
      {2,1, "uLayer1"},
      {2,2, "uLayer2"},
      {2,3, "uLayer3"},
      {2,5, "uLayerHeight0"},
      {2,6, "uLayerHeight1"},
      {2,7, "uLayerHeight2"},
      {2,8, "uLayerHeight3"},
      {2,4, "uAlphaTexture"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,8,9},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/drawDepthShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,2,12},
    },
    {
      {
        {2,2,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {0,3, "diffuse"},
    },
    {
      {
        {3,3,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/ffxgauss4.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,1,32},
    },
    {
      {
        {1,1,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {1,0, "texture0"},
    },
    {
      {
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/m2/forward/m2Shader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {1,9,0},
      {1,7,0},
      {1,3,0},
      {1,1,0},
      {1,2,0},
      {1,4,0},
      {1,5,0},
      {1,6,0},
      {1,8,0},
    },
    {
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/m2Particle/forward/m2ParticleShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/adtLodShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,0,84},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {0,4, "uDiffuseTexture"},
      {0,5, "uNormalTexture"},
    },
    {
      {
        {4,5,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/waterfall/forward/waterfallShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {2,1,0},
      {1,7,0},
      {2,0,0},
      {1,6,0},
      {1,3,0},
      {1,1,0},
      {1,2,0},
      {1,4,0},
      {1,5,0},
      {1,8,0},
      {1,9,0},
    },
    {
      {3,0, "s_Textures"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/m2/forward/m2Shader_opaq.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {1,9,0},
      {1,8,0},
      {1,7,0},
      {1,2,0},
      {1,5,0},
      {1,4,0},
      {1,6,0},
      {1,1,0},
      {1,3,0},
    },
    {
      {2,0, "s_Textures"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/waterfallShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {2,0,112},
      {0,0,560},
      {1,0,64},
      {1,1,384},
      {1,2,16384},
      {1,3,4096},
      {1,4,256},
      {1,5,4096},
    },
    {
      {
        {0,0,1},
        {0,5,6},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {3,4, "uNormalTex"},
      {3,2, "uNoise"},
      {3,1, "uWhiteWater"},
      {3,0, "uMask"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,4,5},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/adt/visbuffer/adtShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {1,3,0},
      {1,1,0},
      {1,2,0},
    },
    {
      {2,0, "s_LayerTextures"},
      {4,0, "s_LayerHeightTextures"},
      {3,0, "s_AlphaTextures"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,1},
        {0,0,1},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/adt/visbuffer/adtShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {1,3,0},
      {1,1,0},
      {1,2,0},
    },
    {
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/lights/pointLight.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {1,3,16},
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {3,3,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {1,0,0},
    },
    {
      {1,2, "normalTex"},
      {1,1, "depthTex"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/m2/deferred/m2Shader_opaq.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {1,9,0},
      {1,8,0},
      {1,7,0},
      {1,2,0},
      {1,5,0},
      {1,4,0},
      {1,6,0},
      {1,1,0},
      {1,3,0},
    },
    {
      {2,0, "s_Textures"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/drawPoints.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,1,12},
    },
    {
      {
        {1,1,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/skyConus.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,560},
      {1,0,96},
    },
    {
      {
        {0,0,1},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/lights/pointLight.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {1,0,0},
    },
    {
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/m2/forward/m2Shader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {1,9,0},
      {1,8,0},
      {1,7,0},
      {1,2,0},
      {1,5,0},
      {1,4,0},
      {1,6,0},
      {1,1,0},
      {1,3,0},
    },
    {
      {2,0, "s_Textures"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/m2Particle/deferred/m2ParticleShader_opaq_deferred.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {1,0,32},
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {2,0, "uTexture"},
      {2,1, "uTexture2"},
      {2,2, "uTexture3"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,2,3},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/ffxglow.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,1,16},
    },
    {
      {
        {1,1,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {1,0, "screenTex"},
      {1,1, "blurTex"},
    },
    {
      {
        {0,0,0},
        {0,1,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/water/waterShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {1,2,0},
      {1,0,0},
      {1,1,0},
    },
    {
      {2,0, "s_Textures"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/water/waterShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {1,2,0},
      {1,1,0},
      {1,0,0},
    },
    {
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
      {2,0, "s_Textures"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/ribbonShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {1,1,16},
      {1,0,4096},
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,1,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {2,0, "uTexture"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/renderFrameBufferShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/adt/forward/adtShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
      {1,3,0},
      {1,1,0},
      {1,2,0},
    },
    {
      {2,0, "s_LayerTextures"},
      {4,0, "s_LayerHeightTextures"},
      {3,0, "s_AlphaTextures"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,1},
        {0,0,1},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/adtShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {1,0,64},
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/drawPortalShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {1,0,16},
    },
    {
      {
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/imguiShaderDepth.frag.spv", 
  {
    ShaderStage::Fragment,
    {
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {1,0, "Texture"},
    },
    {
      {
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/drawBBShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,1,112},
    },
    {
      {
        {1,1,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/drawBBShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,1,112},
      {0,0,560},
    },
    {
      {
        {0,1,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/drawPoints.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,128},
      {0,1,64},
    },
    {
      {
        {0,1,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/drawFrustumShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,2,12},
    },
    {
      {
        {2,2,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/drawFrustumShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,128},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/drawLinesShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,1,12},
    },
    {
      {
        {1,1,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/imguiShader_opaque.frag.spv", 
  {
    ShaderStage::Fragment,
    {
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {1,0, "Texture"},
    },
    {
      {
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/drawLinesShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,128},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/drawPortalShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/m2ParticleShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {1,0,32},
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {2,0, "uTexture"},
      {2,1, "uTexture2"},
      {2,2, "uTexture3"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,2,3},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./bindless/m2Particle/forward/m2ParticleShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {1,0,32},
      {0,0,560},
    },
    {
      {
        {0,0,1},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {2,0, "uTexture"},
      {2,1, "uTexture2"},
      {2,2, "uTexture3"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,2,3},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/imguiShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {1,0, "Texture"},
    },
    {
      {
        {0,0,0},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/skyConus.frag.spv", 
  {
    ShaderStage::Fragment,
    {
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/imguiShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,80},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/renderFrameBufferShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,2,168},
    },
    {
      {
        {2,2,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {0,3, "u_sampler"},
      {0,4, "u_depth"},
    },
    {
      {
        {3,4,2},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "./forwardRendering/m2Shader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {2,0,64},
      {1,4,256},
      {1,3,4096},
      {1,1,384},
      {0,0,560},
      {1,0,64},
      {1,5,4096},
      {1,2,16384},
    },
    {
      {
        {0,0,1},
        {0,5,6},
        {0,0,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    },
    {
    },
    {
      {3,0, "uTexture"},
      {3,1, "uTexture2"},
      {3,2, "uTexture3"},
      {3,3, "uTexture4"},
      {0,1, "lightBuffer"},
      {0,2, "aoBuffer"},
    },
    {
      {
        {1,2,2},
        {0,0,0},
        {0,0,0},
        {0,3,4},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
};

const  std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameVert = {
  {"drawQuad",  {
    {
      0, {
        {"_0_0_uWidth_uHeight_uX_uY", true, 0, 1, 4, 0},
      }
    },
  }},
  {"renderFrameBufferShader",  {
  }},
  {"drawPortalShader",  {
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_uSceneSize_DisableLightBuffer", true, 160, 1, 4, 0},
        {"_0_0_scene_closeRiverColor", true, 176, 1, 4, 0},
        {"_0_0_scene_farRiverColor", true, 192, 1, 4, 0},
        {"_0_0_scene_closeOceanColor", true, 208, 1, 4, 0},
        {"_0_0_scene_farOceanColor", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 240, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 256, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 272, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 288, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 304, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 320, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 336, 1, 4, 0},
        {"_0_0_fogData_classicFogParams", true, 352, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 368, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 384, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 400, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 416, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 432, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 448, 1, 4, 0},
        {"_0_0_fogData_sunDirection_and_fogZScalar", true, 464, 1, 4, 0},
        {"_0_0_fogData_heightFogCoeff", true, 480, 1, 4, 0},
        {"_0_0_fogData_mainFogCoeff", true, 496, 1, 4, 0},
        {"_0_0_fogData_heightDensityFogCoeff", true, 512, 1, 4, 0},
        {"_0_0_fogData_mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 528, 1, 4, 0},
        {"_0_0_fogData_heightFogEndColor_fogStartOffset", true, 544, 1, 4, 0},
      }
    },
  }},
  {"adtShader",  {
    {
      2, {
      }
    },
    {
      1, {
      }
    },
    {
      3, {
      }
    },
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_uSceneSize_DisableLightBuffer", true, 160, 1, 4, 0},
        {"_0_0_scene_closeRiverColor", true, 176, 1, 4, 0},
        {"_0_0_scene_farRiverColor", true, 192, 1, 4, 0},
        {"_0_0_scene_closeOceanColor", true, 208, 1, 4, 0},
        {"_0_0_scene_farOceanColor", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 240, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 256, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 272, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 288, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 304, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 320, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 336, 1, 4, 0},
        {"_0_0_fogData_classicFogParams", true, 352, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 368, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 384, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 400, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 416, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 432, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 448, 1, 4, 0},
        {"_0_0_fogData_sunDirection_and_fogZScalar", true, 464, 1, 4, 0},
        {"_0_0_fogData_heightFogCoeff", true, 480, 1, 4, 0},
        {"_0_0_fogData_mainFogCoeff", true, 496, 1, 4, 0},
        {"_0_0_fogData_heightDensityFogCoeff", true, 512, 1, 4, 0},
        {"_0_0_fogData_mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 528, 1, 4, 0},
        {"_0_0_fogData_heightFogEndColor_fogStartOffset", true, 544, 1, 4, 0},
      }
    },
  }},
  {"pointLight",  {
    {
      0, {
      }
    },
  }},
  {"skyConus",  {
    {
      0, {
        {"_1_0_skyColor[0]", true, 0, 1, 4, 6},
      }
    },
  }},
  {"m2Shader",  {
    {
      8, {
      }
    },
    {
      6, {
        {"_1_6_textureMatrix", true, 0, 4, 4, 0},
      }
    },
    {
      5, {
        {"_1_5_textureMatrix[0]", true, 0, 4, 4, 64},
      }
    },
    {
      4, {
        {"_1_4_textureWeight[0]", true, 0, 1, 4, 16},
      }
    },
    {
      2, {
        {"_1_2_uBoneMatrixes[0]", true, 0, 4, 4, 256},
      }
    },
    {
      1, {
        {"_1_1_intLight_uInteriorAmbientColorAndApplyInteriorLight", true, 0, 1, 4, 0},
        {"_1_1_intLight_uInteriorDirectColorAndApplyExteriorLight", true, 16, 1, 4, 0},
        {"_1_1_pc_lights[0].innerColor", true, 32, 1, 4, 0},
        {"_1_1_pc_lights[0].outerColor", true, 48, 1, 4, 0},
        {"_1_1_pc_lights[0].position", true, 64, 1, 4, 0},
        {"_1_1_pc_lights[0].attenuation", true, 80, 1, 4, 0},
        {"_1_1_pc_lights[0].blendParams", true, 96, 1, 4, 0},
        {"_1_1_pc_lights[1].innerColor", true, 112, 1, 4, 0},
        {"_1_1_pc_lights[1].outerColor", true, 128, 1, 4, 0},
        {"_1_1_pc_lights[1].position", true, 144, 1, 4, 0},
        {"_1_1_pc_lights[1].attenuation", true, 160, 1, 4, 0},
        {"_1_1_pc_lights[1].blendParams", true, 176, 1, 4, 0},
        {"_1_1_pc_lights[2].innerColor", true, 192, 1, 4, 0},
        {"_1_1_pc_lights[2].outerColor", true, 208, 1, 4, 0},
        {"_1_1_pc_lights[2].position", true, 224, 1, 4, 0},
        {"_1_1_pc_lights[2].attenuation", true, 240, 1, 4, 0},
        {"_1_1_pc_lights[2].blendParams", true, 256, 1, 4, 0},
        {"_1_1_pc_lights[3].innerColor", true, 272, 1, 4, 0},
        {"_1_1_pc_lights[3].outerColor", true, 288, 1, 4, 0},
        {"_1_1_pc_lights[3].position", true, 304, 1, 4, 0},
        {"_1_1_pc_lights[3].attenuation", true, 320, 1, 4, 0},
        {"_1_1_pc_lights[3].blendParams", true, 336, 1, 4, 0},
        {"_1_1_lightCountAndBcHack", false, 352, 1, 4, 0},
        {"_1_1_interiorExteriorBlend", true, 368, 1, 4, 0},
      }
    },
    {
      3, {
        {"_1_3_colors[0]", true, 0, 1, 4, 256},
      }
    },
    {
      7, {
      }
    },
    {
      9, {
      }
    },
    {
      0, {
        {"_2_0_vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2", false, 0, 1, 4, 0},
        {"_2_0_PixelShader_UnFogged_blendMode", false, 16, 1, 4, 0},
        {"_2_0_textureWeightIndexes", false, 32, 1, 4, 0},
        {"_2_0_colorIndex_applyWeight", false, 48, 1, 4, 0},
      }
    },
  }},
  {"imguiShader",  {
    {
      0, {
        {"_0_0_ProjMtx", true, 0, 4, 4, 0},
        {"_0_0_uiScale", true, 64, 1, 4, 0},
      }
    },
  }},
  {"wmoShader",  {
    {
      5, {
        {"_1_5_s_wmoAmbient", true, 0, 1, 4, 0},
      }
    },
    {
      3, {
      }
    },
    {
      2, {
        {"_1_2_VertexShader_UseLitColors", false, 0, 1, 4, 0},
      }
    },
    {
      1, {
        {"_1_1_VertexShader_UseLitColor", false, 0, 1, 4, 0},
      }
    },
    {
      4, {
      }
    },
    {
      6, {
      }
    },
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_uSceneSize_DisableLightBuffer", true, 160, 1, 4, 0},
        {"_0_0_scene_closeRiverColor", true, 176, 1, 4, 0},
        {"_0_0_scene_farRiverColor", true, 192, 1, 4, 0},
        {"_0_0_scene_closeOceanColor", true, 208, 1, 4, 0},
        {"_0_0_scene_farOceanColor", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 240, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 256, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 272, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 288, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 304, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 320, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 336, 1, 4, 0},
        {"_0_0_fogData_classicFogParams", true, 352, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 368, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 384, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 400, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 416, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 432, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 448, 1, 4, 0},
        {"_0_0_fogData_sunDirection_and_fogZScalar", true, 464, 1, 4, 0},
        {"_0_0_fogData_heightFogCoeff", true, 480, 1, 4, 0},
        {"_0_0_fogData_mainFogCoeff", true, 496, 1, 4, 0},
        {"_0_0_fogData_heightDensityFogCoeff", true, 512, 1, 4, 0},
        {"_0_0_fogData_mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 528, 1, 4, 0},
        {"_0_0_fogData_heightFogEndColor_fogStartOffset", true, 544, 1, 4, 0},
      }
    },
  }},
  {"drawBBShader",  {
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_uSceneSize_DisableLightBuffer", true, 160, 1, 4, 0},
        {"_0_0_scene_closeRiverColor", true, 176, 1, 4, 0},
        {"_0_0_scene_farRiverColor", true, 192, 1, 4, 0},
        {"_0_0_scene_closeOceanColor", true, 208, 1, 4, 0},
        {"_0_0_scene_farOceanColor", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 240, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 256, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 272, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 288, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 304, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 320, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 336, 1, 4, 0},
        {"_0_0_fogData_classicFogParams", true, 352, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 368, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 384, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 400, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 416, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 432, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 448, 1, 4, 0},
        {"_0_0_fogData_sunDirection_and_fogZScalar", true, 464, 1, 4, 0},
        {"_0_0_fogData_heightFogCoeff", true, 480, 1, 4, 0},
        {"_0_0_fogData_mainFogCoeff", true, 496, 1, 4, 0},
        {"_0_0_fogData_heightDensityFogCoeff", true, 512, 1, 4, 0},
        {"_0_0_fogData_mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 528, 1, 4, 0},
        {"_0_0_fogData_heightFogEndColor_fogStartOffset", true, 544, 1, 4, 0},
      }
    },
    {
      1, {
        {"_0_1_uPlacementMat", true, 0, 4, 4, 0},
        {"_0_1_uBBScale", true, 64, 1, 4, 0},
        {"_0_1_uBBCenter", true, 80, 1, 4, 0},
        {"_0_1_uColor", true, 96, 1, 4, 0},
      }
    },
  }},
  {"drawLinesShader",  {
    {
      0, {
        {"_0_0_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_uPMatrix", true, 64, 4, 4, 0},
      }
    },
  }},
  {"ribbonShader",  {
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_uSceneSize_DisableLightBuffer", true, 160, 1, 4, 0},
        {"_0_0_scene_closeRiverColor", true, 176, 1, 4, 0},
        {"_0_0_scene_farRiverColor", true, 192, 1, 4, 0},
        {"_0_0_scene_closeOceanColor", true, 208, 1, 4, 0},
        {"_0_0_scene_farOceanColor", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 240, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 256, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 272, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 288, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 304, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 320, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 336, 1, 4, 0},
        {"_0_0_fogData_classicFogParams", true, 352, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 368, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 384, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 400, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 416, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 432, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 448, 1, 4, 0},
        {"_0_0_fogData_sunDirection_and_fogZScalar", true, 464, 1, 4, 0},
        {"_0_0_fogData_heightFogCoeff", true, 480, 1, 4, 0},
        {"_0_0_fogData_mainFogCoeff", true, 496, 1, 4, 0},
        {"_0_0_fogData_heightDensityFogCoeff", true, 512, 1, 4, 0},
        {"_0_0_fogData_mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 528, 1, 4, 0},
        {"_0_0_fogData_heightFogEndColor_fogStartOffset", true, 544, 1, 4, 0},
      }
    },
  }},
  {"m2ParticleShader",  {
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_uSceneSize_DisableLightBuffer", true, 160, 1, 4, 0},
        {"_0_0_scene_closeRiverColor", true, 176, 1, 4, 0},
        {"_0_0_scene_farRiverColor", true, 192, 1, 4, 0},
        {"_0_0_scene_closeOceanColor", true, 208, 1, 4, 0},
        {"_0_0_scene_farOceanColor", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 240, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 256, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 272, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 288, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 304, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 320, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 336, 1, 4, 0},
        {"_0_0_fogData_classicFogParams", true, 352, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 368, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 384, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 400, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 416, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 432, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 448, 1, 4, 0},
        {"_0_0_fogData_sunDirection_and_fogZScalar", true, 464, 1, 4, 0},
        {"_0_0_fogData_heightFogCoeff", true, 480, 1, 4, 0},
        {"_0_0_fogData_mainFogCoeff", true, 496, 1, 4, 0},
        {"_0_0_fogData_heightDensityFogCoeff", true, 512, 1, 4, 0},
        {"_0_0_fogData_mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 528, 1, 4, 0},
        {"_0_0_fogData_heightFogEndColor_fogStartOffset", true, 544, 1, 4, 0},
      }
    },
  }},
  {"waterShader",  {
    {
      1, {
        {"_1_1_uPlacementMats", true, 0, 4, 4, 0},
      }
    },
    {
      2, {
      }
    },
    {
      0, {
        {"_1_0_uPlacementMat", true, 0, 4, 4, 0},
      }
    },
  }},
  {"waterfallShader",  {
    {
      9, {
      }
    },
    {
      8, {
      }
    },
    {
      5, {
        {"_1_5_textureMatrix[0]", true, 0, 4, 4, 64},
      }
    },
    {
      4, {
        {"_1_4_textureWeight[0]", true, 0, 1, 4, 16},
      }
    },
    {
      2, {
        {"_1_2_uBoneMatrixes[0]", true, 0, 4, 4, 256},
      }
    },
    {
      3, {
        {"_1_3_colors[0]", true, 0, 1, 4, 256},
      }
    },
    {
      6, {
        {"_1_6_textureMatrix", true, 0, 4, 4, 0},
      }
    },
    {
      7, {
      }
    },
    {
      1, {
        {"_1_1_intLight_uInteriorAmbientColorAndApplyInteriorLight", true, 0, 1, 4, 0},
        {"_1_1_intLight_uInteriorDirectColorAndApplyExteriorLight", true, 16, 1, 4, 0},
        {"_1_1_pc_lights[0].innerColor", true, 32, 1, 4, 0},
        {"_1_1_pc_lights[0].outerColor", true, 48, 1, 4, 0},
        {"_1_1_pc_lights[0].position", true, 64, 1, 4, 0},
        {"_1_1_pc_lights[0].attenuation", true, 80, 1, 4, 0},
        {"_1_1_pc_lights[0].blendParams", true, 96, 1, 4, 0},
        {"_1_1_pc_lights[1].innerColor", true, 112, 1, 4, 0},
        {"_1_1_pc_lights[1].outerColor", true, 128, 1, 4, 0},
        {"_1_1_pc_lights[1].position", true, 144, 1, 4, 0},
        {"_1_1_pc_lights[1].attenuation", true, 160, 1, 4, 0},
        {"_1_1_pc_lights[1].blendParams", true, 176, 1, 4, 0},
        {"_1_1_pc_lights[2].innerColor", true, 192, 1, 4, 0},
        {"_1_1_pc_lights[2].outerColor", true, 208, 1, 4, 0},
        {"_1_1_pc_lights[2].position", true, 224, 1, 4, 0},
        {"_1_1_pc_lights[2].attenuation", true, 240, 1, 4, 0},
        {"_1_1_pc_lights[2].blendParams", true, 256, 1, 4, 0},
        {"_1_1_pc_lights[3].innerColor", true, 272, 1, 4, 0},
        {"_1_1_pc_lights[3].outerColor", true, 288, 1, 4, 0},
        {"_1_1_pc_lights[3].position", true, 304, 1, 4, 0},
        {"_1_1_pc_lights[3].attenuation", true, 320, 1, 4, 0},
        {"_1_1_pc_lights[3].blendParams", true, 336, 1, 4, 0},
        {"_1_1_lightCountAndBcHack", false, 352, 1, 4, 0},
        {"_1_1_interiorExteriorBlend", true, 368, 1, 4, 0},
      }
    },
    {
      0, {
        {"_1_0_uPlacementMat", true, 0, 4, 4, 0},
      }
    },
  }},
  {"adtLodShader",  {
    {
      0, {
        {"_0_0_uPos", true, 0, 1, 3, 0},
        {"_0_0_uLookAtMat", true, 16, 4, 4, 0},
        {"_0_0_uPMatrix", true, 80, 4, 4, 0},
      }
    },
  }},
  {"drawFrustumShader",  {
    {
      0, {
        {"_0_0_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_uPMatrix", true, 64, 4, 4, 0},
      }
    },
  }},
  {"drawPoints",  {
    {
      1, {
        {"_0_1_uPlacementMat", true, 0, 4, 4, 0},
      }
    },
    {
      0, {
        {"_0_0_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_uPMatrix", true, 64, 4, 4, 0},
      }
    },
  }},
};
const  std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameFrag = {
  {"imguiShaderDepth",  {
  }},
  {"ffxglow",  {
    {
      1, {
        {"_0_1_blurAmount", true, 0, 1, 4, 0},
      }
    },
  }},
  {"renderFrameBufferShader",  {
    {
      2, {
        {"_0_2_gauss_offsets[0]", true, 0, 1, 1, 5},
        {"_0_2_gauss_weights[0]", true, 80, 1, 1, 5},
        {"_0_2_uResolution", true, 160, 1, 2, 0},
      }
    },
  }},
  {"drawPortalShader",  {
    {
      0, {
        {"_1_0_uColor", true, 0, 1, 4, 0},
      }
    },
  }},
  {"drawPoints",  {
    {
      1, {
        {"_0_1_uColor", true, 0, 1, 3, 0},
      }
    },
  }},
  {"ffxgauss4",  {
    {
      1, {
        {"_0_1_texOffsetX", true, 0, 1, 4, 0},
        {"_0_1_texOffsetY", true, 16, 1, 4, 0},
      }
    },
  }},
  {"drawFrustumShader",  {
    {
      2, {
        {"_0_2_uColor", true, 0, 1, 3, 0},
      }
    },
  }},
  {"drawDepthShader",  {
    {
      2, {
        {"_0_2_drawDepth", false, 0, 1, 1, 0},
        {"_0_2_uFarPlane", true, 4, 1, 1, 0},
        {"_0_2_uNearPlane", true, 8, 1, 1, 0},
      }
    },
  }},
  {"adtShader",  {
    {
      2, {
      }
    },
    {
      1, {
        {"_1_1_scaleFactorPerLayer", true, 0, 1, 4, 0},
        {"_1_1_animation_rotationPerLayer", false, 16, 1, 4, 0},
        {"_1_1_animation_speedPerLayer", false, 32, 1, 4, 0},
      }
    },
    {
      3, {
      }
    },
    {
      0, {
        {"_1_0_uPos", true, 0, 1, 4, 0},
        {"_1_0_uUseHeightMixFormula", false, 16, 1, 4, 0},
        {"_1_0_uHeightScale", true, 32, 1, 4, 0},
        {"_1_0_uHeightOffset", true, 48, 1, 4, 0},
      }
    },
  }},
  {"drawBBShader",  {
    {
      1, {
        {"_0_1_uPlacementMat", true, 0, 4, 4, 0},
        {"_0_1_uBBScale", true, 64, 1, 4, 0},
        {"_0_1_uBBCenter", true, 80, 1, 4, 0},
        {"_0_1_uColor", true, 96, 1, 4, 0},
      }
    },
  }},
  {"m2Shader_opaq",  {
    {
      3, {
        {"_1_3_uBoneMatrixes", true, 0, 4, 4, 0},
      }
    },
    {
      1, {
        {"_1_1_uPlacementMats", true, 0, 4, 4, 0},
      }
    },
    {
      6, {
        {"_1_6_textureMatrix", true, 0, 4, 4, 0},
      }
    },
    {
      4, {
        {"_1_4_colors", true, 0, 1, 4, 0},
      }
    },
    {
      5, {
        {"_1_5_textureWeight", true, 0, 1, 4, 0},
      }
    },
    {
      2, {
      }
    },
    {
      7, {
      }
    },
    {
      8, {
      }
    },
    {
      9, {
      }
    },
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_uSceneSize_DisableLightBuffer", true, 160, 1, 4, 0},
        {"_0_0_scene_closeRiverColor", true, 176, 1, 4, 0},
        {"_0_0_scene_farRiverColor", true, 192, 1, 4, 0},
        {"_0_0_scene_closeOceanColor", true, 208, 1, 4, 0},
        {"_0_0_scene_farOceanColor", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 240, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 256, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 272, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 288, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 304, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 320, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 336, 1, 4, 0},
        {"_0_0_fogData_classicFogParams", true, 352, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 368, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 384, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 400, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 416, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 432, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 448, 1, 4, 0},
        {"_0_0_fogData_sunDirection_and_fogZScalar", true, 464, 1, 4, 0},
        {"_0_0_fogData_heightFogCoeff", true, 480, 1, 4, 0},
        {"_0_0_fogData_mainFogCoeff", true, 496, 1, 4, 0},
        {"_0_0_fogData_heightDensityFogCoeff", true, 512, 1, 4, 0},
        {"_0_0_fogData_mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 528, 1, 4, 0},
        {"_0_0_fogData_heightFogEndColor_fogStartOffset", true, 544, 1, 4, 0},
      }
    },
  }},
  {"m2ParticleShader_opaq_deferred",  {
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_uSceneSize_DisableLightBuffer", true, 160, 1, 4, 0},
        {"_0_0_scene_closeRiverColor", true, 176, 1, 4, 0},
        {"_0_0_scene_farRiverColor", true, 192, 1, 4, 0},
        {"_0_0_scene_closeOceanColor", true, 208, 1, 4, 0},
        {"_0_0_scene_farOceanColor", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 240, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 256, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 272, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 288, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 304, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 320, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 336, 1, 4, 0},
        {"_0_0_fogData_classicFogParams", true, 352, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 368, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 384, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 400, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 416, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 432, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 448, 1, 4, 0},
        {"_0_0_fogData_sunDirection_and_fogZScalar", true, 464, 1, 4, 0},
        {"_0_0_fogData_heightFogCoeff", true, 480, 1, 4, 0},
        {"_0_0_fogData_mainFogCoeff", true, 496, 1, 4, 0},
        {"_0_0_fogData_heightDensityFogCoeff", true, 512, 1, 4, 0},
        {"_0_0_fogData_mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 528, 1, 4, 0},
        {"_0_0_fogData_heightFogEndColor_fogStartOffset", true, 544, 1, 4, 0},
      }
    },
  }},
  {"imguiShader_opaque",  {
  }},
  {"pointLight",  {
    {
      0, {
      }
    },
    {
      3, {
        {"_1_3_screenSize", true, 0, 1, 4, 0},
      }
    },
  }},
  {"skyConus",  {
  }},
  {"m2Shader",  {
    {
      3, {
        {"_1_3_colors[0]", true, 0, 1, 4, 256},
      }
    },
    {
      1, {
        {"_1_1_intLight_uInteriorAmbientColorAndApplyInteriorLight", true, 0, 1, 4, 0},
        {"_1_1_intLight_uInteriorDirectColorAndApplyExteriorLight", true, 16, 1, 4, 0},
        {"_1_1_pc_lights[0].innerColor", true, 32, 1, 4, 0},
        {"_1_1_pc_lights[0].outerColor", true, 48, 1, 4, 0},
        {"_1_1_pc_lights[0].position", true, 64, 1, 4, 0},
        {"_1_1_pc_lights[0].attenuation", true, 80, 1, 4, 0},
        {"_1_1_pc_lights[0].blendParams", true, 96, 1, 4, 0},
        {"_1_1_pc_lights[1].innerColor", true, 112, 1, 4, 0},
        {"_1_1_pc_lights[1].outerColor", true, 128, 1, 4, 0},
        {"_1_1_pc_lights[1].position", true, 144, 1, 4, 0},
        {"_1_1_pc_lights[1].attenuation", true, 160, 1, 4, 0},
        {"_1_1_pc_lights[1].blendParams", true, 176, 1, 4, 0},
        {"_1_1_pc_lights[2].innerColor", true, 192, 1, 4, 0},
        {"_1_1_pc_lights[2].outerColor", true, 208, 1, 4, 0},
        {"_1_1_pc_lights[2].position", true, 224, 1, 4, 0},
        {"_1_1_pc_lights[2].attenuation", true, 240, 1, 4, 0},
        {"_1_1_pc_lights[2].blendParams", true, 256, 1, 4, 0},
        {"_1_1_pc_lights[3].innerColor", true, 272, 1, 4, 0},
        {"_1_1_pc_lights[3].outerColor", true, 288, 1, 4, 0},
        {"_1_1_pc_lights[3].position", true, 304, 1, 4, 0},
        {"_1_1_pc_lights[3].attenuation", true, 320, 1, 4, 0},
        {"_1_1_pc_lights[3].blendParams", true, 336, 1, 4, 0},
        {"_1_1_lightCountAndBcHack", false, 352, 1, 4, 0},
        {"_1_1_interiorExteriorBlend", true, 368, 1, 4, 0},
      }
    },
    {
      6, {
        {"_1_6_textureMatrix", true, 0, 4, 4, 0},
      }
    },
    {
      4, {
        {"_1_4_textureWeight[0]", true, 0, 1, 4, 16},
      }
    },
    {
      5, {
        {"_1_5_textureMatrix[0]", true, 0, 4, 4, 64},
      }
    },
    {
      2, {
        {"_1_2_uBoneMatrixes[0]", true, 0, 4, 4, 256},
      }
    },
    {
      7, {
      }
    },
    {
      8, {
      }
    },
    {
      9, {
      }
    },
    {
      0, {
        {"_1_0_uPlacementMat", true, 0, 4, 4, 0},
      }
    },
  }},
  {"wmoShader_opaq",  {
    {
      2, {
        {"_1_2_VertexShader_UseLitColors", false, 0, 1, 4, 0},
      }
    },
    {
      1, {
        {"_1_1_uPlacementMats", true, 0, 4, 4, 0},
      }
    },
    {
      5, {
        {"_1_5_s_wmoAmbient", true, 0, 1, 4, 0},
      }
    },
    {
      3, {
      }
    },
    {
      4, {
      }
    },
    {
      6, {
      }
    },
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_uSceneSize_DisableLightBuffer", true, 160, 1, 4, 0},
        {"_0_0_scene_closeRiverColor", true, 176, 1, 4, 0},
        {"_0_0_scene_farRiverColor", true, 192, 1, 4, 0},
        {"_0_0_scene_closeOceanColor", true, 208, 1, 4, 0},
        {"_0_0_scene_farOceanColor", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 240, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 256, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 272, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 288, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 304, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 320, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 336, 1, 4, 0},
        {"_0_0_fogData_classicFogParams", true, 352, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 368, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 384, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 400, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 416, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 432, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 448, 1, 4, 0},
        {"_0_0_fogData_sunDirection_and_fogZScalar", true, 464, 1, 4, 0},
        {"_0_0_fogData_heightFogCoeff", true, 480, 1, 4, 0},
        {"_0_0_fogData_mainFogCoeff", true, 496, 1, 4, 0},
        {"_0_0_fogData_heightDensityFogCoeff", true, 512, 1, 4, 0},
        {"_0_0_fogData_mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 528, 1, 4, 0},
        {"_0_0_fogData_heightFogEndColor_fogStartOffset", true, 544, 1, 4, 0},
      }
    },
  }},
  {"imguiShader",  {
  }},
  {"wmoShader",  {
    {
      2, {
        {"_1_2_UseLitColor_EnableAlpha_PixelShader_BlendMode", false, 0, 1, 4, 0},
        {"_1_2_FogColor_AlphaTest", true, 16, 1, 4, 0},
      }
    },
    {
      1, {
        {"_1_1_uPlacementMats", true, 0, 4, 4, 0},
      }
    },
    {
      5, {
        {"_1_5_s_wmoAmbient", true, 0, 1, 4, 0},
      }
    },
    {
      3, {
      }
    },
    {
      4, {
      }
    },
    {
      6, {
      }
    },
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_uSceneSize_DisableLightBuffer", true, 160, 1, 4, 0},
        {"_0_0_scene_closeRiverColor", true, 176, 1, 4, 0},
        {"_0_0_scene_farRiverColor", true, 192, 1, 4, 0},
        {"_0_0_scene_closeOceanColor", true, 208, 1, 4, 0},
        {"_0_0_scene_farOceanColor", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 240, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 256, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 272, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 288, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 304, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 320, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 336, 1, 4, 0},
        {"_0_0_fogData_classicFogParams", true, 352, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 368, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 384, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 400, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 416, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 432, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 448, 1, 4, 0},
        {"_0_0_fogData_sunDirection_and_fogZScalar", true, 464, 1, 4, 0},
        {"_0_0_fogData_heightFogCoeff", true, 480, 1, 4, 0},
        {"_0_0_fogData_mainFogCoeff", true, 496, 1, 4, 0},
        {"_0_0_fogData_heightDensityFogCoeff", true, 512, 1, 4, 0},
        {"_0_0_fogData_mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 528, 1, 4, 0},
        {"_0_0_fogData_heightFogEndColor_fogStartOffset", true, 544, 1, 4, 0},
      }
    },
  }},
  {"drawLinesShader",  {
    {
      1, {
        {"_0_1_uColor", true, 0, 1, 3, 0},
      }
    },
  }},
  {"ribbonShader",  {
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_uSceneSize_DisableLightBuffer", true, 160, 1, 4, 0},
        {"_0_0_scene_closeRiverColor", true, 176, 1, 4, 0},
        {"_0_0_scene_farRiverColor", true, 192, 1, 4, 0},
        {"_0_0_scene_closeOceanColor", true, 208, 1, 4, 0},
        {"_0_0_scene_farOceanColor", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 240, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 256, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 272, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 288, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 304, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 320, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 336, 1, 4, 0},
        {"_0_0_fogData_classicFogParams", true, 352, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 368, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 384, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 400, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 416, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 432, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 448, 1, 4, 0},
        {"_0_0_fogData_sunDirection_and_fogZScalar", true, 464, 1, 4, 0},
        {"_0_0_fogData_heightFogCoeff", true, 480, 1, 4, 0},
        {"_0_0_fogData_mainFogCoeff", true, 496, 1, 4, 0},
        {"_0_0_fogData_heightDensityFogCoeff", true, 512, 1, 4, 0},
        {"_0_0_fogData_mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 528, 1, 4, 0},
        {"_0_0_fogData_heightFogEndColor_fogStartOffset", true, 544, 1, 4, 0},
      }
    },
    {
      1, {
        {"_1_1_uPixelShader_BlendMode_TextureTransformIndex", false, 0, 1, 4, 0},
      }
    },
  }},
  {"m2ParticleShader",  {
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_uSceneSize_DisableLightBuffer", true, 160, 1, 4, 0},
        {"_0_0_scene_closeRiverColor", true, 176, 1, 4, 0},
        {"_0_0_scene_farRiverColor", true, 192, 1, 4, 0},
        {"_0_0_scene_closeOceanColor", true, 208, 1, 4, 0},
        {"_0_0_scene_farOceanColor", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 240, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 256, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 272, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 288, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 304, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 320, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 336, 1, 4, 0},
        {"_0_0_fogData_classicFogParams", true, 352, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 368, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 384, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 400, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 416, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 432, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 448, 1, 4, 0},
        {"_0_0_fogData_sunDirection_and_fogZScalar", true, 464, 1, 4, 0},
        {"_0_0_fogData_heightFogCoeff", true, 480, 1, 4, 0},
        {"_0_0_fogData_mainFogCoeff", true, 496, 1, 4, 0},
        {"_0_0_fogData_heightDensityFogCoeff", true, 512, 1, 4, 0},
        {"_0_0_fogData_mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 528, 1, 4, 0},
        {"_0_0_fogData_heightFogEndColor_fogStartOffset", true, 544, 1, 4, 0},
      }
    },
  }},
  {"waterShader",  {
    {
      1, {
        {"_1_1_materialId_liquidFlags", false, 0, 1, 4, 0},
        {"_1_1_matColor", true, 16, 1, 4, 0},
        {"_1_1_float0_float1", true, 32, 1, 4, 0},
      }
    },
    {
      2, {
      }
    },
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_uSceneSize_DisableLightBuffer", true, 160, 1, 4, 0},
        {"_0_0_scene_closeRiverColor", true, 176, 1, 4, 0},
        {"_0_0_scene_farRiverColor", true, 192, 1, 4, 0},
        {"_0_0_scene_closeOceanColor", true, 208, 1, 4, 0},
        {"_0_0_scene_farOceanColor", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 240, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 256, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 272, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 288, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 304, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 320, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 336, 1, 4, 0},
        {"_0_0_fogData_classicFogParams", true, 352, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 368, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 384, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 400, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 416, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 432, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 448, 1, 4, 0},
        {"_0_0_fogData_sunDirection_and_fogZScalar", true, 464, 1, 4, 0},
        {"_0_0_fogData_heightFogCoeff", true, 480, 1, 4, 0},
        {"_0_0_fogData_mainFogCoeff", true, 496, 1, 4, 0},
        {"_0_0_fogData_heightDensityFogCoeff", true, 512, 1, 4, 0},
        {"_0_0_fogData_mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 528, 1, 4, 0},
        {"_0_0_fogData_heightFogEndColor_fogStartOffset", true, 544, 1, 4, 0},
      }
    },
  }},
  {"waterFallShader_opaq",  {
    {
      9, {
      }
    },
    {
      8, {
      }
    },
    {
      7, {
      }
    },
    {
      6, {
        {"_1_6_textureMatrix", true, 0, 4, 4, 0},
      }
    },
    {
      5, {
        {"_1_5_textureWeight", true, 0, 1, 4, 0},
      }
    },
    {
      4, {
        {"_1_4_colors", true, 0, 1, 4, 0},
      }
    },
    {
      3, {
        {"_1_3_uBoneMatrixes", true, 0, 4, 4, 0},
      }
    },
    {
      2, {
      }
    },
    {
      1, {
        {"_1_1_uPlacementMats", true, 0, 4, 4, 0},
      }
    },
    {
      0, {
      }
    },
  }},
  {"waterfallShader",  {
    {
      9, {
      }
    },
    {
      8, {
      }
    },
    {
      7, {
      }
    },
    {
      6, {
        {"_1_6_textureMatrix", true, 0, 4, 4, 0},
      }
    },
    {
      5, {
        {"_1_5_textureMatrix[0]", true, 0, 4, 4, 64},
      }
    },
    {
      4, {
        {"_1_4_textureWeight[0]", true, 0, 1, 4, 16},
      }
    },
    {
      3, {
        {"_1_3_colors[0]", true, 0, 1, 4, 256},
      }
    },
    {
      2, {
        {"_1_2_uBoneMatrixes[0]", true, 0, 4, 4, 256},
      }
    },
    {
      1, {
        {"_1_1_intLight_uInteriorAmbientColorAndApplyInteriorLight", true, 0, 1, 4, 0},
        {"_1_1_intLight_uInteriorDirectColorAndApplyExteriorLight", true, 16, 1, 4, 0},
        {"_1_1_pc_lights[0].innerColor", true, 32, 1, 4, 0},
        {"_1_1_pc_lights[0].outerColor", true, 48, 1, 4, 0},
        {"_1_1_pc_lights[0].position", true, 64, 1, 4, 0},
        {"_1_1_pc_lights[0].attenuation", true, 80, 1, 4, 0},
        {"_1_1_pc_lights[0].blendParams", true, 96, 1, 4, 0},
        {"_1_1_pc_lights[1].innerColor", true, 112, 1, 4, 0},
        {"_1_1_pc_lights[1].outerColor", true, 128, 1, 4, 0},
        {"_1_1_pc_lights[1].position", true, 144, 1, 4, 0},
        {"_1_1_pc_lights[1].attenuation", true, 160, 1, 4, 0},
        {"_1_1_pc_lights[1].blendParams", true, 176, 1, 4, 0},
        {"_1_1_pc_lights[2].innerColor", true, 192, 1, 4, 0},
        {"_1_1_pc_lights[2].outerColor", true, 208, 1, 4, 0},
        {"_1_1_pc_lights[2].position", true, 224, 1, 4, 0},
        {"_1_1_pc_lights[2].attenuation", true, 240, 1, 4, 0},
        {"_1_1_pc_lights[2].blendParams", true, 256, 1, 4, 0},
        {"_1_1_pc_lights[3].innerColor", true, 272, 1, 4, 0},
        {"_1_1_pc_lights[3].outerColor", true, 288, 1, 4, 0},
        {"_1_1_pc_lights[3].position", true, 304, 1, 4, 0},
        {"_1_1_pc_lights[3].attenuation", true, 320, 1, 4, 0},
        {"_1_1_pc_lights[3].blendParams", true, 336, 1, 4, 0},
        {"_1_1_lightCountAndBcHack", false, 352, 1, 4, 0},
        {"_1_1_interiorExteriorBlend", true, 368, 1, 4, 0},
      }
    },
    {
      0, {
        {"_1_0_uPlacementMat", true, 0, 4, 4, 0},
      }
    },
  }},
  {"adtLodShader",  {
    {
      0, {
        {"_0_0_uViewUp", true, 0, 1, 4, 0},
        {"_0_0_uSunDir_FogStart", true, 16, 1, 4, 0},
        {"_0_0_uSunColor_uFogEnd", true, 32, 1, 4, 0},
        {"_0_0_uAmbientLight", true, 48, 1, 4, 0},
        {"_0_0_FogColor", true, 64, 1, 4, 0},
        {"_0_0_uNewFormula", false, 80, 1, 1, 0},
      }
    },
  }},
};
#endif


#endif
