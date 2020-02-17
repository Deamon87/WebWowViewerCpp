#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;

#include "../common/commonLightFunctions.glsl"

layout(location = 0) in vec2 vChunkCoords;
layout(location = 1) in vec3 vPosition;
layout(location = 2) in vec4 vColor;
layout(location = 3) in vec3 vNormal;
layout(location = 4) in vec3 vVertexLighting;

layout(set=1, binding=5) uniform sampler2D uLayer0;
layout(set=1, binding=6) uniform sampler2D uLayer1;
layout(set=1, binding=7) uniform sampler2D uLayer2;
layout(set=1, binding=8) uniform sampler2D uLayer3;
layout(set=1, binding=9) uniform sampler2D uAlphaTexture;
layout(set=1, binding=10) uniform sampler2D uLayerHeight0;
layout(set=1, binding=11) uniform sampler2D uLayerHeight1;
layout(set=1, binding=12) uniform sampler2D uLayerHeight2;
layout(set=1, binding=13) uniform sampler2D uLayerHeight3;

layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
};

layout(std140, set=0, binding=3) uniform modelWideBlockPS {
    vec4 uViewUp;
    vec4 uSunDir_FogStart;
    vec4 uSunColor_uFogEnd;
    vec4 uAmbientLight;
    vec4 FogColor;
};

layout(std140, set=0, binding=4) uniform meshWideBlockPS {
    vec4 uHeightScale;
    vec4 uHeightOffset;
};

layout(location = 0) out vec4 outColor;

const InteriorLightParam intLight = {
    vec4(0,0,0,0),
    vec4(0,0,0,1),
    vec4(0,0,0,0)
};

void main() {
    vec2 vTexCoord = vChunkCoords;
    const float threshold = 1.5;

    vec2 alphaCoord = vec2(vChunkCoords.x/8.0, vChunkCoords.y/8.0 );
    vec3 alphaBlend = texture( uAlphaTexture, alphaCoord).gba;

    vec2 tcLayer0 = vTexCoord;
    vec2 tcLayer1 = vTexCoord;
    vec2 tcLayer2 = vTexCoord;
    vec2 tcLayer3 = vTexCoord;

    float minusAlphaBlendSum = (1.0 - clamp(dot(alphaBlend, vec3(1.0)), 0.0, 1.0));
    vec4 weightsVector = vec4(minusAlphaBlendSum, alphaBlend);
    float weightedTexture_x = (minusAlphaBlendSum * ((texture(uLayerHeight0, tcLayer0).w * uHeightScale[0]) + uHeightOffset[0]));
    float weightedTexture_y = (weightsVector.y * ((texture(uLayerHeight1, tcLayer1).w * uHeightScale[1]) + uHeightOffset[1]));
    float weightedTexture_z = (weightsVector.z * ((texture(uLayerHeight2, tcLayer2).w * uHeightScale[2]) + uHeightOffset[2]));
    float weightedTexture_w = (weightsVector.w * ((texture(uLayerHeight3, tcLayer3).w * uHeightScale[3]) + uHeightOffset[3]));
    vec4 weights = vec4(weightedTexture_x, weightedTexture_y, weightedTexture_z, weightedTexture_w);
    vec4 weights_temp = (weights * (vec4(1.0) - clamp((vec4(max(max(weightedTexture_x, weightedTexture_y), max(weightedTexture_z, weightedTexture_w))) - weights), 0.0, 1.0)));
    vec4 weightsNormalized = (weights_temp / vec4(dot(vec4(1.0), weights_temp)));
    
    
    vec4 weightedLayer_0 = (texture(uLayer0, tcLayer0) * weightsNormalized.x);
    vec3 matDiffuse_0 = weightedLayer_0.xyz;
    float specBlend_0 = weightedLayer_0.w;

    vec4 weightedLayer_1 = (texture(uLayer1, tcLayer1) * weightsNormalized.y);
    vec3 matDiffuse_1 = (matDiffuse_0 + weightedLayer_1.xyz);
    float specBlend_1 = (specBlend_0 + weightedLayer_1.w);

    vec4 weightedLayer_2 = (texture(uLayer2, tcLayer1) * weightsNormalized.z);
    vec3 matDiffuse_2 = (matDiffuse_1 + weightedLayer_2.xyz);
    float specBlend_2 = (specBlend_1 + weightedLayer_2.w);

    vec4 weightedLayer_3 = (texture(uLayer3, tcLayer1) * weightsNormalized.w);
    vec3 matDiffuse_3 = (matDiffuse_2 + weightedLayer_3.xyz);
    float specBlend_3 = (specBlend_2 + weightedLayer_3.w);

    vec4 final = vec4(matDiffuse_3, specBlend_3);

    vec3 matDiffuse = final.rgb * vColor.rgb;


    vec4 finalColor = vec4(
        calcLight(
            matDiffuse,
            vNormal,
            true,
            scene,
            intLight,
            vVertexLighting.rgb, /* accumLight */
            vec3(0.0) /*precomputedLight*/

        ),
        1.0
    );

    //Spec part
    float specBlend = final.a;
    vec3 halfVec = -(normalize((scene.extLight.uExteriorDirectColorDir.xyz + normalize(vPosition))));
    vec3 lSpecular = ((scene.extLight.uExteriorDirectColor.xyz * pow(max(0.0, dot(halfVec, vNormal)), 20.0)));
    vec3 specTerm = (vec3(specBlend) * lSpecular);
    finalColor.rgb += specTerm;

    // --- Fog start ---
    /*
    vec3 fogColor = uFogColor;
    float fog_start = uFogStart;
    float fog_end = uFogEnd;
    float fog_rate = 1.5;
    float fog_bias = 0.01;

    //vec4 fogHeightPlane = pc_fog.heightPlane;
    //float heightRate = pc_fog.color_and_heightRate.w;

    float distanceToCamera = length(vPosition.xyz);
    float z_depth = (distanceToCamera - fog_bias);
    float expFog = 1.0 / (exp((max(0.0, (z_depth - fog_start)) * fog_rate)));
    //float height = (dot(fogHeightPlane.xyz, vPosition.xyz) + fogHeightPlane.w);
    //float heightFog = clamp((height * heightRate), 0, 1);
    float heightFog = 1.0;
    expFog = (expFog + heightFog);
    float endFadeFog = clamp(((fog_end - distanceToCamera) / (0.699999988 * fog_end)), 0.0, 1.0);

    finalColor.rgb = mix(fogColor.rgb, finalColor.rgb, vec3(min(expFog, endFadeFog)));
    */
    // --- Fog end ---

    finalColor.a = 1.0;
    outColor = finalColor;
}
