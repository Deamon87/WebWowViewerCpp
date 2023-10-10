#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"

/* vertex shader code */
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec4 aVertexLighting;
layout(location = 3) in vec3 aNormal;

#include "../common/commonUboSceneData.glsl"
#include "../common/commonAdtIndirectDescriptorSet.glsl"


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
layout(location = 5) out flat int vMeshIndex;
layout(location = 6) out vec2 vAlphaCoords;

const float TILESIZE = (1600.0 / 3.0);
const float UNITSIZE =  TILESIZE / 16.0 / 8.0;


void main() {

/*
     Y
  X  0    1    2    3    4    5    6    7    8
        9   10   11   12   13   14   15   16
*/

    //With current implementation ADT's VBO is combined buffer, which contains all vertexes from all MCNK of ADT
    //Thus, we first need to get vertexNumber within MCNK

    int indexInMCNK = gl_VertexIndex % (9 * 9 + 8 * 8);
    float iX = mod(indexInMCNK, 17.0);
    float iY = floor(indexInMCNK/17.0);

    if (iX > 8.01) {
        iY = iY + 0.5;
        iX = iX - 8.5;
    }
    //    vChunkCoords = vec2(iX, iY);

//    vec4 worldPoint = vec4(
//        uPos.x - iY * UNITSIZE,
//        uPos.y - iX * UNITSIZE,
//        uPos.z + aHeight,
//        1);

    vec4 worldPoint = vec4(aPos, 1);

    vChunkCoords = vec2(iX, iY);

    vAlphaCoords = (aPos.yx - floor(aPos.yx / TILESIZE)*TILESIZE) / TILESIZE;
    vAlphaCoords.x = 1.0 - vAlphaCoords.x;
    vAlphaCoords.y = 1.0 - vAlphaCoords.y;

    vPosition = (scene.uLookAtMat * worldPoint).xyz;
    vColor = aColor;
    vVertexLighting = aVertexLighting.rgb;
    vMeshIndex = gl_InstanceIndex;
    mat4 viewMatForNormal = transpose(inverse(scene.uLookAtMat));
    vec3 normal = normalize((viewMatForNormal * vec4(aNormal, 0.0)).xyz);

    vNormal = normal;

    gl_Position = scene.uPMatrix * scene.uLookAtMat * worldPoint;

}
