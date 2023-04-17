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
{ "wmoShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,1,64},
      {0,0,368},
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
{ "wmoShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,4,32},
      {0,0,368},
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
      {1,5, "uTexture"},
      {1,6, "uTexture2"},
      {1,7, "uTexture3"},
      {1,8, "uTexture4"},
      {1,9, "uTexture5"},
      {1,10, "uTexture6"},
      {1,11, "uTexture7"},
      {1,12, "uTexture8"},
      {1,13, "uTexture9"},
    },
    {
      {
        {0,0,0},
        {5,13,9},
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
{ "waterShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,4,16},
      {0,0,368},
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
      {1,5, "uTexture"},
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
{ "waterfallShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,4,96},
      {0,0,368},
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
      {1,9, "uNormalTex"},
      {1,7, "uNoise"},
      {1,6, "uWhiteWater"},
      {1,5, "uMask"},
    },
    {
      {
        {0,0,0},
        {5,9,5},
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
{ "adtShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,368},
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
{ "adtLodShader.vert.spv", 
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
{ "waterfallShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,2,144},
      {0,1,14144},
      {0,0,368},
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
      {1,8, "uBumpTexture"},
    },
    {
      {
        {0,0,0},
        {8,8,1},
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
{ "drawPoints.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,128},
      {0,1,64},
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
{ "drawFrustumShader.vert.spv", 
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
{ "ffxglow.frag.spv", 
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
{ "adtShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,2,256},
      {0,1,64},
      {0,0,368},
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
      {1,9, "uAlphaTexture"},
      {1,10, "uLayerHeight0"},
      {1,11, "uLayerHeight1"},
      {1,12, "uLayerHeight2"},
      {1,13, "uLayerHeight3"},
      {1,5, "uLayer0"},
      {1,6, "uLayer1"},
      {1,7, "uLayer2"},
      {1,8, "uLayer3"},
    },
    {
      {
        {0,0,0},
        {5,13,9},
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
{ "drawBBShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,1,112},
      {0,0,368},
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
{ "drawFrustumShader.frag.spv", 
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
{ "drawDepthShader.frag.spv", 
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
{ "adtLodShader.frag.spv", 
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
{ "drawPoints.frag.spv", 
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
{ "drawBBShader.frag.spv", 
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
{ "drawLinesShader.frag.spv", 
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
{ "skyConus.frag.spv", 
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
{ "drawPortalShader.frag.spv", 
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
{ "drawLinesShader.vert.spv", 
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
{ "ffxgauss4.frag.spv", 
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
{ "imguiShader.frag.spv", 
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
{ "m2ParticleShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,368},
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
{ "drawQuad.vert.spv", 
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
{ "imguiShader.vert.spv", 
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
{ "m2Shader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,7,64},
      {0,5,256},
      {0,4,4096},
      {0,2,256},
      {0,0,368},
      {0,1,64},
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
      {1,6, "uTexture"},
      {1,7, "uTexture2"},
      {1,8, "uTexture3"},
      {1,9, "uTexture4"},
    },
    {
      {
        {0,0,0},
        {6,9,4},
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
{ "waterShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,368},
      {0,1,64},
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
{ "drawPortalShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,368},
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
{ "ribbonShader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,368},
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
{ "skyConus.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,0,368},
      {0,1,96},
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
{ "renderFrameBufferShader.frag.spv", 
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
{ "renderFrameBufferShader.vert.spv", 
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
{ "m2ParticleShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,4,32},
      {0,0,368},
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
      {1,5, "uTexture"},
      {1,6, "uTexture2"},
      {1,7, "uTexture3"},
    },
    {
      {
        {0,0,0},
        {5,7,3},
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
{ "m2Shader.vert.spv", 
  {
    ShaderStage::Vertex,
    {
      {0,3,14080},
      {0,1,64},
      {0,0,368},
      {0,7,64},
      {0,6,4096},
    },
    {
      {
        {6,6,1},
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
{ "ribbonShader.frag.spv", 
  {
    ShaderStage::Fragment,
    {
      {0,4,48},
      {0,0,368},
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
      {1,5, "uTexture"},
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
};

const  std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameVert = {
  {"waterfallShader",  {
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUp", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      1, {
        {"_0_1_uPlacementMat", true, 0, 4, 4, 0},
        {"_0_1_uBoneMatrixes[0]", true, 64, 4, 4, 220},
      }
    },
    {
      2, {
        {"_0_2_bumpScale", true, 0, 1, 4, 0},
        {"_0_2_uTextMat[0]", true, 16, 4, 4, 2},
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
        {"_0_0_scene_uViewUp", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
  }},
  {"drawBBShader",  {
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUp", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 352, 1, 4, 0},
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
        {"_0_1_uPlacementMat", true, 0, 4, 4, 0},
      }
    },
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUp", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
  }},
  {"m2ParticleShader",  {
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUp", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 352, 1, 4, 0},
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
        {"_0_1_skyColor[0]", true, 0, 1, 4, 6},
      }
    },
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUp", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
  }},
  {"m2Shader",  {
    {
      6, {
        {"_0_6_textureMatrix[0]", true, 0, 4, 4, 64},
      }
    },
    {
      7, {
        {"_0_7_vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2", false, 0, 1, 4, 0},
        {"_0_7_PixelShader_UnFogged_blendMode", false, 16, 1, 4, 0},
        {"_0_7_textureWeightIndexes", false, 32, 1, 4, 0},
        {"_0_7_colorIndex_applyWeight", false, 48, 1, 4, 0},
      }
    },
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUp", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      1, {
        {"_0_1_uPlacementMat", true, 0, 4, 4, 0},
      }
    },
    {
      3, {
        {"_0_3_uBoneMatrixes[0]", true, 0, 4, 4, 220},
      }
    },
  }},
  {"drawPortalShader",  {
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUp", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
  }},
  {"renderFrameBufferShader",  {
  }},
  {"wmoShader",  {
    {
      2, {
        {"_0_2_VertexShader_UseLitColor", false, 0, 1, 4, 0},
      }
    },
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUp", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      1, {
        {"_0_1_uPlacementMat", true, 0, 4, 4, 0},
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
        {"_0_0_scene_uViewUp", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 352, 1, 4, 0},
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
        {"_0_0_scene_uViewUp", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      4, {
        {"_0_4_values0", true, 0, 1, 4, 0},
        {"_0_4_values1", true, 16, 1, 4, 0},
        {"_0_4_values2", true, 32, 1, 4, 0},
        {"_0_4_values3", true, 48, 1, 4, 0},
        {"_0_4_values4", true, 64, 1, 4, 0},
        {"_0_4_baseColor", true, 80, 1, 4, 0},
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
        {"_0_1_uColor", true, 0, 1, 4, 0},
      }
    },
  }},
  {"adtShader",  {
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUp", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      1, {
        {"_0_1_uPos", true, 0, 1, 4, 0},
        {"_0_1_uUseHeightMixFormula", false, 16, 1, 4, 0},
        {"_0_1_uHeightScale", true, 32, 1, 4, 0},
        {"_0_1_uHeightOffset", true, 48, 1, 4, 0},
      }
    },
    {
      2, {
        {"_0_2_animationMat[0]", true, 0, 4, 4, 4},
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
        {"_0_0_scene_uViewUp", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      4, {
        {"_0_4_uAlphaTestScalev", true, 0, 1, 4, 0},
        {"_0_4_uPixelShaderv", false, 16, 1, 4, 0},
        {"_0_4_uTextureTranslate", true, 32, 1, 4, 0},
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
        {"_0_0_scene_uViewUp", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      4, {
        {"_0_4_color", true, 0, 1, 4, 0},
      }
    },
  }},
  {"m2ParticleShader",  {
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUp", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      4, {
        {"_0_4_uAlphaTestv", true, 0, 1, 4, 0},
        {"_0_4_uPixelShaderBlendModev", false, 16, 1, 4, 0},
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
      1, {
        {"_0_1_uPlacementMat", true, 0, 4, 4, 0},
      }
    },
    {
      0, {
        {"_0_0_scene_uLookAtMat", true, 0, 4, 4, 0},
        {"_0_0_scene_uPMatrix", true, 64, 4, 4, 0},
        {"_0_0_scene_uViewUp", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      2, {
        {"_0_2_intLight_uInteriorAmbientColorAndApplyInteriorLight", true, 0, 1, 4, 0},
        {"_0_2_intLight_uInteriorDirectColorAndApplyExteriorLight", true, 16, 1, 4, 0},
        {"_0_2_pc_lights[0].color", true, 32, 1, 4, 0},
        {"_0_2_pc_lights[0].position", true, 48, 1, 4, 0},
        {"_0_2_pc_lights[0].attenuation", true, 64, 1, 4, 0},
        {"_0_2_pc_lights[1].color", true, 80, 1, 4, 0},
        {"_0_2_pc_lights[1].position", true, 96, 1, 4, 0},
        {"_0_2_pc_lights[1].attenuation", true, 112, 1, 4, 0},
        {"_0_2_pc_lights[2].color", true, 128, 1, 4, 0},
        {"_0_2_pc_lights[2].position", true, 144, 1, 4, 0},
        {"_0_2_pc_lights[2].attenuation", true, 160, 1, 4, 0},
        {"_0_2_pc_lights[3].color", true, 176, 1, 4, 0},
        {"_0_2_pc_lights[3].position", true, 192, 1, 4, 0},
        {"_0_2_pc_lights[3].attenuation", true, 208, 1, 4, 0},
        {"_0_2_lightCountAndBcHack", false, 224, 1, 4, 0},
        {"_0_2_interiorExteriorBlend", true, 240, 1, 4, 0},
      }
    },
    {
      4, {
        {"_0_4_colors[0]", true, 0, 1, 4, 256},
      }
    },
    {
      5, {
        {"_0_5_textureWeight[0]", true, 0, 1, 4, 16},
      }
    },
    {
      7, {
        {"_0_7_vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2", false, 0, 1, 4, 0},
        {"_0_7_PixelShader_UnFogged_blendMode", false, 16, 1, 4, 0},
        {"_0_7_textureWeightIndexes", false, 32, 1, 4, 0},
        {"_0_7_colorIndex_applyWeight", false, 48, 1, 4, 0},
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
        {"_0_0_scene_uViewUp", true, 128, 1, 4, 0},
        {"_0_0_scene_uInteriorSunDir", true, 144, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorAmbientColor", true, 160, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorHorizontAmbientColor", true, 176, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorGroundAmbientColor", true, 192, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColor", true, 208, 1, 4, 0},
        {"_0_0_scene_extLight_uExteriorDirectColorDir", true, 224, 1, 4, 0},
        {"_0_0_scene_extLight_adtSpecMult", true, 240, 1, 4, 0},
        {"_0_0_fogData_densityParams", true, 256, 1, 4, 0},
        {"_0_0_fogData_heightPlane", true, 272, 1, 4, 0},
        {"_0_0_fogData_color_and_heightRate", true, 288, 1, 4, 0},
        {"_0_0_fogData_heightDensity_and_endColor", true, 304, 1, 4, 0},
        {"_0_0_fogData_sunAngle_and_sunColor", true, 320, 1, 4, 0},
        {"_0_0_fogData_heightColor_and_endFogDistance", true, 336, 1, 4, 0},
        {"_0_0_fogData_sunPercentage", true, 352, 1, 4, 0},
      }
    },
    {
      4, {
        {"_0_4_UseLitColor_EnableAlpha_PixelShader_BlendMode", false, 0, 1, 4, 0},
        {"_0_4_FogColor_AlphaTest", true, 16, 1, 4, 0},
      }
    },
  }},
  {"imguiShader",  {
  }},
};
#endif


#endif
