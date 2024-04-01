#define FRAGMENT_SHADER 1

precision highp float;
precision highp int;

#include "../../common/commonLightFunctions.glsl"
#include "../../common/commonFogFunctions.glsl"
#include "../../common/commonFunctions.glsl"
#include "../../common/commonWMOMaterial.glsl"
#include "../../common/commonUboSceneData.glsl"

#ifdef TRUE_OPAQUE
layout(early_fragment_tests) in;
#endif

layout (location = 0) in vec2 vTexCoord;
layout (location = 1) in vec2 vTexCoord2;
layout (location = 2) in vec2 vTexCoord3;
layout (location = 3) in vec2 vTexCoord4;
layout (location = 4) in vec4 vColor;
layout (location = 5) in vec4 vColor2;
layout (location = 6) in vec4 vColorSecond;
layout (location = 7) in vec4 vPosition;
layout (location = 8) in vec3 vNormal;
layout (location = 9) in flat int vMeshIndex;

#include "../../common/commonWMOIndirectDescriptorSet.glsl"

layout (set = 2, binding = 0) uniform sampler2D s_Textures[];

#ifndef DEFERRED
layout(location = 0) out vec4 outColor;
#else
#include "../deferred_excerpt.glsl"
#endif

void main() {
    WMOPerMeshData perMeshData = perMeshDatas[nonuniformEXT(vMeshIndex)];
    WmoMeshWideBindless meshWideBindless = wmoMeshWideBindlesses[nonuniformEXT(perMeshData.meshWideBindlessIndex_wmoAmbientIndex.x)];
    WmoMeshWide wmoMeshWide = wmoMeshWides[nonuniformEXT(meshWideBindless.placementMat_meshWideIndex_blockVSIndex_texture9.y)];

    int uPixelShader = wmoMeshWide.UseLitColor_EnableAlpha_PixelShader_BlendMode.z;

    vec3 matDiffuse = vec3(0.0);
    vec3 spec = vec3(0.0);
    vec3 emissive = vec3(0.0);
    float finalOpacity = 0.0;
    bool doDiscard;

    int text1 = meshWideBindless.text1_text2_text3_text4.x;
    int text2 = meshWideBindless.text1_text2_text3_text4.y;
    int text3 = meshWideBindless.text1_text2_text3_text4.z;
    int text4 = meshWideBindless.text1_text2_text3_text4.w;
    int text5 = meshWideBindless.text5_text6_text7_text8.x;
    int text6 = meshWideBindless.text5_text6_text7_text8.y;
    int text7 = meshWideBindless.text5_text6_text7_text8.z;
    int text8 = meshWideBindless.text5_text6_text7_text8.w;
    int text9 = meshWideBindless.placementMat_meshWideIndex_blockVSIndex_texture9.w;

    caclWMOFragMat(uPixelShader, wmoMeshWide.UseLitColor_EnableAlpha_PixelShader_BlendMode.y == 1,
                   s_Textures[nonuniformEXT(text1)], s_Textures[nonuniformEXT(text2)], s_Textures[nonuniformEXT(text3)], s_Textures[nonuniformEXT(text4)],
                   s_Textures[text5], s_Textures[text6], s_Textures[text7], s_Textures[text8],
                   s_Textures[text9],
                   vPosition.xyz, vNormal, vTexCoord, vTexCoord2, vTexCoord3, vTexCoord4,
                   vColor2,
                   vColorSecond,
                   matDiffuse, spec, emissive, finalOpacity,
                   doDiscard
    );
#ifndef TRUE_OPAQUE
    if (doDiscard)
        discard;
#endif

    InteriorLightParam intLight;
    intLight.uInteriorAmbientColorAndApplyInteriorLight = s_wmoAmbient[perMeshData.meshWideBindlessIndex_wmoAmbientIndex.y];
    intLight.uInteriorDirectColorAndApplyExteriorLight = vec4(0, 0, 0, 1.0f);

    vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
#ifndef DEFERRED
    finalColor = vec4(
        calcLight(
            matDiffuse,
            vNormal,
            wmoMeshWide.UseLitColor_EnableAlpha_PixelShader_BlendMode.x == 1,
            vColor.w,
            scene,
            intLight,
            vec3(0.0) /*accumLight*/,
            vColor.rgb,
            spec, /* specular */
            emissive
        ),
        finalOpacity
    );
#endif

#ifndef DEFERRED
    finalColor = makeFog2(fogData/*, int(scene.extLight.adtSpecMult_fogCount.y)*/, finalColor, scene.uViewUpSceneTime.xyz,
                          vPosition.xyz, scene.extLight.uExteriorDirectColorDir.xyz, wmoMeshWide.UseLitColor_EnableAlpha_PixelShader_BlendMode.w);
#endif

finalColor.a = 1.0;

#ifndef DEFERRED
    outColor = finalColor;
#else
    writeGBuffer(matDiffuse.xyz, normalize(vNormal), spec.rgb);
#endif
}