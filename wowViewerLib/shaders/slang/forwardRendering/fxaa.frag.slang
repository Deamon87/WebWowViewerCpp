#version 450

import glsl;

precision highp float;
precision highp int;

layout(location = 0) in vec2 texCoord;

layout(std140, binding=1) uniform meshWideBlockPS {
    vec4 screenSize; // .xy = resolution, .zw = 1.0 / resolution
};

layout(set=1, binding=0) uniform sampler2D screenTex;

layout(location = 0) out vec4 outColor;

// FXAA (Timothy Lottes, reduced/console form — the classic geeks3d GLSL port)
const float FXAA_EDGE_THRESHOLD_MIN = 1.0 / 32.0;
const float FXAA_EDGE_THRESHOLD     = 1.0 / 8.0;
const float FXAA_REDUCE_MUL         = 1.0 / 8.0;
const float FXAA_REDUCE_MIN         = 1.0 / 128.0;
const float FXAA_SPAN_MAX           = 8.0;

float fxaaLuma(vec3 rgb) {
    return dot(rgb, vec3(0.299, 0.587, 0.114));
}

[shader("pixel")]
void main() {
    vec2 rcpFrame = screenSize.zw;

    vec4 rgbaM  = texture(screenTex, texCoord);
    vec3 rgbN = textureOffset(screenTex, texCoord, ivec2( 0,-1)).rgb;
    vec3 rgbS = textureOffset(screenTex, texCoord, ivec2( 0, 1)).rgb;
    vec3 rgbW = textureOffset(screenTex, texCoord, ivec2(-1, 0)).rgb;
    vec3 rgbE = textureOffset(screenTex, texCoord, ivec2( 1, 0)).rgb;

    float lumaM = fxaaLuma(rgbaM.rgb);
    float lumaN = fxaaLuma(rgbN);
    float lumaS = fxaaLuma(rgbS);
    float lumaW = fxaaLuma(rgbW);
    float lumaE = fxaaLuma(rgbE);

    float lumaMin = min(lumaM, min(lumaN, min(lumaS, min(lumaW, lumaE))));
    float lumaMax = max(lumaM, max(lumaN, max(lumaS, max(lumaW, lumaE))));

    // Low local contrast: no edge here, keep the original texel
    if (lumaMax - lumaMin < max(FXAA_EDGE_THRESHOLD_MIN, lumaMax * FXAA_EDGE_THRESHOLD)) {
        outColor = rgbaM;
        return;
    }

    // Edge direction (perpendicular to the luma gradient)
    vec2 dir;
    dir.x = -((lumaN + lumaS) - (lumaW + lumaE));
    dir.y =  ((lumaW + lumaE) - (lumaN + lumaS));

    float dirReduce = max((lumaN + lumaS + lumaW + lumaE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(vec2(FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX), dir * rcpDirMin)) * rcpFrame;

    // Blend along the edge
    vec3 rgbA = 0.5 * (texture(screenTex, texCoord + dir * (1.0/3.0 - 0.5)).rgb +
                       texture(screenTex, texCoord + dir * (2.0/3.0 - 0.5)).rgb);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (texture(screenTex, texCoord + dir * -0.5).rgb +
                                     texture(screenTex, texCoord + dir *  0.5).rgb);

    // Reject the wider blend if it escaped the local luma range (keeps corners sharp)
    float lumaB = fxaaLuma(rgbB);
    outColor = vec4((lumaB < lumaMin || lumaB > lumaMax) ? rgbA : rgbB, rgbaM.a);
}
