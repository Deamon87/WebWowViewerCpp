#version 450

attribute vec2 position;
attribute vec2 texture;
varying vec2 v_texcoord;

void main() {
    v_texcoord = texture;
    gl_Position = vec4(position, 0.0, 1.0);
}
