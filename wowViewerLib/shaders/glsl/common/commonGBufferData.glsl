#ifndef COMMON_GBUFFER_DATA
#define COMMON_GBUFFER_DATA

#ifndef GBUFFER_SET_INDEX
#define GBUFFER_SET_INDEX 1 //default value
#endif

layout(set=GBUFFER_SET_INDEX, binding=0) uniform sampler2D depthTex;
layout(set=GBUFFER_SET_INDEX, binding=1) uniform sampler2D normalTex;
layout(std140, set=GBUFFER_SET_INDEX, binding=2) uniform screenData {
    vec4 screenSize; //xy
};

#endif //COMMON_GBUFFER_DATA