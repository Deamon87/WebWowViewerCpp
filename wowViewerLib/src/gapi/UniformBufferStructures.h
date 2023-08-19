//
// Created by deamon on 09.07.18.
//

#ifndef AWEBWOWVIEWERCPP_UNIFORMBUFFERSTRUCTURES_H
#define AWEBWOWVIEWERCPP_UNIFORMBUFFERSTRUCTURES_H

//Uniform buffer structures
#include <mathfu/glsl_mappings.h>
#include "mathfu/glsl_mappings.h"

#define MAX_MATRIX_NUM 256
#define MAX_M2COLORS_NUM 256
#define MAX_TEXTURE_WEIGHT_NUM 64
#define MAX_TEXTURE_MATRIX_NUM 64

struct PSFog
{
    mathfu::vec4_packed densityParams;
    mathfu::vec4_packed heightPlane;
    mathfu::vec4_packed color_and_heightRate;
    mathfu::vec4_packed heightDensity_and_endColor;
    mathfu::vec4_packed sunAngle_and_sunColor;
    mathfu::vec4_packed heightColor_and_endFogDistance;
    mathfu::vec4_packed sunPercentage;
};

struct SceneExteriorLight {
    mathfu::vec4_packed uExteriorAmbientColor;
    mathfu::vec4_packed uExteriorHorizontAmbientColor;
    mathfu::vec4_packed uExteriorGroundAmbientColor;
    mathfu::vec4_packed uExteriorDirectColor;
    mathfu::vec4_packed uExteriorDirectColorDir;
    mathfu::vec4_packed uAdtSpecMult;
};

struct sceneWideBlockVSPS {
    mathfu::mat4 uLookAtMat;
    mathfu::mat4 uPMatrix;
    mathfu::vec4_packed uViewUpSceneTime;
    mathfu::vec4_packed uInteriorSunDir;

    SceneExteriorLight extLight;
    PSFog fogData;
};

struct InteriorLightParam {
    mathfu::vec4_packed uInteriorAmbientColorAndApplyInteriorLight;
    mathfu::vec4_packed uInteriorDirectColorAndApplyExteriorLight;
};

struct LocalLight
{
    mathfu::vec4_packed color;
    mathfu::vec4_packed position;
    mathfu::vec4_packed attenuation;
};

namespace M2 {
    struct PlacementMatrix {
        mathfu::mat4 uPlacementMat;
    };
    struct Bones {
        mathfu::mat4 uBoneMatrixes[MAX_MATRIX_NUM];
    };

    struct M2Colors {
        mathfu::vec4_packed colors[MAX_M2COLORS_NUM];
    };

    struct TextureWeights {
        float textureWeight[MAX_TEXTURE_WEIGHT_NUM];
    };

    struct TextureMatrices {
        mathfu::mat4 textureMatrix[MAX_TEXTURE_MATRIX_NUM];
    };

    struct meshWideBlockVSPS {
        int VertexShader;
        int IsAffectedByLight;
        int textureMatIndex1;
        int textureMatIndex2;
        int PixelShader;
        int UnFogged;
        int BlendMode;
        int unused;
        int textureWeightIndexes[4];
        int colorIndex;
        int applyWeight;
        int unused2;
        int unused3;
    };

    struct M2InstanceRecordBindless {
        int placementMatrixInd;
        int boneMatrixInd;
        int m2ColorsInd;
        int textureWeightsInd;
        int textureMatricesInd;
        int modelFragmentDatasInd;
    };

    struct meshWideBlockVSPS_Bindless {
        int VertexShader;
        int IsAffectedByLight;
        int textureMatIndex1;
        int textureMatIndex2;
        int PixelShader;
        int UnFogged;
        int BlendMode;
        int unused;
        int textureWeightIndexes[4];
        int colorIndex;
        int applyWeight;
        int instanceIndex;
        int unused3;
        int textureIndicies[4];
    };

    //M2 Pixel buffer formats
    struct modelWideBlockPS {
        InteriorLightParam intLight;
        LocalLight pc_lights[4];
        int LightCount;
        int bcHack;
        int notUsed2;
        int notUsed3;
        mathfu::vec4_packed interiorExteriorBlend;
    };

    namespace WaterfallData {
        struct meshWideBlockVS {
            float bumpScale;
            int textureMatIndex1;
            int textureMatIndex2;
            int unused;
        };
        struct meshWideBlockPS {
            mathfu::vec4_packed values0;
            mathfu::vec4_packed values1;
            mathfu::vec4_packed m_values2;
            mathfu::vec4_packed m_values3;
            mathfu::vec4_packed m_values4;
            mathfu::vec4_packed baseColor;
        };
    }
}
namespace Particle {
    struct meshParticleWideBlockPS {
        float uAlphaTest;
        float alphaMult;
        float colorMult;
        float textureScale2;
        int uPixelShader;
        int uBlendMode;
        int padding2;   // according to std140
        int padding3;   // according to std140
    };
}

namespace Ribbon {
    struct meshRibbonWideBlockPS {
        int uPixelShader;
        int uBlendMode;
        int uTextureTransformIndex;   // according to std140
        int padding3;   // according to std140
    };
}

namespace WMO {
    //WMO VertexBuffer format
    struct modelWideBlockVS {
        mathfu::mat4 uPlacementMat;
    };

    struct meshWideBlockVS {
        int VertexShader;
        int UseLitColor;

        int padding[2];
    };

    struct meshWideBlockPS {
//    PACK({struct
        int UseLitColor;
        int EnableAlpha;
        int PixelShader;
        int BlendMode;
        mathfu::vec4_packed uFogColor_AlphaTest;
//    )}
    };
}
namespace ADT {
    struct meshWideBlockVSPS {
        mathfu::vec4 uPos;
        int useHeightMixFormula[4];
        float uHeightScale[4];
        float uHeightOffset[4];
    };
    //This one needs to be updated every frame. The one above - doesn't
    struct meshWideBlockPS {
        float scaleFactorPerLayer[4];
        int animation_rotationPerLayer[4];
        int animation_speedPerLayer[4];
    };
}

namespace Water {
    struct meshWideBlockPS {
        int32_t materialId;
        int32_t unused1;
        int32_t unused2;
        int32_t unused3;
        mathfu::vec4_packed color;
        mathfu::mat4 textureMatrix;
    };
}

namespace ImgUI {
    struct modelWideBlockVS {
        mathfu::mat4 projectionMat;
        float scale[4];
    };
}

namespace FXGauss {
    struct meshWideBlockPS {
        float texOffsetX[4];
        float texOffsetY[4];
    };
}

namespace DnSky {
    struct meshWideBlockVS {
        mathfu::vec4_packed skyColor[6];
    };
}

namespace DrawPortalShader {
    struct meshWideBlockPS {
        mathfu::vec4_packed uColor;
    };
}

struct bbModelWideBlockVS {
    mathfu::mat4 uPlacementMat;

    mathfu::vec4_packed uBBScale;
    mathfu::vec4_packed uBBCenter;
    mathfu::vec4_packed uColor;
};


#endif //AWEBWOWVIEWERCPP_UNIFORMBUFFERSTRUCTURES_H
