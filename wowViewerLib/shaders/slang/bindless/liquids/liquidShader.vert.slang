#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require
#extension GL_EXT_nonuniform_qualifier: require
#extension GL_ARB_shader_draw_parameters: require

precision highp float;
precision highp int;

#include "../../common/commonLightFunctions.slang"
#include "../../common/commonFogFunctions.slang"
#include "../../common/commonUboSceneData.slang"

layout(location=0) in vec4 aPositionTransp;
layout(location=1) in vec2 aTexCoord;

#include "../../common/commonLiquidIndirectDescriptorSet.slang"

//out vec2 vTexCoord;
layout(location=0) out vec4 vPositionDepth;
layout(location=1) out vec3 vWorldPosition;
layout(location=2) out vec2 vTextCoords;
layout(location=3) out vec3 vNormal;
layout(location=4) out vec2 vCellUv;
layout(location=5) out flat int meshInd;

[shader("vertex")]
void main() {
    LiquidBindless liquidBindless = liquidBindlesses[gl_InstanceIndex];

    vec4 aPositionVec4 = vec4(aPositionTransp.xyz, 1);
    mat4 viewModelMat = scene.uLookAtMat * uPlacementMats[liquidBindless.liquidDataInd_placementMatInd_liquidMaterialType.y];

    vec4 cameraPoint = viewModelMat * aPositionVec4;

    mat4 viewModelMatForNormal = transpose(inverse(viewModelMat));

    //const float posToTextPos = 1.0 / (1600.0/3.0/16.0);
    vTextCoords = aTexCoord;//aPositionVec4.xy * posToTextPos;

    gl_Position = scene.uPMatrix * cameraPoint;

    vPositionDepth = vec4(cameraPoint.xyz, aPositionTransp.w);
    vWorldPosition = aPositionVec4.xyz;
    vNormal = normalize(viewModelMatForNormal * vec4(vec3(0,0,1.0), 0.0)).xyz;

    LiquidInstanceData liquidInstanceData = liquidInstanceDatas[gl_InstanceIndex];

    int cellVertexIdx = (gl_VertexIndex - gl_BaseVertex);

    int vtxCellOffetWidthHeight = liquidInstanceData.liquidMaterial_liquidType_isInterior_vtxCellOffetWidthHeight.w;

    int cellHeight =   vtxCellOffetWidthHeight & 0xFF;
    int cellWidth =   (vtxCellOffetWidthHeight >> 8) & 0xFF;
    int cellOffsetY = (vtxCellOffetWidthHeight >> 16) & 0xFF;
    int cellOffsetX = (vtxCellOffetWidthHeight >> 24) & 0xFF;

    int x = cellVertexIdx % (cellWidth + 1);
    int y = cellVertexIdx / (cellHeight + 1);

    vCellUv = vec2((cellOffsetX + x) * (1.0 / 8.0), (cellOffsetY + y) * (1.0 / 8.0));

    meshInd = gl_InstanceIndex;
}
