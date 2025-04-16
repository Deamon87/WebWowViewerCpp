#version 450

#extension GL_GOOGLE_include_directive: require
#extension GL_EXT_nonuniform_qualifier : require

precision highp float;
precision highp int;

//Set 0
#include "../../common/commonUboSceneData.glsl"
//Set 1
#include "../../common/commonM2IndirectDescriptorSet.glsl"
#define GBUFFER_SET_INDEX 3
#include "../../common/commonGBufferData.glsl"

#include "../../common/commonProjectiveConstants.glsl"


layout (set = 2, binding = 0) uniform sampler2D s_Textures[];

layout(location = 0) in flat int vMeshIndex;

layout(location = 0) out vec4 outColor;


void main() {
    meshWideBlockVSPSBindless meshWideBindless = meshWideBindleses[vMeshIndex];
    ProjectiveData projectiveData = projectiveData[nonuniformEXT(meshWideBindless.instanceIndex_meshIndex.z)];

    int instanceIndex = meshWideBindless.instanceIndex_meshIndex.x;

    M2InstanceRecordBindless m2Instance = instances[nonuniformEXT(instanceIndex)];

    modelWideBlockPSStruct modelWide = modelWides[m2Instance.textureMatricesInd_modelFragmentDatasInd.y];

    int placementMatrixInd = m2Instance.placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd.x;
    int textureWeightsInd = m2Instance.placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd.w;
    int m2ColorsInd = m2Instance.placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd.z;
    int textureMatricesInd = m2Instance.textureMatricesInd_modelFragmentDatasInd.x;

    int placementIndex = instances[nonuniformEXT(instanceIndex)].placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd.x;
    mat4 invPlaceMat4 = placementMats[nonuniformEXT(placementIndex)].invPlacementMat;

    vec3 viewPos = gbuff_getViewPos(gl_FragCoord.xy, scene.uInvPMatrix);
    vec4 worldPos = scene.uInvLookAtMat * vec4(viewPos, 1.0);

    vec4 localPoint = invPlaceMat4 * worldPos;
    if (
        any(lessThan(localPoint.xyz, projectiveData.localMin.xyz - vec3(0,0,PROJECTIVE_BOUNDING_BOX_BOTTOM))) ||
        any(greaterThan(localPoint.xyz, projectiveData.localMax.xyz + vec3(0,0,PROJECTIVE_BOUNDING_BOX_UP)))
    ) {
        discard;
    }

    vec2 texCoordinates = (projectiveData.localToUVMat * localPoint).xy;

    if (texCoordinates.x < 0 || texCoordinates.x > 1 ||
        texCoordinates.y < 0 || texCoordinates.y > 1) discard;
//
    //Calculate material based on pixel shader

    outColor = vec4(0,0,1.0f,0.5);
}