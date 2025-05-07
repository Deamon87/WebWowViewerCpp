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


float gbuff_readZBuffer(vec2 fragCoords) {
    vec2 uv = fragCoords / screenSize.xy;

    vec3 viewNormal = texture(normalTex, uv).rgb * 2.0 - vec3(1.0);
    float sceneDepth = texture(depthTex, uv).r;

    float z = (sceneDepth - 0.06f) / (1.0f - 0.06f) ;

    return z;
}

vec3 gbuff_readNormal(vec2 fragCoords) {
    vec2 uv = fragCoords / screenSize.xy;
    vec3 normal = texture(normalTex, uv).rgb * 2.0 - vec3(1.0);

    return normal;
}

vec3 gbuff_getViewPos(vec2 fragCoords, in const mat4 invPMat4) {
    vec2 uv = fragCoords / screenSize.xy;

    float z = gbuff_readZBuffer(fragCoords);

    vec4 viewPos = invPMat4 * vec4(uv.xy * 2.0 - 1.0, z, 1.0);
    viewPos.xyz = viewPos.xyz / viewPos.w;

    return viewPos.xyz;
}



#endif //COMMON_GBUFFER_DATA