#version 450

#extension GL_GOOGLE_include_directive: require
#extension GL_EXT_nonuniform_qualifier : require

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"
#include "../common/commonM2Material.glsl"

layout(location=0) in vec2 vTexCoord;
layout(location=1) in vec2 vTexCoord2;
layout(location=2) in vec2 vTexCoord3;
layout(location=3) in vec3 vNormal;
layout(location=4) in vec4 vPosition_EdgeFade;
layout(location=5) in flat int meshIndex;

layout(location=0) out vec4 outputColor;

layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
    PSFog fogData;
};

//Whole model
#include "../common/commonM2IndirectDescriptorSet.glsl"

//Individual meshes
struct meshWideBlockVSPSBindless {
    ivec4 vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2;
    ivec4 PixelShader_UnFogged_blendMode;
    ivec4 textureWeightIndexes;
    ivec4 colorIndex_applyWeight_instanceIndex;
    ivec4 textureIndicies;
};

layout(std430, set=2, binding=8) buffer meshWideBlockVSPS {
    meshWideBlockVSPSBindless meshWides[];
};


layout (set = 3, binding = 0) uniform sampler2D s_Textures[];

void main() {
    /* Animation support */
    vec2 texCoord = vTexCoord.xy;
    vec2 texCoord2 = vTexCoord2.xy;
    vec2 texCoord3 = vTexCoord3.xy;

    vec4 finalColor = vec4(0);

    meshWideBlockVSPSBindless meshWide = meshWides[meshIndex];
    int instanceIndex = meshWide.colorIndex_applyWeight_instanceIndex.y;

    vec3 uTexSampleAlpha = vec3(
        meshWide.textureWeightIndexes.x < 0 ? 1.0 : textureWeight[meshWide.textureWeightIndexes.x / 4][meshWide.textureWeightIndexes.x % 4],
        meshWide.textureWeightIndexes.y < 0 ? 1.0 : textureWeight[meshWide.textureWeightIndexes.y / 4][meshWide.textureWeightIndexes.y % 4],
        meshWide.textureWeightIndexes.z < 0 ? 1.0 : textureWeight[meshWide.textureWeightIndexes.z / 4][meshWide.textureWeightIndexes.z % 4]
    );

    vec4 vMeshColorAlpha = vec4(
        meshWide.colorIndex_applyWeight_instanceIndex.x < 0 ?
            vec4(1.0,1.0,1.0,1.0) :
            colors[meshWide.colorIndex_applyWeight_instanceIndex.x]
    );
    if (meshWide.colorIndex_applyWeight_instanceIndex.y > 0)
        vMeshColorAlpha.a *=
            meshWide.textureWeightIndexes.x < 0 ?
                1.0 :
                textureWeight[meshWide.textureWeightIndexes.x / 4][meshWide.textureWeightIndexes.x % 4];

    vec3 l_Normal = vNormal;

    //Accumulate and apply lighting

    vec3 meshResColor = vMeshColorAlpha.rgb;

    vec3 accumLight = vec3(0.0);
    modelWideBlockPSStruct modelWide = modelWides[instanceIndex];
    if ((meshWide.vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.y == 1)) {
        mat4 placementMat =
            uPlacementMats[instances[instanceIndex].placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd.x];

        vec3 vPos3 = vPosition_EdgeFade.xyz;
        vec3 vNormal3 = normalize(l_Normal.xyz);
        vec3 lightColor = vec3(0.0);
        int count = int(modelWide.pc_lights[0].attenuation.w);

        for (int index = 0; index < 4; index++)
        {
            if (index >= modelWide.lightCountAndBcHack.x) break;

            LocalLight lightRecord = modelWide.pc_lights[index];
            vec3 vectorToLight = ((scene.uLookAtMat * (placementMat * lightRecord.position)).xyz - vPos3);
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
        accumLight = mix(lightColor.rgb, meshResColor.rgb, modelWide.lightCountAndBcHack.y);
        //finalColor.rgb =  finalColor.rgb * lightColor;
    }

//----------------------
// Calc Diffuse and Specular
//---------------------
    int uVertexShader = meshWide.vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.x;

    mat4 textMat[2];
    int textMatIndex1 = meshWide.vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.z;
    int textMatIndex2 = meshWide.vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.w;

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

    int uPixelShader = meshWide.PixelShader_UnFogged_blendMode.x;
    int blendMode = meshWide.PixelShader_UnFogged_blendMode.z;

    bool doDiscard = false;


    calcM2FragMaterial(uPixelShader,
        s_Textures[meshWide.textureIndicies.x], s_Textures[meshWide.textureIndicies.y],
        s_Textures[meshWide.textureIndicies.z], s_Textures[meshWide.textureIndicies.w],
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
            meshWide.vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2.y > 0,
            modelWide.interiorExteriorBlend.x,
            scene,
            modelWide.intLight,
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

    int uUnFogged = meshWide.PixelShader_UnFogged_blendMode.y;
    if (uUnFogged == 0) {
        vec3 sunDir =
            mix(
                scene.uInteriorSunDir,
                scene.extLight.uExteriorDirectColorDir,
                modelWide.interiorExteriorBlend.x
            )
            .xyz;

        finalColor = makeFog(fogData, finalColor, vPosition_EdgeFade.xyz, sunDir.xyz, meshWide.PixelShader_UnFogged_blendMode.z);
    }

    //Forward rendering without lights
    outputColor = finalColor;
}
