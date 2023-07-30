#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"

layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
	SceneWideParams scene;
	PSFog fogData;
};

layout(location = 0) in vec4 vColor;
layout(location = 0) out vec4 outputColor;

void main() {
//	if (vColor.a < 1.0)
//		discard;

	outputColor = vec4(vColor.xyz, vColor.a);
}
