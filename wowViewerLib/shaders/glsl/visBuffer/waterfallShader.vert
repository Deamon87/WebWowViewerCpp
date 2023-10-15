#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonFunctions.glsl"
#include "../common/commonM2Material.glsl"
#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"
#include "../common/commonUboSceneData.glsl"

precision highp float;

/* vertex shader code */
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in uvec4 bones;
layout(location=3) in vec4 boneWeights;
layout(location=4) in vec2 aTexCoord;
layout(location=5) in vec2 aTexCoord2;

//Whole scene


// Whole model
#include "../common/commonM2DescriptorSet.glsl"

//Individual meshes
layout(std140, set=2, binding=4) uniform meshWideBlockVS {
    vec4 bumpScaleTextIndexes;
};

layout(set=3,binding=9) uniform sampler2D uBumpTexture;

//Shader output
layout(location=0) out vec2 vTexCoord;
layout(location=1) out vec2 vTexCoord2;
layout(location=2) out vec2 vTexCoord2_animated;
layout(location=3) out vec3 vNormal;
layout(location=4) out vec3 vPosition;

void main() {
    float bumpScale = bumpScaleTextIndexes.x;
    int textMatIndex1 = floatBitsToInt(bumpScaleTextIndexes.y);
    int textMatIndex2 = floatBitsToInt(bumpScaleTextIndexes.z);

    mat4 textMat[2];
    textMat[0] = textMatIndex1 < 0 ? mat4(1.0) : textureMatrix[textMatIndex1];
    textMat[1] = textMatIndex2 < 0 ? mat4(1.0) : textureMatrix[textMatIndex2];

    vec2 texCoord2 = (textMat[0] * vec4(aTexCoord2, 0, 1)).xy;

    vec4 bumpValue = texture(uBumpTexture, texCoord2);
    vec3 pos = (aNormal * bumpScale.x) * bumpValue.z + aPosition;

    mat4 boneTransformMat =  mat4(0.0);

    boneTransformMat += (boneWeights.x ) * uBoneMatrixes[bones.x];
    boneTransformMat += (boneWeights.y ) * uBoneMatrixes[bones.y];
    boneTransformMat += (boneWeights.z ) * uBoneMatrixes[bones.z];
    boneTransformMat += (boneWeights.w ) * uBoneMatrixes[bones.w];

    mat4 viewModelMat = scene.uLookAtMat * uPlacementMat  * boneTransformMat ;
    vec4 cameraPoint = viewModelMat * vec4(pos, 1.0);

    mat4 viewModelMatForNormal = transpose(inverse(viewModelMat));
    vec3 normal = normalize((viewModelMatForNormal * vec4(aNormal, 0.0)).xyz);

    vNormal = (scene.uLookAtMat * uPlacementMat * vec4(aNormal, 0)).xyz;
    vPosition = pos;

    vTexCoord = aTexCoord;
    vTexCoord2_animated = texCoord2;
    vTexCoord2 = aTexCoord2;

    gl_Position = scene.uPMatrix * cameraPoint;
}
