#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require
#extension GL_EXT_nonuniform_qualifier : require

precision highp float;
precision highp int;

#define PROJECTIVE_MATERIAL

//Set 0
#include "../../common/commonUboSceneData.slang"
//Set 1
#include "../../common/commonM2DescriptorSet.slang"
//#define GBUFFER_SET_INDEX 3
//#include "../../common/commonGBufferData.slang"

#include "../../common/commonProjectiveConstants.slang"

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

// 2, 0, 1,
// 2, 1, 3
const vec2 quadVerticies[4] = vec2[](
    vec2(-1.0, -1.0),  // Vertex 0: bottom right back
    vec2( 1.0,  1.0),  // Vertex 1: bottom left back
    vec2(-1.0,  1.0),  // Vertex 2: bottom right front
    vec2( 1.0, -1.0)   // Vertex 3: bottom left front
);

float extractNear(mat4 proj) {
    float C = proj[2][2];
    float D = proj[2][3];
    return D / (C - 1.0);
}

[shader("vertex")]
void main()
{
    mat4 invPlacementMat = placementMats.invPlacementMat;
    mat4 placementMat = placementMats.placementMat;

    {
        // Check if camera inside bounding box of m2
        vec3 localCameraPos = (invPlacementMat * (scene.uInvLookAtMat * vec4(0, 0, 0, 1))).xyz;

        if (
            all(greaterThan(localCameraPos, projectiveData.localMin.xyz - vec3(0, 0, PROJECTIVE_BOUNDING_BOX_BOTTOM))) &&
            all(lessThan(localCameraPos, projectiveData.localMax.xyz + vec3(0, 0, PROJECTIVE_BOUNDING_BOX_UP)))
        ) {
            //Render full quad
            if (gl_VertexIndex >= 4) {
                gl_Position = vec4(0, 0, 0, -1);
                return;
            }
            vec2 quadVert = quadVerticies[gl_VertexIndex];
            gl_Position = vec4(quadVert, 0, 1);


            return;
        }
    }

    vec3 center = (projectiveData.localMin.xyz + projectiveData.localMax.xyz) * 0.5f;

    vec3 scale = projectiveData.localMax.xyz - center;

    float newHalfOfHeight = (PROJECTIVE_BOUNDING_BOX_UP+PROJECTIVE_BOUNDING_BOX_BOTTOM) * 0.5;
    float newCenter = PROJECTIVE_BOUNDING_BOX_UP - newHalfOfHeight;

    // Fetch the unique vertex position from the uniquePositions array.
    vec3 bboxPos = boundingBox[gl_VertexIndex];

    vec3 localPoint = center + bboxPos * scale + vec3(0,0,newCenter + bboxPos.z * newHalfOfHeight);

    vec4 worldPoint = placementMat * vec4(localPoint, 1.0);
    vec4 viewPoint = scene.uLookAtMat * worldPoint;

    vec4 clipSpace = scene.uPMatrix * viewPoint;

    gl_Position = clipSpace;

}