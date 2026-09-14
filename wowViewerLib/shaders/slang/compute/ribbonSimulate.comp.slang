//
// GPU ribbon simulation: one thread per ribbon — a port of CRibbonEmitter::SetPos +
// CRibbonEmitter::Update (engine/managers/CRibbonEmitter.cpp) writing the per-edge
// ring buffer (GpuRibbonEdge) and rewriting the ribbon's strip index buffer every
// frame (live arc + degenerate fill), so the draw can cover the whole buffer with
// the CPU's per-frame start/end patching replaced by GPU-written content.
//

import glsl;

#include "../common/ribbonGpu.slang"

layout(std430, set=0, binding=0) StructuredBuffer<uint, ScalarDataLayout> ribbonDispatchList;
layout(std430, set=0, binding=1) RWStructuredBuffer<GpuRibbonState, ScalarDataLayout> ribbonStates;
layout(std430, set=0, binding=2) StructuredBuffer<GpuM2RibbonStatic, ScalarDataLayout> ribbonStatics;
layout(std430, set=0, binding=3) StructuredBuffer<GpuM2RibbonFrameProps, ScalarDataLayout> ribbonProps;
layout(std430, set=0, binding=4) RWStructuredBuffer<GpuRibbonEdge, ScalarDataLayout> ribbonEdges;
// uint16 index pairs packed into uint32 words (two strip vertex indices per word)
layout(std430, set=0, binding=5) RWStructuredBuffer<uint, ScalarDataLayout> ribbonIndexPool;
layout(std430, set=0, binding=6) StructuredBuffer<mat4, ScalarDataLayout> boneMatrix;
struct PlacementMatStruct { mat4 placementMat; mat4 invPlacementMat; };
layout(std430, set=0, binding=7) StructuredBuffer<PlacementMatStruct, ScalarDataLayout> placementMats;

struct RibbonSimPushConsts {
    float deltaSeconds;
    float pad0;
    float pad1;
    float pad2;
};
[[vk::push_constant]] RibbonSimPushConsts pushConsts;

vec3 rd3(in float a[4]) { return vec3(a[0], a[1], a[2]); }
void wr3(out float a[4], in vec3 v) { a[0] = v.x; a[1] = v.y; a[2] = v.z; }
vec3 rd3e(in float a[3]) { return vec3(a[0], a[1], a[2]); }

GpuRibbonEdge readEdge(in GpuRibbonState state, int edgeIdx) {
    return ribbonEdges[uint(state.edgesOffset + edgeIdx)];
}
void writeEdge(in GpuRibbonState state, int edgeIdx, vec3 belowPos, float age, vec3 abovePos) {
    GpuRibbonEdge e;
    e.belowPos[0] = belowPos.x; e.belowPos[1] = belowPos.y; e.belowPos[2] = belowPos.z;
    e.age = age;
    e.abovePos[0] = abovePos.x; e.abovePos[1] = abovePos.y; e.abovePos[2] = abovePos.z;
    e.pad = 0.0;
    ribbonEdges[uint(state.edgesOffset + edgeIdx)] = e;
}

void writeIndexWord(in GpuRibbonState state, int wordIdx, uint vertA, uint vertB) {
    ribbonIndexPool[uint(state.iboOffset + wordIdx)] = vertA | (vertB << 16);
}

// CRibbonEmitter::InitInterpDeltas
void initInterpDeltas(inout GpuRibbonState state, float below, float above) {
    vec3 prevPos = rd3(state.prevPos);
    vec3 currPos = rd3(state.currPos);
    vec3 prevVertical = rd3(state.prevVertical);
    vec3 currVertical = rd3(state.currVertical);

    float scale = length(prevPos - currPos); // CPU: sqrtf(LengthSquared)

    wr3(state.below0, prevPos - below * prevVertical);
    wr3(state.below1, currPos - below * currVertical);
    wr3(state.above0, above * prevVertical + prevPos);
    wr3(state.above1, above * currVertical + currPos);
    wr3(state.prevDirScaled, scale * rd3(state.prevDir));
    wr3(state.currDirScaled, scale * rd3(state.currDir));
}

