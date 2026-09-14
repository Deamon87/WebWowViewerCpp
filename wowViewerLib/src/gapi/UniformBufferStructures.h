//
// Created by deamon on 09.07.18.
//

#ifndef AWEBWOWVIEWERCPP_UNIFORMBUFFERSTRUCTURES_H
#define AWEBWOWVIEWERCPP_UNIFORMBUFFERSTRUCTURES_H

//Uniform buffer structures
#include <mathfu/glsl_mappings.h>
#include "mathfu/glsl_mappings.h"

#define MAX_WMO_GROUPS 512

#define MAX_MATRIX_NUM 256
#define MAX_M2COLORS_NUM 256
#define MAX_TEXTURE_WEIGHT_NUM 64
#define MAX_TEXTURE_MATRIX_NUM 64

#if defined(_MSC_VER) && !defined(__clang__)
#pragma message("Detected MSVC version")
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#else
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif


struct PSFog
{
    mathfu::vec4_packed  densityParams;
    mathfu::vec4_packed  classicFogParams;
    mathfu::vec4_packed  heightPlane;
    mathfu::vec4_packed  color_and_heightRate;
    mathfu::vec4_packed  heightDensity_and_endColor;
    mathfu::vec4_packed  sunAngle_and_sunColor;
    mathfu::vec4_packed  heightColor_and_endFogDistance;
    mathfu::vec4_packed  sunPercentage_sunFogStrength;
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
    mathfu::vec4_packed uExteriorSpecularColor;
    mathfu::vec4_packed uSunPosition;
    mathfu::vec4_packed uSunAttenuation;
    mathfu::vec4_packed uAdtSpecMult_FogCount;
};

const constexpr int FOG_MAX_SHADER_COUNT = 1;

struct sceneWideBlockVSPS {
    mathfu::mat4 uLookAtMat;
    mathfu::mat4 uPMatrix;
    mathfu::mat4 uInvLookAtMat;
    mathfu::mat4 uInvPMatrix;
    mathfu::vec4_packed uViewUpSceneTime;
    mathfu::vec4_packed uInteriorSunDir;
    mathfu::vec4_packed uSceneSize_DisableLightBuffer;

    mathfu::vec4_packed closeRiverColor;
    mathfu::vec4_packed farRiverColor;
    mathfu::vec4_packed closeOceanColor;
    mathfu::vec4_packed farOceanColor;

    SceneExteriorLight extLight;
    PSFog fogData;

    // Underwater fog (Light.db2 LightParams slot 1), used by liquid above shaders
    mathfu::vec4_packed underWaterFog;              // densityParams (start, end, density, bias)
    mathfu::vec4_packed underWaterClassicFogParams; // (enabled, end, endMinusStartInv, rate) — unused, zero
    mathfu::vec4_packed underWaterFogColor;         // rgb
};

struct InteriorLightParam {
    mathfu::vec4_packed uInteriorAmbientColorAndInteriorExteriorBlend;
    mathfu::vec4_packed uInteriorGroundAmbientColor;
    mathfu::vec4_packed uInteriorHorizontAmbientColor;
    mathfu::vec4_packed uInteriorDirectColor;
    mathfu::vec4_packed uPersonalInteriorSunDirAndApplyPersonalSunDir;
};

struct LocalLight
{
    mathfu::vec4_packed innerColor;
    mathfu::vec4_packed outerColor;
    mathfu::vec4_packed position;
    mathfu::vec4_packed attenuation;
    mathfu::vec4_packed blendParams;
};

struct SpotLight
{
    mathfu::mat4 lightModelMat;
    mathfu::vec4_packed innerColorAndBlendStart;
    mathfu::vec4_packed outerColorAndBlendEnd;
    mathfu::vec4_packed positionAndCosInnerAngle;
    mathfu::vec4_packed attenuationAndCosOuterAngle;
    mathfu::vec4_packed directionAndCosAngleDiff;
    mathfu::vec4_packed interiorAndSpotLightLenAndFallOff;
};

namespace M2 {
    struct PlacementMatrix {
        mathfu::mat4 uPlacementMat;
        mathfu::mat4 invPlacementMat;
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
        int txac1;
        int txac2;
    };
    static_assert(sizeof(meshWideBlockVSPS) == 16 * 4);

    struct ProjectiveData {
        mathfu::vec4 localMin;
        mathfu::vec4 localMax;
        mathfu::mat4 localToUVMat;
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
        int projectiveDataIndex;
        int notUsed4;
        int textureIndicies[4];
    };

