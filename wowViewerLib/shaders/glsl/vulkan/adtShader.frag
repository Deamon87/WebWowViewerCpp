#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"

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
    PSFog fogData;
};

layout(std140, set=0, binding=3) uniform modelWideBlockPS {
    ivec4 uUseHeightMixFormula;

};

layout(std140, set=0, binding=4) uniform meshWideBlockPS {
    vec4 uHeightScale;
    vec4 uHeightOffset;
    mat4 animationMat[4];
};

layout(location = 0) out vec4 outColor;

const InteriorLightParam intLight = {
    vec4(0,0,0,0),
    vec4(0,0,0,1)
};

vec4 mixTextures(vec4 tex0, vec4 tex1, float alpha) {
    return  (alpha*(tex1-tex0)+tex0);
}

void main() {
    vec2 vTexCoord = vChunkCoords;
    const float threshold = 1.5;

    vec2 alphaCoord = vec2(vChunkCoords.x/8.0, vChunkCoords.y/8.0 );
    vec3 alphaBlend = texture( uAlphaTexture, alphaCoord).gba;

    vec2 tcLayer0 = (animationMat[0]*vec4(vTexCoord, 0, 1)).xy;
    vec2 tcLayer1 = (animationMat[1]*vec4(vTexCoord, 0, 1)).xy;
    vec2 tcLayer2 = (animationMat[2]*vec4(vTexCoord, 0, 1)).xy;
    vec2 tcLayer3 = (animationMat[3]*vec4(vTexCoord, 0, 1)).xy;

    vec4 final;
    if (uUseHeightMixFormula.r > 0) {
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

        vec4 weightedLayer_2 = (texture(uLayer2, tcLayer2) * weightsNormalized.z);
        vec3 matDiffuse_2 = (matDiffuse_1 + weightedLayer_2.xyz);
        float specBlend_2 = (specBlend_1 + weightedLayer_2.w);

        vec4 weightedLayer_3 = (texture(uLayer3, tcLayer3) * weightsNormalized.w);
        vec3 matDiffuse_3 = (matDiffuse_2 + weightedLayer_3.xyz);
        float specBlend_3 = (specBlend_2 + weightedLayer_3.w);

        final = vec4(matDiffuse_3, specBlend_3);
    } else {
        vec4 tex1 = texture(uLayer0, tcLayer0).rgba;
        vec4 tex2 = texture(uLayer1, tcLayer1).rgba;
        vec4 tex3 = texture(uLayer2, tcLayer2).rgba;
        vec4 tex4 = texture(uLayer3, tcLayer3).rgba;

        final = mixTextures(mixTextures(mixTextures(tex1, tex2, alphaBlend.r), tex3, alphaBlend.g), tex4, alphaBlend.b);
    }

    vec3 matDiffuse = final.rgb * 2.0 * vColor.rgb;


    vec4 finalColor = vec4(
        calcLight(
            matDiffuse,
            vNormal,
            true,
            0.0,
            scene,
            intLight,
            vVertexLighting.rgb, /* accumLight */
            vec3(0.0), /*precomputedLight*/
            vec3(0.0) /* specular */
        ),
        1.0
    );

    //Spec part
    float specBlend = final.a;
    vec3 halfVec = -(normalize((scene.extLight.uExteriorDirectColorDir.xyz + normalize(vPosition))));
    vec3 lSpecular = ((scene.extLight.uExteriorDirectColor.xyz * pow(max(0.0, dot(halfVec, vNormal)), 20.0)));
    vec3 specTerm = (vec3(specBlend) * lSpecular) * scene.extLight.adtSpecMult.x;
    finalColor.rgb += specTerm;

    finalColor.rgb = makeFog(fogData, finalColor.rgb, vPosition.xyz, scene.extLight.uExteriorDirectColorDir.xyz, 0);

    finalColor.a = 1.0;
    outColor = finalColor;
}
