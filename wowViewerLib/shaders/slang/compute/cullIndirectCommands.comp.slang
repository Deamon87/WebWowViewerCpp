import glsl;

struct DrawIndexedIndirectCommand {
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int  vertexOffset;
    uint firstInstance;
};

// One record of the per-frame active draw list built on the CPU.
// dynIndexCount != 0 marks particle/ribbon meshes whose start/end change per frame:
// the gather then overrides the static template's firstIndex/indexCount.
struct ActiveDrawEntry {
    uint meshId;         // index into staticCommandBuffer
    uint cullObjectId;   // index into visibilityBuffer/classificationBuffer
    uint dynFirstIndex;
    uint dynIndexCount;
};

// x = drawCommandCount, y = renderMode (0=below liquid, 1=above liquid, 2=all/no filter),
// z = startIndex (offset into buffers, for processing only a subrange)
[vk::push_constant]
ivec4 drawCommandCount_renderMode_startIndex;

StructuredBuffer<ActiveDrawEntry, ScalarDataLayout> activeDrawList;   // per-frame draw order
StructuredBuffer<uint, ScalarDataLayout> visibilityBuffer;            // 0 = culled, non-zero = visible
RWStructuredBuffer<DrawIndexedIndirectCommand, ScalarDataLayout> indirectDrawBuffer;
StructuredBuffer<uint, ScalarDataLayout> classificationBuffer;        // per-object liquid classification bits
StructuredBuffer<DrawIndexedIndirectCommand, ScalarDataLayout> staticCommandBuffer; // per-mesh templates

// Gather pass: instantiate the compact per-frame indirect commands from the static
// per-mesh templates, applying frustum/occlusion visibility and liquid classification.
[shader("compute")]
[numthreads(64, 1, 1)]
void main() {
    uint localIdx = gl_GlobalInvocationID.x;
    uint count = drawCommandCount_renderMode_startIndex.x;
    uint startIndex = drawCommandCount_renderMode_startIndex.z;

    if (localIdx >= count) return;

    uint idx = startIndex + localIdx;

    ActiveDrawEntry entry = activeDrawList[idx];

    uint visible = visibilityBuffer[entry.cullObjectId];

    // Liquid classification filtering
    uint renderMode = drawCommandCount_renderMode_startIndex.y;
    if (visible != 0u && renderMode < 2u) {
        uint classification = classificationBuffer[entry.cullObjectId];
        // renderMode 0 (below): require bit 0 (value 1)
        // renderMode 1 (above): require bit 1 (value 2)
        uint requiredBit = renderMode + 1u;
        if ((classification & requiredBit) == 0u) {
            visible = 0u;
        }
    }

    if (visible == 0u) {
        indirectDrawBuffer[idx].instanceCount = 0u;
        return;
    }

    DrawIndexedIndirectCommand cmd = staticCommandBuffer[entry.meshId];
    if (entry.dynIndexCount != 0u) {
        cmd.indexCount = entry.dynIndexCount;
        cmd.firstIndex = entry.dynFirstIndex;
    }
    indirectDrawBuffer[idx] = cmd;
}
