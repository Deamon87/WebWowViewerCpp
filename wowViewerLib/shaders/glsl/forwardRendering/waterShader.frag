#version 450

#extension GL_GOOGLE_include_directive: require

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"

precision highp float;
precision highp int;

layout(location=0) in vec3 vPosition;
layout(location=1) in vec2 vTextCoords;
layout(location=2) in vec3 vNormal;

layout(location=0) out vec4 outputColor;

layout(set=2,binding=5) uniform sampler2D uTexture;

#include "../common/commonUboSceneData.glsl"

//Individual meshes
layout(std140, set=1, binding=4) uniform meshWideBlockPS {
    ivec4 materialId;
    vec4 color;
    mat4 textureMatrix;
};

const InteriorLightParam intLight = {
    vec4(0,0,0,0),
    vec4(0,0,0,1)
};

void main() {
// MaterialId:
//    1,3 - Water
//    2,4 - Magma
//    5 - Mercury,
//    10 - Fog
//    12 - LeyLine
//    13 - Fel
//    14 - Swamp
//    18 - Azerithe
//    8 - is probably debug material called Grid


    vec2 animatedUV = (textureMatrix*vec4(vTextCoords, 0.0, 1.0)).st;

    vec3 matDiffuse = color.rgb+texture(uTexture, animatedUV).rgb;

    vec3 sunDir = scene.extLight.uExteriorDirectColorDir.xyz;


    vec4 finalColor = vec4(matDiffuse, 1.0);

    //Magma is not affected by light
    if (materialId.x != 2 && materialId.x != 4) {
        finalColor = vec4(
            calcLight(
                matDiffuse,
                vNormal,
                true,
                0,
                scene,
                intLight,
                vec3(0.0) /*accumLight*/,
                vec3(0.0),
                vec3(0.0), /* specular */
                vec3(0.0)
            ),
            1.0
        );
    }

    //BlendMode is always GxBlend_Alpha
    finalColor.rgb = makeFog2(fogData/*, int(scene.extLight.adtSpecMult_fogCount.y)*/, finalColor, scene.uViewUpSceneTime.xyz, vPosition.xyz, sunDir.xyz, 2).rgb;

    outputColor = vec4(finalColor.rgb, 0.7);
}
