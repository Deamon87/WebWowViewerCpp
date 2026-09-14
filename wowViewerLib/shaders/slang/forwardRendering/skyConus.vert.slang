#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.slang"
#include "../common/commonFogFunctions.slang"
#include "../common/commonUboSceneData.slang"

precision highp float;


layout(std140, set=1, binding=0) uniform meshWideBlockVS {
    vec4 skyColor[6];
};

layout(location = 0) in vec4 aPosition;

layout(location = 0) out vec4 vColor;
layout(location = 1) out vec4 vPosition;


[shader("vertex")]
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
    vPosition = vec4(cameraPos.xyz, inputPos.z);
	gl_Position = scene.uPMatrix * cameraPos;
}
