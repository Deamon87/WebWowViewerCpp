#version 450

#extension GL_GOOGLE_include_directive: require


precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"
#include "../common/commonFunctions.glsl"
#include "../common/commonM2Material.glsl"

/* vertex shader code */
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in uvec4 bones;
layout(location=3) in vec4 boneWeights;
layout(location=4) in vec2 aTexCoord;
layout(location=5) in vec2 aTexCoord2;

//Whole scene
layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
    PSFog fogData[8];
};

//Whole model
#include "../common/commonM2DescriptorSet.glsl"

//Individual meshes
layout(std140, set=2, binding=7) uniform meshWideBlockVSPS {
    ivec4 vertexShader_IsAffectedByLight_TextureMatIndex1_TextureMatIndex2;
    ivec4 PixelShader_UnFogged_blendMode;
    ivec4 textureWeightIndexes;
    ivec4 colorIndex_applyWeight;
};

//Shader output
layout(location=0) out vec2 vTexCoord;
layout(location=1) out vec2 vTexCoord2;
layout(location=2) out vec2 vTexCoord3;
layout(location=3) out vec3 vNormal;
layout(location=4) out vec4 vPosition_EdgeFade;


void main() {
    vec4 modelPoint = vec4(0,0,0,0);

    vec4 aPositionVec4 = vec4(aPosition, 1);
    mat4 boneTransformMat =  mat4(0.0);

    if (dot(boneWeights, boneWeights) > 0) {
        //
        boneTransformMat += (boneWeights.x) * uBoneMatrixes[bones.x];
        boneTransformMat += (boneWeights.y) * uBoneMatrixes[bones.y];
        boneTransformMat += (boneWeights.z) * uBoneMatrixes[bones.z];
        boneTransformMat += (boneWeights.w) * uBoneMatrixes[bones.w];
    } else {
        boneTransformMat = mat4(1.0);
    }

    mat4 placementMat;
    placementMat = uPlacementMat;

    mat4 viewModelMat = scene.uLookAtMat * placementMat  * boneTransformMat ;
    vec4 vertexPosInView = viewModelMat * aPositionVec4;
    mat4 viewModelMatForNormal = transpose(inverse(viewModelMat));

    vec3 normal = normalize((viewModelMatForNormal * vec4(aNormal, 0.0)).xyz);
    vec3 minusNormal = normalize((viewModelMatForNormal * vec4(-aNormal, 0.0)).xyz);

    vTexCoord = aTexCoord;
    vTexCoord2 = aTexCoord2;
    vTexCoord3 = aTexCoord2;

    gl_Position = scene.uPMatrix * vertexPosInView;
    vNormal = normal;
    vPosition_EdgeFade = vec4(vertexPosInView.xyz, 0.0);
}


