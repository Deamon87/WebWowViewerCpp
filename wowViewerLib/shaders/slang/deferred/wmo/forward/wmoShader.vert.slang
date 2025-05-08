#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../../../common/commonLightFunctions.slang"
#include "../../../common/commonFogFunctions.slang"
#include "../../../common/commonFunctions.slang"
#include "../../../common/commonWMOMaterial.slang"

/* vertex shader code */
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec2 aTexCoord2;
layout (location = 4) in vec2 aTexCoord3;
layout (location = 5) in vec2 aTexCoord4;
layout (location = 6) in vec4 aColor;
layout (location = 7) in vec4 aColor2;
layout (location = 8) in vec4 aColorSecond;

#include "../../../common/commonUboSceneData.slang"
#include "../../../common/commonWMODescriptorSet.slang"

layout(location=0) out vec2 vTexCoord;
layout(location=1) out vec2 vTexCoord2;
layout(location=2) out vec2 vTexCoord3;
layout(location=3) out vec2 vTexCoord4;
layout(location=4) out vec4 vColor;
layout(location=5) out vec4 vColor2;
layout(location=6) out vec4 vColorSecond;
layout(location=7) out vec4 vPosition;
layout(location=8) out vec3 vNormal;
layout(location=9) out flat int o_groupNum;
layout(location=10) out flat int o_wmoObjId;

[shader("vertex")]
void main() {
    float canHaveExteriorLit = (( gl_InstanceIndex >> 10 ) & 1) > 0 ? 1.0 : 0.0;
    o_groupNum = gl_InstanceIndex & ((1 << 10) -1);
    o_wmoObjId = (gl_InstanceIndex >> 11) & 0xFFFFF;

    mat4 placementMat = uPlacementMat;
    vec4 worldPoint = placementMat * vec4(aPosition, 1);

    vec4 cameraPoint = scene.uLookAtMat * worldPoint;

    mat4 viewModelMat = scene.uLookAtMat * placementMat;
    mat4 viewModelMatForNormal = transpose(inverse(viewModelMat));

    gl_Position = scene.uPMatrix * cameraPoint;
    vPosition = vec4(cameraPoint.xyz, 0);
    vNormal = normalize((viewModelMatForNormal * vec4(aNormal, 0.0)).xyz);

    vColor = vec4(aColor.bgr, aColor.a * canHaveExteriorLit);
    vColor2 = aColor2;
    vColorSecond = aColorSecond;
    vTexCoord4 = aTexCoord4;

    int uVertexShader = vertMeshWide.VertexShader_UseLitColors.x;

    calcWMOVertMat(uVertexShader,
        vPosition.xyz, vNormal,
        aTexCoord, aTexCoord2, aTexCoord3,
        scene.uViewUpSceneTime.w,
        vertMeshWide.translationSpeedXY,
        vTexCoord, vTexCoord2, vTexCoord3
    );
}
