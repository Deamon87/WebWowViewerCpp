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
    mathfu::vec4_packed  densityParams;
    mathfu::vec4_packed  classicFogParams;
    mathfu::vec4_packed  heightPlane;
    mathfu::vec4_packed  color_and_heightRate;
    mathfu::vec4_packed  heightDensity_and_endColor;
    mathfu::vec4_packed  sunAngle_and_sunColor;
    mathfu::vec4_packed  heightColor_and_endFogDistance;
    mathfu::vec4_packed  sunPercentage;
    mathfu::vec4_packed  sunDirection_and_fogZScalar;
    mathfu::vec4_packed  heightFogCoeff;
    mathfu::vec4_packed  mainFogCoeff;
    mathfu::vec4_packed  heightDensityFogCoeff;
    mathfu::vec4_packed  mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha;
    mathfu::vec4_packed  heightFogEndColor_fogStartOffset;
};


struct SceneExteriorLight {
    mathfu::vec4_packed uExteriorAmbientColor;
    mathfu::vec4_packed uExteriorHorizontAmbientColor;
    mathfu::vec4_packed uExteriorGroundAmbientColor;
    mathfu::vec4_packed uExteriorDirectColor;
    mathfu::vec4_packed uExteriorDirectColorDir;
    mathfu::vec4_packed uAdtSpecMult_FogCount;
};

const constexpr int FOG_MAX_SHADER_COUNT = 1;

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
        int unused0;
        int unused1;
    };

    struct meshWideBlockVSPS_Bindless {
        int instanceIndex;
        int meshIndex;
        int notUsed3;
        int notUsed4;
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
        struct WaterfallCommon {
            float bumpScale;
            int textureMatIndex1;
            int textureMatIndex2;
            int unused;
            mathfu::vec4_packed values0;
            mathfu::vec4_packed values1;
            mathfu::vec4_packed m_values2;
            mathfu::vec4_packed m_values3;
            mathfu::vec4_packed m_values4;
            mathfu::vec4_packed baseColor;
        };
        struct WaterfallBindless {
            int instanceIndex;
            int waterfallInd;

            int bumpTextureInd;
            int maskInd;
            int whiteWaterInd;
            int noiseInd;
            int normalTexInd;
            int unused;
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

    struct meshWideBlockBindless {
//    PACK({struct
        int placementMat;
        int meshWideIndex;
        int blockVSIndex;
        int texture9;
        int texture1;
        int texture2;
        int texture3;
        int texture4;
        int texture5;
        int texture6;
        int texture7;
        int texture8;

//    )}
    };
    struct perMeshData {
        int meshWideBindlessIndex;
        int wmoAmbientIndex;
        int unused0;
        int unused1;
    };
}
namespace ADT {
    struct meshWideBlockVSPS {
        mathfu::vec4 uPos;
        int useHeightMixFormula[4];
        float uHeightScale[4];
        float uHeightOffset[4];
    };
    struct meshWideBlockPS {
        float scaleFactorPerLayer[4];
        int animation_rotationPerLayer[4];
        int animation_speedPerLayer[4];
    };

    struct AdtInstanceData {
        int meshIndexVSPS;
        int meshIndexPS;
        int AlphaTextureInd;
        int unused;
        int LayerIndexes[4];
        int LayerHeight[4];
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

    struct WaterBindless {
        int waterDataInd;
        int placementMatInd;
        int textureInd;
        int unused;
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
