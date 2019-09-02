#version 450

attribute vec4 a_position;
varying vec2 v_texcoord;

void main() {
      gl_Position = a_position;
      v_texcoord = a_position.xy * 0.5 + 0.5;
}