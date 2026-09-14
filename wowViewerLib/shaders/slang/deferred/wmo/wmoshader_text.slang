#define FRAGMENT_SHADER 1

precision highp float;
precision highp int;

#include "../../common/commonLightFunctions.slang"
#include "../../common/commonFogFunctions.slang"
#include "../../common/commonFunctions.slang"
#include "../../common/commonWMOMaterial.slang"
#include "../../common/commonUboSceneData.slang"
#include "../../common/commonLiquidDepthTest.slang"

layout (location = 0) in vec2 vTexCoord;
layout (location = 1) in vec2 vTexCoord2;
layout (location = 2) in vec2 vTexCoord3;
layout (location = 3) in vec2 vTexCoord4;
layout (location = 4) in vec4 vColor;
layout (location = 5) in vec4 vColor2;
layout (location = 6) in vec4 vColorSecond;
layout (location = 7) in vec4 vPosition;
layout (location = 8) in vec3 vNormal;
layout (location = 9) in flat int wmoGroupNum;
layout (location = 10) in flat int wmoObjId;

#include "../../common/commonWMODescriptorSet.slang"


#ifndef DEFERRED
layout(location = 0) out vec4 outColor;
#include "../../common/commonObjectId.slang"
#else
#include "../deferred_excerpt.slang"
#endif

#ifdef TRUE_OPAQUE
[earlydepthstencil]
#endif
[shader("pixel")]
void main() {
     int uPixelShader = wmoMeshWide.UseLitColor_EnableAlpha_PixelShader_BlendMode.z;

    vec3 matDiffuse = vec3(0.0);
    vec3 spec = vec3(0.0);
    vec3 emissive = vec3(0.0);
    float finalOpacity = 0.0;
    bool doDiscard;

    caclWMOFragMat(uPixelShader, wmoMeshWide.UseLitColor_EnableAlpha_PixelShader_BlendMode.y == 1,
        uTexture, uTexture2, uTexture3, uTexture4,
        uTexture5, uTexture6, uTexture7, uTexture8, uTexture9,
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

    WmoInteriorBlockData currentInteriorData = interiorData[wmoGroupNum];

        //vColor.w = [0, 1]. 0 = full interior, 1.0 = full exterior
    //currentInteriorData.uAmbientColorAndIsExteriorLit.w 1.0 = use exterior lighting no matter what, 0.0 otherwise

    float interiorExteriorBlend = mix(vColor.w, 1.0, currentInteriorData.uAmbientColorAndIsExteriorLit.w);

    InteriorLightParam intLight;
    intLight.uInteriorAmbientColorAndInteriorExteriorBlend = vec4(currentInteriorData.uAmbientColorAndIsExteriorLit.xyz, interiorExteriorBlend);
    intLight.uInteriorGroundAmbientColor =                   vec4(currentInteriorData.uGroundAmbientColor.xyz, 0);
    intLight.uInteriorHorizontAmbientColor =                 vec4(currentInteriorData.uHorizontAmbientColor.xyz, 0);

    intLight.uInteriorDirectColor = vec4(0, 0, 0, 1.0f);
    intLight.uPersonalInteriorSunDirAndApplyPersonalSunDir = vec4(0, 0, 0, 0.0f);

    vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
#ifndef DEFERRED
    vec3 accumLight = vec3(0.0);
    if (scene.uSceneSize_DisableLightBuffer.z == 0.0 && wmoMeshWide.UseLitColor_EnableAlpha_PixelShader_BlendMode.w <= 1) {
        accumLight = texture(lightBuffer, (gl_FragCoord.xy / scene.uSceneSize_DisableLightBuffer.xy)).xyz;
    }

//    matDiffuse.rbg = vec3(0, 0, 1);

    finalColor = vec4(
        calcLight(
            matDiffuse,
            vNormal,
            vPosition.xyz,
            wmoMeshWide.UseLitColor_EnableAlpha_PixelShader_BlendMode.x == 1,
            scene,
            intLight,
            accumLight /*accumLight*/,
            vColor.rgb * 2.0,
            spec, /* specular */
            emissive,
            1.0 /* ao */
        ),
        finalOpacity
    );
#endif

#ifndef DEFERRED
    finalColor = makeFog2(fogData,
                          finalColor,
                          scene.uViewUpSceneTime.xyz,
                          vPosition.xyz,
                          scene.extLight.uExteriorDirectColorDir.xyz,
                          wmoMeshWide.UseLitColor_EnableAlpha_PixelShader_BlendMode.w);
#endif

finalColor.a = 1.0;

#ifndef DEFERRED
    liquidDepthClip();
    outColor = finalColor;
    writeObjectId(0xC0000000u | ((uint(wmoObjId) & 0xFFFFFu) << 10) | (uint(wmoGroupNum) & 0x3FFu));
#else
    writeGBuffer(matDiffuse.xyz, normalize(vNormal), spec.rgb, vPosition.xyz);
#endif
}