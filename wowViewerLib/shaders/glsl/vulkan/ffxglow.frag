#version 420

layout(location = 1) in vec2 texCoord;

layout(std140, binding=4) uniform meshWideBlockPS {
    vec4 blurAmount;
};

layout(set=1,binding=5) uniform sampler2D screenTex;
layout(set=1,binding=6) uniform sampler2D blurTex;

layout (location = 0) out vec4 Out_Color;

void main()
{
    vec4 screen = texture(screenTex, texCoord);
    vec3 blurred = texture(blurTex, texCoord).xyz;
    vec3 mixed = mix(screen.xyz, blurred, vec3(blurAmount.z));
    vec3 glow = ((blurred * blurred) * blurAmount.w);

    Out_Color = vec4(mixed.rgb + glow, screen.a);
}
