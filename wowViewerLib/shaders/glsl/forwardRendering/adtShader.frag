#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"
#include "../common/commonADTMaterial.glsl"

layout(location = 0) in vec2 vChunkCoords;
layout(location = 1) in vec3 vPosition;
layout(location = 2) in vec4 vColor;
layout(location = 3) in vec3 vNormal;
layout(location = 4) in vec3 vVertexLighting;
layout(location = 5) in vec2 vAlphaCoords;

layout(set=2, binding=0) uniform sampler2D uLayer0;
layout(set=2, binding=1) uniform sampler2D uLayer1;
layout(set=2, binding=2) uniform sampler2D uLayer2;
layout(set=2, binding=3) uniform sampler2D uLayer3;
layout(set=2, binding=4) uniform sampler2D uAlphaTexture;
layout(set=2, binding=5) uniform sampler2D uLayerHeight0;
layout(set=2, binding=6) uniform sampler2D uLayerHeight1;
layout(set=2, binding=7) uniform sampler2D uLayerHeight2;
layout(set=2, binding=8) uniform sampler2D uLayerHeight3;

#include "../common/commonUboSceneData.glsl"

layout(std140, set=1, binding=0) uniform meshWideBlockVSPS {
    vec4 uPos;
    ivec4 uUseHeightMixFormula_useWeightedBlend;
    vec4 uHeightScale;
    vec4 uHeightOffset;
};

layout(std140, set=1, binding=1) uniform meshWideBlockPS {
    vec4 scaleFactorPerLayer;
    ivec4 animation_rotationPerLayer;
    ivec4 animation_speedPerLayer;
};


layout(location = 0) out vec4 outColor;

const InteriorLightParam intLight = {
    vec4(0,0,0,0),
    vec4(0,0,0,1)
};

void main() {
    vec2 vTexCoord = vChunkCoords;
    const float threshold = 1.5;

    float sceneTime = scene.uViewUpSceneTime.w;
    vec2 tcLayer0 = transformADTUV(vTexCoord, 0, sceneTime, animation_rotationPerLayer, animation_speedPerLayer, scaleFactorPerLayer);
    vec2 tcLayer1 = transformADTUV(vTexCoord, 1, sceneTime, animation_rotationPerLayer, animation_speedPerLayer, scaleFactorPerLayer);
    vec2 tcLayer2 = transformADTUV(vTexCoord, 2, sceneTime, animation_rotationPerLayer, animation_speedPerLayer, scaleFactorPerLayer);
    vec2 tcLayer3 = transformADTUV(vTexCoord, 3, sceneTime, animation_rotationPerLayer, animation_speedPerLayer, scaleFactorPerLayer);

    vec4 final;
    if (uUseHeightMixFormula_useWeightedBlend.r > 0) {
        calcADTHeightFragMaterial(
            tcLayer0, tcLayer1, tcLayer2, tcLayer3,
            uLayer0, uLayer1, uLayer2, uLayer3,
            uLayerHeight0, uLayerHeight1, uLayerHeight2, uLayerHeight3,
            vAlphaCoords,
            uAlphaTexture,
            uHeightOffset, uHeightScale,
            final
        );
    } else {
        calcADTOrigFragMaterial(
            tcLayer0, tcLayer1, tcLayer2, tcLayer3,
            uLayer0, uLayer1, uLayer2, uLayer3,
            vAlphaCoords,
            uAlphaTexture,
            uUseHeightMixFormula_useWeightedBlend.g > 0,
            final
        );
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
            vec3(0.0), /* specular */
            vec3(0.0), /* emissive */
            1.0 /* ao */
        ),
        1.0
    );

    //Spec part
    float specBlend = final.a;
    vec3 halfVec = -(normalize((scene.extLight.uExteriorDirectColorDir.xyz + normalize(vPosition))));
    vec3 lSpecular = ((scene.extLight.uExteriorDirectColor.xyz * pow(max(0.0, dot(halfVec, normalize(vNormal))), 20.0)));
    vec3 specTerm = (vec3(specBlend) * lSpecular) * scene.extLight.adtSpecMult_fogCount.x;
    finalColor.rgb += specTerm;

    finalColor = makeFog2(fogData/*, int(scene.extLight.adtSpecMult_fogCount.y)*/, finalColor, scene.uViewUpSceneTime.xyz,
        vPosition.xyz, scene.extLight.uExteriorDirectColorDir.xyz, 0);

    finalColor.a = 1.0;
    outColor = finalColor;
}
