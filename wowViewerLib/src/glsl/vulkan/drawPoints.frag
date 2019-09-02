#version 450

precision highp float;

varying vec4 vPos;

uniform vec3 uColor;

void main() {
    gl_FragColor = vec4(uColor.rgb, 1.0);
}