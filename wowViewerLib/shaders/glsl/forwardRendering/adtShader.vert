#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"
#include "../common/commonADTMaterial.glsl"

/* vertex shader code */
layout(location = 0) in float aHeight;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec4 aVertexLighting;
layout(location = 3) in vec4 aNormal;

#include "../common/commonUboSceneData.glsl"

layout(std140, set=1, binding=0) uniform meshWideBlockVSPS {
    vec4 uPos;
    ivec4 uUseHeightMixFormula;
    vec4 uHeightScale;
    vec4 uHeightOffset;
};

mat3 blizzTranspose(mat4 value) {
    return mat3(
        value[0].xyz,
        value[1].xyz,
        value[2].xyz
    );
}

layout(location = 0) out vec2 vChunkCoords;
layout(location = 1) out vec3 vPosition;
layout(location = 2) out vec4 vColor;
layout(location = 3) out vec3 vNormal;
layout(location = 4) out vec3 vVertexLighting;
layout(location = 5) out vec2 vAlphaCoords;

void main() {

/*
     Y
  X  0    1    2    3    4    5    6    7    8
        9   10   11   12   13   14   15   16
*/

    //With current implementation ADT's VBO is combined buffer, which contains all vertexes from all MCNK of ADT
    //Thus, we first need to get vertexNumber within MCNK

    int indexInMCNK = gl_VertexIndex % (9 * 9 + 8 * 8);
    vec2 worldPointXY;
    calcAdtAlphaUV(indexInMCNK, uPos.xyz, vAlphaCoords, vChunkCoords, worldPointXY);

    mat4 viewMatForNormal = transpose(inverse(scene.uLookAtMat));
    vec3 normal = normalize((viewMatForNormal * vec4(aNormal.xyz, 0.0)).xyz);
    vec4 worldPoint = vec4(worldPointXY, aHeight, 1);

    vPosition = (scene.uLookAtMat * worldPoint).xyz;
    vNormal = normal;
    vColor = aColor;
    vVertexLighting = aVertexLighting.rgb;

    gl_Position = scene.uPMatrix * scene.uLookAtMat * worldPoint;
}
