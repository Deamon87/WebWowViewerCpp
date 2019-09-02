#version 450

precision highp float;
varying vec2 v_texcoord;
uniform sampler2D u_sampler;

void main() {
    float depth = texture2D(u_sampler, v_texcoord).x;
    float depth65535 = depth * 65535.0;
    float depth_high = floor(depth65535/256.0) / 255.0;
    float depth_low = mod(depth65535, 256.0) / 255.0;

    vec4 color = vec4(depth_high, depth_low, 0 ,0);

    gl_FragColor = color;
}
