import glsl;

precision highp float;
precision highp int;

#include "../../common/commonLightFunctions.slang"
#include "../../common/commonFogFunctions.slang"
#include "../../common/commonM2Material.slang"
#include "../../common/commonLiquidDepthTest.slang"

layout(location=0) in vec2 vTexCoord;
layout(location=1) in vec2 vTexCoord2;
layout(location=2) in vec3 vNormal;
layout(location=3) in vec4 vPosition_EdgeFade;
layout(location=4) in flat int vMeshIndex;

#include "../../common/commonUboSceneData.slang"

#ifndef DEFERRED
layout(location = 0) out vec4 outColor;
#include "../../common/commonObjectId.slang"
#else
#include "../deferred_excerpt.slang"
#endif

#include "../../common/commonUboSceneData.slang"
//Whole model
#include "../../common/commonM2DescriptorSet.slang"

#ifdef TRUE_OPAQUE
[earlydepthstencil]
#endif
[shader("pixel")]
void main() {
    /* Animation support */
    vec2 texCoord = vTexCoord.xy;
    vec2 texCoord2 = vTexCoord2.xy;
    vec2 texCoord3 = vTexCoord2.xy;

    vec4 finalColor = vec4(0);

    bool calcMaterial = true;
#ifdef DEFERRED
//    {
//        int blendMode = meshWide.PixelShader_UnFogged_blendMode.z;
//        calcMaterial = (blendMode == 1);
//    }
#endif

    vec3 uTexSampleAlpha = vec3(
        meshWide.textureWeightIndexes.x < 0 ? 1.0 : textureWeight[meshWide.textureWeightIndexes.x / 4][meshWide.textureWeightIndexes.x % 4],
        meshWide.textureWeightIndexes.y < 0 ? 1.0 : textureWeight[meshWide.textureWeightIndexes.y / 4][meshWide.textureWeightIndexes.y % 4],
        meshWide.textureWeightIndexes.z < 0 ? 1.0 : textureWeight[meshWide.textureWeightIndexes.z / 4][meshWide.textureWeightIndexes.z % 4]
    );

    vec4 vMeshColorAlpha = vec4(
        meshWide.colorIndex_applyWeight_txac1_txac2.x < 0 ? vec4(1.0,1.0,1.0,1.0) : colors[meshWide.colorIndex_applyWeight_txac1_txac2.x]
    );
    if (meshWide.colorIndex_applyWeight_txac1_txac2.y > 0)
        vMeshColorAlpha.a *=
            meshWide.textureWeightIndexes.x < 0 ?
            1.0 :
            textureWeight[meshWide.textureWeightIndexes.x / 4][meshWide.textureWeightIndexes.x % 4];

    vMeshColorAlpha.a *= modelWide.modelAlpha.x;

    vec3 l_Normal = vNormal;

    //----------------------
    // Calc Diffuse and Specular
    //---------------------
    int uVertexShader = meshWide.vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.x;

    mat4 textMat[2];
    int textMatIndex1 = meshWide.vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.z;
    int textMatIndex2 = meshWide.vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.w;

    textMat[0] = textMatIndex1 < 0 ? ident4() : textureMatrix[textMatIndex1];
    textMat[1] = textMatIndex2 < 0 ? ident4() : textureMatrix[textMatIndex2];
    float edgeFade = 1.0;

    calcM2VertexMat(uVertexShader,
        vPosition_EdgeFade.xyz, l_Normal,
        vTexCoord, vTexCoord2,
        textMat, edgeFade,
        texCoord, texCoord2, texCoord3);

    vMeshColorAlpha *= edgeFade;

    float finalOpacity = 0.0;
    vec3 matDiffuse;
    vec3 specular;

    int uPixelShader = meshWide.PixelShader_UnFogged_blendMode.x;
    int blendMode = meshWide.PixelShader_UnFogged_blendMode.z;

    bool doDiscard = false;

    if (calcMaterial) {
        calcM2FragMaterial(uPixelShader,
            uTexture, uTexture2, uTexture3, uTexture4,
            texCoord, texCoord2, texCoord3,
            vMeshColorAlpha.rgb, vMeshColorAlpha.a,
            uTexSampleAlpha.rgb,
            blendMode,
            matDiffuse, specular, finalOpacity, doDiscard
        );

#ifndef TRUE_OPAQUE
        if (doDiscard)
            discard;
#endif

        // ------------------------------
        // Apply lighting
        // ------------------------------


        specular *= vMeshColorAlpha.rgb;

#ifndef DEFERRED
        vec3 accumLight = vec3(0.0);
        if (scene.uSceneSize_DisableLightBuffer.z == 0.0 && blendMode <= 1) {
            accumLight = texture(lightBuffer, (gl_FragCoord.xy / scene.uSceneSize_DisableLightBuffer.xy)).xyz;
        }

//    matDiffuse.rbg = vec3(0, 1, 0);


        finalColor = vec4(
            calcLight(
                matDiffuse,
                l_Normal,
                vPosition_EdgeFade.xyz,
                meshWide.vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.y > 0,
                scene,
                modelWide.intLight,
                accumLight,
                vec3(0.0),
                specular,
                vec3(0.0),
                1.0 /* ao */
            ) ,
            finalOpacity
        );
#endif
    }

    // ------------------------------
    // Apply Fog
    // ------------------------------

#ifndef DEFERRED
    int uUnFogged = meshWide.PixelShader_UnFogged_blendMode.y;
    if (uUnFogged == 0) {
        float interiorExteriorBlend = modelWide.intLight.uInteriorAmbientColorAndInteriorExteriorBlend.w;
        vec3 sunDir =
            mix(
                scene.uInteriorSunDir,
                scene.extLight.uExteriorDirectColorDir,
                interiorExteriorBlend
            ).xyz;

        finalColor = makeFog2(fogData,
            finalColor,
            scene.uViewUpSceneTime.xyz,
            vPosition_EdgeFade.xyz,
            sunDir.xyz,
            meshWide.PixelShader_UnFogged_blendMode.z
        );
    }
#endif


    //Forward rendering without lights
#ifndef DEFERRED
    liquidDepthClip();
    outColor = finalColor;
    writeObjectId(0x80000000u | modelWide.objectId_pad.x);
#else
    writeGBuffer(matDiffuse.xyz, normalize(vNormal), specular.rgb, vPosition_EdgeFade.xyz);
#endif
}