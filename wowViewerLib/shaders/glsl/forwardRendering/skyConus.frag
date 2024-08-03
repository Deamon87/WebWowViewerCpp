#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"
#include "../common/commonUboSceneData.glsl"

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec4 vPosition;


layout(location = 0) out vec4 outputColor;

void main() {

	vec3 sunDir = normalize(fogData.sunDirection_and_fogZScalar.xyz);
	vec3 normal = normalize(vPosition.xyz);

 	float nDotSun = dot(normal, sunDir);

	vec3 sunFogColor = fogData.sunAngle_and_sunColor.yzw;
	float sunAngle = fogData.sunAngle_and_sunColor.x;
	float scalar = fogData.sunPercentage_sunFogStrength.y;

    nDotSun = clamp(nDotSun - sunAngle, 0, 1);

    vec3 resultColor = vColor.xyz;

	 if ( nDotSun > 0.0 ) {
         nDotSun = (1.0 / (1.0 - sunAngle) * nDotSun);
         nDotSun = ((nDotSun * nDotSun) * nDotSun);
	     resultColor = mix(vColor.xyz, sunFogColor, vec3(clamp(nDotSun * scalar, 0, 1))).xyz;
    }

	outputColor = vec4(vec3(resultColor), vColor.a);
}
