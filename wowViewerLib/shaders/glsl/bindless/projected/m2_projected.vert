#version 450

#extension GL_GOOGLE_include_directive: require
#extension GL_EXT_nonuniform_qualifier : require

precision highp float;
precision highp int;

//Set 0
#include "../../common/commonUboSceneData.glsl"
//Set 1
#include "../../common/commonM2IndirectDescriptorSet.glsl"
//#define GBUFFER_SET_INDEX 3
//#include "../../common/commonGBufferData.glsl"

#include "../../common/commonProjectiveConstants.glsl"

const vec3 boundingBox[8] = vec3[](
    vec3(-1.0, -1.0, -1.0),  // Vertex 0: bottom right back
    vec3( 1.0, -1.0, -1.0),  // Vertex 1: bottom left back
    vec3(-1.0,  1.0, -1.0),  // Vertex 2: bottom right front
    vec3( 1.0,  1.0, -1.0),  // Vertex 3: bottom left front
    vec3(-1.0, -1.0,  1.0),  // Vertex 4: top right back
    vec3( 1.0, -1.0,  1.0),  // Vertex 5: top left back
    vec3(-1.0,  1.0,  1.0),  // Vertex 6: top right front
    vec3( 1.0,  1.0,  1.0)   // Vertex 7: top left front
);

layout(location = 0) out flat int vMeshIndex;

void main()
{
    // Fetch the unique vertex position from the uniquePositions array.
    vec3 bboxPos = boundingBox[gl_VertexIndex];

    meshWideBlockVSPSBindless meshWideBindless = meshWideBindleses[gl_InstanceIndex];
    int instanceIndex = meshWideBindless.instanceIndex_meshIndex.x;

    ProjectiveData projectiveData = projectiveData[nonuniformEXT(meshWideBindless.instanceIndex_meshIndex.z)];

    int placementIndex = instances[nonuniformEXT(instanceIndex)].placementMatrixInd_boneMatrixInd_m2ColorsInd_textureWeightsInd.x;
    mat4 placementMat = placementMats[nonuniformEXT(placementIndex)].placementMat;

    vec3 center = (projectiveData.localMin.xyz + projectiveData.localMax.xyz) * 0.5f;

    vec3 scale = projectiveData.localMax.xyz - center;

    float newHalfOfHeight = (PROJECTIVE_BOUNDING_BOX_UP+PROJECTIVE_BOUNDING_BOX_BOTTOM) * 0.5;
    float newCenter = PROJECTIVE_BOUNDING_BOX_UP - newHalfOfHeight;

    vec3 localPoint = center + bboxPos * scale + vec3(0,0,newCenter + bboxPos.z * newHalfOfHeight);

    // Transform the vertex position to clip space using the provided mvp matrix.
    gl_Position = scene.uPMatrix * viewPoint;
    vMeshIndex = gl_InstanceIndex;
}