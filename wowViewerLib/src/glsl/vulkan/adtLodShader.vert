#version 450

layout(location = 0) in float aHeight;
layout(location = 1) in float aIndex;

layout(std140) uniform modelWideBlockVS {
    vec3 uPos;
    mat4 uLookAtMat;
    mat4 uPMatrix;
};

layout(location = 0) out vec2 vChunkCoords;
layout(location = 1) out vec3 vPosition;

const float UNITSIZE_X =  (1600.0 / 3.0) / 128.0;
const float UNITSIZE_Y =  (1600.0 / 3.0) / 128.0;
const float ADTLOD_UNITSIZE = (1600.0 / 3.0) / 128.0;

void main() {

    float stepX = floor(aIndex / (129.0*129.0));
    float division = 129.0;
    if (stepX > 0.1)
        division = 128.0;
    float offset =stepX*129.0*129.0;

    float iX = mod(aIndex - offset, division) + stepX*0.5;
    float iY = floor((aIndex - offset)/division) + stepX*0.5;

//    if (stepX < 0.1 && iY > 126) {
//        iY  = 0;
//    }

    vec4 worldPoint = vec4(
        uPos.x - iY * UNITSIZE_Y,
        uPos.y - iX * UNITSIZE_X,
        aHeight,
        1);


//
//    float iX = mod(aIndex, 256.0);
//    float iY = floor(aIndex/256.0);
//
//    if (iX > 128.01) {
//        iY = iY + 0.5;
//        iX = iX - 128.5;
//    }
//
//    vec4 worldPoint = vec4(
//        uPos.x - iX * UNITSIZE_Y,
//        uPos.y - iY * UNITSIZE_X,
//        uPos.z + aHeight,
//        1);

    vChunkCoords = vec2(iX / 128.0, iY / 128.0);

    vPosition = (uLookAtMat * worldPoint).xyz;
    gl_Position = uPMatrix * uLookAtMat * worldPoint;
}