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

layout(set=1,binding=5) uniform sampler2D uTexture;

layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
    PSFog fogData;
};

//Individual meshes
layout(std140, binding=4) uniform meshWideBlockPS {
    //ivec4 waterTypeV;
    vec4 color;
};

const InteriorLightParam intLight = {
    vec4(0,0,0,0),
    vec4(0,0,0,1)
};

void main() {
    vec3 matDiffuse = color.rgb+texture(uTexture, vTextCoords).rgb;

    vec3 sunDir = scene.extLight.uExteriorDirectColorDir.xyz;

    vec4 finalColor = vec4(
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


    //BlendMode is always GxBlend_Alpha
    finalColor.rgb = makeFog(fogData, finalColor, vPosition.xyz, sunDir.xyz, 2).rgb;

    outputColor = vec4(finalColor.rgb, 0.7);
}
