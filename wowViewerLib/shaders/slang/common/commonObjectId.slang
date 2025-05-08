#ifndef COMMON_OBJECT_ID_SLANG
#define COMMON_OBJECT_ID_SLANG

// Object-id selection output.
// Must match SELECTION_ENABLED_SPEC_CONSTANT_ID in MaterialBuilderVLK.cpp.
// When the device does not support independentBlend, the forward pass is created
// without the object-id attachment and pipelines are specialized with
// selectionEnabled=0, eliminating the write below.
layout(constant_id = 30) const int selectionEnabled = 1;

layout(location = 1) out uint outObjectId;

// Macro (not a function) so the write stays lexically inside the entry point,
// matching how the code looked before this helper existed.
#define writeObjectId(objectId) do { if (selectionEnabled != 0) outObjectId = (objectId); } while(false)

#endif
