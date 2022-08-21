#version 450


precision highp float;
precision highp int;

layout(location = 0) in vec4 a_position;
layout(location = 0) out vec2 v_texcoord;

void main() {
      gl_Position = a_position;
      v_texcoord = a_position.xy * 0.5 + 0.5;
}