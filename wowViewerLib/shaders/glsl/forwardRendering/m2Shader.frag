#version 450

#extension GL_GOOGLE_include_directive: require

#ifndef MAX_MATRIX_NUM
#define MAX_MATRIX_NUM 220
#endif

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"
#include "../common/commonM2Material.glsl"

struct LocalLight
{
    vec4 color;
    vec4 position;
    vec4 attenuation;
};

layout(location=0) in vec2 vTexCoord;
layout(location=1) in vec2 vTexCoord2;
layout(location=2) in vec2 vTexCoord3;
layout(location=3) in vec3 vNormal;
layout(location=4) in vec4 vPosition_EdgeFade;

layout(location=0) out vec4 outputColor;

layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
    PSFog fogData;
};

layout(std140, set=0, binding=1) uniform placementMat {
    mat4 uPlacementMat;
};

//Whole model
layout(std140, set=0, binding=2) uniform modelWideBlockPS {
    InteriorLightParam intLight;
    LocalLight pc_lights[4];
    ivec4 lightCountAndBcHack;
    vec4 interiorExteriorBlend;
};

layout(std140, set=0, binding=4) uniform m2Colors {
    vec4 colors[256];
};

layout(std140, set=0, binding=5) uniform textureWeights {
    vec4 textureWeight[16];
};

layout(std140, set=0, binding=6) uniform textureMatrices {
    mat4 textureMatrix[64];
};

//Individual meshes
layout(std140, set=0, binding=7) uniform meshWideBlockVSPS {
    ivec4 vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2;
    ivec4 PixelShader_UnFogged_blendMode;
    ivec4 textureWeightIndexes;
    ivec4 colorIndex_applyWeight;
};

layout(set=1,binding=6) uniform sampler2D uTexture;
layout(set=1,binding=7) uniform sampler2D uTexture2;
layout(set=1,binding=8) uniform sampler2D uTexture3;
layout(set=1,binding=9) uniform sampler2D uTexture4;

void main() {
    /* Animation support */
    vec2 texCoord = vTexCoord.xy;
    vec2 texCoord2 = vTexCoord2.xy;
    vec2 texCoord3 = vTexCoord3.xy;

    vec4 finalColor = vec4(0);

    vec3 uTexSampleAlpha = vec3(
        textureWeightIndexes.x < 0 ? 1.0 : textureWeight[textureWeightIndexes.x / 4][textureWeightIndexes.x % 4],
        textureWeightIndexes.y < 0 ? 1.0 : textureWeight[textureWeightIndexes.y / 4][textureWeightIndexes.y % 4],
        textureWeightIndexes.z < 0 ? 1.0 : textureWeight[textureWeightIndexes.z / 4][textureWeightIndexes.z % 4]
    );

    vec4 vMeshColorAlpha = vec4(
        colorIndex_applyWeight.x < 0 ? vec4(1.0,1.0,1.0,1.0) : colors[colorIndex_applyWeight.x]
    );
    if (colorIndex_applyWeight.y > 0)
        vMeshColorAlpha.a *=
        textureWeightIndexes.x < 0 ? 1.0 : textureWeight[textureWeightIndexes.x / 4][textureWeightIndexes.x % 4];


    //Accumulate and apply lighting

    vec3 meshResColor = vMeshColorAlpha.rgb;

    vec3 accumLight = vec3(0.0);
    if ((vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.y == 1)) {
        vec3 vPos3 = vPosition_EdgeFade.xyz;
        vec3 vNormal3 = normalize(vNormal.xyz);
        vec3 lightColor = vec3(0.0);
        int count = int(pc_lights[0].attenuation.w);

        for (int index = 0; index < 4; index++)
        {
            if (index >= lightCountAndBcHack.x) break;

            LocalLight lightRecord = pc_lights[index];
            vec3 vectorToLight = ((scene.uLookAtMat * (uPlacementMat * lightRecord.position)).xyz - vPos3);
            float distanceToLightSqr = dot(vectorToLight, vectorToLight);
            float distanceToLightInv = inversesqrt(distanceToLightSqr);
            float distanceToLight = (distanceToLightSqr * distanceToLightInv);
            float diffuseTerm1 = max((dot(vectorToLight, vNormal3) * distanceToLightInv), 0.0);
            vec4 attenuationRec = lightRecord.attenuation;

            float attenuation = (1.0 - clamp((distanceToLight - attenuationRec.x) * (1.0 / (attenuationRec.z - attenuationRec.x)), 0.0, 1.0));

            vec3 attenuatedColor = attenuation * lightRecord.color.xyz;
            lightColor = (lightColor + vec3(attenuatedColor * attenuatedColor * diffuseTerm1 ));
        }

        meshResColor.rgb = clamp(lightColor , 0.0, 1.0);
        accumLight = mix(lightColor.rgb, meshResColor.rgb, lightCountAndBcHack.y);
        //finalColor.rgb =  finalColor.rgb * lightColor;
    }

//----------------------
// Calc Diffuse and Specular
//---------------------
    int uVertexShader = vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.x;

    mat4 textMat[2];
    int textMatIndex1 = vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.z;
    int textMatIndex2 = vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.w;

    textMat[0] = textMatIndex1 < 0 ? mat4(1.0) : textureMatrix[textMatIndex1];
    textMat[1] = textMatIndex2 < 0 ? mat4(1.0) : textureMatrix[textMatIndex2];
    float edgeFade = 1.0;

    calcM2VertexMat(uVertexShader,
        vPosition_EdgeFade.xyz, vNormal,
        vTexCoord, vTexCoord2,
        textMat, edgeFade,
        texCoord, texCoord2, texCoord3);

    vMeshColorAlpha *= edgeFade;

    float finalOpacity = 0.0;
    vec3 matDiffuse;
    vec3 specular;

    int uPixelShader = PixelShader_UnFogged_blendMode.x;
    int blendMode = PixelShader_UnFogged_blendMode.z;

    bool doDiscard = false;

    calcM2FragMaterial(uPixelShader,
        uTexture, uTexture2, uTexture3, uTexture4,
        texCoord, texCoord2, texCoord3,
        vMeshColorAlpha.rgb, vMeshColorAlpha.a,
        uTexSampleAlpha.rgb,
        blendMode,
        matDiffuse, specular, finalOpacity, doDiscard
    );

    if (doDiscard)
        discard;

// ------------------------------
// Apply lighting
// ------------------------------

//    specular *= vMeshColorAlpha.rgb;

    finalColor = vec4(
        calcLight(
            matDiffuse,
            vNormal,
            vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.y > 0,
            interiorExteriorBlend.x,
            scene,
            intLight,
            accumLight,
            vec3(0.0),
            specular,
            vec3(0.0)
        ) ,
        finalOpacity
    );

// ------------------------------
// Apply Fog
// ------------------------------

    int uUnFogged = PixelShader_UnFogged_blendMode.y;
    if (uUnFogged == 0) {
        vec3 sunDir =
            mix(
                scene.uInteriorSunDir,
                scene.extLight.uExteriorDirectColorDir,
                interiorExteriorBlend.x
            )
            .xyz;

        finalColor = makeFog(fogData, finalColor, vPosition_EdgeFade.xyz, sunDir.xyz, PixelShader_UnFogged_blendMode.z);
    }

    //Forward rendering without lights
    outputColor = finalColor;
}
