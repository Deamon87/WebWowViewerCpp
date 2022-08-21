#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"

precision highp float;

layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
    PSFog fogData;
};
layout(std140, set=0, binding=2) uniform meshWideBlockVS {
    vec4 skyColor[6];
};

layout(location = 0) in vec4 aPosition;

layout(location = 0) out vec4 vColor;



void main() {


    vec3 inputPos = aPosition.xyz;
//Correction of conus
    inputPos = inputPos * 33.333;

//    inputPos.z = -1.0-inputPos.z;
    vec4 cameraPos = scene.uLookAtMat * vec4(inputPos.xyz, 1.0);
    cameraPos.xyz = cameraPos.xyz - scene.uLookAtMat[3].xyz;
    cameraPos.z = cameraPos.z ;

    vec4 vertPosInNDC = scene.uPMatrix * cameraPos;


    vColor = skyColor[int(aPosition.w)].xyzw;
	gl_Position = scene.uPMatrix * cameraPos;
}