    //M2 Pixel buffer formats
    struct modelWideBlockPS {
        InteriorLightParam intLight;
        float modelAlpha;
        float unused0;
        float unused1;
        float unused2;
        uint32_t objectId; // dense M2ObjId, used for GPU object-id picking
        uint32_t pad0;
        uint32_t pad1;
        uint32_t pad2;
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
        int txac1;
        int txac2;
        // GPU particle path (unused on the CPU path; written once when the emitter's
        // GPU sim data is created). Mirrors gpuBind0/1/2 in m2ParticleGpuShader.vert.slang.
        int gpuStateIndex = -1;        // GpuParticleState slot
        int gpuParticleOffset = 0;     // first GpuParticle in the particle pools
        int gpuParticleCapacity = 0;
        int gpuStaticsIndex = 0;       // GpuM2ParticleStatic slot
        int gpuPropsIndex = 0;         // GpuM2ParticleFrameProps slot
        int gpuColorReplOffset = -1;   // vec4 elements into the particle-color-replacement pool
        uint32_t gpuObjectId = 0;      // owning M2's dense M2ObjId
        int gpuQuadsPerParticle = 1;   // 2 when the emitter has both head and tail quads
        int gpuValuesVec4Offset = 0;   // owning object's chunk offsets in the track pools
        int gpuValuesFloatOffset = 0;
        int gpuPartTimesOffset = 0;
        int gpuPad = 0;
    };
    static_assert(sizeof(meshParticleWideBlockPS) == 80, "std140 tail extension");
}

namespace Ribbon {
    struct meshRibbonWideBlockPS {
        int uPixelShader;
        int uBlendMode;
        int uTextureTransformIndex;   // according to std140
        uint32_t objectId;            // owning M2's dense M2ObjId, used for GPU object-id picking
        // GPU ribbon path (unused on the CPU path): ribbonGpuBind in ribbonGpuShader.vert.slang
        int gpuStateIndex = -1;       // GpuRibbonState slot
        int gpuEdgesOffset = 0;       // first GpuRibbonEdge in the edges pool
        int gpuEdgeCount = 0;
        int gpuPropsIndex = 0;        // GpuM2RibbonFrameProps slot
    };
    static_assert(sizeof(meshRibbonWideBlockPS) == 32, "std140 tail extension");
}

namespace WMO {
    struct InteriorBlockData {
        mathfu::vec4_packed uAmbientColorAndIsExteriorLit;
        mathfu::vec4_packed uHorizontAmbientColor;
        mathfu::vec4_packed uGroundAmbientColor;
    };

    struct GroupInteriorData {
        InteriorBlockData interiorData[MAX_WMO_GROUPS];
    };

    //WMO VertexBuffer format
    struct modelWideBlockVS {
        mathfu::mat4 uPlacementMat;
    };

    struct meshWideBlockVS {
        int VertexShader;
        int UseLitColor;
        int padding[2];
        mathfu::vec4_packed translationSpeedXY;
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
        int interiorDataIndex;
        int canHaveExteriorLit;
        int unused1;
    };
}
namespace ADT {
    struct meshWideBlockVSPS {
        mathfu::vec4 uGlobalPosOffset;
        int globalChunkIndex[4];
        int useHeightMixFormula[4];
        float uHeightScale[8];
        float uHeightOffset[8];
    };
    struct meshWideBlockPS {
        float scaleFactorPerLayer[8];
        int animation_rotationPerLayer[8];
        int animation_speedPerLayer[8];
    };

    struct AdtInstanceData {
        int meshIndexVSPS;
        int meshIndexPS;
        int AlphaTextureInd;
        int AlphaTextureInd2;
        int LayerIndexes[8];
        int LayerHeight[8];
    };
}

namespace Liquid {
    PACK(
    struct LiquidInstance {
        //0
        int liquidMaterial;
        int liquidType;
        int isInterior;
        int vtxCellWidthHeight;

        float flowSpeed;
        float flowDirection;
        float unused4;
        float unused5;
    });
    static_assert(sizeof(LiquidInstance) == 32);

    // Which liquid shader path a liquid instance uses — must match LIQUID_MAT_* in
    // commonLiquidIndirectDescriptorSet.slang
    enum class LiquidMaterialType : int {
        Water = 0,
        Magma = 1,
        Mercury = 2,
        Fog = 3,
        LeyLine = 4,
        Fel = 5,
        Swamp = 6,
        Azerithe = 7,
    };

