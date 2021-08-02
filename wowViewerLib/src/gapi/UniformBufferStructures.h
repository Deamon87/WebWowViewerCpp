//
// Created by deamon on 09.07.18.
//

#ifndef AWEBWOWVIEWERCPP_UNIFORMBUFFERSTRUCTURES_H
#define AWEBWOWVIEWERCPP_UNIFORMBUFFERSTRUCTURES_H

//Uniform buffer structures
#include <mathfu/glsl_mappings.h>
#include "mathfu/glsl_mappings.h"

#define MAX_MATRIX_NUM 220

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
    mathfu::vec4_packed uViewUp;
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
    struct modelWideBlockVS {
        mathfu::mat4 uPlacementMat;
        mathfu::mat4 uBoneMatrixes[MAX_MATRIX_NUM];
    };

    struct meshWideBlockVS {
        int VertexShader;
        int IsAffectedByLight;
        int isSkyBox;
        int padding;
        mathfu::vec4_packed Color_Transparency;

        mathfu::mat4 uTextMat[2];
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

    struct  meshWideBlockPS {
        int PixelShader;
        int UnFogged;
        int IsAffectedByLight;
        int BlendMode;

        mathfu::vec4_packed uFogColorAndAlphaTest;
        mathfu::vec4_packed uTexSampleAlpha;

        mathfu::vec4_packed uPcColor;
    };

    namespace WaterfallData {
        struct meshWideBlockVS {
            mathfu::vec4_packed bumpScale;
            mathfu::mat4 uTextMat[2];
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
        float textureScale0;
        float textureScale1;
        float textureScale2;
        int uPixelShader;
        int uBlendMode;
        int padding2;   // according to std140
        int padding3;   // according to std140
        float textureTranslate0;
        float textureTranslate1;
        float textureTranslate2;
        float padding4; // according to std140

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

    struct modelWideBlockPS {
        InteriorLightParam intLight;
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
    struct meshWideBlockVS {
        mathfu::vec4 uPos;
    };

    struct modelWideBlockPS {
        int useHeightMixFormula[4];
    };

    struct meshWideBlockPS {
        float uHeightScale[4];
        float uHeightOffset[4];
        mathfu::mat4 animationMat[4];
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

struct bbModelWideBlockVS {
    mathfu::mat4 uPlacementMat;

    mathfu::vec4_packed uBBScale;
    mathfu::vec4_packed uBBCenter;
    mathfu::vec4_packed uColor;
};


#endif //AWEBWOWVIEWERCPP_UNIFORMBUFFERSTRUCTURES_H
