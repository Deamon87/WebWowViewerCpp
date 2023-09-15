#ifndef WOWMAPVIEWERREVIVED_SHADERDEFINITIONS_H
#define WOWMAPVIEWERREVIVED_SHADERDEFINITIONS_H

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
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
        aPos = 0, aColor = 1, aVertexLighting = 2, aNormal = 3, adtShaderAttributeEnd
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
        aPosition = 0, aColor = 1, aTexcoord0 = 2, aTexcoord1 = 3, aTexcoord2 = 4, aAlphaCutoff = 5, m2ParticleShaderAttributeEnd
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
        aPosition = 0, aNormal = 1, aTexCoord = 2, aTexCoord2 = 3, aTexCoord3 = 4, aTexCoord4 = 5, aColor = 6, aColor2 = 7, aColorSecond = 8, wmoAmbient = 9, wmoShaderAttributeEnd
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
{ "adtShader",
  {
    { "aPos", 0},
    { "aColor", 1},
    { "aVertexLighting", 2},
    { "aNormal", 3},
  }
},
{ "drawBBShader",
  {
    { "aPosition", 0},
  }
},
{ "waterShader",
  {
    { "aPositionTransp", 0},
    { "aTexCoord", 1},
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
{ "drawQuad",
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
{ "drawPortalShader",
  {
    { "aPosition", 0},
  }
},
{ "renderFrameBufferShader",
  {
    { "a_position", 0},
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
{ "imguiShader",
  {
    { "Position", 0},
    { "UV", 1},
    { "Color", 2},
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
};

const std::unordered_map<std::string, shaderMetaData> shaderMetaInfo = { 
{ "visBuffer/m2Shader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {1,3,16384},
      {1,1,64},
      {0,0,2048},
    },
    {
      {
        {0,0,1},
        {1,3,3},
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
{ "forwardRendering/wmoShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {1,1,64},
      {0,0,2048},
      {1,2,16},
    },
    {
      {
        {0,0,1},
        {1,2,2},
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
{ "forwardRendering/waterfallShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {2,4,16},
      {1,6,4096},
      {1,3,16384},
      {0,0,2048},
      {1,1,64},
    },
    {
      {
        {0,0,1},
        {1,6,6},
        {4,4,1},
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
      {3,9, "uBumpTexture"},
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {9,9,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "forwardRendering/waterShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,2048},
      {1,1,64},
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
{ "forwardRendering/skyConus.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,2048},
      {1,1,96},
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
{ "forwardRendering/drawPoints.frag.spv", 
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
{ "forwardRendering/drawPortalShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,2048},
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
{ "forwardRendering/drawLinesShader.vert.spv", 
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
{ "forwardRendering/imguiShader.frag.spv", 
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
      {1,5, "Texture"},
    },
    {
      {
        {0,0,0},
        {5,5,1},
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
{ "forwardRendering/renderFrameBufferShader.frag.spv", 
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
{ "forwardRendering/waterfallShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {2,5,96},
      {0,0,2048},
    },
    {
      {
        {0,0,1},
        {0,0,0},
        {5,5,1},
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
      {3,10, "uNormalTex"},
      {3,8, "uNoise"},
      {3,7, "uWhiteWater"},
      {3,6, "uMask"},
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {6,10,5},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "forwardRendering/drawLinesShader.frag.spv", 
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
{ "forwardRendering/drawBBShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,1,112},
      {0,0,2048},
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
{ "forwardRendering/drawPortalShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {1,1,16},
    },
    {
      {
        {0,0,0},
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
{ "forwardRendering/drawFrustumShader.frag.spv", 
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
{ "forwardRendering/imguiShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,1,80},
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
{ "forwardRendering/adtLodShader.vert.spv", 
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
{ "forwardRendering/adtShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {1,2,48},
      {0,0,2048},
      {1,1,64},
    },
    {
      {
        {0,0,1},
        {1,2,2},
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
      {2,9, "uAlphaTexture"},
      {2,10, "uLayerHeight0"},
      {2,11, "uLayerHeight1"},
      {2,12, "uLayerHeight2"},
      {2,13, "uLayerHeight3"},
      {2,5, "uLayer0"},
      {2,6, "uLayer1"},
      {2,7, "uLayer2"},
      {2,8, "uLayer3"},
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {5,13,9},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "visBuffer/m2Shader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,0,2048},
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
      {2,8,0},
      {1,5,0},
      {1,4,0},
      {1,2,0},
      {1,1,0},
      {1,7,0},
      {1,6,0},
    },
    {
      {3,0, "s_Textures"},
    },
    {
      {
        {0,0,0},
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
{ "forwardRendering/drawFrustumShader.vert.spv", 
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
{ "forwardRendering/adtShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,2048},
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
{ "forwardRendering/adtLodShader.frag.spv", 
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
{ "forwardRendering/wmoShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {1,4,32},
      {0,0,2048},
    },
    {
      {
        {0,0,1},
        {4,4,1},
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
      {2,5, "uTexture"},
      {2,6, "uTexture2"},
      {2,7, "uTexture3"},
      {2,8, "uTexture4"},
      {2,9, "uTexture5"},
      {2,10, "uTexture6"},
      {2,11, "uTexture7"},
      {2,12, "uTexture8"},
      {2,13, "uTexture9"},
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {5,13,9},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "forwardRendering/ffxgauss4.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,4,32},
    },
    {
      {
        {4,4,1},
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
      {1,5, "texture0"},
    },
    {
      {
        {0,0,0},
        {5,5,1},
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
{ "forwardRendering/drawQuad.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,2,16},
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
{ "forwardRendering/ffxglow.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,4,16},
    },
    {
      {
        {4,4,1},
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
      {1,5, "screenTex"},
      {1,6, "blurTex"},
    },
    {
      {
        {0,0,0},
        {5,6,2},
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
{ "forwardRendering/waterShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {1,4,96},
      {0,0,2048},
    },
    {
      {
        {0,0,1},
        {4,4,1},
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
      {2,5, "uTexture"},
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {5,5,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "forwardRendering/m2Shader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {2,7,64},
      {1,5,256},
      {1,4,4096},
      {1,2,256},
      {0,0,2048},
      {1,1,64},
      {1,6,4096},
    },
    {
      {
        {0,0,1},
        {1,6,6},
        {7,7,1},
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
      {3,6, "uTexture"},
      {3,7, "uTexture2"},
      {3,8, "uTexture3"},
      {3,9, "uTexture4"},
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {6,9,4},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "forwardRendering/drawPoints.vert.spv", 
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
{ "forwardRendering/drawBBShader.frag.spv", 
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
{ "forwardRendering/m2ParticleShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {1,4,32},
      {0,0,2048},
    },
    {
      {
        {0,0,1},
        {4,4,1},
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
      {2,5, "uTexture"},
      {2,6, "uTexture2"},
      {2,7, "uTexture3"},
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {5,7,3},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "forwardRendering/m2ParticleShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,2048},
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
{ "forwardRendering/skyConus.frag.spv", 
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
{ "forwardRendering/renderFrameBufferShader.vert.spv", 
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
{ "forwardRendering/drawDepthShader.frag.spv", 
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
{ "forwardRendering/m2Shader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {1,3,16384},
      {1,1,64},
      {0,0,2048},
    },
    {
      {
        {0,0,1},
        {1,3,3},
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
{ "forwardRendering/ribbonShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {1,4,16},
      {0,0,2048},
    },
    {
      {
        {0,0,1},
        {4,4,1},
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
      {2,5, "uTexture"},
    },
    {
      {
        {0,0,0},
        {0,0,0},
        {5,5,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0},
      }
    }
  }
},
{ "forwardRendering/ribbonShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,2048},
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
};

const  std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameVert = {
  {"waterfallShader",  {
    {
      1, {
        {"_1_1_uPlacementMat", true, 0, 4, 4, 0},
      }
    },
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 240, 1, 4, 0},
        {"_0_0_fogData[0].densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData[0].classicFogParams", true, 272, 1, 4, 0},
        {"_0_0_fogData[0].heightPlane", true, 288, 1, 4, 0},
        {"_0_0_fogData[0].color_and_heightRate", true, 304, 1, 4, 0},
        {"_0_0_fogData[0].heightDensity_and_endColor", true, 320, 1, 4, 0},
        {"_0_0_fogData[0].sunAngle_and_sunColor", true, 336, 1, 4, 0},
        {"_0_0_fogData[0].heightColor_and_endFogDistance", true, 352, 1, 4, 0},
        {"_0_0_fogData[0].sunPercentage", true, 368, 1, 4, 0},
        {"_0_0_fogData[0].sunDirection_and_fogZScalar", true, 384, 1, 4, 0},
        {"_0_0_fogData[0].heightFogCoeff", true, 400, 1, 4, 0},
        {"_0_0_fogData[0].mainFogCoeff", true, 416, 1, 4, 0},
        {"_0_0_fogData[0].heightDensityFogCoeff", true, 432, 1, 4, 0},
        {"_0_0_fogData[0].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 448, 1, 4, 0},
        {"_0_0_fogData[0].heightFogEndColor_fogStartOffset", true, 464, 1, 4, 0},
        {"_0_0_fogData[1].densityParams", true, 480, 1, 4, 0},
        {"_0_0_fogData[1].classicFogParams", true, 496, 1, 4, 0},
        {"_0_0_fogData[1].heightPlane", true, 512, 1, 4, 0},
        {"_0_0_fogData[1].color_and_heightRate", true, 528, 1, 4, 0},
        {"_0_0_fogData[1].heightDensity_and_endColor", true, 544, 1, 4, 0},
        {"_0_0_fogData[1].sunAngle_and_sunColor", true, 560, 1, 4, 0},
        {"_0_0_fogData[1].heightColor_and_endFogDistance", true, 576, 1, 4, 0},
        {"_0_0_fogData[1].sunPercentage", true, 592, 1, 4, 0},
        {"_0_0_fogData[1].sunDirection_and_fogZScalar", true, 608, 1, 4, 0},
        {"_0_0_fogData[1].heightFogCoeff", true, 624, 1, 4, 0},
        {"_0_0_fogData[1].mainFogCoeff", true, 640, 1, 4, 0},
        {"_0_0_fogData[1].heightDensityFogCoeff", true, 656, 1, 4, 0},
        {"_0_0_fogData[1].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 672, 1, 4, 0},
        {"_0_0_fogData[1].heightFogEndColor_fogStartOffset", true, 688, 1, 4, 0},
        {"_0_0_fogData[2].densityParams", true, 704, 1, 4, 0},
        {"_0_0_fogData[2].classicFogParams", true, 720, 1, 4, 0},
        {"_0_0_fogData[2].heightPlane", true, 736, 1, 4, 0},
        {"_0_0_fogData[2].color_and_heightRate", true, 752, 1, 4, 0},
        {"_0_0_fogData[2].heightDensity_and_endColor", true, 768, 1, 4, 0},
        {"_0_0_fogData[2].sunAngle_and_sunColor", true, 784, 1, 4, 0},
        {"_0_0_fogData[2].heightColor_and_endFogDistance", true, 800, 1, 4, 0},
        {"_0_0_fogData[2].sunPercentage", true, 816, 1, 4, 0},
        {"_0_0_fogData[2].sunDirection_and_fogZScalar", true, 832, 1, 4, 0},
        {"_0_0_fogData[2].heightFogCoeff", true, 848, 1, 4, 0},
        {"_0_0_fogData[2].mainFogCoeff", true, 864, 1, 4, 0},
        {"_0_0_fogData[2].heightDensityFogCoeff", true, 880, 1, 4, 0},
        {"_0_0_fogData[2].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 896, 1, 4, 0},
        {"_0_0_fogData[2].heightFogEndColor_fogStartOffset", true, 912, 1, 4, 0},
        {"_0_0_fogData[3].densityParams", true, 928, 1, 4, 0},
        {"_0_0_fogData[3].classicFogParams", true, 944, 1, 4, 0},
        {"_0_0_fogData[3].heightPlane", true, 960, 1, 4, 0},
        {"_0_0_fogData[3].color_and_heightRate", true, 976, 1, 4, 0},
        {"_0_0_fogData[3].heightDensity_and_endColor", true, 992, 1, 4, 0},
        {"_0_0_fogData[3].sunAngle_and_sunColor", true, 1008, 1, 4, 0},
        {"_0_0_fogData[3].heightColor_and_endFogDistance", true, 1024, 1, 4, 0},
        {"_0_0_fogData[3].sunPercentage", true, 1040, 1, 4, 0},
        {"_0_0_fogData[3].sunDirection_and_fogZScalar", true, 1056, 1, 4, 0},
        {"_0_0_fogData[3].heightFogCoeff", true, 1072, 1, 4, 0},
        {"_0_0_fogData[3].mainFogCoeff", true, 1088, 1, 4, 0},
        {"_0_0_fogData[3].heightDensityFogCoeff", true, 1104, 1, 4, 0},
        {"_0_0_fogData[3].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1120, 1, 4, 0},
        {"_0_0_fogData[3].heightFogEndColor_fogStartOffset", true, 1136, 1, 4, 0},
        {"_0_0_fogData[4].densityParams", true, 1152, 1, 4, 0},
        {"_0_0_fogData[4].classicFogParams", true, 1168, 1, 4, 0},
        {"_0_0_fogData[4].heightPlane", true, 1184, 1, 4, 0},
        {"_0_0_fogData[4].color_and_heightRate", true, 1200, 1, 4, 0},
        {"_0_0_fogData[4].heightDensity_and_endColor", true, 1216, 1, 4, 0},
        {"_0_0_fogData[4].sunAngle_and_sunColor", true, 1232, 1, 4, 0},
        {"_0_0_fogData[4].heightColor_and_endFogDistance", true, 1248, 1, 4, 0},
        {"_0_0_fogData[4].sunPercentage", true, 1264, 1, 4, 0},
        {"_0_0_fogData[4].sunDirection_and_fogZScalar", true, 1280, 1, 4, 0},
        {"_0_0_fogData[4].heightFogCoeff", true, 1296, 1, 4, 0},
        {"_0_0_fogData[4].mainFogCoeff", true, 1312, 1, 4, 0},
        {"_0_0_fogData[4].heightDensityFogCoeff", true, 1328, 1, 4, 0},
        {"_0_0_fogData[4].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1344, 1, 4, 0},
        {"_0_0_fogData[4].heightFogEndColor_fogStartOffset", true, 1360, 1, 4, 0},
        {"_0_0_fogData[5].densityParams", true, 1376, 1, 4, 0},
        {"_0_0_fogData[5].classicFogParams", true, 1392, 1, 4, 0},
        {"_0_0_fogData[5].heightPlane", true, 1408, 1, 4, 0},
        {"_0_0_fogData[5].color_and_heightRate", true, 1424, 1, 4, 0},
        {"_0_0_fogData[5].heightDensity_and_endColor", true, 1440, 1, 4, 0},
        {"_0_0_fogData[5].sunAngle_and_sunColor", true, 1456, 1, 4, 0},
        {"_0_0_fogData[5].heightColor_and_endFogDistance", true, 1472, 1, 4, 0},
        {"_0_0_fogData[5].sunPercentage", true, 1488, 1, 4, 0},
        {"_0_0_fogData[5].sunDirection_and_fogZScalar", true, 1504, 1, 4, 0},
        {"_0_0_fogData[5].heightFogCoeff", true, 1520, 1, 4, 0},
        {"_0_0_fogData[5].mainFogCoeff", true, 1536, 1, 4, 0},
        {"_0_0_fogData[5].heightDensityFogCoeff", true, 1552, 1, 4, 0},
        {"_0_0_fogData[5].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1568, 1, 4, 0},
        {"_0_0_fogData[5].heightFogEndColor_fogStartOffset", true, 1584, 1, 4, 0},
        {"_0_0_fogData[6].densityParams", true, 1600, 1, 4, 0},
        {"_0_0_fogData[6].classicFogParams", true, 1616, 1, 4, 0},
        {"_0_0_fogData[6].heightPlane", true, 1632, 1, 4, 0},
        {"_0_0_fogData[6].color_and_heightRate", true, 1648, 1, 4, 0},
        {"_0_0_fogData[6].heightDensity_and_endColor", true, 1664, 1, 4, 0},
        {"_0_0_fogData[6].sunAngle_and_sunColor", true, 1680, 1, 4, 0},
        {"_0_0_fogData[6].heightColor_and_endFogDistance", true, 1696, 1, 4, 0},
        {"_0_0_fogData[6].sunPercentage", true, 1712, 1, 4, 0},
        {"_0_0_fogData[6].sunDirection_and_fogZScalar", true, 1728, 1, 4, 0},
        {"_0_0_fogData[6].heightFogCoeff", true, 1744, 1, 4, 0},
        {"_0_0_fogData[6].mainFogCoeff", true, 1760, 1, 4, 0},
        {"_0_0_fogData[6].heightDensityFogCoeff", true, 1776, 1, 4, 0},
        {"_0_0_fogData[6].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1792, 1, 4, 0},
        {"_0_0_fogData[6].heightFogEndColor_fogStartOffset", true, 1808, 1, 4, 0},
        {"_0_0_fogData[7].densityParams", true, 1824, 1, 4, 0},
        {"_0_0_fogData[7].classicFogParams", true, 1840, 1, 4, 0},
        {"_0_0_fogData[7].heightPlane", true, 1856, 1, 4, 0},
        {"_0_0_fogData[7].color_and_heightRate", true, 1872, 1, 4, 0},
        {"_0_0_fogData[7].heightDensity_and_endColor", true, 1888, 1, 4, 0},
        {"_0_0_fogData[7].sunAngle_and_sunColor", true, 1904, 1, 4, 0},
        {"_0_0_fogData[7].heightColor_and_endFogDistance", true, 1920, 1, 4, 0},
        {"_0_0_fogData[7].sunPercentage", true, 1936, 1, 4, 0},
        {"_0_0_fogData[7].sunDirection_and_fogZScalar", true, 1952, 1, 4, 0},
        {"_0_0_fogData[7].heightFogCoeff", true, 1968, 1, 4, 0},
        {"_0_0_fogData[7].mainFogCoeff", true, 1984, 1, 4, 0},
        {"_0_0_fogData[7].heightDensityFogCoeff", true, 2000, 1, 4, 0},
        {"_0_0_fogData[7].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 2016, 1, 4, 0},
        {"_0_0_fogData[7].heightFogEndColor_fogStartOffset", true, 2032, 1, 4, 0},
      }
    },
    {
      3, {
        {"_1_3_uBoneMatrixes[0]", true, 0, 4, 4, 256},
      }
    },
    {
      6, {
        {"_1_6_textureMatrix[0]", true, 0, 4, 4, 64},
      }
    },
    {
      4, {
        {"_2_4_bumpScaleTextIndexes", true, 0, 1, 4, 0},
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
  {"adtShader",  {
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 240, 1, 4, 0},
        {"_0_0_fogData[0].densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData[0].classicFogParams", true, 272, 1, 4, 0},
        {"_0_0_fogData[0].heightPlane", true, 288, 1, 4, 0},
        {"_0_0_fogData[0].color_and_heightRate", true, 304, 1, 4, 0},
        {"_0_0_fogData[0].heightDensity_and_endColor", true, 320, 1, 4, 0},
        {"_0_0_fogData[0].sunAngle_and_sunColor", true, 336, 1, 4, 0},
        {"_0_0_fogData[0].heightColor_and_endFogDistance", true, 352, 1, 4, 0},
        {"_0_0_fogData[0].sunPercentage", true, 368, 1, 4, 0},
        {"_0_0_fogData[0].sunDirection_and_fogZScalar", true, 384, 1, 4, 0},
        {"_0_0_fogData[0].heightFogCoeff", true, 400, 1, 4, 0},
        {"_0_0_fogData[0].mainFogCoeff", true, 416, 1, 4, 0},
        {"_0_0_fogData[0].heightDensityFogCoeff", true, 432, 1, 4, 0},
        {"_0_0_fogData[0].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 448, 1, 4, 0},
        {"_0_0_fogData[0].heightFogEndColor_fogStartOffset", true, 464, 1, 4, 0},
        {"_0_0_fogData[1].densityParams", true, 480, 1, 4, 0},
        {"_0_0_fogData[1].classicFogParams", true, 496, 1, 4, 0},
        {"_0_0_fogData[1].heightPlane", true, 512, 1, 4, 0},
        {"_0_0_fogData[1].color_and_heightRate", true, 528, 1, 4, 0},
        {"_0_0_fogData[1].heightDensity_and_endColor", true, 544, 1, 4, 0},
        {"_0_0_fogData[1].sunAngle_and_sunColor", true, 560, 1, 4, 0},
        {"_0_0_fogData[1].heightColor_and_endFogDistance", true, 576, 1, 4, 0},
        {"_0_0_fogData[1].sunPercentage", true, 592, 1, 4, 0},
        {"_0_0_fogData[1].sunDirection_and_fogZScalar", true, 608, 1, 4, 0},
        {"_0_0_fogData[1].heightFogCoeff", true, 624, 1, 4, 0},
        {"_0_0_fogData[1].mainFogCoeff", true, 640, 1, 4, 0},
        {"_0_0_fogData[1].heightDensityFogCoeff", true, 656, 1, 4, 0},
        {"_0_0_fogData[1].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 672, 1, 4, 0},
        {"_0_0_fogData[1].heightFogEndColor_fogStartOffset", true, 688, 1, 4, 0},
        {"_0_0_fogData[2].densityParams", true, 704, 1, 4, 0},
        {"_0_0_fogData[2].classicFogParams", true, 720, 1, 4, 0},
        {"_0_0_fogData[2].heightPlane", true, 736, 1, 4, 0},
        {"_0_0_fogData[2].color_and_heightRate", true, 752, 1, 4, 0},
        {"_0_0_fogData[2].heightDensity_and_endColor", true, 768, 1, 4, 0},
        {"_0_0_fogData[2].sunAngle_and_sunColor", true, 784, 1, 4, 0},
        {"_0_0_fogData[2].heightColor_and_endFogDistance", true, 800, 1, 4, 0},
        {"_0_0_fogData[2].sunPercentage", true, 816, 1, 4, 0},
        {"_0_0_fogData[2].sunDirection_and_fogZScalar", true, 832, 1, 4, 0},
        {"_0_0_fogData[2].heightFogCoeff", true, 848, 1, 4, 0},
        {"_0_0_fogData[2].mainFogCoeff", true, 864, 1, 4, 0},
        {"_0_0_fogData[2].heightDensityFogCoeff", true, 880, 1, 4, 0},
        {"_0_0_fogData[2].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 896, 1, 4, 0},
        {"_0_0_fogData[2].heightFogEndColor_fogStartOffset", true, 912, 1, 4, 0},
        {"_0_0_fogData[3].densityParams", true, 928, 1, 4, 0},
        {"_0_0_fogData[3].classicFogParams", true, 944, 1, 4, 0},
        {"_0_0_fogData[3].heightPlane", true, 960, 1, 4, 0},
        {"_0_0_fogData[3].color_and_heightRate", true, 976, 1, 4, 0},
        {"_0_0_fogData[3].heightDensity_and_endColor", true, 992, 1, 4, 0},
        {"_0_0_fogData[3].sunAngle_and_sunColor", true, 1008, 1, 4, 0},
        {"_0_0_fogData[3].heightColor_and_endFogDistance", true, 1024, 1, 4, 0},
        {"_0_0_fogData[3].sunPercentage", true, 1040, 1, 4, 0},
        {"_0_0_fogData[3].sunDirection_and_fogZScalar", true, 1056, 1, 4, 0},
        {"_0_0_fogData[3].heightFogCoeff", true, 1072, 1, 4, 0},
        {"_0_0_fogData[3].mainFogCoeff", true, 1088, 1, 4, 0},
        {"_0_0_fogData[3].heightDensityFogCoeff", true, 1104, 1, 4, 0},
        {"_0_0_fogData[3].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1120, 1, 4, 0},
        {"_0_0_fogData[3].heightFogEndColor_fogStartOffset", true, 1136, 1, 4, 0},
        {"_0_0_fogData[4].densityParams", true, 1152, 1, 4, 0},
        {"_0_0_fogData[4].classicFogParams", true, 1168, 1, 4, 0},
        {"_0_0_fogData[4].heightPlane", true, 1184, 1, 4, 0},
        {"_0_0_fogData[4].color_and_heightRate", true, 1200, 1, 4, 0},
        {"_0_0_fogData[4].heightDensity_and_endColor", true, 1216, 1, 4, 0},
        {"_0_0_fogData[4].sunAngle_and_sunColor", true, 1232, 1, 4, 0},
        {"_0_0_fogData[4].heightColor_and_endFogDistance", true, 1248, 1, 4, 0},
        {"_0_0_fogData[4].sunPercentage", true, 1264, 1, 4, 0},
        {"_0_0_fogData[4].sunDirection_and_fogZScalar", true, 1280, 1, 4, 0},
        {"_0_0_fogData[4].heightFogCoeff", true, 1296, 1, 4, 0},
        {"_0_0_fogData[4].mainFogCoeff", true, 1312, 1, 4, 0},
        {"_0_0_fogData[4].heightDensityFogCoeff", true, 1328, 1, 4, 0},
        {"_0_0_fogData[4].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1344, 1, 4, 0},
        {"_0_0_fogData[4].heightFogEndColor_fogStartOffset", true, 1360, 1, 4, 0},
        {"_0_0_fogData[5].densityParams", true, 1376, 1, 4, 0},
        {"_0_0_fogData[5].classicFogParams", true, 1392, 1, 4, 0},
        {"_0_0_fogData[5].heightPlane", true, 1408, 1, 4, 0},
        {"_0_0_fogData[5].color_and_heightRate", true, 1424, 1, 4, 0},
        {"_0_0_fogData[5].heightDensity_and_endColor", true, 1440, 1, 4, 0},
        {"_0_0_fogData[5].sunAngle_and_sunColor", true, 1456, 1, 4, 0},
        {"_0_0_fogData[5].heightColor_and_endFogDistance", true, 1472, 1, 4, 0},
        {"_0_0_fogData[5].sunPercentage", true, 1488, 1, 4, 0},
        {"_0_0_fogData[5].sunDirection_and_fogZScalar", true, 1504, 1, 4, 0},
        {"_0_0_fogData[5].heightFogCoeff", true, 1520, 1, 4, 0},
        {"_0_0_fogData[5].mainFogCoeff", true, 1536, 1, 4, 0},
        {"_0_0_fogData[5].heightDensityFogCoeff", true, 1552, 1, 4, 0},
        {"_0_0_fogData[5].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1568, 1, 4, 0},
        {"_0_0_fogData[5].heightFogEndColor_fogStartOffset", true, 1584, 1, 4, 0},
        {"_0_0_fogData[6].densityParams", true, 1600, 1, 4, 0},
        {"_0_0_fogData[6].classicFogParams", true, 1616, 1, 4, 0},
        {"_0_0_fogData[6].heightPlane", true, 1632, 1, 4, 0},
        {"_0_0_fogData[6].color_and_heightRate", true, 1648, 1, 4, 0},
        {"_0_0_fogData[6].heightDensity_and_endColor", true, 1664, 1, 4, 0},
        {"_0_0_fogData[6].sunAngle_and_sunColor", true, 1680, 1, 4, 0},
        {"_0_0_fogData[6].heightColor_and_endFogDistance", true, 1696, 1, 4, 0},
        {"_0_0_fogData[6].sunPercentage", true, 1712, 1, 4, 0},
        {"_0_0_fogData[6].sunDirection_and_fogZScalar", true, 1728, 1, 4, 0},
        {"_0_0_fogData[6].heightFogCoeff", true, 1744, 1, 4, 0},
        {"_0_0_fogData[6].mainFogCoeff", true, 1760, 1, 4, 0},
        {"_0_0_fogData[6].heightDensityFogCoeff", true, 1776, 1, 4, 0},
        {"_0_0_fogData[6].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1792, 1, 4, 0},
        {"_0_0_fogData[6].heightFogEndColor_fogStartOffset", true, 1808, 1, 4, 0},
        {"_0_0_fogData[7].densityParams", true, 1824, 1, 4, 0},
        {"_0_0_fogData[7].classicFogParams", true, 1840, 1, 4, 0},
        {"_0_0_fogData[7].heightPlane", true, 1856, 1, 4, 0},
        {"_0_0_fogData[7].color_and_heightRate", true, 1872, 1, 4, 0},
        {"_0_0_fogData[7].heightDensity_and_endColor", true, 1888, 1, 4, 0},
        {"_0_0_fogData[7].sunAngle_and_sunColor", true, 1904, 1, 4, 0},
        {"_0_0_fogData[7].heightColor_and_endFogDistance", true, 1920, 1, 4, 0},
        {"_0_0_fogData[7].sunPercentage", true, 1936, 1, 4, 0},
        {"_0_0_fogData[7].sunDirection_and_fogZScalar", true, 1952, 1, 4, 0},
        {"_0_0_fogData[7].heightFogCoeff", true, 1968, 1, 4, 0},
        {"_0_0_fogData[7].mainFogCoeff", true, 1984, 1, 4, 0},
        {"_0_0_fogData[7].heightDensityFogCoeff", true, 2000, 1, 4, 0},
        {"_0_0_fogData[7].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 2016, 1, 4, 0},
        {"_0_0_fogData[7].heightFogEndColor_fogStartOffset", true, 2032, 1, 4, 0},
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
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 240, 1, 4, 0},
        {"_0_0_fogData[0].densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData[0].classicFogParams", true, 272, 1, 4, 0},
        {"_0_0_fogData[0].heightPlane", true, 288, 1, 4, 0},
        {"_0_0_fogData[0].color_and_heightRate", true, 304, 1, 4, 0},
        {"_0_0_fogData[0].heightDensity_and_endColor", true, 320, 1, 4, 0},
        {"_0_0_fogData[0].sunAngle_and_sunColor", true, 336, 1, 4, 0},
        {"_0_0_fogData[0].heightColor_and_endFogDistance", true, 352, 1, 4, 0},
        {"_0_0_fogData[0].sunPercentage", true, 368, 1, 4, 0},
        {"_0_0_fogData[0].sunDirection_and_fogZScalar", true, 384, 1, 4, 0},
        {"_0_0_fogData[0].heightFogCoeff", true, 400, 1, 4, 0},
        {"_0_0_fogData[0].mainFogCoeff", true, 416, 1, 4, 0},
        {"_0_0_fogData[0].heightDensityFogCoeff", true, 432, 1, 4, 0},
        {"_0_0_fogData[0].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 448, 1, 4, 0},
        {"_0_0_fogData[0].heightFogEndColor_fogStartOffset", true, 464, 1, 4, 0},
        {"_0_0_fogData[1].densityParams", true, 480, 1, 4, 0},
        {"_0_0_fogData[1].classicFogParams", true, 496, 1, 4, 0},
        {"_0_0_fogData[1].heightPlane", true, 512, 1, 4, 0},
        {"_0_0_fogData[1].color_and_heightRate", true, 528, 1, 4, 0},
        {"_0_0_fogData[1].heightDensity_and_endColor", true, 544, 1, 4, 0},
        {"_0_0_fogData[1].sunAngle_and_sunColor", true, 560, 1, 4, 0},
        {"_0_0_fogData[1].heightColor_and_endFogDistance", true, 576, 1, 4, 0},
        {"_0_0_fogData[1].sunPercentage", true, 592, 1, 4, 0},
        {"_0_0_fogData[1].sunDirection_and_fogZScalar", true, 608, 1, 4, 0},
        {"_0_0_fogData[1].heightFogCoeff", true, 624, 1, 4, 0},
        {"_0_0_fogData[1].mainFogCoeff", true, 640, 1, 4, 0},
        {"_0_0_fogData[1].heightDensityFogCoeff", true, 656, 1, 4, 0},
        {"_0_0_fogData[1].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 672, 1, 4, 0},
        {"_0_0_fogData[1].heightFogEndColor_fogStartOffset", true, 688, 1, 4, 0},
        {"_0_0_fogData[2].densityParams", true, 704, 1, 4, 0},
        {"_0_0_fogData[2].classicFogParams", true, 720, 1, 4, 0},
        {"_0_0_fogData[2].heightPlane", true, 736, 1, 4, 0},
        {"_0_0_fogData[2].color_and_heightRate", true, 752, 1, 4, 0},
        {"_0_0_fogData[2].heightDensity_and_endColor", true, 768, 1, 4, 0},
        {"_0_0_fogData[2].sunAngle_and_sunColor", true, 784, 1, 4, 0},
        {"_0_0_fogData[2].heightColor_and_endFogDistance", true, 800, 1, 4, 0},
        {"_0_0_fogData[2].sunPercentage", true, 816, 1, 4, 0},
        {"_0_0_fogData[2].sunDirection_and_fogZScalar", true, 832, 1, 4, 0},
        {"_0_0_fogData[2].heightFogCoeff", true, 848, 1, 4, 0},
        {"_0_0_fogData[2].mainFogCoeff", true, 864, 1, 4, 0},
        {"_0_0_fogData[2].heightDensityFogCoeff", true, 880, 1, 4, 0},
        {"_0_0_fogData[2].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 896, 1, 4, 0},
        {"_0_0_fogData[2].heightFogEndColor_fogStartOffset", true, 912, 1, 4, 0},
        {"_0_0_fogData[3].densityParams", true, 928, 1, 4, 0},
        {"_0_0_fogData[3].classicFogParams", true, 944, 1, 4, 0},
        {"_0_0_fogData[3].heightPlane", true, 960, 1, 4, 0},
        {"_0_0_fogData[3].color_and_heightRate", true, 976, 1, 4, 0},
        {"_0_0_fogData[3].heightDensity_and_endColor", true, 992, 1, 4, 0},
        {"_0_0_fogData[3].sunAngle_and_sunColor", true, 1008, 1, 4, 0},
        {"_0_0_fogData[3].heightColor_and_endFogDistance", true, 1024, 1, 4, 0},
        {"_0_0_fogData[3].sunPercentage", true, 1040, 1, 4, 0},
        {"_0_0_fogData[3].sunDirection_and_fogZScalar", true, 1056, 1, 4, 0},
        {"_0_0_fogData[3].heightFogCoeff", true, 1072, 1, 4, 0},
        {"_0_0_fogData[3].mainFogCoeff", true, 1088, 1, 4, 0},
        {"_0_0_fogData[3].heightDensityFogCoeff", true, 1104, 1, 4, 0},
        {"_0_0_fogData[3].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1120, 1, 4, 0},
        {"_0_0_fogData[3].heightFogEndColor_fogStartOffset", true, 1136, 1, 4, 0},
        {"_0_0_fogData[4].densityParams", true, 1152, 1, 4, 0},
        {"_0_0_fogData[4].classicFogParams", true, 1168, 1, 4, 0},
        {"_0_0_fogData[4].heightPlane", true, 1184, 1, 4, 0},
        {"_0_0_fogData[4].color_and_heightRate", true, 1200, 1, 4, 0},
        {"_0_0_fogData[4].heightDensity_and_endColor", true, 1216, 1, 4, 0},
        {"_0_0_fogData[4].sunAngle_and_sunColor", true, 1232, 1, 4, 0},
        {"_0_0_fogData[4].heightColor_and_endFogDistance", true, 1248, 1, 4, 0},
        {"_0_0_fogData[4].sunPercentage", true, 1264, 1, 4, 0},
        {"_0_0_fogData[4].sunDirection_and_fogZScalar", true, 1280, 1, 4, 0},
        {"_0_0_fogData[4].heightFogCoeff", true, 1296, 1, 4, 0},
        {"_0_0_fogData[4].mainFogCoeff", true, 1312, 1, 4, 0},
        {"_0_0_fogData[4].heightDensityFogCoeff", true, 1328, 1, 4, 0},
        {"_0_0_fogData[4].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1344, 1, 4, 0},
        {"_0_0_fogData[4].heightFogEndColor_fogStartOffset", true, 1360, 1, 4, 0},
        {"_0_0_fogData[5].densityParams", true, 1376, 1, 4, 0},
        {"_0_0_fogData[5].classicFogParams", true, 1392, 1, 4, 0},
        {"_0_0_fogData[5].heightPlane", true, 1408, 1, 4, 0},
        {"_0_0_fogData[5].color_and_heightRate", true, 1424, 1, 4, 0},
        {"_0_0_fogData[5].heightDensity_and_endColor", true, 1440, 1, 4, 0},
        {"_0_0_fogData[5].sunAngle_and_sunColor", true, 1456, 1, 4, 0},
        {"_0_0_fogData[5].heightColor_and_endFogDistance", true, 1472, 1, 4, 0},
        {"_0_0_fogData[5].sunPercentage", true, 1488, 1, 4, 0},
        {"_0_0_fogData[5].sunDirection_and_fogZScalar", true, 1504, 1, 4, 0},
        {"_0_0_fogData[5].heightFogCoeff", true, 1520, 1, 4, 0},
        {"_0_0_fogData[5].mainFogCoeff", true, 1536, 1, 4, 0},
        {"_0_0_fogData[5].heightDensityFogCoeff", true, 1552, 1, 4, 0},
        {"_0_0_fogData[5].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1568, 1, 4, 0},
        {"_0_0_fogData[5].heightFogEndColor_fogStartOffset", true, 1584, 1, 4, 0},
        {"_0_0_fogData[6].densityParams", true, 1600, 1, 4, 0},
        {"_0_0_fogData[6].classicFogParams", true, 1616, 1, 4, 0},
        {"_0_0_fogData[6].heightPlane", true, 1632, 1, 4, 0},
        {"_0_0_fogData[6].color_and_heightRate", true, 1648, 1, 4, 0},
        {"_0_0_fogData[6].heightDensity_and_endColor", true, 1664, 1, 4, 0},
        {"_0_0_fogData[6].sunAngle_and_sunColor", true, 1680, 1, 4, 0},
        {"_0_0_fogData[6].heightColor_and_endFogDistance", true, 1696, 1, 4, 0},
        {"_0_0_fogData[6].sunPercentage", true, 1712, 1, 4, 0},
        {"_0_0_fogData[6].sunDirection_and_fogZScalar", true, 1728, 1, 4, 0},
        {"_0_0_fogData[6].heightFogCoeff", true, 1744, 1, 4, 0},
        {"_0_0_fogData[6].mainFogCoeff", true, 1760, 1, 4, 0},
        {"_0_0_fogData[6].heightDensityFogCoeff", true, 1776, 1, 4, 0},
        {"_0_0_fogData[6].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1792, 1, 4, 0},
        {"_0_0_fogData[6].heightFogEndColor_fogStartOffset", true, 1808, 1, 4, 0},
        {"_0_0_fogData[7].densityParams", true, 1824, 1, 4, 0},
        {"_0_0_fogData[7].classicFogParams", true, 1840, 1, 4, 0},
        {"_0_0_fogData[7].heightPlane", true, 1856, 1, 4, 0},
        {"_0_0_fogData[7].color_and_heightRate", true, 1872, 1, 4, 0},
        {"_0_0_fogData[7].heightDensity_and_endColor", true, 1888, 1, 4, 0},
        {"_0_0_fogData[7].sunAngle_and_sunColor", true, 1904, 1, 4, 0},
        {"_0_0_fogData[7].heightColor_and_endFogDistance", true, 1920, 1, 4, 0},
        {"_0_0_fogData[7].sunPercentage", true, 1936, 1, 4, 0},
        {"_0_0_fogData[7].sunDirection_and_fogZScalar", true, 1952, 1, 4, 0},
        {"_0_0_fogData[7].heightFogCoeff", true, 1968, 1, 4, 0},
        {"_0_0_fogData[7].mainFogCoeff", true, 1984, 1, 4, 0},
        {"_0_0_fogData[7].heightDensityFogCoeff", true, 2000, 1, 4, 0},
        {"_0_0_fogData[7].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 2016, 1, 4, 0},
        {"_0_0_fogData[7].heightFogEndColor_fogStartOffset", true, 2032, 1, 4, 0},
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
  {"waterShader",  {
    {
      1, {
        {"_1_1_uPlacementMat", true, 0, 4, 4, 0},
      }
    },
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 240, 1, 4, 0},
        {"_0_0_fogData[0].densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData[0].classicFogParams", true, 272, 1, 4, 0},
        {"_0_0_fogData[0].heightPlane", true, 288, 1, 4, 0},
        {"_0_0_fogData[0].color_and_heightRate", true, 304, 1, 4, 0},
        {"_0_0_fogData[0].heightDensity_and_endColor", true, 320, 1, 4, 0},
        {"_0_0_fogData[0].sunAngle_and_sunColor", true, 336, 1, 4, 0},
        {"_0_0_fogData[0].heightColor_and_endFogDistance", true, 352, 1, 4, 0},
        {"_0_0_fogData[0].sunPercentage", true, 368, 1, 4, 0},
        {"_0_0_fogData[0].sunDirection_and_fogZScalar", true, 384, 1, 4, 0},
        {"_0_0_fogData[0].heightFogCoeff", true, 400, 1, 4, 0},
        {"_0_0_fogData[0].mainFogCoeff", true, 416, 1, 4, 0},
        {"_0_0_fogData[0].heightDensityFogCoeff", true, 432, 1, 4, 0},
        {"_0_0_fogData[0].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 448, 1, 4, 0},
        {"_0_0_fogData[0].heightFogEndColor_fogStartOffset", true, 464, 1, 4, 0},
        {"_0_0_fogData[1].densityParams", true, 480, 1, 4, 0},
        {"_0_0_fogData[1].classicFogParams", true, 496, 1, 4, 0},
        {"_0_0_fogData[1].heightPlane", true, 512, 1, 4, 0},
        {"_0_0_fogData[1].color_and_heightRate", true, 528, 1, 4, 0},
        {"_0_0_fogData[1].heightDensity_and_endColor", true, 544, 1, 4, 0},
        {"_0_0_fogData[1].sunAngle_and_sunColor", true, 560, 1, 4, 0},
        {"_0_0_fogData[1].heightColor_and_endFogDistance", true, 576, 1, 4, 0},
        {"_0_0_fogData[1].sunPercentage", true, 592, 1, 4, 0},
        {"_0_0_fogData[1].sunDirection_and_fogZScalar", true, 608, 1, 4, 0},
        {"_0_0_fogData[1].heightFogCoeff", true, 624, 1, 4, 0},
        {"_0_0_fogData[1].mainFogCoeff", true, 640, 1, 4, 0},
        {"_0_0_fogData[1].heightDensityFogCoeff", true, 656, 1, 4, 0},
        {"_0_0_fogData[1].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 672, 1, 4, 0},
        {"_0_0_fogData[1].heightFogEndColor_fogStartOffset", true, 688, 1, 4, 0},
        {"_0_0_fogData[2].densityParams", true, 704, 1, 4, 0},
        {"_0_0_fogData[2].classicFogParams", true, 720, 1, 4, 0},
        {"_0_0_fogData[2].heightPlane", true, 736, 1, 4, 0},
        {"_0_0_fogData[2].color_and_heightRate", true, 752, 1, 4, 0},
        {"_0_0_fogData[2].heightDensity_and_endColor", true, 768, 1, 4, 0},
        {"_0_0_fogData[2].sunAngle_and_sunColor", true, 784, 1, 4, 0},
        {"_0_0_fogData[2].heightColor_and_endFogDistance", true, 800, 1, 4, 0},
        {"_0_0_fogData[2].sunPercentage", true, 816, 1, 4, 0},
        {"_0_0_fogData[2].sunDirection_and_fogZScalar", true, 832, 1, 4, 0},
        {"_0_0_fogData[2].heightFogCoeff", true, 848, 1, 4, 0},
        {"_0_0_fogData[2].mainFogCoeff", true, 864, 1, 4, 0},
        {"_0_0_fogData[2].heightDensityFogCoeff", true, 880, 1, 4, 0},
        {"_0_0_fogData[2].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 896, 1, 4, 0},
        {"_0_0_fogData[2].heightFogEndColor_fogStartOffset", true, 912, 1, 4, 0},
        {"_0_0_fogData[3].densityParams", true, 928, 1, 4, 0},
        {"_0_0_fogData[3].classicFogParams", true, 944, 1, 4, 0},
        {"_0_0_fogData[3].heightPlane", true, 960, 1, 4, 0},
        {"_0_0_fogData[3].color_and_heightRate", true, 976, 1, 4, 0},
        {"_0_0_fogData[3].heightDensity_and_endColor", true, 992, 1, 4, 0},
        {"_0_0_fogData[3].sunAngle_and_sunColor", true, 1008, 1, 4, 0},
        {"_0_0_fogData[3].heightColor_and_endFogDistance", true, 1024, 1, 4, 0},
        {"_0_0_fogData[3].sunPercentage", true, 1040, 1, 4, 0},
        {"_0_0_fogData[3].sunDirection_and_fogZScalar", true, 1056, 1, 4, 0},
        {"_0_0_fogData[3].heightFogCoeff", true, 1072, 1, 4, 0},
        {"_0_0_fogData[3].mainFogCoeff", true, 1088, 1, 4, 0},
        {"_0_0_fogData[3].heightDensityFogCoeff", true, 1104, 1, 4, 0},
        {"_0_0_fogData[3].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1120, 1, 4, 0},
        {"_0_0_fogData[3].heightFogEndColor_fogStartOffset", true, 1136, 1, 4, 0},
        {"_0_0_fogData[4].densityParams", true, 1152, 1, 4, 0},
        {"_0_0_fogData[4].classicFogParams", true, 1168, 1, 4, 0},
        {"_0_0_fogData[4].heightPlane", true, 1184, 1, 4, 0},
        {"_0_0_fogData[4].color_and_heightRate", true, 1200, 1, 4, 0},
        {"_0_0_fogData[4].heightDensity_and_endColor", true, 1216, 1, 4, 0},
        {"_0_0_fogData[4].sunAngle_and_sunColor", true, 1232, 1, 4, 0},
        {"_0_0_fogData[4].heightColor_and_endFogDistance", true, 1248, 1, 4, 0},
        {"_0_0_fogData[4].sunPercentage", true, 1264, 1, 4, 0},
        {"_0_0_fogData[4].sunDirection_and_fogZScalar", true, 1280, 1, 4, 0},
        {"_0_0_fogData[4].heightFogCoeff", true, 1296, 1, 4, 0},
        {"_0_0_fogData[4].mainFogCoeff", true, 1312, 1, 4, 0},
        {"_0_0_fogData[4].heightDensityFogCoeff", true, 1328, 1, 4, 0},
        {"_0_0_fogData[4].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1344, 1, 4, 0},
        {"_0_0_fogData[4].heightFogEndColor_fogStartOffset", true, 1360, 1, 4, 0},
        {"_0_0_fogData[5].densityParams", true, 1376, 1, 4, 0},
        {"_0_0_fogData[5].classicFogParams", true, 1392, 1, 4, 0},
        {"_0_0_fogData[5].heightPlane", true, 1408, 1, 4, 0},
        {"_0_0_fogData[5].color_and_heightRate", true, 1424, 1, 4, 0},
        {"_0_0_fogData[5].heightDensity_and_endColor", true, 1440, 1, 4, 0},
        {"_0_0_fogData[5].sunAngle_and_sunColor", true, 1456, 1, 4, 0},
        {"_0_0_fogData[5].heightColor_and_endFogDistance", true, 1472, 1, 4, 0},
        {"_0_0_fogData[5].sunPercentage", true, 1488, 1, 4, 0},
        {"_0_0_fogData[5].sunDirection_and_fogZScalar", true, 1504, 1, 4, 0},
        {"_0_0_fogData[5].heightFogCoeff", true, 1520, 1, 4, 0},
        {"_0_0_fogData[5].mainFogCoeff", true, 1536, 1, 4, 0},
        {"_0_0_fogData[5].heightDensityFogCoeff", true, 1552, 1, 4, 0},
        {"_0_0_fogData[5].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1568, 1, 4, 0},
        {"_0_0_fogData[5].heightFogEndColor_fogStartOffset", true, 1584, 1, 4, 0},
        {"_0_0_fogData[6].densityParams", true, 1600, 1, 4, 0},
        {"_0_0_fogData[6].classicFogParams", true, 1616, 1, 4, 0},
        {"_0_0_fogData[6].heightPlane", true, 1632, 1, 4, 0},
        {"_0_0_fogData[6].color_and_heightRate", true, 1648, 1, 4, 0},
        {"_0_0_fogData[6].heightDensity_and_endColor", true, 1664, 1, 4, 0},
        {"_0_0_fogData[6].sunAngle_and_sunColor", true, 1680, 1, 4, 0},
        {"_0_0_fogData[6].heightColor_and_endFogDistance", true, 1696, 1, 4, 0},
        {"_0_0_fogData[6].sunPercentage", true, 1712, 1, 4, 0},
        {"_0_0_fogData[6].sunDirection_and_fogZScalar", true, 1728, 1, 4, 0},
        {"_0_0_fogData[6].heightFogCoeff", true, 1744, 1, 4, 0},
        {"_0_0_fogData[6].mainFogCoeff", true, 1760, 1, 4, 0},
        {"_0_0_fogData[6].heightDensityFogCoeff", true, 1776, 1, 4, 0},
        {"_0_0_fogData[6].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1792, 1, 4, 0},
        {"_0_0_fogData[6].heightFogEndColor_fogStartOffset", true, 1808, 1, 4, 0},
        {"_0_0_fogData[7].densityParams", true, 1824, 1, 4, 0},
        {"_0_0_fogData[7].classicFogParams", true, 1840, 1, 4, 0},
        {"_0_0_fogData[7].heightPlane", true, 1856, 1, 4, 0},
        {"_0_0_fogData[7].color_and_heightRate", true, 1872, 1, 4, 0},
        {"_0_0_fogData[7].heightDensity_and_endColor", true, 1888, 1, 4, 0},
        {"_0_0_fogData[7].sunAngle_and_sunColor", true, 1904, 1, 4, 0},
        {"_0_0_fogData[7].heightColor_and_endFogDistance", true, 1920, 1, 4, 0},
        {"_0_0_fogData[7].sunPercentage", true, 1936, 1, 4, 0},
        {"_0_0_fogData[7].sunDirection_and_fogZScalar", true, 1952, 1, 4, 0},
        {"_0_0_fogData[7].heightFogCoeff", true, 1968, 1, 4, 0},
        {"_0_0_fogData[7].mainFogCoeff", true, 1984, 1, 4, 0},
        {"_0_0_fogData[7].heightDensityFogCoeff", true, 2000, 1, 4, 0},
        {"_0_0_fogData[7].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 2016, 1, 4, 0},
        {"_0_0_fogData[7].heightFogEndColor_fogStartOffset", true, 2032, 1, 4, 0},
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
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 240, 1, 4, 0},
        {"_0_0_fogData[0].densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData[0].classicFogParams", true, 272, 1, 4, 0},
        {"_0_0_fogData[0].heightPlane", true, 288, 1, 4, 0},
        {"_0_0_fogData[0].color_and_heightRate", true, 304, 1, 4, 0},
        {"_0_0_fogData[0].heightDensity_and_endColor", true, 320, 1, 4, 0},
        {"_0_0_fogData[0].sunAngle_and_sunColor", true, 336, 1, 4, 0},
        {"_0_0_fogData[0].heightColor_and_endFogDistance", true, 352, 1, 4, 0},
        {"_0_0_fogData[0].sunPercentage", true, 368, 1, 4, 0},
        {"_0_0_fogData[0].sunDirection_and_fogZScalar", true, 384, 1, 4, 0},
        {"_0_0_fogData[0].heightFogCoeff", true, 400, 1, 4, 0},
        {"_0_0_fogData[0].mainFogCoeff", true, 416, 1, 4, 0},
        {"_0_0_fogData[0].heightDensityFogCoeff", true, 432, 1, 4, 0},
        {"_0_0_fogData[0].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 448, 1, 4, 0},
        {"_0_0_fogData[0].heightFogEndColor_fogStartOffset", true, 464, 1, 4, 0},
        {"_0_0_fogData[1].densityParams", true, 480, 1, 4, 0},
        {"_0_0_fogData[1].classicFogParams", true, 496, 1, 4, 0},
        {"_0_0_fogData[1].heightPlane", true, 512, 1, 4, 0},
        {"_0_0_fogData[1].color_and_heightRate", true, 528, 1, 4, 0},
        {"_0_0_fogData[1].heightDensity_and_endColor", true, 544, 1, 4, 0},
        {"_0_0_fogData[1].sunAngle_and_sunColor", true, 560, 1, 4, 0},
        {"_0_0_fogData[1].heightColor_and_endFogDistance", true, 576, 1, 4, 0},
        {"_0_0_fogData[1].sunPercentage", true, 592, 1, 4, 0},
        {"_0_0_fogData[1].sunDirection_and_fogZScalar", true, 608, 1, 4, 0},
        {"_0_0_fogData[1].heightFogCoeff", true, 624, 1, 4, 0},
        {"_0_0_fogData[1].mainFogCoeff", true, 640, 1, 4, 0},
        {"_0_0_fogData[1].heightDensityFogCoeff", true, 656, 1, 4, 0},
        {"_0_0_fogData[1].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 672, 1, 4, 0},
        {"_0_0_fogData[1].heightFogEndColor_fogStartOffset", true, 688, 1, 4, 0},
        {"_0_0_fogData[2].densityParams", true, 704, 1, 4, 0},
        {"_0_0_fogData[2].classicFogParams", true, 720, 1, 4, 0},
        {"_0_0_fogData[2].heightPlane", true, 736, 1, 4, 0},
        {"_0_0_fogData[2].color_and_heightRate", true, 752, 1, 4, 0},
        {"_0_0_fogData[2].heightDensity_and_endColor", true, 768, 1, 4, 0},
        {"_0_0_fogData[2].sunAngle_and_sunColor", true, 784, 1, 4, 0},
        {"_0_0_fogData[2].heightColor_and_endFogDistance", true, 800, 1, 4, 0},
        {"_0_0_fogData[2].sunPercentage", true, 816, 1, 4, 0},
        {"_0_0_fogData[2].sunDirection_and_fogZScalar", true, 832, 1, 4, 0},
        {"_0_0_fogData[2].heightFogCoeff", true, 848, 1, 4, 0},
        {"_0_0_fogData[2].mainFogCoeff", true, 864, 1, 4, 0},
        {"_0_0_fogData[2].heightDensityFogCoeff", true, 880, 1, 4, 0},
        {"_0_0_fogData[2].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 896, 1, 4, 0},
        {"_0_0_fogData[2].heightFogEndColor_fogStartOffset", true, 912, 1, 4, 0},
        {"_0_0_fogData[3].densityParams", true, 928, 1, 4, 0},
        {"_0_0_fogData[3].classicFogParams", true, 944, 1, 4, 0},
        {"_0_0_fogData[3].heightPlane", true, 960, 1, 4, 0},
        {"_0_0_fogData[3].color_and_heightRate", true, 976, 1, 4, 0},
        {"_0_0_fogData[3].heightDensity_and_endColor", true, 992, 1, 4, 0},
        {"_0_0_fogData[3].sunAngle_and_sunColor", true, 1008, 1, 4, 0},
        {"_0_0_fogData[3].heightColor_and_endFogDistance", true, 1024, 1, 4, 0},
        {"_0_0_fogData[3].sunPercentage", true, 1040, 1, 4, 0},
        {"_0_0_fogData[3].sunDirection_and_fogZScalar", true, 1056, 1, 4, 0},
        {"_0_0_fogData[3].heightFogCoeff", true, 1072, 1, 4, 0},
        {"_0_0_fogData[3].mainFogCoeff", true, 1088, 1, 4, 0},
        {"_0_0_fogData[3].heightDensityFogCoeff", true, 1104, 1, 4, 0},
        {"_0_0_fogData[3].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1120, 1, 4, 0},
        {"_0_0_fogData[3].heightFogEndColor_fogStartOffset", true, 1136, 1, 4, 0},
        {"_0_0_fogData[4].densityParams", true, 1152, 1, 4, 0},
        {"_0_0_fogData[4].classicFogParams", true, 1168, 1, 4, 0},
        {"_0_0_fogData[4].heightPlane", true, 1184, 1, 4, 0},
        {"_0_0_fogData[4].color_and_heightRate", true, 1200, 1, 4, 0},
        {"_0_0_fogData[4].heightDensity_and_endColor", true, 1216, 1, 4, 0},
        {"_0_0_fogData[4].sunAngle_and_sunColor", true, 1232, 1, 4, 0},
        {"_0_0_fogData[4].heightColor_and_endFogDistance", true, 1248, 1, 4, 0},
        {"_0_0_fogData[4].sunPercentage", true, 1264, 1, 4, 0},
        {"_0_0_fogData[4].sunDirection_and_fogZScalar", true, 1280, 1, 4, 0},
        {"_0_0_fogData[4].heightFogCoeff", true, 1296, 1, 4, 0},
        {"_0_0_fogData[4].mainFogCoeff", true, 1312, 1, 4, 0},
        {"_0_0_fogData[4].heightDensityFogCoeff", true, 1328, 1, 4, 0},
        {"_0_0_fogData[4].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1344, 1, 4, 0},
        {"_0_0_fogData[4].heightFogEndColor_fogStartOffset", true, 1360, 1, 4, 0},
        {"_0_0_fogData[5].densityParams", true, 1376, 1, 4, 0},
        {"_0_0_fogData[5].classicFogParams", true, 1392, 1, 4, 0},
        {"_0_0_fogData[5].heightPlane", true, 1408, 1, 4, 0},
        {"_0_0_fogData[5].color_and_heightRate", true, 1424, 1, 4, 0},
        {"_0_0_fogData[5].heightDensity_and_endColor", true, 1440, 1, 4, 0},
        {"_0_0_fogData[5].sunAngle_and_sunColor", true, 1456, 1, 4, 0},
        {"_0_0_fogData[5].heightColor_and_endFogDistance", true, 1472, 1, 4, 0},
        {"_0_0_fogData[5].sunPercentage", true, 1488, 1, 4, 0},
        {"_0_0_fogData[5].sunDirection_and_fogZScalar", true, 1504, 1, 4, 0},
        {"_0_0_fogData[5].heightFogCoeff", true, 1520, 1, 4, 0},
        {"_0_0_fogData[5].mainFogCoeff", true, 1536, 1, 4, 0},
        {"_0_0_fogData[5].heightDensityFogCoeff", true, 1552, 1, 4, 0},
        {"_0_0_fogData[5].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1568, 1, 4, 0},
        {"_0_0_fogData[5].heightFogEndColor_fogStartOffset", true, 1584, 1, 4, 0},
        {"_0_0_fogData[6].densityParams", true, 1600, 1, 4, 0},
        {"_0_0_fogData[6].classicFogParams", true, 1616, 1, 4, 0},
        {"_0_0_fogData[6].heightPlane", true, 1632, 1, 4, 0},
        {"_0_0_fogData[6].color_and_heightRate", true, 1648, 1, 4, 0},
        {"_0_0_fogData[6].heightDensity_and_endColor", true, 1664, 1, 4, 0},
        {"_0_0_fogData[6].sunAngle_and_sunColor", true, 1680, 1, 4, 0},
        {"_0_0_fogData[6].heightColor_and_endFogDistance", true, 1696, 1, 4, 0},
        {"_0_0_fogData[6].sunPercentage", true, 1712, 1, 4, 0},
        {"_0_0_fogData[6].sunDirection_and_fogZScalar", true, 1728, 1, 4, 0},
        {"_0_0_fogData[6].heightFogCoeff", true, 1744, 1, 4, 0},
        {"_0_0_fogData[6].mainFogCoeff", true, 1760, 1, 4, 0},
        {"_0_0_fogData[6].heightDensityFogCoeff", true, 1776, 1, 4, 0},
        {"_0_0_fogData[6].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1792, 1, 4, 0},
        {"_0_0_fogData[6].heightFogEndColor_fogStartOffset", true, 1808, 1, 4, 0},
        {"_0_0_fogData[7].densityParams", true, 1824, 1, 4, 0},
        {"_0_0_fogData[7].classicFogParams", true, 1840, 1, 4, 0},
        {"_0_0_fogData[7].heightPlane", true, 1856, 1, 4, 0},
        {"_0_0_fogData[7].color_and_heightRate", true, 1872, 1, 4, 0},
        {"_0_0_fogData[7].heightDensity_and_endColor", true, 1888, 1, 4, 0},
        {"_0_0_fogData[7].sunAngle_and_sunColor", true, 1904, 1, 4, 0},
        {"_0_0_fogData[7].heightColor_and_endFogDistance", true, 1920, 1, 4, 0},
        {"_0_0_fogData[7].sunPercentage", true, 1936, 1, 4, 0},
        {"_0_0_fogData[7].sunDirection_and_fogZScalar", true, 1952, 1, 4, 0},
        {"_0_0_fogData[7].heightFogCoeff", true, 1968, 1, 4, 0},
        {"_0_0_fogData[7].mainFogCoeff", true, 1984, 1, 4, 0},
        {"_0_0_fogData[7].heightDensityFogCoeff", true, 2000, 1, 4, 0},
        {"_0_0_fogData[7].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 2016, 1, 4, 0},
        {"_0_0_fogData[7].heightFogEndColor_fogStartOffset", true, 2032, 1, 4, 0},
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
  {"drawQuad",  {
    {
      2, {
        {"_0_2_uWidth_uHeight_uX_uY", true, 0, 1, 4, 0},
      }
    },
  }},
  {"skyConus",  {
    {
      1, {
        {"_1_1_skyColor[0]", true, 0, 1, 4, 6},
      }
    },
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 240, 1, 4, 0},
        {"_0_0_fogData[0].densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData[0].classicFogParams", true, 272, 1, 4, 0},
        {"_0_0_fogData[0].heightPlane", true, 288, 1, 4, 0},
        {"_0_0_fogData[0].color_and_heightRate", true, 304, 1, 4, 0},
        {"_0_0_fogData[0].heightDensity_and_endColor", true, 320, 1, 4, 0},
        {"_0_0_fogData[0].sunAngle_and_sunColor", true, 336, 1, 4, 0},
        {"_0_0_fogData[0].heightColor_and_endFogDistance", true, 352, 1, 4, 0},
        {"_0_0_fogData[0].sunPercentage", true, 368, 1, 4, 0},
        {"_0_0_fogData[0].sunDirection_and_fogZScalar", true, 384, 1, 4, 0},
        {"_0_0_fogData[0].heightFogCoeff", true, 400, 1, 4, 0},
        {"_0_0_fogData[0].mainFogCoeff", true, 416, 1, 4, 0},
        {"_0_0_fogData[0].heightDensityFogCoeff", true, 432, 1, 4, 0},
        {"_0_0_fogData[0].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 448, 1, 4, 0},
        {"_0_0_fogData[0].heightFogEndColor_fogStartOffset", true, 464, 1, 4, 0},
        {"_0_0_fogData[1].densityParams", true, 480, 1, 4, 0},
        {"_0_0_fogData[1].classicFogParams", true, 496, 1, 4, 0},
        {"_0_0_fogData[1].heightPlane", true, 512, 1, 4, 0},
        {"_0_0_fogData[1].color_and_heightRate", true, 528, 1, 4, 0},
        {"_0_0_fogData[1].heightDensity_and_endColor", true, 544, 1, 4, 0},
        {"_0_0_fogData[1].sunAngle_and_sunColor", true, 560, 1, 4, 0},
        {"_0_0_fogData[1].heightColor_and_endFogDistance", true, 576, 1, 4, 0},
        {"_0_0_fogData[1].sunPercentage", true, 592, 1, 4, 0},
        {"_0_0_fogData[1].sunDirection_and_fogZScalar", true, 608, 1, 4, 0},
        {"_0_0_fogData[1].heightFogCoeff", true, 624, 1, 4, 0},
        {"_0_0_fogData[1].mainFogCoeff", true, 640, 1, 4, 0},
        {"_0_0_fogData[1].heightDensityFogCoeff", true, 656, 1, 4, 0},
        {"_0_0_fogData[1].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 672, 1, 4, 0},
        {"_0_0_fogData[1].heightFogEndColor_fogStartOffset", true, 688, 1, 4, 0},
        {"_0_0_fogData[2].densityParams", true, 704, 1, 4, 0},
        {"_0_0_fogData[2].classicFogParams", true, 720, 1, 4, 0},
        {"_0_0_fogData[2].heightPlane", true, 736, 1, 4, 0},
        {"_0_0_fogData[2].color_and_heightRate", true, 752, 1, 4, 0},
        {"_0_0_fogData[2].heightDensity_and_endColor", true, 768, 1, 4, 0},
        {"_0_0_fogData[2].sunAngle_and_sunColor", true, 784, 1, 4, 0},
        {"_0_0_fogData[2].heightColor_and_endFogDistance", true, 800, 1, 4, 0},
        {"_0_0_fogData[2].sunPercentage", true, 816, 1, 4, 0},
        {"_0_0_fogData[2].sunDirection_and_fogZScalar", true, 832, 1, 4, 0},
        {"_0_0_fogData[2].heightFogCoeff", true, 848, 1, 4, 0},
        {"_0_0_fogData[2].mainFogCoeff", true, 864, 1, 4, 0},
        {"_0_0_fogData[2].heightDensityFogCoeff", true, 880, 1, 4, 0},
        {"_0_0_fogData[2].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 896, 1, 4, 0},
        {"_0_0_fogData[2].heightFogEndColor_fogStartOffset", true, 912, 1, 4, 0},
        {"_0_0_fogData[3].densityParams", true, 928, 1, 4, 0},
        {"_0_0_fogData[3].classicFogParams", true, 944, 1, 4, 0},
        {"_0_0_fogData[3].heightPlane", true, 960, 1, 4, 0},
        {"_0_0_fogData[3].color_and_heightRate", true, 976, 1, 4, 0},
        {"_0_0_fogData[3].heightDensity_and_endColor", true, 992, 1, 4, 0},
        {"_0_0_fogData[3].sunAngle_and_sunColor", true, 1008, 1, 4, 0},
        {"_0_0_fogData[3].heightColor_and_endFogDistance", true, 1024, 1, 4, 0},
        {"_0_0_fogData[3].sunPercentage", true, 1040, 1, 4, 0},
        {"_0_0_fogData[3].sunDirection_and_fogZScalar", true, 1056, 1, 4, 0},
        {"_0_0_fogData[3].heightFogCoeff", true, 1072, 1, 4, 0},
        {"_0_0_fogData[3].mainFogCoeff", true, 1088, 1, 4, 0},
        {"_0_0_fogData[3].heightDensityFogCoeff", true, 1104, 1, 4, 0},
        {"_0_0_fogData[3].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1120, 1, 4, 0},
        {"_0_0_fogData[3].heightFogEndColor_fogStartOffset", true, 1136, 1, 4, 0},
        {"_0_0_fogData[4].densityParams", true, 1152, 1, 4, 0},
        {"_0_0_fogData[4].classicFogParams", true, 1168, 1, 4, 0},
        {"_0_0_fogData[4].heightPlane", true, 1184, 1, 4, 0},
        {"_0_0_fogData[4].color_and_heightRate", true, 1200, 1, 4, 0},
        {"_0_0_fogData[4].heightDensity_and_endColor", true, 1216, 1, 4, 0},
        {"_0_0_fogData[4].sunAngle_and_sunColor", true, 1232, 1, 4, 0},
        {"_0_0_fogData[4].heightColor_and_endFogDistance", true, 1248, 1, 4, 0},
        {"_0_0_fogData[4].sunPercentage", true, 1264, 1, 4, 0},
        {"_0_0_fogData[4].sunDirection_and_fogZScalar", true, 1280, 1, 4, 0},
        {"_0_0_fogData[4].heightFogCoeff", true, 1296, 1, 4, 0},
        {"_0_0_fogData[4].mainFogCoeff", true, 1312, 1, 4, 0},
        {"_0_0_fogData[4].heightDensityFogCoeff", true, 1328, 1, 4, 0},
        {"_0_0_fogData[4].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1344, 1, 4, 0},
        {"_0_0_fogData[4].heightFogEndColor_fogStartOffset", true, 1360, 1, 4, 0},
        {"_0_0_fogData[5].densityParams", true, 1376, 1, 4, 0},
        {"_0_0_fogData[5].classicFogParams", true, 1392, 1, 4, 0},
        {"_0_0_fogData[5].heightPlane", true, 1408, 1, 4, 0},
        {"_0_0_fogData[5].color_and_heightRate", true, 1424, 1, 4, 0},
        {"_0_0_fogData[5].heightDensity_and_endColor", true, 1440, 1, 4, 0},
        {"_0_0_fogData[5].sunAngle_and_sunColor", true, 1456, 1, 4, 0},
        {"_0_0_fogData[5].heightColor_and_endFogDistance", true, 1472, 1, 4, 0},
        {"_0_0_fogData[5].sunPercentage", true, 1488, 1, 4, 0},
        {"_0_0_fogData[5].sunDirection_and_fogZScalar", true, 1504, 1, 4, 0},
        {"_0_0_fogData[5].heightFogCoeff", true, 1520, 1, 4, 0},
        {"_0_0_fogData[5].mainFogCoeff", true, 1536, 1, 4, 0},
        {"_0_0_fogData[5].heightDensityFogCoeff", true, 1552, 1, 4, 0},
        {"_0_0_fogData[5].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1568, 1, 4, 0},
        {"_0_0_fogData[5].heightFogEndColor_fogStartOffset", true, 1584, 1, 4, 0},
        {"_0_0_fogData[6].densityParams", true, 1600, 1, 4, 0},
        {"_0_0_fogData[6].classicFogParams", true, 1616, 1, 4, 0},
        {"_0_0_fogData[6].heightPlane", true, 1632, 1, 4, 0},
        {"_0_0_fogData[6].color_and_heightRate", true, 1648, 1, 4, 0},
        {"_0_0_fogData[6].heightDensity_and_endColor", true, 1664, 1, 4, 0},
        {"_0_0_fogData[6].sunAngle_and_sunColor", true, 1680, 1, 4, 0},
        {"_0_0_fogData[6].heightColor_and_endFogDistance", true, 1696, 1, 4, 0},
        {"_0_0_fogData[6].sunPercentage", true, 1712, 1, 4, 0},
        {"_0_0_fogData[6].sunDirection_and_fogZScalar", true, 1728, 1, 4, 0},
        {"_0_0_fogData[6].heightFogCoeff", true, 1744, 1, 4, 0},
        {"_0_0_fogData[6].mainFogCoeff", true, 1760, 1, 4, 0},
        {"_0_0_fogData[6].heightDensityFogCoeff", true, 1776, 1, 4, 0},
        {"_0_0_fogData[6].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1792, 1, 4, 0},
        {"_0_0_fogData[6].heightFogEndColor_fogStartOffset", true, 1808, 1, 4, 0},
        {"_0_0_fogData[7].densityParams", true, 1824, 1, 4, 0},
        {"_0_0_fogData[7].classicFogParams", true, 1840, 1, 4, 0},
        {"_0_0_fogData[7].heightPlane", true, 1856, 1, 4, 0},
        {"_0_0_fogData[7].color_and_heightRate", true, 1872, 1, 4, 0},
        {"_0_0_fogData[7].heightDensity_and_endColor", true, 1888, 1, 4, 0},
        {"_0_0_fogData[7].sunAngle_and_sunColor", true, 1904, 1, 4, 0},
        {"_0_0_fogData[7].heightColor_and_endFogDistance", true, 1920, 1, 4, 0},
        {"_0_0_fogData[7].sunPercentage", true, 1936, 1, 4, 0},
        {"_0_0_fogData[7].sunDirection_and_fogZScalar", true, 1952, 1, 4, 0},
        {"_0_0_fogData[7].heightFogCoeff", true, 1968, 1, 4, 0},
        {"_0_0_fogData[7].mainFogCoeff", true, 1984, 1, 4, 0},
        {"_0_0_fogData[7].heightDensityFogCoeff", true, 2000, 1, 4, 0},
        {"_0_0_fogData[7].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 2016, 1, 4, 0},
        {"_0_0_fogData[7].heightFogEndColor_fogStartOffset", true, 2032, 1, 4, 0},
      }
    },
  }},
  {"m2Shader",  {
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 240, 1, 4, 0},
        {"_0_0_fogData[0].densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData[0].classicFogParams", true, 272, 1, 4, 0},
        {"_0_0_fogData[0].heightPlane", true, 288, 1, 4, 0},
        {"_0_0_fogData[0].color_and_heightRate", true, 304, 1, 4, 0},
        {"_0_0_fogData[0].heightDensity_and_endColor", true, 320, 1, 4, 0},
        {"_0_0_fogData[0].sunAngle_and_sunColor", true, 336, 1, 4, 0},
        {"_0_0_fogData[0].heightColor_and_endFogDistance", true, 352, 1, 4, 0},
        {"_0_0_fogData[0].sunPercentage", true, 368, 1, 4, 0},
        {"_0_0_fogData[0].sunDirection_and_fogZScalar", true, 384, 1, 4, 0},
        {"_0_0_fogData[0].heightFogCoeff", true, 400, 1, 4, 0},
        {"_0_0_fogData[0].mainFogCoeff", true, 416, 1, 4, 0},
        {"_0_0_fogData[0].heightDensityFogCoeff", true, 432, 1, 4, 0},
        {"_0_0_fogData[0].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 448, 1, 4, 0},
        {"_0_0_fogData[0].heightFogEndColor_fogStartOffset", true, 464, 1, 4, 0},
        {"_0_0_fogData[1].densityParams", true, 480, 1, 4, 0},
        {"_0_0_fogData[1].classicFogParams", true, 496, 1, 4, 0},
        {"_0_0_fogData[1].heightPlane", true, 512, 1, 4, 0},
        {"_0_0_fogData[1].color_and_heightRate", true, 528, 1, 4, 0},
        {"_0_0_fogData[1].heightDensity_and_endColor", true, 544, 1, 4, 0},
        {"_0_0_fogData[1].sunAngle_and_sunColor", true, 560, 1, 4, 0},
        {"_0_0_fogData[1].heightColor_and_endFogDistance", true, 576, 1, 4, 0},
        {"_0_0_fogData[1].sunPercentage", true, 592, 1, 4, 0},
        {"_0_0_fogData[1].sunDirection_and_fogZScalar", true, 608, 1, 4, 0},
        {"_0_0_fogData[1].heightFogCoeff", true, 624, 1, 4, 0},
        {"_0_0_fogData[1].mainFogCoeff", true, 640, 1, 4, 0},
        {"_0_0_fogData[1].heightDensityFogCoeff", true, 656, 1, 4, 0},
        {"_0_0_fogData[1].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 672, 1, 4, 0},
        {"_0_0_fogData[1].heightFogEndColor_fogStartOffset", true, 688, 1, 4, 0},
        {"_0_0_fogData[2].densityParams", true, 704, 1, 4, 0},
        {"_0_0_fogData[2].classicFogParams", true, 720, 1, 4, 0},
        {"_0_0_fogData[2].heightPlane", true, 736, 1, 4, 0},
        {"_0_0_fogData[2].color_and_heightRate", true, 752, 1, 4, 0},
        {"_0_0_fogData[2].heightDensity_and_endColor", true, 768, 1, 4, 0},
        {"_0_0_fogData[2].sunAngle_and_sunColor", true, 784, 1, 4, 0},
        {"_0_0_fogData[2].heightColor_and_endFogDistance", true, 800, 1, 4, 0},
        {"_0_0_fogData[2].sunPercentage", true, 816, 1, 4, 0},
        {"_0_0_fogData[2].sunDirection_and_fogZScalar", true, 832, 1, 4, 0},
        {"_0_0_fogData[2].heightFogCoeff", true, 848, 1, 4, 0},
        {"_0_0_fogData[2].mainFogCoeff", true, 864, 1, 4, 0},
        {"_0_0_fogData[2].heightDensityFogCoeff", true, 880, 1, 4, 0},
        {"_0_0_fogData[2].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 896, 1, 4, 0},
        {"_0_0_fogData[2].heightFogEndColor_fogStartOffset", true, 912, 1, 4, 0},
        {"_0_0_fogData[3].densityParams", true, 928, 1, 4, 0},
        {"_0_0_fogData[3].classicFogParams", true, 944, 1, 4, 0},
        {"_0_0_fogData[3].heightPlane", true, 960, 1, 4, 0},
        {"_0_0_fogData[3].color_and_heightRate", true, 976, 1, 4, 0},
        {"_0_0_fogData[3].heightDensity_and_endColor", true, 992, 1, 4, 0},
        {"_0_0_fogData[3].sunAngle_and_sunColor", true, 1008, 1, 4, 0},
        {"_0_0_fogData[3].heightColor_and_endFogDistance", true, 1024, 1, 4, 0},
        {"_0_0_fogData[3].sunPercentage", true, 1040, 1, 4, 0},
        {"_0_0_fogData[3].sunDirection_and_fogZScalar", true, 1056, 1, 4, 0},
        {"_0_0_fogData[3].heightFogCoeff", true, 1072, 1, 4, 0},
        {"_0_0_fogData[3].mainFogCoeff", true, 1088, 1, 4, 0},
        {"_0_0_fogData[3].heightDensityFogCoeff", true, 1104, 1, 4, 0},
        {"_0_0_fogData[3].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1120, 1, 4, 0},
        {"_0_0_fogData[3].heightFogEndColor_fogStartOffset", true, 1136, 1, 4, 0},
        {"_0_0_fogData[4].densityParams", true, 1152, 1, 4, 0},
        {"_0_0_fogData[4].classicFogParams", true, 1168, 1, 4, 0},
        {"_0_0_fogData[4].heightPlane", true, 1184, 1, 4, 0},
        {"_0_0_fogData[4].color_and_heightRate", true, 1200, 1, 4, 0},
        {"_0_0_fogData[4].heightDensity_and_endColor", true, 1216, 1, 4, 0},
        {"_0_0_fogData[4].sunAngle_and_sunColor", true, 1232, 1, 4, 0},
        {"_0_0_fogData[4].heightColor_and_endFogDistance", true, 1248, 1, 4, 0},
        {"_0_0_fogData[4].sunPercentage", true, 1264, 1, 4, 0},
        {"_0_0_fogData[4].sunDirection_and_fogZScalar", true, 1280, 1, 4, 0},
        {"_0_0_fogData[4].heightFogCoeff", true, 1296, 1, 4, 0},
        {"_0_0_fogData[4].mainFogCoeff", true, 1312, 1, 4, 0},
        {"_0_0_fogData[4].heightDensityFogCoeff", true, 1328, 1, 4, 0},
        {"_0_0_fogData[4].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1344, 1, 4, 0},
        {"_0_0_fogData[4].heightFogEndColor_fogStartOffset", true, 1360, 1, 4, 0},
        {"_0_0_fogData[5].densityParams", true, 1376, 1, 4, 0},
        {"_0_0_fogData[5].classicFogParams", true, 1392, 1, 4, 0},
        {"_0_0_fogData[5].heightPlane", true, 1408, 1, 4, 0},
        {"_0_0_fogData[5].color_and_heightRate", true, 1424, 1, 4, 0},
        {"_0_0_fogData[5].heightDensity_and_endColor", true, 1440, 1, 4, 0},
        {"_0_0_fogData[5].sunAngle_and_sunColor", true, 1456, 1, 4, 0},
        {"_0_0_fogData[5].heightColor_and_endFogDistance", true, 1472, 1, 4, 0},
        {"_0_0_fogData[5].sunPercentage", true, 1488, 1, 4, 0},
        {"_0_0_fogData[5].sunDirection_and_fogZScalar", true, 1504, 1, 4, 0},
        {"_0_0_fogData[5].heightFogCoeff", true, 1520, 1, 4, 0},
        {"_0_0_fogData[5].mainFogCoeff", true, 1536, 1, 4, 0},
        {"_0_0_fogData[5].heightDensityFogCoeff", true, 1552, 1, 4, 0},
        {"_0_0_fogData[5].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1568, 1, 4, 0},
        {"_0_0_fogData[5].heightFogEndColor_fogStartOffset", true, 1584, 1, 4, 0},
        {"_0_0_fogData[6].densityParams", true, 1600, 1, 4, 0},
        {"_0_0_fogData[6].classicFogParams", true, 1616, 1, 4, 0},
        {"_0_0_fogData[6].heightPlane", true, 1632, 1, 4, 0},
        {"_0_0_fogData[6].color_and_heightRate", true, 1648, 1, 4, 0},
        {"_0_0_fogData[6].heightDensity_and_endColor", true, 1664, 1, 4, 0},
        {"_0_0_fogData[6].sunAngle_and_sunColor", true, 1680, 1, 4, 0},
        {"_0_0_fogData[6].heightColor_and_endFogDistance", true, 1696, 1, 4, 0},
        {"_0_0_fogData[6].sunPercentage", true, 1712, 1, 4, 0},
        {"_0_0_fogData[6].sunDirection_and_fogZScalar", true, 1728, 1, 4, 0},
        {"_0_0_fogData[6].heightFogCoeff", true, 1744, 1, 4, 0},
        {"_0_0_fogData[6].mainFogCoeff", true, 1760, 1, 4, 0},
        {"_0_0_fogData[6].heightDensityFogCoeff", true, 1776, 1, 4, 0},
        {"_0_0_fogData[6].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1792, 1, 4, 0},
        {"_0_0_fogData[6].heightFogEndColor_fogStartOffset", true, 1808, 1, 4, 0},
        {"_0_0_fogData[7].densityParams", true, 1824, 1, 4, 0},
        {"_0_0_fogData[7].classicFogParams", true, 1840, 1, 4, 0},
        {"_0_0_fogData[7].heightPlane", true, 1856, 1, 4, 0},
        {"_0_0_fogData[7].color_and_heightRate", true, 1872, 1, 4, 0},
        {"_0_0_fogData[7].heightDensity_and_endColor", true, 1888, 1, 4, 0},
        {"_0_0_fogData[7].sunAngle_and_sunColor", true, 1904, 1, 4, 0},
        {"_0_0_fogData[7].heightColor_and_endFogDistance", true, 1920, 1, 4, 0},
        {"_0_0_fogData[7].sunPercentage", true, 1936, 1, 4, 0},
        {"_0_0_fogData[7].sunDirection_and_fogZScalar", true, 1952, 1, 4, 0},
        {"_0_0_fogData[7].heightFogCoeff", true, 1968, 1, 4, 0},
        {"_0_0_fogData[7].mainFogCoeff", true, 1984, 1, 4, 0},
        {"_0_0_fogData[7].heightDensityFogCoeff", true, 2000, 1, 4, 0},
        {"_0_0_fogData[7].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 2016, 1, 4, 0},
        {"_0_0_fogData[7].heightFogEndColor_fogStartOffset", true, 2032, 1, 4, 0},
      }
    },
    {
      1, {
        {"_1_1_uPlacementMat", true, 0, 4, 4, 0},
      }
    },
    {
      3, {
        {"_1_3_uBoneMatrixes[0]", true, 0, 4, 4, 256},
      }
    },
  }},
  {"drawPortalShader",  {
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 240, 1, 4, 0},
        {"_0_0_fogData[0].densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData[0].classicFogParams", true, 272, 1, 4, 0},
        {"_0_0_fogData[0].heightPlane", true, 288, 1, 4, 0},
        {"_0_0_fogData[0].color_and_heightRate", true, 304, 1, 4, 0},
        {"_0_0_fogData[0].heightDensity_and_endColor", true, 320, 1, 4, 0},
        {"_0_0_fogData[0].sunAngle_and_sunColor", true, 336, 1, 4, 0},
        {"_0_0_fogData[0].heightColor_and_endFogDistance", true, 352, 1, 4, 0},
        {"_0_0_fogData[0].sunPercentage", true, 368, 1, 4, 0},
        {"_0_0_fogData[0].sunDirection_and_fogZScalar", true, 384, 1, 4, 0},
        {"_0_0_fogData[0].heightFogCoeff", true, 400, 1, 4, 0},
        {"_0_0_fogData[0].mainFogCoeff", true, 416, 1, 4, 0},
        {"_0_0_fogData[0].heightDensityFogCoeff", true, 432, 1, 4, 0},
        {"_0_0_fogData[0].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 448, 1, 4, 0},
        {"_0_0_fogData[0].heightFogEndColor_fogStartOffset", true, 464, 1, 4, 0},
        {"_0_0_fogData[1].densityParams", true, 480, 1, 4, 0},
        {"_0_0_fogData[1].classicFogParams", true, 496, 1, 4, 0},
        {"_0_0_fogData[1].heightPlane", true, 512, 1, 4, 0},
        {"_0_0_fogData[1].color_and_heightRate", true, 528, 1, 4, 0},
        {"_0_0_fogData[1].heightDensity_and_endColor", true, 544, 1, 4, 0},
        {"_0_0_fogData[1].sunAngle_and_sunColor", true, 560, 1, 4, 0},
        {"_0_0_fogData[1].heightColor_and_endFogDistance", true, 576, 1, 4, 0},
        {"_0_0_fogData[1].sunPercentage", true, 592, 1, 4, 0},
        {"_0_0_fogData[1].sunDirection_and_fogZScalar", true, 608, 1, 4, 0},
        {"_0_0_fogData[1].heightFogCoeff", true, 624, 1, 4, 0},
        {"_0_0_fogData[1].mainFogCoeff", true, 640, 1, 4, 0},
        {"_0_0_fogData[1].heightDensityFogCoeff", true, 656, 1, 4, 0},
        {"_0_0_fogData[1].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 672, 1, 4, 0},
        {"_0_0_fogData[1].heightFogEndColor_fogStartOffset", true, 688, 1, 4, 0},
        {"_0_0_fogData[2].densityParams", true, 704, 1, 4, 0},
        {"_0_0_fogData[2].classicFogParams", true, 720, 1, 4, 0},
        {"_0_0_fogData[2].heightPlane", true, 736, 1, 4, 0},
        {"_0_0_fogData[2].color_and_heightRate", true, 752, 1, 4, 0},
        {"_0_0_fogData[2].heightDensity_and_endColor", true, 768, 1, 4, 0},
        {"_0_0_fogData[2].sunAngle_and_sunColor", true, 784, 1, 4, 0},
        {"_0_0_fogData[2].heightColor_and_endFogDistance", true, 800, 1, 4, 0},
        {"_0_0_fogData[2].sunPercentage", true, 816, 1, 4, 0},
        {"_0_0_fogData[2].sunDirection_and_fogZScalar", true, 832, 1, 4, 0},
        {"_0_0_fogData[2].heightFogCoeff", true, 848, 1, 4, 0},
        {"_0_0_fogData[2].mainFogCoeff", true, 864, 1, 4, 0},
        {"_0_0_fogData[2].heightDensityFogCoeff", true, 880, 1, 4, 0},
        {"_0_0_fogData[2].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 896, 1, 4, 0},
        {"_0_0_fogData[2].heightFogEndColor_fogStartOffset", true, 912, 1, 4, 0},
        {"_0_0_fogData[3].densityParams", true, 928, 1, 4, 0},
        {"_0_0_fogData[3].classicFogParams", true, 944, 1, 4, 0},
        {"_0_0_fogData[3].heightPlane", true, 960, 1, 4, 0},
        {"_0_0_fogData[3].color_and_heightRate", true, 976, 1, 4, 0},
        {"_0_0_fogData[3].heightDensity_and_endColor", true, 992, 1, 4, 0},
        {"_0_0_fogData[3].sunAngle_and_sunColor", true, 1008, 1, 4, 0},
        {"_0_0_fogData[3].heightColor_and_endFogDistance", true, 1024, 1, 4, 0},
        {"_0_0_fogData[3].sunPercentage", true, 1040, 1, 4, 0},
        {"_0_0_fogData[3].sunDirection_and_fogZScalar", true, 1056, 1, 4, 0},
        {"_0_0_fogData[3].heightFogCoeff", true, 1072, 1, 4, 0},
        {"_0_0_fogData[3].mainFogCoeff", true, 1088, 1, 4, 0},
        {"_0_0_fogData[3].heightDensityFogCoeff", true, 1104, 1, 4, 0},
        {"_0_0_fogData[3].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1120, 1, 4, 0},
        {"_0_0_fogData[3].heightFogEndColor_fogStartOffset", true, 1136, 1, 4, 0},
        {"_0_0_fogData[4].densityParams", true, 1152, 1, 4, 0},
        {"_0_0_fogData[4].classicFogParams", true, 1168, 1, 4, 0},
        {"_0_0_fogData[4].heightPlane", true, 1184, 1, 4, 0},
        {"_0_0_fogData[4].color_and_heightRate", true, 1200, 1, 4, 0},
        {"_0_0_fogData[4].heightDensity_and_endColor", true, 1216, 1, 4, 0},
        {"_0_0_fogData[4].sunAngle_and_sunColor", true, 1232, 1, 4, 0},
        {"_0_0_fogData[4].heightColor_and_endFogDistance", true, 1248, 1, 4, 0},
        {"_0_0_fogData[4].sunPercentage", true, 1264, 1, 4, 0},
        {"_0_0_fogData[4].sunDirection_and_fogZScalar", true, 1280, 1, 4, 0},
        {"_0_0_fogData[4].heightFogCoeff", true, 1296, 1, 4, 0},
        {"_0_0_fogData[4].mainFogCoeff", true, 1312, 1, 4, 0},
        {"_0_0_fogData[4].heightDensityFogCoeff", true, 1328, 1, 4, 0},
        {"_0_0_fogData[4].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1344, 1, 4, 0},
        {"_0_0_fogData[4].heightFogEndColor_fogStartOffset", true, 1360, 1, 4, 0},
        {"_0_0_fogData[5].densityParams", true, 1376, 1, 4, 0},
        {"_0_0_fogData[5].classicFogParams", true, 1392, 1, 4, 0},
        {"_0_0_fogData[5].heightPlane", true, 1408, 1, 4, 0},
        {"_0_0_fogData[5].color_and_heightRate", true, 1424, 1, 4, 0},
        {"_0_0_fogData[5].heightDensity_and_endColor", true, 1440, 1, 4, 0},
        {"_0_0_fogData[5].sunAngle_and_sunColor", true, 1456, 1, 4, 0},
        {"_0_0_fogData[5].heightColor_and_endFogDistance", true, 1472, 1, 4, 0},
        {"_0_0_fogData[5].sunPercentage", true, 1488, 1, 4, 0},
        {"_0_0_fogData[5].sunDirection_and_fogZScalar", true, 1504, 1, 4, 0},
        {"_0_0_fogData[5].heightFogCoeff", true, 1520, 1, 4, 0},
        {"_0_0_fogData[5].mainFogCoeff", true, 1536, 1, 4, 0},
        {"_0_0_fogData[5].heightDensityFogCoeff", true, 1552, 1, 4, 0},
        {"_0_0_fogData[5].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1568, 1, 4, 0},
        {"_0_0_fogData[5].heightFogEndColor_fogStartOffset", true, 1584, 1, 4, 0},
        {"_0_0_fogData[6].densityParams", true, 1600, 1, 4, 0},
        {"_0_0_fogData[6].classicFogParams", true, 1616, 1, 4, 0},
        {"_0_0_fogData[6].heightPlane", true, 1632, 1, 4, 0},
        {"_0_0_fogData[6].color_and_heightRate", true, 1648, 1, 4, 0},
        {"_0_0_fogData[6].heightDensity_and_endColor", true, 1664, 1, 4, 0},
        {"_0_0_fogData[6].sunAngle_and_sunColor", true, 1680, 1, 4, 0},
        {"_0_0_fogData[6].heightColor_and_endFogDistance", true, 1696, 1, 4, 0},
        {"_0_0_fogData[6].sunPercentage", true, 1712, 1, 4, 0},
        {"_0_0_fogData[6].sunDirection_and_fogZScalar", true, 1728, 1, 4, 0},
        {"_0_0_fogData[6].heightFogCoeff", true, 1744, 1, 4, 0},
        {"_0_0_fogData[6].mainFogCoeff", true, 1760, 1, 4, 0},
        {"_0_0_fogData[6].heightDensityFogCoeff", true, 1776, 1, 4, 0},
        {"_0_0_fogData[6].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1792, 1, 4, 0},
        {"_0_0_fogData[6].heightFogEndColor_fogStartOffset", true, 1808, 1, 4, 0},
        {"_0_0_fogData[7].densityParams", true, 1824, 1, 4, 0},
        {"_0_0_fogData[7].classicFogParams", true, 1840, 1, 4, 0},
        {"_0_0_fogData[7].heightPlane", true, 1856, 1, 4, 0},
        {"_0_0_fogData[7].color_and_heightRate", true, 1872, 1, 4, 0},
        {"_0_0_fogData[7].heightDensity_and_endColor", true, 1888, 1, 4, 0},
        {"_0_0_fogData[7].sunAngle_and_sunColor", true, 1904, 1, 4, 0},
        {"_0_0_fogData[7].heightColor_and_endFogDistance", true, 1920, 1, 4, 0},
        {"_0_0_fogData[7].sunPercentage", true, 1936, 1, 4, 0},
        {"_0_0_fogData[7].sunDirection_and_fogZScalar", true, 1952, 1, 4, 0},
        {"_0_0_fogData[7].heightFogCoeff", true, 1968, 1, 4, 0},
        {"_0_0_fogData[7].mainFogCoeff", true, 1984, 1, 4, 0},
        {"_0_0_fogData[7].heightDensityFogCoeff", true, 2000, 1, 4, 0},
        {"_0_0_fogData[7].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 2016, 1, 4, 0},
        {"_0_0_fogData[7].heightFogEndColor_fogStartOffset", true, 2032, 1, 4, 0},
      }
    },
  }},
  {"renderFrameBufferShader",  {
  }},
  {"wmoShader",  {
    {
      2, {
        {"_1_2_VertexShader_UseLitColor", false, 0, 1, 4, 0},
      }
    },
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 240, 1, 4, 0},
        {"_0_0_fogData[0].densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData[0].classicFogParams", true, 272, 1, 4, 0},
        {"_0_0_fogData[0].heightPlane", true, 288, 1, 4, 0},
        {"_0_0_fogData[0].color_and_heightRate", true, 304, 1, 4, 0},
        {"_0_0_fogData[0].heightDensity_and_endColor", true, 320, 1, 4, 0},
        {"_0_0_fogData[0].sunAngle_and_sunColor", true, 336, 1, 4, 0},
        {"_0_0_fogData[0].heightColor_and_endFogDistance", true, 352, 1, 4, 0},
        {"_0_0_fogData[0].sunPercentage", true, 368, 1, 4, 0},
        {"_0_0_fogData[0].sunDirection_and_fogZScalar", true, 384, 1, 4, 0},
        {"_0_0_fogData[0].heightFogCoeff", true, 400, 1, 4, 0},
        {"_0_0_fogData[0].mainFogCoeff", true, 416, 1, 4, 0},
        {"_0_0_fogData[0].heightDensityFogCoeff", true, 432, 1, 4, 0},
        {"_0_0_fogData[0].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 448, 1, 4, 0},
        {"_0_0_fogData[0].heightFogEndColor_fogStartOffset", true, 464, 1, 4, 0},
        {"_0_0_fogData[1].densityParams", true, 480, 1, 4, 0},
        {"_0_0_fogData[1].classicFogParams", true, 496, 1, 4, 0},
        {"_0_0_fogData[1].heightPlane", true, 512, 1, 4, 0},
        {"_0_0_fogData[1].color_and_heightRate", true, 528, 1, 4, 0},
        {"_0_0_fogData[1].heightDensity_and_endColor", true, 544, 1, 4, 0},
        {"_0_0_fogData[1].sunAngle_and_sunColor", true, 560, 1, 4, 0},
        {"_0_0_fogData[1].heightColor_and_endFogDistance", true, 576, 1, 4, 0},
        {"_0_0_fogData[1].sunPercentage", true, 592, 1, 4, 0},
        {"_0_0_fogData[1].sunDirection_and_fogZScalar", true, 608, 1, 4, 0},
        {"_0_0_fogData[1].heightFogCoeff", true, 624, 1, 4, 0},
        {"_0_0_fogData[1].mainFogCoeff", true, 640, 1, 4, 0},
        {"_0_0_fogData[1].heightDensityFogCoeff", true, 656, 1, 4, 0},
        {"_0_0_fogData[1].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 672, 1, 4, 0},
        {"_0_0_fogData[1].heightFogEndColor_fogStartOffset", true, 688, 1, 4, 0},
        {"_0_0_fogData[2].densityParams", true, 704, 1, 4, 0},
        {"_0_0_fogData[2].classicFogParams", true, 720, 1, 4, 0},
        {"_0_0_fogData[2].heightPlane", true, 736, 1, 4, 0},
        {"_0_0_fogData[2].color_and_heightRate", true, 752, 1, 4, 0},
        {"_0_0_fogData[2].heightDensity_and_endColor", true, 768, 1, 4, 0},
        {"_0_0_fogData[2].sunAngle_and_sunColor", true, 784, 1, 4, 0},
        {"_0_0_fogData[2].heightColor_and_endFogDistance", true, 800, 1, 4, 0},
        {"_0_0_fogData[2].sunPercentage", true, 816, 1, 4, 0},
        {"_0_0_fogData[2].sunDirection_and_fogZScalar", true, 832, 1, 4, 0},
        {"_0_0_fogData[2].heightFogCoeff", true, 848, 1, 4, 0},
        {"_0_0_fogData[2].mainFogCoeff", true, 864, 1, 4, 0},
        {"_0_0_fogData[2].heightDensityFogCoeff", true, 880, 1, 4, 0},
        {"_0_0_fogData[2].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 896, 1, 4, 0},
        {"_0_0_fogData[2].heightFogEndColor_fogStartOffset", true, 912, 1, 4, 0},
        {"_0_0_fogData[3].densityParams", true, 928, 1, 4, 0},
        {"_0_0_fogData[3].classicFogParams", true, 944, 1, 4, 0},
        {"_0_0_fogData[3].heightPlane", true, 960, 1, 4, 0},
        {"_0_0_fogData[3].color_and_heightRate", true, 976, 1, 4, 0},
        {"_0_0_fogData[3].heightDensity_and_endColor", true, 992, 1, 4, 0},
        {"_0_0_fogData[3].sunAngle_and_sunColor", true, 1008, 1, 4, 0},
        {"_0_0_fogData[3].heightColor_and_endFogDistance", true, 1024, 1, 4, 0},
        {"_0_0_fogData[3].sunPercentage", true, 1040, 1, 4, 0},
        {"_0_0_fogData[3].sunDirection_and_fogZScalar", true, 1056, 1, 4, 0},
        {"_0_0_fogData[3].heightFogCoeff", true, 1072, 1, 4, 0},
        {"_0_0_fogData[3].mainFogCoeff", true, 1088, 1, 4, 0},
        {"_0_0_fogData[3].heightDensityFogCoeff", true, 1104, 1, 4, 0},
        {"_0_0_fogData[3].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1120, 1, 4, 0},
        {"_0_0_fogData[3].heightFogEndColor_fogStartOffset", true, 1136, 1, 4, 0},
        {"_0_0_fogData[4].densityParams", true, 1152, 1, 4, 0},
        {"_0_0_fogData[4].classicFogParams", true, 1168, 1, 4, 0},
        {"_0_0_fogData[4].heightPlane", true, 1184, 1, 4, 0},
        {"_0_0_fogData[4].color_and_heightRate", true, 1200, 1, 4, 0},
        {"_0_0_fogData[4].heightDensity_and_endColor", true, 1216, 1, 4, 0},
        {"_0_0_fogData[4].sunAngle_and_sunColor", true, 1232, 1, 4, 0},
        {"_0_0_fogData[4].heightColor_and_endFogDistance", true, 1248, 1, 4, 0},
        {"_0_0_fogData[4].sunPercentage", true, 1264, 1, 4, 0},
        {"_0_0_fogData[4].sunDirection_and_fogZScalar", true, 1280, 1, 4, 0},
        {"_0_0_fogData[4].heightFogCoeff", true, 1296, 1, 4, 0},
        {"_0_0_fogData[4].mainFogCoeff", true, 1312, 1, 4, 0},
        {"_0_0_fogData[4].heightDensityFogCoeff", true, 1328, 1, 4, 0},
        {"_0_0_fogData[4].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1344, 1, 4, 0},
        {"_0_0_fogData[4].heightFogEndColor_fogStartOffset", true, 1360, 1, 4, 0},
        {"_0_0_fogData[5].densityParams", true, 1376, 1, 4, 0},
        {"_0_0_fogData[5].classicFogParams", true, 1392, 1, 4, 0},
        {"_0_0_fogData[5].heightPlane", true, 1408, 1, 4, 0},
        {"_0_0_fogData[5].color_and_heightRate", true, 1424, 1, 4, 0},
        {"_0_0_fogData[5].heightDensity_and_endColor", true, 1440, 1, 4, 0},
        {"_0_0_fogData[5].sunAngle_and_sunColor", true, 1456, 1, 4, 0},
        {"_0_0_fogData[5].heightColor_and_endFogDistance", true, 1472, 1, 4, 0},
        {"_0_0_fogData[5].sunPercentage", true, 1488, 1, 4, 0},
        {"_0_0_fogData[5].sunDirection_and_fogZScalar", true, 1504, 1, 4, 0},
        {"_0_0_fogData[5].heightFogCoeff", true, 1520, 1, 4, 0},
        {"_0_0_fogData[5].mainFogCoeff", true, 1536, 1, 4, 0},
        {"_0_0_fogData[5].heightDensityFogCoeff", true, 1552, 1, 4, 0},
        {"_0_0_fogData[5].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1568, 1, 4, 0},
        {"_0_0_fogData[5].heightFogEndColor_fogStartOffset", true, 1584, 1, 4, 0},
        {"_0_0_fogData[6].densityParams", true, 1600, 1, 4, 0},
        {"_0_0_fogData[6].classicFogParams", true, 1616, 1, 4, 0},
        {"_0_0_fogData[6].heightPlane", true, 1632, 1, 4, 0},
        {"_0_0_fogData[6].color_and_heightRate", true, 1648, 1, 4, 0},
        {"_0_0_fogData[6].heightDensity_and_endColor", true, 1664, 1, 4, 0},
        {"_0_0_fogData[6].sunAngle_and_sunColor", true, 1680, 1, 4, 0},
        {"_0_0_fogData[6].heightColor_and_endFogDistance", true, 1696, 1, 4, 0},
        {"_0_0_fogData[6].sunPercentage", true, 1712, 1, 4, 0},
        {"_0_0_fogData[6].sunDirection_and_fogZScalar", true, 1728, 1, 4, 0},
        {"_0_0_fogData[6].heightFogCoeff", true, 1744, 1, 4, 0},
        {"_0_0_fogData[6].mainFogCoeff", true, 1760, 1, 4, 0},
        {"_0_0_fogData[6].heightDensityFogCoeff", true, 1776, 1, 4, 0},
        {"_0_0_fogData[6].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1792, 1, 4, 0},
        {"_0_0_fogData[6].heightFogEndColor_fogStartOffset", true, 1808, 1, 4, 0},
        {"_0_0_fogData[7].densityParams", true, 1824, 1, 4, 0},
        {"_0_0_fogData[7].classicFogParams", true, 1840, 1, 4, 0},
        {"_0_0_fogData[7].heightPlane", true, 1856, 1, 4, 0},
        {"_0_0_fogData[7].color_and_heightRate", true, 1872, 1, 4, 0},
        {"_0_0_fogData[7].heightDensity_and_endColor", true, 1888, 1, 4, 0},
        {"_0_0_fogData[7].sunAngle_and_sunColor", true, 1904, 1, 4, 0},
        {"_0_0_fogData[7].heightColor_and_endFogDistance", true, 1920, 1, 4, 0},
        {"_0_0_fogData[7].sunPercentage", true, 1936, 1, 4, 0},
        {"_0_0_fogData[7].sunDirection_and_fogZScalar", true, 1952, 1, 4, 0},
        {"_0_0_fogData[7].heightFogCoeff", true, 1968, 1, 4, 0},
        {"_0_0_fogData[7].mainFogCoeff", true, 1984, 1, 4, 0},
        {"_0_0_fogData[7].heightDensityFogCoeff", true, 2000, 1, 4, 0},
        {"_0_0_fogData[7].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 2016, 1, 4, 0},
        {"_0_0_fogData[7].heightFogEndColor_fogStartOffset", true, 2032, 1, 4, 0},
      }
    },
    {
      1, {
        {"_1_1_uPlacementMat", true, 0, 4, 4, 0},
      }
    },
  }},
  {"imguiShader",  {
    {
      1, {
        {"_0_1_ProjMtx", true, 0, 4, 4, 0},
        {"_0_1_uiScale", true, 64, 1, 4, 0},
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
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 240, 1, 4, 0},
        {"_0_0_fogData[0].densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData[0].classicFogParams", true, 272, 1, 4, 0},
        {"_0_0_fogData[0].heightPlane", true, 288, 1, 4, 0},
        {"_0_0_fogData[0].color_and_heightRate", true, 304, 1, 4, 0},
        {"_0_0_fogData[0].heightDensity_and_endColor", true, 320, 1, 4, 0},
        {"_0_0_fogData[0].sunAngle_and_sunColor", true, 336, 1, 4, 0},
        {"_0_0_fogData[0].heightColor_and_endFogDistance", true, 352, 1, 4, 0},
        {"_0_0_fogData[0].sunPercentage", true, 368, 1, 4, 0},
        {"_0_0_fogData[0].sunDirection_and_fogZScalar", true, 384, 1, 4, 0},
        {"_0_0_fogData[0].heightFogCoeff", true, 400, 1, 4, 0},
        {"_0_0_fogData[0].mainFogCoeff", true, 416, 1, 4, 0},
        {"_0_0_fogData[0].heightDensityFogCoeff", true, 432, 1, 4, 0},
        {"_0_0_fogData[0].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 448, 1, 4, 0},
        {"_0_0_fogData[0].heightFogEndColor_fogStartOffset", true, 464, 1, 4, 0},
        {"_0_0_fogData[1].densityParams", true, 480, 1, 4, 0},
        {"_0_0_fogData[1].classicFogParams", true, 496, 1, 4, 0},
        {"_0_0_fogData[1].heightPlane", true, 512, 1, 4, 0},
        {"_0_0_fogData[1].color_and_heightRate", true, 528, 1, 4, 0},
        {"_0_0_fogData[1].heightDensity_and_endColor", true, 544, 1, 4, 0},
        {"_0_0_fogData[1].sunAngle_and_sunColor", true, 560, 1, 4, 0},
        {"_0_0_fogData[1].heightColor_and_endFogDistance", true, 576, 1, 4, 0},
        {"_0_0_fogData[1].sunPercentage", true, 592, 1, 4, 0},
        {"_0_0_fogData[1].sunDirection_and_fogZScalar", true, 608, 1, 4, 0},
        {"_0_0_fogData[1].heightFogCoeff", true, 624, 1, 4, 0},
        {"_0_0_fogData[1].mainFogCoeff", true, 640, 1, 4, 0},
        {"_0_0_fogData[1].heightDensityFogCoeff", true, 656, 1, 4, 0},
        {"_0_0_fogData[1].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 672, 1, 4, 0},
        {"_0_0_fogData[1].heightFogEndColor_fogStartOffset", true, 688, 1, 4, 0},
        {"_0_0_fogData[2].densityParams", true, 704, 1, 4, 0},
        {"_0_0_fogData[2].classicFogParams", true, 720, 1, 4, 0},
        {"_0_0_fogData[2].heightPlane", true, 736, 1, 4, 0},
        {"_0_0_fogData[2].color_and_heightRate", true, 752, 1, 4, 0},
        {"_0_0_fogData[2].heightDensity_and_endColor", true, 768, 1, 4, 0},
        {"_0_0_fogData[2].sunAngle_and_sunColor", true, 784, 1, 4, 0},
        {"_0_0_fogData[2].heightColor_and_endFogDistance", true, 800, 1, 4, 0},
        {"_0_0_fogData[2].sunPercentage", true, 816, 1, 4, 0},
        {"_0_0_fogData[2].sunDirection_and_fogZScalar", true, 832, 1, 4, 0},
        {"_0_0_fogData[2].heightFogCoeff", true, 848, 1, 4, 0},
        {"_0_0_fogData[2].mainFogCoeff", true, 864, 1, 4, 0},
        {"_0_0_fogData[2].heightDensityFogCoeff", true, 880, 1, 4, 0},
        {"_0_0_fogData[2].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 896, 1, 4, 0},
        {"_0_0_fogData[2].heightFogEndColor_fogStartOffset", true, 912, 1, 4, 0},
        {"_0_0_fogData[3].densityParams", true, 928, 1, 4, 0},
        {"_0_0_fogData[3].classicFogParams", true, 944, 1, 4, 0},
        {"_0_0_fogData[3].heightPlane", true, 960, 1, 4, 0},
        {"_0_0_fogData[3].color_and_heightRate", true, 976, 1, 4, 0},
        {"_0_0_fogData[3].heightDensity_and_endColor", true, 992, 1, 4, 0},
        {"_0_0_fogData[3].sunAngle_and_sunColor", true, 1008, 1, 4, 0},
        {"_0_0_fogData[3].heightColor_and_endFogDistance", true, 1024, 1, 4, 0},
        {"_0_0_fogData[3].sunPercentage", true, 1040, 1, 4, 0},
        {"_0_0_fogData[3].sunDirection_and_fogZScalar", true, 1056, 1, 4, 0},
        {"_0_0_fogData[3].heightFogCoeff", true, 1072, 1, 4, 0},
        {"_0_0_fogData[3].mainFogCoeff", true, 1088, 1, 4, 0},
        {"_0_0_fogData[3].heightDensityFogCoeff", true, 1104, 1, 4, 0},
        {"_0_0_fogData[3].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1120, 1, 4, 0},
        {"_0_0_fogData[3].heightFogEndColor_fogStartOffset", true, 1136, 1, 4, 0},
        {"_0_0_fogData[4].densityParams", true, 1152, 1, 4, 0},
        {"_0_0_fogData[4].classicFogParams", true, 1168, 1, 4, 0},
        {"_0_0_fogData[4].heightPlane", true, 1184, 1, 4, 0},
        {"_0_0_fogData[4].color_and_heightRate", true, 1200, 1, 4, 0},
        {"_0_0_fogData[4].heightDensity_and_endColor", true, 1216, 1, 4, 0},
        {"_0_0_fogData[4].sunAngle_and_sunColor", true, 1232, 1, 4, 0},
        {"_0_0_fogData[4].heightColor_and_endFogDistance", true, 1248, 1, 4, 0},
        {"_0_0_fogData[4].sunPercentage", true, 1264, 1, 4, 0},
        {"_0_0_fogData[4].sunDirection_and_fogZScalar", true, 1280, 1, 4, 0},
        {"_0_0_fogData[4].heightFogCoeff", true, 1296, 1, 4, 0},
        {"_0_0_fogData[4].mainFogCoeff", true, 1312, 1, 4, 0},
        {"_0_0_fogData[4].heightDensityFogCoeff", true, 1328, 1, 4, 0},
        {"_0_0_fogData[4].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1344, 1, 4, 0},
        {"_0_0_fogData[4].heightFogEndColor_fogStartOffset", true, 1360, 1, 4, 0},
        {"_0_0_fogData[5].densityParams", true, 1376, 1, 4, 0},
        {"_0_0_fogData[5].classicFogParams", true, 1392, 1, 4, 0},
        {"_0_0_fogData[5].heightPlane", true, 1408, 1, 4, 0},
        {"_0_0_fogData[5].color_and_heightRate", true, 1424, 1, 4, 0},
        {"_0_0_fogData[5].heightDensity_and_endColor", true, 1440, 1, 4, 0},
        {"_0_0_fogData[5].sunAngle_and_sunColor", true, 1456, 1, 4, 0},
        {"_0_0_fogData[5].heightColor_and_endFogDistance", true, 1472, 1, 4, 0},
        {"_0_0_fogData[5].sunPercentage", true, 1488, 1, 4, 0},
        {"_0_0_fogData[5].sunDirection_and_fogZScalar", true, 1504, 1, 4, 0},
        {"_0_0_fogData[5].heightFogCoeff", true, 1520, 1, 4, 0},
        {"_0_0_fogData[5].mainFogCoeff", true, 1536, 1, 4, 0},
        {"_0_0_fogData[5].heightDensityFogCoeff", true, 1552, 1, 4, 0},
        {"_0_0_fogData[5].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1568, 1, 4, 0},
        {"_0_0_fogData[5].heightFogEndColor_fogStartOffset", true, 1584, 1, 4, 0},
        {"_0_0_fogData[6].densityParams", true, 1600, 1, 4, 0},
        {"_0_0_fogData[6].classicFogParams", true, 1616, 1, 4, 0},
        {"_0_0_fogData[6].heightPlane", true, 1632, 1, 4, 0},
        {"_0_0_fogData[6].color_and_heightRate", true, 1648, 1, 4, 0},
        {"_0_0_fogData[6].heightDensity_and_endColor", true, 1664, 1, 4, 0},
        {"_0_0_fogData[6].sunAngle_and_sunColor", true, 1680, 1, 4, 0},
        {"_0_0_fogData[6].heightColor_and_endFogDistance", true, 1696, 1, 4, 0},
        {"_0_0_fogData[6].sunPercentage", true, 1712, 1, 4, 0},
        {"_0_0_fogData[6].sunDirection_and_fogZScalar", true, 1728, 1, 4, 0},
        {"_0_0_fogData[6].heightFogCoeff", true, 1744, 1, 4, 0},
        {"_0_0_fogData[6].mainFogCoeff", true, 1760, 1, 4, 0},
        {"_0_0_fogData[6].heightDensityFogCoeff", true, 1776, 1, 4, 0},
        {"_0_0_fogData[6].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1792, 1, 4, 0},
        {"_0_0_fogData[6].heightFogEndColor_fogStartOffset", true, 1808, 1, 4, 0},
        {"_0_0_fogData[7].densityParams", true, 1824, 1, 4, 0},
        {"_0_0_fogData[7].classicFogParams", true, 1840, 1, 4, 0},
        {"_0_0_fogData[7].heightPlane", true, 1856, 1, 4, 0},
        {"_0_0_fogData[7].color_and_heightRate", true, 1872, 1, 4, 0},
        {"_0_0_fogData[7].heightDensity_and_endColor", true, 1888, 1, 4, 0},
        {"_0_0_fogData[7].sunAngle_and_sunColor", true, 1904, 1, 4, 0},
        {"_0_0_fogData[7].heightColor_and_endFogDistance", true, 1920, 1, 4, 0},
        {"_0_0_fogData[7].sunPercentage", true, 1936, 1, 4, 0},
        {"_0_0_fogData[7].sunDirection_and_fogZScalar", true, 1952, 1, 4, 0},
        {"_0_0_fogData[7].heightFogCoeff", true, 1968, 1, 4, 0},
        {"_0_0_fogData[7].mainFogCoeff", true, 1984, 1, 4, 0},
        {"_0_0_fogData[7].heightDensityFogCoeff", true, 2000, 1, 4, 0},
        {"_0_0_fogData[7].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 2016, 1, 4, 0},
        {"_0_0_fogData[7].heightFogEndColor_fogStartOffset", true, 2032, 1, 4, 0},
      }
    },
  }},
};
const  std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameFrag = {
  {"waterfallShader",  {
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 240, 1, 4, 0},
        {"_0_0_fogData[0].densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData[0].classicFogParams", true, 272, 1, 4, 0},
        {"_0_0_fogData[0].heightPlane", true, 288, 1, 4, 0},
        {"_0_0_fogData[0].color_and_heightRate", true, 304, 1, 4, 0},
        {"_0_0_fogData[0].heightDensity_and_endColor", true, 320, 1, 4, 0},
        {"_0_0_fogData[0].sunAngle_and_sunColor", true, 336, 1, 4, 0},
        {"_0_0_fogData[0].heightColor_and_endFogDistance", true, 352, 1, 4, 0},
        {"_0_0_fogData[0].sunPercentage", true, 368, 1, 4, 0},
        {"_0_0_fogData[0].sunDirection_and_fogZScalar", true, 384, 1, 4, 0},
        {"_0_0_fogData[0].heightFogCoeff", true, 400, 1, 4, 0},
        {"_0_0_fogData[0].mainFogCoeff", true, 416, 1, 4, 0},
        {"_0_0_fogData[0].heightDensityFogCoeff", true, 432, 1, 4, 0},
        {"_0_0_fogData[0].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 448, 1, 4, 0},
        {"_0_0_fogData[0].heightFogEndColor_fogStartOffset", true, 464, 1, 4, 0},
        {"_0_0_fogData[1].densityParams", true, 480, 1, 4, 0},
        {"_0_0_fogData[1].classicFogParams", true, 496, 1, 4, 0},
        {"_0_0_fogData[1].heightPlane", true, 512, 1, 4, 0},
        {"_0_0_fogData[1].color_and_heightRate", true, 528, 1, 4, 0},
        {"_0_0_fogData[1].heightDensity_and_endColor", true, 544, 1, 4, 0},
        {"_0_0_fogData[1].sunAngle_and_sunColor", true, 560, 1, 4, 0},
        {"_0_0_fogData[1].heightColor_and_endFogDistance", true, 576, 1, 4, 0},
        {"_0_0_fogData[1].sunPercentage", true, 592, 1, 4, 0},
        {"_0_0_fogData[1].sunDirection_and_fogZScalar", true, 608, 1, 4, 0},
        {"_0_0_fogData[1].heightFogCoeff", true, 624, 1, 4, 0},
        {"_0_0_fogData[1].mainFogCoeff", true, 640, 1, 4, 0},
        {"_0_0_fogData[1].heightDensityFogCoeff", true, 656, 1, 4, 0},
        {"_0_0_fogData[1].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 672, 1, 4, 0},
        {"_0_0_fogData[1].heightFogEndColor_fogStartOffset", true, 688, 1, 4, 0},
        {"_0_0_fogData[2].densityParams", true, 704, 1, 4, 0},
        {"_0_0_fogData[2].classicFogParams", true, 720, 1, 4, 0},
        {"_0_0_fogData[2].heightPlane", true, 736, 1, 4, 0},
        {"_0_0_fogData[2].color_and_heightRate", true, 752, 1, 4, 0},
        {"_0_0_fogData[2].heightDensity_and_endColor", true, 768, 1, 4, 0},
        {"_0_0_fogData[2].sunAngle_and_sunColor", true, 784, 1, 4, 0},
        {"_0_0_fogData[2].heightColor_and_endFogDistance", true, 800, 1, 4, 0},
        {"_0_0_fogData[2].sunPercentage", true, 816, 1, 4, 0},
        {"_0_0_fogData[2].sunDirection_and_fogZScalar", true, 832, 1, 4, 0},
        {"_0_0_fogData[2].heightFogCoeff", true, 848, 1, 4, 0},
        {"_0_0_fogData[2].mainFogCoeff", true, 864, 1, 4, 0},
        {"_0_0_fogData[2].heightDensityFogCoeff", true, 880, 1, 4, 0},
        {"_0_0_fogData[2].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 896, 1, 4, 0},
        {"_0_0_fogData[2].heightFogEndColor_fogStartOffset", true, 912, 1, 4, 0},
        {"_0_0_fogData[3].densityParams", true, 928, 1, 4, 0},
        {"_0_0_fogData[3].classicFogParams", true, 944, 1, 4, 0},
        {"_0_0_fogData[3].heightPlane", true, 960, 1, 4, 0},
        {"_0_0_fogData[3].color_and_heightRate", true, 976, 1, 4, 0},
        {"_0_0_fogData[3].heightDensity_and_endColor", true, 992, 1, 4, 0},
        {"_0_0_fogData[3].sunAngle_and_sunColor", true, 1008, 1, 4, 0},
        {"_0_0_fogData[3].heightColor_and_endFogDistance", true, 1024, 1, 4, 0},
        {"_0_0_fogData[3].sunPercentage", true, 1040, 1, 4, 0},
        {"_0_0_fogData[3].sunDirection_and_fogZScalar", true, 1056, 1, 4, 0},
        {"_0_0_fogData[3].heightFogCoeff", true, 1072, 1, 4, 0},
        {"_0_0_fogData[3].mainFogCoeff", true, 1088, 1, 4, 0},
        {"_0_0_fogData[3].heightDensityFogCoeff", true, 1104, 1, 4, 0},
        {"_0_0_fogData[3].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1120, 1, 4, 0},
        {"_0_0_fogData[3].heightFogEndColor_fogStartOffset", true, 1136, 1, 4, 0},
        {"_0_0_fogData[4].densityParams", true, 1152, 1, 4, 0},
        {"_0_0_fogData[4].classicFogParams", true, 1168, 1, 4, 0},
        {"_0_0_fogData[4].heightPlane", true, 1184, 1, 4, 0},
        {"_0_0_fogData[4].color_and_heightRate", true, 1200, 1, 4, 0},
        {"_0_0_fogData[4].heightDensity_and_endColor", true, 1216, 1, 4, 0},
        {"_0_0_fogData[4].sunAngle_and_sunColor", true, 1232, 1, 4, 0},
        {"_0_0_fogData[4].heightColor_and_endFogDistance", true, 1248, 1, 4, 0},
        {"_0_0_fogData[4].sunPercentage", true, 1264, 1, 4, 0},
        {"_0_0_fogData[4].sunDirection_and_fogZScalar", true, 1280, 1, 4, 0},
        {"_0_0_fogData[4].heightFogCoeff", true, 1296, 1, 4, 0},
        {"_0_0_fogData[4].mainFogCoeff", true, 1312, 1, 4, 0},
        {"_0_0_fogData[4].heightDensityFogCoeff", true, 1328, 1, 4, 0},
        {"_0_0_fogData[4].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1344, 1, 4, 0},
        {"_0_0_fogData[4].heightFogEndColor_fogStartOffset", true, 1360, 1, 4, 0},
        {"_0_0_fogData[5].densityParams", true, 1376, 1, 4, 0},
        {"_0_0_fogData[5].classicFogParams", true, 1392, 1, 4, 0},
        {"_0_0_fogData[5].heightPlane", true, 1408, 1, 4, 0},
        {"_0_0_fogData[5].color_and_heightRate", true, 1424, 1, 4, 0},
        {"_0_0_fogData[5].heightDensity_and_endColor", true, 1440, 1, 4, 0},
        {"_0_0_fogData[5].sunAngle_and_sunColor", true, 1456, 1, 4, 0},
        {"_0_0_fogData[5].heightColor_and_endFogDistance", true, 1472, 1, 4, 0},
        {"_0_0_fogData[5].sunPercentage", true, 1488, 1, 4, 0},
        {"_0_0_fogData[5].sunDirection_and_fogZScalar", true, 1504, 1, 4, 0},
        {"_0_0_fogData[5].heightFogCoeff", true, 1520, 1, 4, 0},
        {"_0_0_fogData[5].mainFogCoeff", true, 1536, 1, 4, 0},
        {"_0_0_fogData[5].heightDensityFogCoeff", true, 1552, 1, 4, 0},
        {"_0_0_fogData[5].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1568, 1, 4, 0},
        {"_0_0_fogData[5].heightFogEndColor_fogStartOffset", true, 1584, 1, 4, 0},
        {"_0_0_fogData[6].densityParams", true, 1600, 1, 4, 0},
        {"_0_0_fogData[6].classicFogParams", true, 1616, 1, 4, 0},
        {"_0_0_fogData[6].heightPlane", true, 1632, 1, 4, 0},
        {"_0_0_fogData[6].color_and_heightRate", true, 1648, 1, 4, 0},
        {"_0_0_fogData[6].heightDensity_and_endColor", true, 1664, 1, 4, 0},
        {"_0_0_fogData[6].sunAngle_and_sunColor", true, 1680, 1, 4, 0},
        {"_0_0_fogData[6].heightColor_and_endFogDistance", true, 1696, 1, 4, 0},
        {"_0_0_fogData[6].sunPercentage", true, 1712, 1, 4, 0},
        {"_0_0_fogData[6].sunDirection_and_fogZScalar", true, 1728, 1, 4, 0},
        {"_0_0_fogData[6].heightFogCoeff", true, 1744, 1, 4, 0},
        {"_0_0_fogData[6].mainFogCoeff", true, 1760, 1, 4, 0},
        {"_0_0_fogData[6].heightDensityFogCoeff", true, 1776, 1, 4, 0},
        {"_0_0_fogData[6].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1792, 1, 4, 0},
        {"_0_0_fogData[6].heightFogEndColor_fogStartOffset", true, 1808, 1, 4, 0},
        {"_0_0_fogData[7].densityParams", true, 1824, 1, 4, 0},
        {"_0_0_fogData[7].classicFogParams", true, 1840, 1, 4, 0},
        {"_0_0_fogData[7].heightPlane", true, 1856, 1, 4, 0},
        {"_0_0_fogData[7].color_and_heightRate", true, 1872, 1, 4, 0},
        {"_0_0_fogData[7].heightDensity_and_endColor", true, 1888, 1, 4, 0},
        {"_0_0_fogData[7].sunAngle_and_sunColor", true, 1904, 1, 4, 0},
        {"_0_0_fogData[7].heightColor_and_endFogDistance", true, 1920, 1, 4, 0},
        {"_0_0_fogData[7].sunPercentage", true, 1936, 1, 4, 0},
        {"_0_0_fogData[7].sunDirection_and_fogZScalar", true, 1952, 1, 4, 0},
        {"_0_0_fogData[7].heightFogCoeff", true, 1968, 1, 4, 0},
        {"_0_0_fogData[7].mainFogCoeff", true, 1984, 1, 4, 0},
        {"_0_0_fogData[7].heightDensityFogCoeff", true, 2000, 1, 4, 0},
        {"_0_0_fogData[7].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 2016, 1, 4, 0},
        {"_0_0_fogData[7].heightFogEndColor_fogStartOffset", true, 2032, 1, 4, 0},
      }
    },
    {
      5, {
        {"_2_5_values0", true, 0, 1, 4, 0},
        {"_2_5_values1", true, 16, 1, 4, 0},
        {"_2_5_values2", true, 32, 1, 4, 0},
        {"_2_5_values3", true, 48, 1, 4, 0},
        {"_2_5_values4", true, 64, 1, 4, 0},
        {"_2_5_baseColor", true, 80, 1, 4, 0},
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
      1, {
        {"_1_1_uColor", true, 0, 1, 4, 0},
      }
    },
  }},
  {"adtShader",  {
    {
      1, {
        {"_1_1_uPos", true, 0, 1, 4, 0},
        {"_1_1_uUseHeightMixFormula", false, 16, 1, 4, 0},
        {"_1_1_uHeightScale", true, 32, 1, 4, 0},
        {"_1_1_uHeightOffset", true, 48, 1, 4, 0},
      }
    },
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 240, 1, 4, 0},
        {"_0_0_fogData[0].densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData[0].classicFogParams", true, 272, 1, 4, 0},
        {"_0_0_fogData[0].heightPlane", true, 288, 1, 4, 0},
        {"_0_0_fogData[0].color_and_heightRate", true, 304, 1, 4, 0},
        {"_0_0_fogData[0].heightDensity_and_endColor", true, 320, 1, 4, 0},
        {"_0_0_fogData[0].sunAngle_and_sunColor", true, 336, 1, 4, 0},
        {"_0_0_fogData[0].heightColor_and_endFogDistance", true, 352, 1, 4, 0},
        {"_0_0_fogData[0].sunPercentage", true, 368, 1, 4, 0},
        {"_0_0_fogData[0].sunDirection_and_fogZScalar", true, 384, 1, 4, 0},
        {"_0_0_fogData[0].heightFogCoeff", true, 400, 1, 4, 0},
        {"_0_0_fogData[0].mainFogCoeff", true, 416, 1, 4, 0},
        {"_0_0_fogData[0].heightDensityFogCoeff", true, 432, 1, 4, 0},
        {"_0_0_fogData[0].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 448, 1, 4, 0},
        {"_0_0_fogData[0].heightFogEndColor_fogStartOffset", true, 464, 1, 4, 0},
        {"_0_0_fogData[1].densityParams", true, 480, 1, 4, 0},
        {"_0_0_fogData[1].classicFogParams", true, 496, 1, 4, 0},
        {"_0_0_fogData[1].heightPlane", true, 512, 1, 4, 0},
        {"_0_0_fogData[1].color_and_heightRate", true, 528, 1, 4, 0},
        {"_0_0_fogData[1].heightDensity_and_endColor", true, 544, 1, 4, 0},
        {"_0_0_fogData[1].sunAngle_and_sunColor", true, 560, 1, 4, 0},
        {"_0_0_fogData[1].heightColor_and_endFogDistance", true, 576, 1, 4, 0},
        {"_0_0_fogData[1].sunPercentage", true, 592, 1, 4, 0},
        {"_0_0_fogData[1].sunDirection_and_fogZScalar", true, 608, 1, 4, 0},
        {"_0_0_fogData[1].heightFogCoeff", true, 624, 1, 4, 0},
        {"_0_0_fogData[1].mainFogCoeff", true, 640, 1, 4, 0},
        {"_0_0_fogData[1].heightDensityFogCoeff", true, 656, 1, 4, 0},
        {"_0_0_fogData[1].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 672, 1, 4, 0},
        {"_0_0_fogData[1].heightFogEndColor_fogStartOffset", true, 688, 1, 4, 0},
        {"_0_0_fogData[2].densityParams", true, 704, 1, 4, 0},
        {"_0_0_fogData[2].classicFogParams", true, 720, 1, 4, 0},
        {"_0_0_fogData[2].heightPlane", true, 736, 1, 4, 0},
        {"_0_0_fogData[2].color_and_heightRate", true, 752, 1, 4, 0},
        {"_0_0_fogData[2].heightDensity_and_endColor", true, 768, 1, 4, 0},
        {"_0_0_fogData[2].sunAngle_and_sunColor", true, 784, 1, 4, 0},
        {"_0_0_fogData[2].heightColor_and_endFogDistance", true, 800, 1, 4, 0},
        {"_0_0_fogData[2].sunPercentage", true, 816, 1, 4, 0},
        {"_0_0_fogData[2].sunDirection_and_fogZScalar", true, 832, 1, 4, 0},
        {"_0_0_fogData[2].heightFogCoeff", true, 848, 1, 4, 0},
        {"_0_0_fogData[2].mainFogCoeff", true, 864, 1, 4, 0},
        {"_0_0_fogData[2].heightDensityFogCoeff", true, 880, 1, 4, 0},
        {"_0_0_fogData[2].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 896, 1, 4, 0},
        {"_0_0_fogData[2].heightFogEndColor_fogStartOffset", true, 912, 1, 4, 0},
        {"_0_0_fogData[3].densityParams", true, 928, 1, 4, 0},
        {"_0_0_fogData[3].classicFogParams", true, 944, 1, 4, 0},
        {"_0_0_fogData[3].heightPlane", true, 960, 1, 4, 0},
        {"_0_0_fogData[3].color_and_heightRate", true, 976, 1, 4, 0},
        {"_0_0_fogData[3].heightDensity_and_endColor", true, 992, 1, 4, 0},
        {"_0_0_fogData[3].sunAngle_and_sunColor", true, 1008, 1, 4, 0},
        {"_0_0_fogData[3].heightColor_and_endFogDistance", true, 1024, 1, 4, 0},
        {"_0_0_fogData[3].sunPercentage", true, 1040, 1, 4, 0},
        {"_0_0_fogData[3].sunDirection_and_fogZScalar", true, 1056, 1, 4, 0},
        {"_0_0_fogData[3].heightFogCoeff", true, 1072, 1, 4, 0},
        {"_0_0_fogData[3].mainFogCoeff", true, 1088, 1, 4, 0},
        {"_0_0_fogData[3].heightDensityFogCoeff", true, 1104, 1, 4, 0},
        {"_0_0_fogData[3].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1120, 1, 4, 0},
        {"_0_0_fogData[3].heightFogEndColor_fogStartOffset", true, 1136, 1, 4, 0},
        {"_0_0_fogData[4].densityParams", true, 1152, 1, 4, 0},
        {"_0_0_fogData[4].classicFogParams", true, 1168, 1, 4, 0},
        {"_0_0_fogData[4].heightPlane", true, 1184, 1, 4, 0},
        {"_0_0_fogData[4].color_and_heightRate", true, 1200, 1, 4, 0},
        {"_0_0_fogData[4].heightDensity_and_endColor", true, 1216, 1, 4, 0},
        {"_0_0_fogData[4].sunAngle_and_sunColor", true, 1232, 1, 4, 0},
        {"_0_0_fogData[4].heightColor_and_endFogDistance", true, 1248, 1, 4, 0},
        {"_0_0_fogData[4].sunPercentage", true, 1264, 1, 4, 0},
        {"_0_0_fogData[4].sunDirection_and_fogZScalar", true, 1280, 1, 4, 0},
        {"_0_0_fogData[4].heightFogCoeff", true, 1296, 1, 4, 0},
        {"_0_0_fogData[4].mainFogCoeff", true, 1312, 1, 4, 0},
        {"_0_0_fogData[4].heightDensityFogCoeff", true, 1328, 1, 4, 0},
        {"_0_0_fogData[4].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1344, 1, 4, 0},
        {"_0_0_fogData[4].heightFogEndColor_fogStartOffset", true, 1360, 1, 4, 0},
        {"_0_0_fogData[5].densityParams", true, 1376, 1, 4, 0},
        {"_0_0_fogData[5].classicFogParams", true, 1392, 1, 4, 0},
        {"_0_0_fogData[5].heightPlane", true, 1408, 1, 4, 0},
        {"_0_0_fogData[5].color_and_heightRate", true, 1424, 1, 4, 0},
        {"_0_0_fogData[5].heightDensity_and_endColor", true, 1440, 1, 4, 0},
        {"_0_0_fogData[5].sunAngle_and_sunColor", true, 1456, 1, 4, 0},
        {"_0_0_fogData[5].heightColor_and_endFogDistance", true, 1472, 1, 4, 0},
        {"_0_0_fogData[5].sunPercentage", true, 1488, 1, 4, 0},
        {"_0_0_fogData[5].sunDirection_and_fogZScalar", true, 1504, 1, 4, 0},
        {"_0_0_fogData[5].heightFogCoeff", true, 1520, 1, 4, 0},
        {"_0_0_fogData[5].mainFogCoeff", true, 1536, 1, 4, 0},
        {"_0_0_fogData[5].heightDensityFogCoeff", true, 1552, 1, 4, 0},
        {"_0_0_fogData[5].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1568, 1, 4, 0},
        {"_0_0_fogData[5].heightFogEndColor_fogStartOffset", true, 1584, 1, 4, 0},
        {"_0_0_fogData[6].densityParams", true, 1600, 1, 4, 0},
        {"_0_0_fogData[6].classicFogParams", true, 1616, 1, 4, 0},
        {"_0_0_fogData[6].heightPlane", true, 1632, 1, 4, 0},
        {"_0_0_fogData[6].color_and_heightRate", true, 1648, 1, 4, 0},
        {"_0_0_fogData[6].heightDensity_and_endColor", true, 1664, 1, 4, 0},
        {"_0_0_fogData[6].sunAngle_and_sunColor", true, 1680, 1, 4, 0},
        {"_0_0_fogData[6].heightColor_and_endFogDistance", true, 1696, 1, 4, 0},
        {"_0_0_fogData[6].sunPercentage", true, 1712, 1, 4, 0},
        {"_0_0_fogData[6].sunDirection_and_fogZScalar", true, 1728, 1, 4, 0},
        {"_0_0_fogData[6].heightFogCoeff", true, 1744, 1, 4, 0},
        {"_0_0_fogData[6].mainFogCoeff", true, 1760, 1, 4, 0},
        {"_0_0_fogData[6].heightDensityFogCoeff", true, 1776, 1, 4, 0},
        {"_0_0_fogData[6].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1792, 1, 4, 0},
        {"_0_0_fogData[6].heightFogEndColor_fogStartOffset", true, 1808, 1, 4, 0},
        {"_0_0_fogData[7].densityParams", true, 1824, 1, 4, 0},
        {"_0_0_fogData[7].classicFogParams", true, 1840, 1, 4, 0},
        {"_0_0_fogData[7].heightPlane", true, 1856, 1, 4, 0},
        {"_0_0_fogData[7].color_and_heightRate", true, 1872, 1, 4, 0},
        {"_0_0_fogData[7].heightDensity_and_endColor", true, 1888, 1, 4, 0},
        {"_0_0_fogData[7].sunAngle_and_sunColor", true, 1904, 1, 4, 0},
        {"_0_0_fogData[7].heightColor_and_endFogDistance", true, 1920, 1, 4, 0},
        {"_0_0_fogData[7].sunPercentage", true, 1936, 1, 4, 0},
        {"_0_0_fogData[7].sunDirection_and_fogZScalar", true, 1952, 1, 4, 0},
        {"_0_0_fogData[7].heightFogCoeff", true, 1968, 1, 4, 0},
        {"_0_0_fogData[7].mainFogCoeff", true, 1984, 1, 4, 0},
        {"_0_0_fogData[7].heightDensityFogCoeff", true, 2000, 1, 4, 0},
        {"_0_0_fogData[7].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 2016, 1, 4, 0},
        {"_0_0_fogData[7].heightFogEndColor_fogStartOffset", true, 2032, 1, 4, 0},
      }
    },
    {
      2, {
        {"_1_2_scaleFactorPerLayer", true, 0, 1, 4, 0},
        {"_1_2_animation_rotationPerLayer", false, 16, 1, 4, 0},
        {"_1_2_animation_speedPerLayer", false, 32, 1, 4, 0},
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
  {"ribbonShader",  {
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 240, 1, 4, 0},
        {"_0_0_fogData[0].densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData[0].classicFogParams", true, 272, 1, 4, 0},
        {"_0_0_fogData[0].heightPlane", true, 288, 1, 4, 0},
        {"_0_0_fogData[0].color_and_heightRate", true, 304, 1, 4, 0},
        {"_0_0_fogData[0].heightDensity_and_endColor", true, 320, 1, 4, 0},
        {"_0_0_fogData[0].sunAngle_and_sunColor", true, 336, 1, 4, 0},
        {"_0_0_fogData[0].heightColor_and_endFogDistance", true, 352, 1, 4, 0},
        {"_0_0_fogData[0].sunPercentage", true, 368, 1, 4, 0},
        {"_0_0_fogData[0].sunDirection_and_fogZScalar", true, 384, 1, 4, 0},
        {"_0_0_fogData[0].heightFogCoeff", true, 400, 1, 4, 0},
        {"_0_0_fogData[0].mainFogCoeff", true, 416, 1, 4, 0},
        {"_0_0_fogData[0].heightDensityFogCoeff", true, 432, 1, 4, 0},
        {"_0_0_fogData[0].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 448, 1, 4, 0},
        {"_0_0_fogData[0].heightFogEndColor_fogStartOffset", true, 464, 1, 4, 0},
        {"_0_0_fogData[1].densityParams", true, 480, 1, 4, 0},
        {"_0_0_fogData[1].classicFogParams", true, 496, 1, 4, 0},
        {"_0_0_fogData[1].heightPlane", true, 512, 1, 4, 0},
        {"_0_0_fogData[1].color_and_heightRate", true, 528, 1, 4, 0},
        {"_0_0_fogData[1].heightDensity_and_endColor", true, 544, 1, 4, 0},
        {"_0_0_fogData[1].sunAngle_and_sunColor", true, 560, 1, 4, 0},
        {"_0_0_fogData[1].heightColor_and_endFogDistance", true, 576, 1, 4, 0},
        {"_0_0_fogData[1].sunPercentage", true, 592, 1, 4, 0},
        {"_0_0_fogData[1].sunDirection_and_fogZScalar", true, 608, 1, 4, 0},
        {"_0_0_fogData[1].heightFogCoeff", true, 624, 1, 4, 0},
        {"_0_0_fogData[1].mainFogCoeff", true, 640, 1, 4, 0},
        {"_0_0_fogData[1].heightDensityFogCoeff", true, 656, 1, 4, 0},
        {"_0_0_fogData[1].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 672, 1, 4, 0},
        {"_0_0_fogData[1].heightFogEndColor_fogStartOffset", true, 688, 1, 4, 0},
        {"_0_0_fogData[2].densityParams", true, 704, 1, 4, 0},
        {"_0_0_fogData[2].classicFogParams", true, 720, 1, 4, 0},
        {"_0_0_fogData[2].heightPlane", true, 736, 1, 4, 0},
        {"_0_0_fogData[2].color_and_heightRate", true, 752, 1, 4, 0},
        {"_0_0_fogData[2].heightDensity_and_endColor", true, 768, 1, 4, 0},
        {"_0_0_fogData[2].sunAngle_and_sunColor", true, 784, 1, 4, 0},
        {"_0_0_fogData[2].heightColor_and_endFogDistance", true, 800, 1, 4, 0},
        {"_0_0_fogData[2].sunPercentage", true, 816, 1, 4, 0},
        {"_0_0_fogData[2].sunDirection_and_fogZScalar", true, 832, 1, 4, 0},
        {"_0_0_fogData[2].heightFogCoeff", true, 848, 1, 4, 0},
        {"_0_0_fogData[2].mainFogCoeff", true, 864, 1, 4, 0},
        {"_0_0_fogData[2].heightDensityFogCoeff", true, 880, 1, 4, 0},
        {"_0_0_fogData[2].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 896, 1, 4, 0},
        {"_0_0_fogData[2].heightFogEndColor_fogStartOffset", true, 912, 1, 4, 0},
        {"_0_0_fogData[3].densityParams", true, 928, 1, 4, 0},
        {"_0_0_fogData[3].classicFogParams", true, 944, 1, 4, 0},
        {"_0_0_fogData[3].heightPlane", true, 960, 1, 4, 0},
        {"_0_0_fogData[3].color_and_heightRate", true, 976, 1, 4, 0},
        {"_0_0_fogData[3].heightDensity_and_endColor", true, 992, 1, 4, 0},
        {"_0_0_fogData[3].sunAngle_and_sunColor", true, 1008, 1, 4, 0},
        {"_0_0_fogData[3].heightColor_and_endFogDistance", true, 1024, 1, 4, 0},
        {"_0_0_fogData[3].sunPercentage", true, 1040, 1, 4, 0},
        {"_0_0_fogData[3].sunDirection_and_fogZScalar", true, 1056, 1, 4, 0},
        {"_0_0_fogData[3].heightFogCoeff", true, 1072, 1, 4, 0},
        {"_0_0_fogData[3].mainFogCoeff", true, 1088, 1, 4, 0},
        {"_0_0_fogData[3].heightDensityFogCoeff", true, 1104, 1, 4, 0},
        {"_0_0_fogData[3].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1120, 1, 4, 0},
        {"_0_0_fogData[3].heightFogEndColor_fogStartOffset", true, 1136, 1, 4, 0},
        {"_0_0_fogData[4].densityParams", true, 1152, 1, 4, 0},
        {"_0_0_fogData[4].classicFogParams", true, 1168, 1, 4, 0},
        {"_0_0_fogData[4].heightPlane", true, 1184, 1, 4, 0},
        {"_0_0_fogData[4].color_and_heightRate", true, 1200, 1, 4, 0},
        {"_0_0_fogData[4].heightDensity_and_endColor", true, 1216, 1, 4, 0},
        {"_0_0_fogData[4].sunAngle_and_sunColor", true, 1232, 1, 4, 0},
        {"_0_0_fogData[4].heightColor_and_endFogDistance", true, 1248, 1, 4, 0},
        {"_0_0_fogData[4].sunPercentage", true, 1264, 1, 4, 0},
        {"_0_0_fogData[4].sunDirection_and_fogZScalar", true, 1280, 1, 4, 0},
        {"_0_0_fogData[4].heightFogCoeff", true, 1296, 1, 4, 0},
        {"_0_0_fogData[4].mainFogCoeff", true, 1312, 1, 4, 0},
        {"_0_0_fogData[4].heightDensityFogCoeff", true, 1328, 1, 4, 0},
        {"_0_0_fogData[4].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1344, 1, 4, 0},
        {"_0_0_fogData[4].heightFogEndColor_fogStartOffset", true, 1360, 1, 4, 0},
        {"_0_0_fogData[5].densityParams", true, 1376, 1, 4, 0},
        {"_0_0_fogData[5].classicFogParams", true, 1392, 1, 4, 0},
        {"_0_0_fogData[5].heightPlane", true, 1408, 1, 4, 0},
        {"_0_0_fogData[5].color_and_heightRate", true, 1424, 1, 4, 0},
        {"_0_0_fogData[5].heightDensity_and_endColor", true, 1440, 1, 4, 0},
        {"_0_0_fogData[5].sunAngle_and_sunColor", true, 1456, 1, 4, 0},
        {"_0_0_fogData[5].heightColor_and_endFogDistance", true, 1472, 1, 4, 0},
        {"_0_0_fogData[5].sunPercentage", true, 1488, 1, 4, 0},
        {"_0_0_fogData[5].sunDirection_and_fogZScalar", true, 1504, 1, 4, 0},
        {"_0_0_fogData[5].heightFogCoeff", true, 1520, 1, 4, 0},
        {"_0_0_fogData[5].mainFogCoeff", true, 1536, 1, 4, 0},
        {"_0_0_fogData[5].heightDensityFogCoeff", true, 1552, 1, 4, 0},
        {"_0_0_fogData[5].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1568, 1, 4, 0},
        {"_0_0_fogData[5].heightFogEndColor_fogStartOffset", true, 1584, 1, 4, 0},
        {"_0_0_fogData[6].densityParams", true, 1600, 1, 4, 0},
        {"_0_0_fogData[6].classicFogParams", true, 1616, 1, 4, 0},
        {"_0_0_fogData[6].heightPlane", true, 1632, 1, 4, 0},
        {"_0_0_fogData[6].color_and_heightRate", true, 1648, 1, 4, 0},
        {"_0_0_fogData[6].heightDensity_and_endColor", true, 1664, 1, 4, 0},
        {"_0_0_fogData[6].sunAngle_and_sunColor", true, 1680, 1, 4, 0},
        {"_0_0_fogData[6].heightColor_and_endFogDistance", true, 1696, 1, 4, 0},
        {"_0_0_fogData[6].sunPercentage", true, 1712, 1, 4, 0},
        {"_0_0_fogData[6].sunDirection_and_fogZScalar", true, 1728, 1, 4, 0},
        {"_0_0_fogData[6].heightFogCoeff", true, 1744, 1, 4, 0},
        {"_0_0_fogData[6].mainFogCoeff", true, 1760, 1, 4, 0},
        {"_0_0_fogData[6].heightDensityFogCoeff", true, 1776, 1, 4, 0},
        {"_0_0_fogData[6].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1792, 1, 4, 0},
        {"_0_0_fogData[6].heightFogEndColor_fogStartOffset", true, 1808, 1, 4, 0},
        {"_0_0_fogData[7].densityParams", true, 1824, 1, 4, 0},
        {"_0_0_fogData[7].classicFogParams", true, 1840, 1, 4, 0},
        {"_0_0_fogData[7].heightPlane", true, 1856, 1, 4, 0},
        {"_0_0_fogData[7].color_and_heightRate", true, 1872, 1, 4, 0},
        {"_0_0_fogData[7].heightDensity_and_endColor", true, 1888, 1, 4, 0},
        {"_0_0_fogData[7].sunAngle_and_sunColor", true, 1904, 1, 4, 0},
        {"_0_0_fogData[7].heightColor_and_endFogDistance", true, 1920, 1, 4, 0},
        {"_0_0_fogData[7].sunPercentage", true, 1936, 1, 4, 0},
        {"_0_0_fogData[7].sunDirection_and_fogZScalar", true, 1952, 1, 4, 0},
        {"_0_0_fogData[7].heightFogCoeff", true, 1968, 1, 4, 0},
        {"_0_0_fogData[7].mainFogCoeff", true, 1984, 1, 4, 0},
        {"_0_0_fogData[7].heightDensityFogCoeff", true, 2000, 1, 4, 0},
        {"_0_0_fogData[7].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 2016, 1, 4, 0},
        {"_0_0_fogData[7].heightFogEndColor_fogStartOffset", true, 2032, 1, 4, 0},
      }
    },
    {
      4, {
        {"_1_4_uPixelShader_BlendMode_TextureTransformIndex", false, 0, 1, 4, 0},
      }
    },
  }},
  {"drawLinesShader",  {
  }},
  {"waterShader",  {
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 240, 1, 4, 0},
        {"_0_0_fogData[0].densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData[0].classicFogParams", true, 272, 1, 4, 0},
        {"_0_0_fogData[0].heightPlane", true, 288, 1, 4, 0},
        {"_0_0_fogData[0].color_and_heightRate", true, 304, 1, 4, 0},
        {"_0_0_fogData[0].heightDensity_and_endColor", true, 320, 1, 4, 0},
        {"_0_0_fogData[0].sunAngle_and_sunColor", true, 336, 1, 4, 0},
        {"_0_0_fogData[0].heightColor_and_endFogDistance", true, 352, 1, 4, 0},
        {"_0_0_fogData[0].sunPercentage", true, 368, 1, 4, 0},
        {"_0_0_fogData[0].sunDirection_and_fogZScalar", true, 384, 1, 4, 0},
        {"_0_0_fogData[0].heightFogCoeff", true, 400, 1, 4, 0},
        {"_0_0_fogData[0].mainFogCoeff", true, 416, 1, 4, 0},
        {"_0_0_fogData[0].heightDensityFogCoeff", true, 432, 1, 4, 0},
        {"_0_0_fogData[0].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 448, 1, 4, 0},
        {"_0_0_fogData[0].heightFogEndColor_fogStartOffset", true, 464, 1, 4, 0},
        {"_0_0_fogData[1].densityParams", true, 480, 1, 4, 0},
        {"_0_0_fogData[1].classicFogParams", true, 496, 1, 4, 0},
        {"_0_0_fogData[1].heightPlane", true, 512, 1, 4, 0},
        {"_0_0_fogData[1].color_and_heightRate", true, 528, 1, 4, 0},
        {"_0_0_fogData[1].heightDensity_and_endColor", true, 544, 1, 4, 0},
        {"_0_0_fogData[1].sunAngle_and_sunColor", true, 560, 1, 4, 0},
        {"_0_0_fogData[1].heightColor_and_endFogDistance", true, 576, 1, 4, 0},
        {"_0_0_fogData[1].sunPercentage", true, 592, 1, 4, 0},
        {"_0_0_fogData[1].sunDirection_and_fogZScalar", true, 608, 1, 4, 0},
        {"_0_0_fogData[1].heightFogCoeff", true, 624, 1, 4, 0},
        {"_0_0_fogData[1].mainFogCoeff", true, 640, 1, 4, 0},
        {"_0_0_fogData[1].heightDensityFogCoeff", true, 656, 1, 4, 0},
        {"_0_0_fogData[1].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 672, 1, 4, 0},
        {"_0_0_fogData[1].heightFogEndColor_fogStartOffset", true, 688, 1, 4, 0},
        {"_0_0_fogData[2].densityParams", true, 704, 1, 4, 0},
        {"_0_0_fogData[2].classicFogParams", true, 720, 1, 4, 0},
        {"_0_0_fogData[2].heightPlane", true, 736, 1, 4, 0},
        {"_0_0_fogData[2].color_and_heightRate", true, 752, 1, 4, 0},
        {"_0_0_fogData[2].heightDensity_and_endColor", true, 768, 1, 4, 0},
        {"_0_0_fogData[2].sunAngle_and_sunColor", true, 784, 1, 4, 0},
        {"_0_0_fogData[2].heightColor_and_endFogDistance", true, 800, 1, 4, 0},
        {"_0_0_fogData[2].sunPercentage", true, 816, 1, 4, 0},
        {"_0_0_fogData[2].sunDirection_and_fogZScalar", true, 832, 1, 4, 0},
        {"_0_0_fogData[2].heightFogCoeff", true, 848, 1, 4, 0},
        {"_0_0_fogData[2].mainFogCoeff", true, 864, 1, 4, 0},
        {"_0_0_fogData[2].heightDensityFogCoeff", true, 880, 1, 4, 0},
        {"_0_0_fogData[2].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 896, 1, 4, 0},
        {"_0_0_fogData[2].heightFogEndColor_fogStartOffset", true, 912, 1, 4, 0},
        {"_0_0_fogData[3].densityParams", true, 928, 1, 4, 0},
        {"_0_0_fogData[3].classicFogParams", true, 944, 1, 4, 0},
        {"_0_0_fogData[3].heightPlane", true, 960, 1, 4, 0},
        {"_0_0_fogData[3].color_and_heightRate", true, 976, 1, 4, 0},
        {"_0_0_fogData[3].heightDensity_and_endColor", true, 992, 1, 4, 0},
        {"_0_0_fogData[3].sunAngle_and_sunColor", true, 1008, 1, 4, 0},
        {"_0_0_fogData[3].heightColor_and_endFogDistance", true, 1024, 1, 4, 0},
        {"_0_0_fogData[3].sunPercentage", true, 1040, 1, 4, 0},
        {"_0_0_fogData[3].sunDirection_and_fogZScalar", true, 1056, 1, 4, 0},
        {"_0_0_fogData[3].heightFogCoeff", true, 1072, 1, 4, 0},
        {"_0_0_fogData[3].mainFogCoeff", true, 1088, 1, 4, 0},
        {"_0_0_fogData[3].heightDensityFogCoeff", true, 1104, 1, 4, 0},
        {"_0_0_fogData[3].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1120, 1, 4, 0},
        {"_0_0_fogData[3].heightFogEndColor_fogStartOffset", true, 1136, 1, 4, 0},
        {"_0_0_fogData[4].densityParams", true, 1152, 1, 4, 0},
        {"_0_0_fogData[4].classicFogParams", true, 1168, 1, 4, 0},
        {"_0_0_fogData[4].heightPlane", true, 1184, 1, 4, 0},
        {"_0_0_fogData[4].color_and_heightRate", true, 1200, 1, 4, 0},
        {"_0_0_fogData[4].heightDensity_and_endColor", true, 1216, 1, 4, 0},
        {"_0_0_fogData[4].sunAngle_and_sunColor", true, 1232, 1, 4, 0},
        {"_0_0_fogData[4].heightColor_and_endFogDistance", true, 1248, 1, 4, 0},
        {"_0_0_fogData[4].sunPercentage", true, 1264, 1, 4, 0},
        {"_0_0_fogData[4].sunDirection_and_fogZScalar", true, 1280, 1, 4, 0},
        {"_0_0_fogData[4].heightFogCoeff", true, 1296, 1, 4, 0},
        {"_0_0_fogData[4].mainFogCoeff", true, 1312, 1, 4, 0},
        {"_0_0_fogData[4].heightDensityFogCoeff", true, 1328, 1, 4, 0},
        {"_0_0_fogData[4].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1344, 1, 4, 0},
        {"_0_0_fogData[4].heightFogEndColor_fogStartOffset", true, 1360, 1, 4, 0},
        {"_0_0_fogData[5].densityParams", true, 1376, 1, 4, 0},
        {"_0_0_fogData[5].classicFogParams", true, 1392, 1, 4, 0},
        {"_0_0_fogData[5].heightPlane", true, 1408, 1, 4, 0},
        {"_0_0_fogData[5].color_and_heightRate", true, 1424, 1, 4, 0},
        {"_0_0_fogData[5].heightDensity_and_endColor", true, 1440, 1, 4, 0},
        {"_0_0_fogData[5].sunAngle_and_sunColor", true, 1456, 1, 4, 0},
        {"_0_0_fogData[5].heightColor_and_endFogDistance", true, 1472, 1, 4, 0},
        {"_0_0_fogData[5].sunPercentage", true, 1488, 1, 4, 0},
        {"_0_0_fogData[5].sunDirection_and_fogZScalar", true, 1504, 1, 4, 0},
        {"_0_0_fogData[5].heightFogCoeff", true, 1520, 1, 4, 0},
        {"_0_0_fogData[5].mainFogCoeff", true, 1536, 1, 4, 0},
        {"_0_0_fogData[5].heightDensityFogCoeff", true, 1552, 1, 4, 0},
        {"_0_0_fogData[5].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1568, 1, 4, 0},
        {"_0_0_fogData[5].heightFogEndColor_fogStartOffset", true, 1584, 1, 4, 0},
        {"_0_0_fogData[6].densityParams", true, 1600, 1, 4, 0},
        {"_0_0_fogData[6].classicFogParams", true, 1616, 1, 4, 0},
        {"_0_0_fogData[6].heightPlane", true, 1632, 1, 4, 0},
        {"_0_0_fogData[6].color_and_heightRate", true, 1648, 1, 4, 0},
        {"_0_0_fogData[6].heightDensity_and_endColor", true, 1664, 1, 4, 0},
        {"_0_0_fogData[6].sunAngle_and_sunColor", true, 1680, 1, 4, 0},
        {"_0_0_fogData[6].heightColor_and_endFogDistance", true, 1696, 1, 4, 0},
        {"_0_0_fogData[6].sunPercentage", true, 1712, 1, 4, 0},
        {"_0_0_fogData[6].sunDirection_and_fogZScalar", true, 1728, 1, 4, 0},
        {"_0_0_fogData[6].heightFogCoeff", true, 1744, 1, 4, 0},
        {"_0_0_fogData[6].mainFogCoeff", true, 1760, 1, 4, 0},
        {"_0_0_fogData[6].heightDensityFogCoeff", true, 1776, 1, 4, 0},
        {"_0_0_fogData[6].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1792, 1, 4, 0},
        {"_0_0_fogData[6].heightFogEndColor_fogStartOffset", true, 1808, 1, 4, 0},
        {"_0_0_fogData[7].densityParams", true, 1824, 1, 4, 0},
        {"_0_0_fogData[7].classicFogParams", true, 1840, 1, 4, 0},
        {"_0_0_fogData[7].heightPlane", true, 1856, 1, 4, 0},
        {"_0_0_fogData[7].color_and_heightRate", true, 1872, 1, 4, 0},
        {"_0_0_fogData[7].heightDensity_and_endColor", true, 1888, 1, 4, 0},
        {"_0_0_fogData[7].sunAngle_and_sunColor", true, 1904, 1, 4, 0},
        {"_0_0_fogData[7].heightColor_and_endFogDistance", true, 1920, 1, 4, 0},
        {"_0_0_fogData[7].sunPercentage", true, 1936, 1, 4, 0},
        {"_0_0_fogData[7].sunDirection_and_fogZScalar", true, 1952, 1, 4, 0},
        {"_0_0_fogData[7].heightFogCoeff", true, 1968, 1, 4, 0},
        {"_0_0_fogData[7].mainFogCoeff", true, 1984, 1, 4, 0},
        {"_0_0_fogData[7].heightDensityFogCoeff", true, 2000, 1, 4, 0},
        {"_0_0_fogData[7].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 2016, 1, 4, 0},
        {"_0_0_fogData[7].heightFogEndColor_fogStartOffset", true, 2032, 1, 4, 0},
      }
    },
    {
      4, {
        {"_1_4_materialId", false, 0, 1, 4, 0},
        {"_1_4_color", true, 16, 1, 4, 0},
        {"_1_4_textureMatrix", true, 32, 4, 4, 0},
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
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 240, 1, 4, 0},
        {"_0_0_fogData[0].densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData[0].classicFogParams", true, 272, 1, 4, 0},
        {"_0_0_fogData[0].heightPlane", true, 288, 1, 4, 0},
        {"_0_0_fogData[0].color_and_heightRate", true, 304, 1, 4, 0},
        {"_0_0_fogData[0].heightDensity_and_endColor", true, 320, 1, 4, 0},
        {"_0_0_fogData[0].sunAngle_and_sunColor", true, 336, 1, 4, 0},
        {"_0_0_fogData[0].heightColor_and_endFogDistance", true, 352, 1, 4, 0},
        {"_0_0_fogData[0].sunPercentage", true, 368, 1, 4, 0},
        {"_0_0_fogData[0].sunDirection_and_fogZScalar", true, 384, 1, 4, 0},
        {"_0_0_fogData[0].heightFogCoeff", true, 400, 1, 4, 0},
        {"_0_0_fogData[0].mainFogCoeff", true, 416, 1, 4, 0},
        {"_0_0_fogData[0].heightDensityFogCoeff", true, 432, 1, 4, 0},
        {"_0_0_fogData[0].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 448, 1, 4, 0},
        {"_0_0_fogData[0].heightFogEndColor_fogStartOffset", true, 464, 1, 4, 0},
        {"_0_0_fogData[1].densityParams", true, 480, 1, 4, 0},
        {"_0_0_fogData[1].classicFogParams", true, 496, 1, 4, 0},
        {"_0_0_fogData[1].heightPlane", true, 512, 1, 4, 0},
        {"_0_0_fogData[1].color_and_heightRate", true, 528, 1, 4, 0},
        {"_0_0_fogData[1].heightDensity_and_endColor", true, 544, 1, 4, 0},
        {"_0_0_fogData[1].sunAngle_and_sunColor", true, 560, 1, 4, 0},
        {"_0_0_fogData[1].heightColor_and_endFogDistance", true, 576, 1, 4, 0},
        {"_0_0_fogData[1].sunPercentage", true, 592, 1, 4, 0},
        {"_0_0_fogData[1].sunDirection_and_fogZScalar", true, 608, 1, 4, 0},
        {"_0_0_fogData[1].heightFogCoeff", true, 624, 1, 4, 0},
        {"_0_0_fogData[1].mainFogCoeff", true, 640, 1, 4, 0},
        {"_0_0_fogData[1].heightDensityFogCoeff", true, 656, 1, 4, 0},
        {"_0_0_fogData[1].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 672, 1, 4, 0},
        {"_0_0_fogData[1].heightFogEndColor_fogStartOffset", true, 688, 1, 4, 0},
        {"_0_0_fogData[2].densityParams", true, 704, 1, 4, 0},
        {"_0_0_fogData[2].classicFogParams", true, 720, 1, 4, 0},
        {"_0_0_fogData[2].heightPlane", true, 736, 1, 4, 0},
        {"_0_0_fogData[2].color_and_heightRate", true, 752, 1, 4, 0},
        {"_0_0_fogData[2].heightDensity_and_endColor", true, 768, 1, 4, 0},
        {"_0_0_fogData[2].sunAngle_and_sunColor", true, 784, 1, 4, 0},
        {"_0_0_fogData[2].heightColor_and_endFogDistance", true, 800, 1, 4, 0},
        {"_0_0_fogData[2].sunPercentage", true, 816, 1, 4, 0},
        {"_0_0_fogData[2].sunDirection_and_fogZScalar", true, 832, 1, 4, 0},
        {"_0_0_fogData[2].heightFogCoeff", true, 848, 1, 4, 0},
        {"_0_0_fogData[2].mainFogCoeff", true, 864, 1, 4, 0},
        {"_0_0_fogData[2].heightDensityFogCoeff", true, 880, 1, 4, 0},
        {"_0_0_fogData[2].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 896, 1, 4, 0},
        {"_0_0_fogData[2].heightFogEndColor_fogStartOffset", true, 912, 1, 4, 0},
        {"_0_0_fogData[3].densityParams", true, 928, 1, 4, 0},
        {"_0_0_fogData[3].classicFogParams", true, 944, 1, 4, 0},
        {"_0_0_fogData[3].heightPlane", true, 960, 1, 4, 0},
        {"_0_0_fogData[3].color_and_heightRate", true, 976, 1, 4, 0},
        {"_0_0_fogData[3].heightDensity_and_endColor", true, 992, 1, 4, 0},
        {"_0_0_fogData[3].sunAngle_and_sunColor", true, 1008, 1, 4, 0},
        {"_0_0_fogData[3].heightColor_and_endFogDistance", true, 1024, 1, 4, 0},
        {"_0_0_fogData[3].sunPercentage", true, 1040, 1, 4, 0},
        {"_0_0_fogData[3].sunDirection_and_fogZScalar", true, 1056, 1, 4, 0},
        {"_0_0_fogData[3].heightFogCoeff", true, 1072, 1, 4, 0},
        {"_0_0_fogData[3].mainFogCoeff", true, 1088, 1, 4, 0},
        {"_0_0_fogData[3].heightDensityFogCoeff", true, 1104, 1, 4, 0},
        {"_0_0_fogData[3].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1120, 1, 4, 0},
        {"_0_0_fogData[3].heightFogEndColor_fogStartOffset", true, 1136, 1, 4, 0},
        {"_0_0_fogData[4].densityParams", true, 1152, 1, 4, 0},
        {"_0_0_fogData[4].classicFogParams", true, 1168, 1, 4, 0},
        {"_0_0_fogData[4].heightPlane", true, 1184, 1, 4, 0},
        {"_0_0_fogData[4].color_and_heightRate", true, 1200, 1, 4, 0},
        {"_0_0_fogData[4].heightDensity_and_endColor", true, 1216, 1, 4, 0},
        {"_0_0_fogData[4].sunAngle_and_sunColor", true, 1232, 1, 4, 0},
        {"_0_0_fogData[4].heightColor_and_endFogDistance", true, 1248, 1, 4, 0},
        {"_0_0_fogData[4].sunPercentage", true, 1264, 1, 4, 0},
        {"_0_0_fogData[4].sunDirection_and_fogZScalar", true, 1280, 1, 4, 0},
        {"_0_0_fogData[4].heightFogCoeff", true, 1296, 1, 4, 0},
        {"_0_0_fogData[4].mainFogCoeff", true, 1312, 1, 4, 0},
        {"_0_0_fogData[4].heightDensityFogCoeff", true, 1328, 1, 4, 0},
        {"_0_0_fogData[4].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1344, 1, 4, 0},
        {"_0_0_fogData[4].heightFogEndColor_fogStartOffset", true, 1360, 1, 4, 0},
        {"_0_0_fogData[5].densityParams", true, 1376, 1, 4, 0},
        {"_0_0_fogData[5].classicFogParams", true, 1392, 1, 4, 0},
        {"_0_0_fogData[5].heightPlane", true, 1408, 1, 4, 0},
        {"_0_0_fogData[5].color_and_heightRate", true, 1424, 1, 4, 0},
        {"_0_0_fogData[5].heightDensity_and_endColor", true, 1440, 1, 4, 0},
        {"_0_0_fogData[5].sunAngle_and_sunColor", true, 1456, 1, 4, 0},
        {"_0_0_fogData[5].heightColor_and_endFogDistance", true, 1472, 1, 4, 0},
        {"_0_0_fogData[5].sunPercentage", true, 1488, 1, 4, 0},
        {"_0_0_fogData[5].sunDirection_and_fogZScalar", true, 1504, 1, 4, 0},
        {"_0_0_fogData[5].heightFogCoeff", true, 1520, 1, 4, 0},
        {"_0_0_fogData[5].mainFogCoeff", true, 1536, 1, 4, 0},
        {"_0_0_fogData[5].heightDensityFogCoeff", true, 1552, 1, 4, 0},
        {"_0_0_fogData[5].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1568, 1, 4, 0},
        {"_0_0_fogData[5].heightFogEndColor_fogStartOffset", true, 1584, 1, 4, 0},
        {"_0_0_fogData[6].densityParams", true, 1600, 1, 4, 0},
        {"_0_0_fogData[6].classicFogParams", true, 1616, 1, 4, 0},
        {"_0_0_fogData[6].heightPlane", true, 1632, 1, 4, 0},
        {"_0_0_fogData[6].color_and_heightRate", true, 1648, 1, 4, 0},
        {"_0_0_fogData[6].heightDensity_and_endColor", true, 1664, 1, 4, 0},
        {"_0_0_fogData[6].sunAngle_and_sunColor", true, 1680, 1, 4, 0},
        {"_0_0_fogData[6].heightColor_and_endFogDistance", true, 1696, 1, 4, 0},
        {"_0_0_fogData[6].sunPercentage", true, 1712, 1, 4, 0},
        {"_0_0_fogData[6].sunDirection_and_fogZScalar", true, 1728, 1, 4, 0},
        {"_0_0_fogData[6].heightFogCoeff", true, 1744, 1, 4, 0},
        {"_0_0_fogData[6].mainFogCoeff", true, 1760, 1, 4, 0},
        {"_0_0_fogData[6].heightDensityFogCoeff", true, 1776, 1, 4, 0},
        {"_0_0_fogData[6].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1792, 1, 4, 0},
        {"_0_0_fogData[6].heightFogEndColor_fogStartOffset", true, 1808, 1, 4, 0},
        {"_0_0_fogData[7].densityParams", true, 1824, 1, 4, 0},
        {"_0_0_fogData[7].classicFogParams", true, 1840, 1, 4, 0},
        {"_0_0_fogData[7].heightPlane", true, 1856, 1, 4, 0},
        {"_0_0_fogData[7].color_and_heightRate", true, 1872, 1, 4, 0},
        {"_0_0_fogData[7].heightDensity_and_endColor", true, 1888, 1, 4, 0},
        {"_0_0_fogData[7].sunAngle_and_sunColor", true, 1904, 1, 4, 0},
        {"_0_0_fogData[7].heightColor_and_endFogDistance", true, 1920, 1, 4, 0},
        {"_0_0_fogData[7].sunPercentage", true, 1936, 1, 4, 0},
        {"_0_0_fogData[7].sunDirection_and_fogZScalar", true, 1952, 1, 4, 0},
        {"_0_0_fogData[7].heightFogCoeff", true, 1968, 1, 4, 0},
        {"_0_0_fogData[7].mainFogCoeff", true, 1984, 1, 4, 0},
        {"_0_0_fogData[7].heightDensityFogCoeff", true, 2000, 1, 4, 0},
        {"_0_0_fogData[7].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 2016, 1, 4, 0},
        {"_0_0_fogData[7].heightFogEndColor_fogStartOffset", true, 2032, 1, 4, 0},
      }
    },
    {
      4, {
        {"_1_4_uAlphaTest_alphaMult_colorMult", true, 0, 1, 4, 0},
        {"_1_4_uPixelShaderBlendModev", false, 16, 1, 4, 0},
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
  {"drawPoints",  {
    {
      1, {
        {"_0_1_uColor", true, 0, 1, 3, 0},
      }
    },
  }},
  {"drawFrustumShader",  {
  }},
  {"ffxgauss4",  {
    {
      4, {
        {"_0_4_texOffsetX", true, 0, 1, 4, 0},
        {"_0_4_texOffsetY", true, 16, 1, 4, 0},
      }
    },
  }},
  {"skyConus",  {
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
      1, {
        {"_1_1_uPlacementMats", true, 0, 4, 4, 0},
      }
    },
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 240, 1, 4, 0},
        {"_0_0_fogData[0].densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData[0].classicFogParams", true, 272, 1, 4, 0},
        {"_0_0_fogData[0].heightPlane", true, 288, 1, 4, 0},
        {"_0_0_fogData[0].color_and_heightRate", true, 304, 1, 4, 0},
        {"_0_0_fogData[0].heightDensity_and_endColor", true, 320, 1, 4, 0},
        {"_0_0_fogData[0].sunAngle_and_sunColor", true, 336, 1, 4, 0},
        {"_0_0_fogData[0].heightColor_and_endFogDistance", true, 352, 1, 4, 0},
        {"_0_0_fogData[0].sunPercentage", true, 368, 1, 4, 0},
        {"_0_0_fogData[0].sunDirection_and_fogZScalar", true, 384, 1, 4, 0},
        {"_0_0_fogData[0].heightFogCoeff", true, 400, 1, 4, 0},
        {"_0_0_fogData[0].mainFogCoeff", true, 416, 1, 4, 0},
        {"_0_0_fogData[0].heightDensityFogCoeff", true, 432, 1, 4, 0},
        {"_0_0_fogData[0].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 448, 1, 4, 0},
        {"_0_0_fogData[0].heightFogEndColor_fogStartOffset", true, 464, 1, 4, 0},
        {"_0_0_fogData[1].densityParams", true, 480, 1, 4, 0},
        {"_0_0_fogData[1].classicFogParams", true, 496, 1, 4, 0},
        {"_0_0_fogData[1].heightPlane", true, 512, 1, 4, 0},
        {"_0_0_fogData[1].color_and_heightRate", true, 528, 1, 4, 0},
        {"_0_0_fogData[1].heightDensity_and_endColor", true, 544, 1, 4, 0},
        {"_0_0_fogData[1].sunAngle_and_sunColor", true, 560, 1, 4, 0},
        {"_0_0_fogData[1].heightColor_and_endFogDistance", true, 576, 1, 4, 0},
        {"_0_0_fogData[1].sunPercentage", true, 592, 1, 4, 0},
        {"_0_0_fogData[1].sunDirection_and_fogZScalar", true, 608, 1, 4, 0},
        {"_0_0_fogData[1].heightFogCoeff", true, 624, 1, 4, 0},
        {"_0_0_fogData[1].mainFogCoeff", true, 640, 1, 4, 0},
        {"_0_0_fogData[1].heightDensityFogCoeff", true, 656, 1, 4, 0},
        {"_0_0_fogData[1].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 672, 1, 4, 0},
        {"_0_0_fogData[1].heightFogEndColor_fogStartOffset", true, 688, 1, 4, 0},
        {"_0_0_fogData[2].densityParams", true, 704, 1, 4, 0},
        {"_0_0_fogData[2].classicFogParams", true, 720, 1, 4, 0},
        {"_0_0_fogData[2].heightPlane", true, 736, 1, 4, 0},
        {"_0_0_fogData[2].color_and_heightRate", true, 752, 1, 4, 0},
        {"_0_0_fogData[2].heightDensity_and_endColor", true, 768, 1, 4, 0},
        {"_0_0_fogData[2].sunAngle_and_sunColor", true, 784, 1, 4, 0},
        {"_0_0_fogData[2].heightColor_and_endFogDistance", true, 800, 1, 4, 0},
        {"_0_0_fogData[2].sunPercentage", true, 816, 1, 4, 0},
        {"_0_0_fogData[2].sunDirection_and_fogZScalar", true, 832, 1, 4, 0},
        {"_0_0_fogData[2].heightFogCoeff", true, 848, 1, 4, 0},
        {"_0_0_fogData[2].mainFogCoeff", true, 864, 1, 4, 0},
        {"_0_0_fogData[2].heightDensityFogCoeff", true, 880, 1, 4, 0},
        {"_0_0_fogData[2].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 896, 1, 4, 0},
        {"_0_0_fogData[2].heightFogEndColor_fogStartOffset", true, 912, 1, 4, 0},
        {"_0_0_fogData[3].densityParams", true, 928, 1, 4, 0},
        {"_0_0_fogData[3].classicFogParams", true, 944, 1, 4, 0},
        {"_0_0_fogData[3].heightPlane", true, 960, 1, 4, 0},
        {"_0_0_fogData[3].color_and_heightRate", true, 976, 1, 4, 0},
        {"_0_0_fogData[3].heightDensity_and_endColor", true, 992, 1, 4, 0},
        {"_0_0_fogData[3].sunAngle_and_sunColor", true, 1008, 1, 4, 0},
        {"_0_0_fogData[3].heightColor_and_endFogDistance", true, 1024, 1, 4, 0},
        {"_0_0_fogData[3].sunPercentage", true, 1040, 1, 4, 0},
        {"_0_0_fogData[3].sunDirection_and_fogZScalar", true, 1056, 1, 4, 0},
        {"_0_0_fogData[3].heightFogCoeff", true, 1072, 1, 4, 0},
        {"_0_0_fogData[3].mainFogCoeff", true, 1088, 1, 4, 0},
        {"_0_0_fogData[3].heightDensityFogCoeff", true, 1104, 1, 4, 0},
        {"_0_0_fogData[3].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1120, 1, 4, 0},
        {"_0_0_fogData[3].heightFogEndColor_fogStartOffset", true, 1136, 1, 4, 0},
        {"_0_0_fogData[4].densityParams", true, 1152, 1, 4, 0},
        {"_0_0_fogData[4].classicFogParams", true, 1168, 1, 4, 0},
        {"_0_0_fogData[4].heightPlane", true, 1184, 1, 4, 0},
        {"_0_0_fogData[4].color_and_heightRate", true, 1200, 1, 4, 0},
        {"_0_0_fogData[4].heightDensity_and_endColor", true, 1216, 1, 4, 0},
        {"_0_0_fogData[4].sunAngle_and_sunColor", true, 1232, 1, 4, 0},
        {"_0_0_fogData[4].heightColor_and_endFogDistance", true, 1248, 1, 4, 0},
        {"_0_0_fogData[4].sunPercentage", true, 1264, 1, 4, 0},
        {"_0_0_fogData[4].sunDirection_and_fogZScalar", true, 1280, 1, 4, 0},
        {"_0_0_fogData[4].heightFogCoeff", true, 1296, 1, 4, 0},
        {"_0_0_fogData[4].mainFogCoeff", true, 1312, 1, 4, 0},
        {"_0_0_fogData[4].heightDensityFogCoeff", true, 1328, 1, 4, 0},
        {"_0_0_fogData[4].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1344, 1, 4, 0},
        {"_0_0_fogData[4].heightFogEndColor_fogStartOffset", true, 1360, 1, 4, 0},
        {"_0_0_fogData[5].densityParams", true, 1376, 1, 4, 0},
        {"_0_0_fogData[5].classicFogParams", true, 1392, 1, 4, 0},
        {"_0_0_fogData[5].heightPlane", true, 1408, 1, 4, 0},
        {"_0_0_fogData[5].color_and_heightRate", true, 1424, 1, 4, 0},
        {"_0_0_fogData[5].heightDensity_and_endColor", true, 1440, 1, 4, 0},
        {"_0_0_fogData[5].sunAngle_and_sunColor", true, 1456, 1, 4, 0},
        {"_0_0_fogData[5].heightColor_and_endFogDistance", true, 1472, 1, 4, 0},
        {"_0_0_fogData[5].sunPercentage", true, 1488, 1, 4, 0},
        {"_0_0_fogData[5].sunDirection_and_fogZScalar", true, 1504, 1, 4, 0},
        {"_0_0_fogData[5].heightFogCoeff", true, 1520, 1, 4, 0},
        {"_0_0_fogData[5].mainFogCoeff", true, 1536, 1, 4, 0},
        {"_0_0_fogData[5].heightDensityFogCoeff", true, 1552, 1, 4, 0},
        {"_0_0_fogData[5].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1568, 1, 4, 0},
        {"_0_0_fogData[5].heightFogEndColor_fogStartOffset", true, 1584, 1, 4, 0},
        {"_0_0_fogData[6].densityParams", true, 1600, 1, 4, 0},
        {"_0_0_fogData[6].classicFogParams", true, 1616, 1, 4, 0},
        {"_0_0_fogData[6].heightPlane", true, 1632, 1, 4, 0},
        {"_0_0_fogData[6].color_and_heightRate", true, 1648, 1, 4, 0},
        {"_0_0_fogData[6].heightDensity_and_endColor", true, 1664, 1, 4, 0},
        {"_0_0_fogData[6].sunAngle_and_sunColor", true, 1680, 1, 4, 0},
        {"_0_0_fogData[6].heightColor_and_endFogDistance", true, 1696, 1, 4, 0},
        {"_0_0_fogData[6].sunPercentage", true, 1712, 1, 4, 0},
        {"_0_0_fogData[6].sunDirection_and_fogZScalar", true, 1728, 1, 4, 0},
        {"_0_0_fogData[6].heightFogCoeff", true, 1744, 1, 4, 0},
        {"_0_0_fogData[6].mainFogCoeff", true, 1760, 1, 4, 0},
        {"_0_0_fogData[6].heightDensityFogCoeff", true, 1776, 1, 4, 0},
        {"_0_0_fogData[6].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1792, 1, 4, 0},
        {"_0_0_fogData[6].heightFogEndColor_fogStartOffset", true, 1808, 1, 4, 0},
        {"_0_0_fogData[7].densityParams", true, 1824, 1, 4, 0},
        {"_0_0_fogData[7].classicFogParams", true, 1840, 1, 4, 0},
        {"_0_0_fogData[7].heightPlane", true, 1856, 1, 4, 0},
        {"_0_0_fogData[7].color_and_heightRate", true, 1872, 1, 4, 0},
        {"_0_0_fogData[7].heightDensity_and_endColor", true, 1888, 1, 4, 0},
        {"_0_0_fogData[7].sunAngle_and_sunColor", true, 1904, 1, 4, 0},
        {"_0_0_fogData[7].heightColor_and_endFogDistance", true, 1920, 1, 4, 0},
        {"_0_0_fogData[7].sunPercentage", true, 1936, 1, 4, 0},
        {"_0_0_fogData[7].sunDirection_and_fogZScalar", true, 1952, 1, 4, 0},
        {"_0_0_fogData[7].heightFogCoeff", true, 1968, 1, 4, 0},
        {"_0_0_fogData[7].mainFogCoeff", true, 1984, 1, 4, 0},
        {"_0_0_fogData[7].heightDensityFogCoeff", true, 2000, 1, 4, 0},
        {"_0_0_fogData[7].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 2016, 1, 4, 0},
        {"_0_0_fogData[7].heightFogEndColor_fogStartOffset", true, 2032, 1, 4, 0},
      }
    },
    {
      2, {
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
      7, {
      }
    },
  }},
  {"ffxglow",  {
    {
      4, {
        {"_0_4_blurAmount", true, 0, 1, 4, 0},
      }
    },
  }},
  {"wmoShader",  {
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUpSceneTime", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult_fogCount", true, 240, 1, 4, 0},
        {"_0_0_fogData[0].densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData[0].classicFogParams", true, 272, 1, 4, 0},
        {"_0_0_fogData[0].heightPlane", true, 288, 1, 4, 0},
        {"_0_0_fogData[0].color_and_heightRate", true, 304, 1, 4, 0},
        {"_0_0_fogData[0].heightDensity_and_endColor", true, 320, 1, 4, 0},
        {"_0_0_fogData[0].sunAngle_and_sunColor", true, 336, 1, 4, 0},
        {"_0_0_fogData[0].heightColor_and_endFogDistance", true, 352, 1, 4, 0},
        {"_0_0_fogData[0].sunPercentage", true, 368, 1, 4, 0},
        {"_0_0_fogData[0].sunDirection_and_fogZScalar", true, 384, 1, 4, 0},
        {"_0_0_fogData[0].heightFogCoeff", true, 400, 1, 4, 0},
        {"_0_0_fogData[0].mainFogCoeff", true, 416, 1, 4, 0},
        {"_0_0_fogData[0].heightDensityFogCoeff", true, 432, 1, 4, 0},
        {"_0_0_fogData[0].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 448, 1, 4, 0},
        {"_0_0_fogData[0].heightFogEndColor_fogStartOffset", true, 464, 1, 4, 0},
        {"_0_0_fogData[1].densityParams", true, 480, 1, 4, 0},
        {"_0_0_fogData[1].classicFogParams", true, 496, 1, 4, 0},
        {"_0_0_fogData[1].heightPlane", true, 512, 1, 4, 0},
        {"_0_0_fogData[1].color_and_heightRate", true, 528, 1, 4, 0},
        {"_0_0_fogData[1].heightDensity_and_endColor", true, 544, 1, 4, 0},
        {"_0_0_fogData[1].sunAngle_and_sunColor", true, 560, 1, 4, 0},
        {"_0_0_fogData[1].heightColor_and_endFogDistance", true, 576, 1, 4, 0},
        {"_0_0_fogData[1].sunPercentage", true, 592, 1, 4, 0},
        {"_0_0_fogData[1].sunDirection_and_fogZScalar", true, 608, 1, 4, 0},
        {"_0_0_fogData[1].heightFogCoeff", true, 624, 1, 4, 0},
        {"_0_0_fogData[1].mainFogCoeff", true, 640, 1, 4, 0},
        {"_0_0_fogData[1].heightDensityFogCoeff", true, 656, 1, 4, 0},
        {"_0_0_fogData[1].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 672, 1, 4, 0},
        {"_0_0_fogData[1].heightFogEndColor_fogStartOffset", true, 688, 1, 4, 0},
        {"_0_0_fogData[2].densityParams", true, 704, 1, 4, 0},
        {"_0_0_fogData[2].classicFogParams", true, 720, 1, 4, 0},
        {"_0_0_fogData[2].heightPlane", true, 736, 1, 4, 0},
        {"_0_0_fogData[2].color_and_heightRate", true, 752, 1, 4, 0},
        {"_0_0_fogData[2].heightDensity_and_endColor", true, 768, 1, 4, 0},
        {"_0_0_fogData[2].sunAngle_and_sunColor", true, 784, 1, 4, 0},
        {"_0_0_fogData[2].heightColor_and_endFogDistance", true, 800, 1, 4, 0},
        {"_0_0_fogData[2].sunPercentage", true, 816, 1, 4, 0},
        {"_0_0_fogData[2].sunDirection_and_fogZScalar", true, 832, 1, 4, 0},
        {"_0_0_fogData[2].heightFogCoeff", true, 848, 1, 4, 0},
        {"_0_0_fogData[2].mainFogCoeff", true, 864, 1, 4, 0},
        {"_0_0_fogData[2].heightDensityFogCoeff", true, 880, 1, 4, 0},
        {"_0_0_fogData[2].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 896, 1, 4, 0},
        {"_0_0_fogData[2].heightFogEndColor_fogStartOffset", true, 912, 1, 4, 0},
        {"_0_0_fogData[3].densityParams", true, 928, 1, 4, 0},
        {"_0_0_fogData[3].classicFogParams", true, 944, 1, 4, 0},
        {"_0_0_fogData[3].heightPlane", true, 960, 1, 4, 0},
        {"_0_0_fogData[3].color_and_heightRate", true, 976, 1, 4, 0},
        {"_0_0_fogData[3].heightDensity_and_endColor", true, 992, 1, 4, 0},
        {"_0_0_fogData[3].sunAngle_and_sunColor", true, 1008, 1, 4, 0},
        {"_0_0_fogData[3].heightColor_and_endFogDistance", true, 1024, 1, 4, 0},
        {"_0_0_fogData[3].sunPercentage", true, 1040, 1, 4, 0},
        {"_0_0_fogData[3].sunDirection_and_fogZScalar", true, 1056, 1, 4, 0},
        {"_0_0_fogData[3].heightFogCoeff", true, 1072, 1, 4, 0},
        {"_0_0_fogData[3].mainFogCoeff", true, 1088, 1, 4, 0},
        {"_0_0_fogData[3].heightDensityFogCoeff", true, 1104, 1, 4, 0},
        {"_0_0_fogData[3].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1120, 1, 4, 0},
        {"_0_0_fogData[3].heightFogEndColor_fogStartOffset", true, 1136, 1, 4, 0},
        {"_0_0_fogData[4].densityParams", true, 1152, 1, 4, 0},
        {"_0_0_fogData[4].classicFogParams", true, 1168, 1, 4, 0},
        {"_0_0_fogData[4].heightPlane", true, 1184, 1, 4, 0},
        {"_0_0_fogData[4].color_and_heightRate", true, 1200, 1, 4, 0},
        {"_0_0_fogData[4].heightDensity_and_endColor", true, 1216, 1, 4, 0},
        {"_0_0_fogData[4].sunAngle_and_sunColor", true, 1232, 1, 4, 0},
        {"_0_0_fogData[4].heightColor_and_endFogDistance", true, 1248, 1, 4, 0},
        {"_0_0_fogData[4].sunPercentage", true, 1264, 1, 4, 0},
        {"_0_0_fogData[4].sunDirection_and_fogZScalar", true, 1280, 1, 4, 0},
        {"_0_0_fogData[4].heightFogCoeff", true, 1296, 1, 4, 0},
        {"_0_0_fogData[4].mainFogCoeff", true, 1312, 1, 4, 0},
        {"_0_0_fogData[4].heightDensityFogCoeff", true, 1328, 1, 4, 0},
        {"_0_0_fogData[4].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1344, 1, 4, 0},
        {"_0_0_fogData[4].heightFogEndColor_fogStartOffset", true, 1360, 1, 4, 0},
        {"_0_0_fogData[5].densityParams", true, 1376, 1, 4, 0},
        {"_0_0_fogData[5].classicFogParams", true, 1392, 1, 4, 0},
        {"_0_0_fogData[5].heightPlane", true, 1408, 1, 4, 0},
        {"_0_0_fogData[5].color_and_heightRate", true, 1424, 1, 4, 0},
        {"_0_0_fogData[5].heightDensity_and_endColor", true, 1440, 1, 4, 0},
        {"_0_0_fogData[5].sunAngle_and_sunColor", true, 1456, 1, 4, 0},
        {"_0_0_fogData[5].heightColor_and_endFogDistance", true, 1472, 1, 4, 0},
        {"_0_0_fogData[5].sunPercentage", true, 1488, 1, 4, 0},
        {"_0_0_fogData[5].sunDirection_and_fogZScalar", true, 1504, 1, 4, 0},
        {"_0_0_fogData[5].heightFogCoeff", true, 1520, 1, 4, 0},
        {"_0_0_fogData[5].mainFogCoeff", true, 1536, 1, 4, 0},
        {"_0_0_fogData[5].heightDensityFogCoeff", true, 1552, 1, 4, 0},
        {"_0_0_fogData[5].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1568, 1, 4, 0},
        {"_0_0_fogData[5].heightFogEndColor_fogStartOffset", true, 1584, 1, 4, 0},
        {"_0_0_fogData[6].densityParams", true, 1600, 1, 4, 0},
        {"_0_0_fogData[6].classicFogParams", true, 1616, 1, 4, 0},
        {"_0_0_fogData[6].heightPlane", true, 1632, 1, 4, 0},
        {"_0_0_fogData[6].color_and_heightRate", true, 1648, 1, 4, 0},
        {"_0_0_fogData[6].heightDensity_and_endColor", true, 1664, 1, 4, 0},
        {"_0_0_fogData[6].sunAngle_and_sunColor", true, 1680, 1, 4, 0},
        {"_0_0_fogData[6].heightColor_and_endFogDistance", true, 1696, 1, 4, 0},
        {"_0_0_fogData[6].sunPercentage", true, 1712, 1, 4, 0},
        {"_0_0_fogData[6].sunDirection_and_fogZScalar", true, 1728, 1, 4, 0},
        {"_0_0_fogData[6].heightFogCoeff", true, 1744, 1, 4, 0},
        {"_0_0_fogData[6].mainFogCoeff", true, 1760, 1, 4, 0},
        {"_0_0_fogData[6].heightDensityFogCoeff", true, 1776, 1, 4, 0},
        {"_0_0_fogData[6].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 1792, 1, 4, 0},
        {"_0_0_fogData[6].heightFogEndColor_fogStartOffset", true, 1808, 1, 4, 0},
        {"_0_0_fogData[7].densityParams", true, 1824, 1, 4, 0},
        {"_0_0_fogData[7].classicFogParams", true, 1840, 1, 4, 0},
        {"_0_0_fogData[7].heightPlane", true, 1856, 1, 4, 0},
        {"_0_0_fogData[7].color_and_heightRate", true, 1872, 1, 4, 0},
        {"_0_0_fogData[7].heightDensity_and_endColor", true, 1888, 1, 4, 0},
        {"_0_0_fogData[7].sunAngle_and_sunColor", true, 1904, 1, 4, 0},
        {"_0_0_fogData[7].heightColor_and_endFogDistance", true, 1920, 1, 4, 0},
        {"_0_0_fogData[7].sunPercentage", true, 1936, 1, 4, 0},
        {"_0_0_fogData[7].sunDirection_and_fogZScalar", true, 1952, 1, 4, 0},
        {"_0_0_fogData[7].heightFogCoeff", true, 1968, 1, 4, 0},
        {"_0_0_fogData[7].mainFogCoeff", true, 1984, 1, 4, 0},
        {"_0_0_fogData[7].heightDensityFogCoeff", true, 2000, 1, 4, 0},
        {"_0_0_fogData[7].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha", true, 2016, 1, 4, 0},
        {"_0_0_fogData[7].heightFogEndColor_fogStartOffset", true, 2032, 1, 4, 0},
      }
    },
    {
      4, {
        {"_1_4_UseLitColor_EnableAlpha_PixelShader_BlendMode", false, 0, 1, 4, 0},
        {"_1_4_FogColor_AlphaTest", true, 16, 1, 4, 0},
      }
    },
  }},
  {"imguiShader",  {
  }},
};
#endif


#endif
