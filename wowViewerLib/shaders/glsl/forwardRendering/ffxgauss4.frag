#version 450

precision highp float;
precision highp int;

layout(location = 0) in vec2 texCoord;
layout(location = 0) out vec4 out_result;


layout(std140, set=0, binding=1) uniform meshWideBlockPS {
vec4 texOffsetX;
vec4 texOffsetY;
};

layout(set=1,binding=0) uniform sampler2D texture0;

const float weight[4] = float[] (0.125, 0.375, 0.375, 0.125);

void main()
{
    vec3 result = vec3(0.0);
    result += texture(texture0, texCoord + vec2(texOffsetX.x, texOffsetY.x)).rgb * weight[0];
    result += texture(texture0, texCoord + vec2(texOffsetX.y, texOffsetY.y)).rgb * weight[1];
    result += texture(texture0, texCoord + vec2(texOffsetX.z, texOffsetY.z)).rgb * weight[2];
    result += texture(texture0, texCoord + vec2(texOffsetX.w, texOffsetY.w)).rgb * weight[3];
    out_result = vec4(result, 1.0);
}
