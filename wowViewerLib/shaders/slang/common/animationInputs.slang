#ifndef ANIMATION_INPUTS_SLANG
#define ANIMATION_INPUTS_SLANG

import glsl;

#include "./animation.slang"

StructuredBuffer<vec3, ScalarDataLayout> vec3AnimVals;
StructuredBuffer<float, ScalarDataLayout> floatAnimVals;
StructuredBuffer<int, ScalarDataLayout> intAnimVals;
StructuredBuffer<M2PartTrackElement, ScalarDataLayout> timeAnimVals;

#endif //ANIMATION_INPUTS_SLANG