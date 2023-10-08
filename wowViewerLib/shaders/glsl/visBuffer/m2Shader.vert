#version 450

#extension GL_GOOGLE_include_directive: require


precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"
#include "../common/commonFunctions.glsl"
#include "../common/commonM2Material.glsl"
#include "../common/commonUboSceneData.glsl"

/* vertex shader code */
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in uvec4 bones;
layout(location=3) in vec4 boneWeights;
layout(location=4) in vec2 aTexCoord;
layout(location=5) in vec2 aTexCoord2;

//Whole model
#include "../common/commonM2IndirectDescriptorSet.glsl"

//Shader output
layout(location=0) out vec2 vTexCoord;
layout(location=1) out vec2 vTexCoord2;
layout(location=2) out vec2 vTexCoord3;
layout(location=3) out vec3 vNormal;
layout(location=4) out vec4 vPosition_EdgeFade;
layout(location=5) out flat int instanceIndex;


void main() {
    vec4 modelPoint = vec4(0,0,0,0);

    vec4 aPositionVec4 = vec4(aPosition, 1);
    mat4 boneTransformMat =  mat4(0.0);

    meshWideBlockVSPSBindless meshWideBindless = meshWideBindleses[gl_InstanceIndex];
    meshWideBlockVSPS meshWide = meshWides[meshWideBindless.instanceIndex_meshIndex.y];
    int instanceIndex = meshWideBindless.instanceIndex_meshIndex.x;

    if (dot(boneWeights, boneWeights) > 0) {
        int boneIndex = instances[instanceIndex].placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd.y;

        boneTransformMat += (boneWeights.x) * uBoneMatrixes[boneIndex + bones.x];
        boneTransformMat += (boneWeights.y) * uBoneMatrixes[boneIndex + bones.y];
        boneTransformMat += (boneWeights.z) * uBoneMatrixes[boneIndex + bones.z];
        boneTransformMat += (boneWeights.w) * uBoneMatrixes[boneIndex + bones.w];
    } else {
        boneTransformMat = mat4(1.0);
    }

    int placementIndex = instances[instanceIndex].placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd.x;
    mat4 placementMat = uPlacementMats[placementIndex];

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
    instanceIndex = gl_InstanceIndex;
}