    struct LiquidBindless {
        int liquidDataInd;
        int placementMatInd;
        int liquidMaterialType;
        int unused;
        // Indices into the bindless liquid texture array (s_Textures), one per sampler slot
        // (slots 0-5 = animated frames, 6 = extra, 7 = extraTexture2, 8 = extraTexture3)
        int textureInd[9];
        int unused2[3]; // pad to 64 bytes to match std430 array stride
    };

    static_assert(sizeof(LiquidBindless) == 64);

    struct WaterDataVtx {
        mathfu::mat4            texMtx[4];

        mathfu::vec4_packed     waveParams;

    //	mathfu::vec4_packed     dynDisplMap0;
    //	mathfu::vec4_packed     dynDisplMap1;

        mathfu::vec4_packed     texTrans;
        mathfu::vec4_packed     texOffset;
    };
    struct WaterDataPS {

    };
    struct WaterData {
        mathfu::vec4_packed f0_f1_f2_f3;
        mathfu::vec4_packed f4_f5_f6_f7;
        mathfu::vec4_packed f8_f9_f10_f11;
        mathfu::vec4_packed f12_f13_f14_f15;
        mathfu::vec4_packed f16_f17_pad_pad;
        mathfu::vec4_packed depthTable;
        mathfu::vec4i_packed waterType_pad_pad_pad;
        mathfu::vec4_packed computedOffset0_computedOffsets1_pad_pad;
    };

    struct MagmaData {
        mathfu::vec4_packed f0_f1_f2_f3;
        mathfu::vec4_packed f4_f5_f6_f7;
        mathfu::vec4_packed f8_f9_f10_f11;
        mathfu::vec4_packed f12_f13_f14_f15;
        mathfu::vec4_packed f16_f17_pad_pad;
        mathfu::vec4_packed depthTable;
        mathfu::vec4_packed colorRGB_pad;
    };
    struct MercuryData {
        mathfu::vec4_packed f0_f1_f2_f3;
        mathfu::vec4_packed f4_f5_f6_f7;
        mathfu::vec4_packed f8_f9_f10_f11;
        mathfu::vec4_packed f12_f13_f14_f15;
        mathfu::vec4_packed f16_pad_pad_pad;
    };
    struct FogData {
        mathfu::vec4_packed f0_f1_f2_f3;
        mathfu::vec4_packed f4_f5_f6_f7;
        mathfu::vec4_packed f8_f9_f10_f11;
        mathfu::vec4_packed fogColor;
    };
    struct LeyLineData {
        mathfu::vec4_packed f0_f1_f2_f3;
        mathfu::vec4_packed f4_f5_f6_f7;
        mathfu::vec4_packed f8_f9_f10_f11;
        mathfu::vec4_packed f12_f13_f14_f15;
        mathfu::vec4_packed f16_f17_pad_pad;
        mathfu::vec4_packed color0;
        mathfu::vec4_packed color1;
        mathfu::vec4_packed depthTable;
    };
    struct FelData {
        mathfu::vec4_packed f0_f1_f2_f3;
        mathfu::vec4_packed f4_f5_f6_f7;
        mathfu::vec4_packed f8_f9_f10_f11;
        mathfu::vec4_packed f12_f13_f14_f15;
        mathfu::vec4_packed f16_f17_pad_pad;
        mathfu::vec4_packed intParams;
        mathfu::vec4_packed depthTable;
    };
    struct SwampData {
        mathfu::vec4_packed f0_f1_f2_f3;
        mathfu::vec4_packed f4_f5_f6_f7;
        mathfu::vec4_packed f8_f9_f10_f11;
        mathfu::vec4_packed f12_f13_f14_f15;
        mathfu::vec4_packed f16_f17_pad_pad;
        mathfu::vec4_packed depthTable;
        mathfu::vec4_packed colorRGB_pad;
    };
    struct AzeritheData {
        mathfu::vec4_packed f0_f1_f2_f3;
        mathfu::vec4_packed f4_f5_f6_f7;
        mathfu::vec4_packed f8_f9_f10_f11;
        mathfu::vec4_packed f12_f13_f14_f15;
        mathfu::vec4_packed f16_pad_pad_pad;
        mathfu::vec4_packed color0;
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
        mathfu::vec4_packed texOffsetX;
        mathfu::vec4_packed texOffsetY;
    };
}

namespace DnSky {
    struct meshWideBlockVS {
        mathfu::vec4_packed skyColor[6];
    };
}

namespace Planet {
    struct meshWideBlockVS {
        mathfu::vec4_packed uWorldPosAndScale;
        mathfu::vec4_packed uColorAndAlpha;
        mathfu::vec4_packed uCamPos;
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
