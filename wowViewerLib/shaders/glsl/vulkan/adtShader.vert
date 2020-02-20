#version 450

#extension GL_GOOGLE_include_directive: require

#include "../common/commonLightFunctions.glsl"

/* vertex shader code */
layout(location = 0) in vec3 aHeight;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec4 aVertexLighting;
layout(location = 3) in vec3 aNormal;
layout(location = 4) in float aIndex;

layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
};
layout(std140, binding=2) uniform meshWideBlockVS {
    vec4 uPos;
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

const float UNITSIZE_X =  (1600.0 / 3.0) / 16.0 / 8.0;
const float UNITSIZE_Y =  (1600.0 / 3.0) / 16.0 / 8.0;

void main() {

/*
     Y
  X  0    1    2    3    4    5    6    7    8
        9   10   11   12   13   14   15   16
*/
    float iX = mod(aIndex, 17.0);
    float iY = floor(aIndex/17.0);

    if (iX > 8.01) {
        iY = iY + 0.5;
        iX = iX - 8.5;
    }

//    vec4 worldPoint = vec4(
//        uPos.x - iY * UNITSIZE_Y,
//        uPos.y - iX * UNITSIZE_X,
//        uPos.z + aHeight,
//        1);

    vec4 worldPoint = vec4(
        aHeight,
        1);

    vChunkCoords = vec2(iX, iY);

    vPosition = (scene.uLookAtMat * worldPoint).xyz;
    vColor = aColor;
    vVertexLighting = aVertexLighting.rgb;
    vNormal = blizzTranspose(scene.uLookAtMat) * aNormal;

    gl_Position = scene.uPMatrix * scene.uLookAtMat * worldPoint;
}
