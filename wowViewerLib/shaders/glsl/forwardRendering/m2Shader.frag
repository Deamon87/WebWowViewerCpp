#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"
#include "../common/commonM2Material.glsl"
#include "../common/commonUboSceneData.glsl"

layout(location=0) in vec2 vTexCoord;
layout(location=1) in vec2 vTexCoord2;
layout(location=2) in vec3 vNormal;
layout(location=3) in vec4 vPosition_EdgeFade;

layout(location=0) out vec4 outputColor;



//Whole model
#include "../common/commonM2DescriptorSet.glsl"

//Individual meshes
layout(std140, set=2, binding=0) uniform meshWideBlockVSPS {
    ivec4 vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2;
    ivec4 PixelShader_UnFogged_blendMode;
    ivec4 textureWeightIndexes;
    ivec4 colorIndex_applyWeight;
};

layout(set=3,binding=0) uniform sampler2D uTexture;
layout(set=3,binding=1) uniform sampler2D uTexture2;
layout(set=3,binding=2) uniform sampler2D uTexture3;
layout(set=3,binding=3) uniform sampler2D uTexture4;

void main() {
    /* Animation support */
    vec2 texCoord = vTexCoord.xy;
    vec2 texCoord2 = vTexCoord2.xy;
    vec2 texCoord3 = vTexCoord2.xy;

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

    vec3 l_Normal = vNormal;

    //Accumulate and apply lighting

    vec3 meshResColor = vMeshColorAlpha.rgb;

    vec3 accumLight = vec3(0.0);
    if ((vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.y == 1)) {
        vec3 vPos3 = vPosition_EdgeFade.xyz;
        vec3 vNormal3 = normalize(l_Normal.xyz);
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
        vPosition_EdgeFade.xyz, l_Normal,
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
            l_Normal,
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

        finalColor = makeFog2(fogData/*, int(scene.extLight.adtSpecMult_fogCount.y)*/, finalColor, scene.uViewUpSceneTime.xyz, vPosition_EdgeFade.xyz, sunDir.xyz, PixelShader_UnFogged_blendMode.z);
    }

    //Forward rendering without lights
    outputColor = finalColor;
}
