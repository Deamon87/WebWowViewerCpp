//
// Pull-model GPU ribbon vertex shader: replaces the CPU's per-frame ring-buffer
// upload (CRibbonEmitter::updateBuffers). The strip index buffer is rewritten by
// ribbonSimulate.comp.slang every frame (live arc + degenerate fill), so vertex
// positions come straight from the ribbon edge SSBO.
//
// Reproduces CRibbonVertex semantics: world-space position, BGRA-normalized color
// (the CImVector float->u8 truncation and channel order are replicated exactly),
// and the age-scrolled texture coordinates.
//
#version 450

import glsl;
#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec4 vColor;
layout(location = 2) out vec2 vTexcoord0;

#include "../../../common/commonLightFunctions.slang"
#include "../../../common/commonFogFunctions.slang"
#include "../../../common/commonUboSceneData.slang"

#include "../../../common/ribbonGpu.slang"

struct M2TextureMatrices {
    mat4 textureMatrix[64];
};

struct M2MeshWideBlockPS {
   ivec4 uPixelShader_BlendMode_TextureTransformIndex;
   ivec4 ribbonGpuBind; // gpuStateIndex, gpuEdgesOffset, gpuEdgeCount, gpuPropsIndex
};

layout(std140, set=1, binding=0) StructuredBuffer<M2TextureMatrices> textureMatrices;
layout(std140, set=1, binding=1) ConstantBuffer<M2MeshWideBlockPS> meshWideBlockPS;

layout(std430, set=3, binding=0) StructuredBuffer<GpuRibbonState, ScalarDataLayout> ribbonStates;
layout(std430, set=3, binding=1) StructuredBuffer<GpuM2RibbonStatic, ScalarDataLayout> ribbonStatics;
layout(std430, set=3, binding=2) StructuredBuffer<GpuM2RibbonFrameProps, ScalarDataLayout> ribbonProps;
layout(std430, set=3, binding=3) StructuredBuffer<GpuRibbonEdge, ScalarDataLayout> ribbonEdges;

// float -> u8 (truncation, wrap mod 256 like the C++ unsigned char assignment) -> /255
float colorChannel(float v) {
    return float(uint(int(v)) & 0xFFu) / 255.0;
}

[shader("vertex")]
void main() {
    int stateIndex = meshWideBlockPS.ribbonGpuBind.x;
    int propsIndex = meshWideBlockPS.ribbonGpuBind.w;

    GpuRibbonState state = ribbonStates[stateIndex];
    GpuM2RibbonStatic st = ribbonStatics[state.staticsIndex];
    GpuM2RibbonFrameProps props = ribbonProps[propsIndex];

    uint vertIdx = uint(gl_VertexIndex); // strip index buffer value
    int edgeIdx = int(vertIdx >> 1);
    bool isAbove = (vertIdx & 1u) != 0u;

    GpuRibbonEdge edge = ribbonEdges[uint(state.edgesOffset + edgeIdx)];

    vec3 pos = isAbove ? vec3(edge.abovePos[0], edge.abovePos[1], edge.abovePos[2])
                       : vec3(edge.belowPos[0], edge.belowPos[1], edge.belowPos[2]);

    // CRibbonEmitter::SetColor/SetAlpha + CImVector memory order (b,g,r,a), which the
    // normalized ubyte4 attribute reads as (x,y,z,w) — the two swaps cancel out.
    vColor = vec4(
        colorChannel(props.color[0] * 255.0),
        colorChannel(props.color[1] * 255.0),
        colorChannel(props.color[2] * 255.0),
        colorChannel(max(255.0 * props.alpha, 0.0))
    );

    vTexcoord0 = vec2(
        st.tmpDU * edge.age * st.ooLifeSpan + props.texSlotBox[0],
        isAbove ? props.texSlotBox[3] : props.texSlotBox[1]
    );

    vec4 vertexViewSpace = (scene.uLookAtMat * vec4(pos, 1.0));
    vPosition = vertexViewSpace.xyz;
    gl_Position = scene.uPMatrix * vertexViewSpace;
}