// CRibbonEmitter::InterpEdge
void interpEdge(inout GpuRibbonState state, float age, float t, int advance) {
    vec3 below0 = rd3(state.below0);
    vec3 below1 = rd3(state.below1);
    vec3 above0 = rd3(state.above0);
    vec3 above1 = rd3(state.above1);
    vec3 prevDirScaled = rd3(state.prevDirScaled);
    vec3 currDirScaled = rd3(state.currDirScaled);

    vec3 belowPos = ((below1 - ((1.0 - t) * currDirScaled)) * t)
                  + (((t * prevDirScaled) + below0) * (1.0 - t));
    vec3 abovePos = ((above1 - ((1.0 - t) * currDirScaled)) * t)
                  + ((1.0 - t) * ((t * prevDirScaled) + above0));

    writeEdge(state, state.writePos, belowPos, age, abovePos);

    state.writePos += advance;
    if (state.writePos >= state.edgeCount) {
        state.writePos -= state.edgeCount;
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {
    uint ribbonIdx = ribbonDispatchList[gl_WorkGroupID.x];

    GpuRibbonState state = ribbonStates[ribbonIdx];
    GpuM2RibbonStatic st = ribbonStatics[state.staticsIndex];
    GpuM2RibbonFrameProps props = ribbonProps[state.propsIndex];

    float deltaTime = pushConsts.deltaSeconds;

    // ---------------- CRibbonEmitter::SetPos ----------------
    // (userEnabled is always 1 after Initialize; M2Object passes nullPos + no frameOfReference)
    if (props.dataEnabled != 0) {
        mat4 rhs = placementMats[state.placementMatIndex].placementMat
                 * boneMatrix[state.boneOutOffset + st.boneIndex]
                 * mat4(1,0,0,0, 0,1,0,0, 0,0,1,0, st.position[0], st.position[1], st.position[2], 1);

        vec3 position = rhs[3].xyz; // + nullPos (0,0,0)
        if (state.posSet != 0) {
            wr3(state.prevPos, rd3(state.currPos));
            wr3(state.prevDir, rd3(state.currDir));
            wr3(state.prevVertical, rd3(state.currVertical));
        } else {
            wr3(state.prevPos, position);
            wr3(state.prevDir, rhs[2].xyz);
            wr3(state.prevVertical, rhs[1].xyz);
            state.startTime = 0.0;
            state.posSet = 1;
        }
        wr3(state.currPos, position);
        wr3(state.currDir, rhs[2].xyz);
        wr3(state.currVertical, rhs[1].xyz);
    }

    // ---------------- CRibbonEmitter::Update ----------------
    float dt = deltaTime;
    if (state.singletonUpdated == 0) {
        if (st.edgesPerSec > 0.0)
            dt = 1.0 / st.edgesPerSec + 0.000099999997;
    }
    if (dt >= 0.0) {
        if (st.edgeLifeSpan <= dt)
            dt = st.edgeLifeSpan;
    } else {
        dt = 0.0;
    }

    // Expire old edges; expired edges copy the new first-live edge's content so the
    // strip joint at the read position stays coincident (degenerate) — though dead
    // edges are never referenced by the rewritten index buffer anyway.
    while (state.readPos != state.writePos) {
        float age = readEdge(state, state.readPos).age;
        if (dt + age <= st.edgeLifeSpan)
            break;
        int newReadPos = state.readPos + 1;
        if (newReadPos >= state.edgeCount) newReadPos -= state.edgeCount;
        state.readPos = newReadPos;
    }

    // Spawn new edges
    if (props.dataEnabled != 0 && state.posSet != 0) {
        float v76 = dt * st.edgesPerSec + state.startTime;

        initInterpDeltas(state, props.below, props.above);

        if (v76 >= 1.0) {
            float v94 = state.startTime;
            float ooDenom = 1.0 / (v76 - v94);
            float v39 = floor(v76 - 1.0);
            int v79 = int(ceil(max(v39, 0.0)));

            float i = 1.0;
            while (true) {
                float t = (i - v94) * ooDenom;
                interpEdge(state, t * (-dt), t, 1);
                if (--v79 == -1)
                    break;
                i += 1.0;
            }
        }
        state.startTime = v76 - floor(v76);
        interpEdge(state, 0.0, 1.0, 0);
    }

    // Age + gravity per live edge
    int j = state.readPos;
    while (j != state.writePos) {
        GpuRibbonEdge e = readEdge(state, j);
        float zadd = ((st.gravity + st.gravity) * e.age * dt) + (dt * st.gravity * dt);
        vec3 belowPos = rd3e(e.belowPos);
        vec3 abovePos = rd3e(e.abovePos);
        belowPos.z += zadd;
        abovePos.z += zadd;
        writeEdge(state, j, belowPos, e.age + dt, abovePos);
        j = j + 1;
        if (j >= state.edgeCount) j -= state.edgeCount;
    }

    state.singletonUpdated = 1;

    // ---- Rewrite the strip index buffer: live arc [readPos, writePos] then
    // degenerate fill (repeat the last vertex) ----
    int wordCount = state.iboCount / 2; // uint32 words (2 uint16 indices each)
    int word = 0;
    uint lastVert = 0;
    if (state.readPos != state.writePos || state.posSet != 0) {
        // The CPU draws edges [readPos, writePos] (inclusive of writePos — the
        // current edge written by InterpEdge(0,1,0))
        int e = state.readPos;
        while (true) {
            uint vertA = uint(2 * e);
            uint vertB = uint(2 * e + 1);
            writeIndexWord(state, word, vertA, vertB);
            lastVert = vertB;
            word++;
            if (e == state.writePos) break;
            e = e + 1;
            if (e >= state.edgeCount) e -= state.edgeCount;
        }
    }
    // fill the rest with the last vertex (degenerate triangles, no slivers)
    while (word < wordCount) {
        writeIndexWord(state, word, lastVert, lastVert);
        word++;
    }

    ribbonStates[ribbonIdx] = state;
}
