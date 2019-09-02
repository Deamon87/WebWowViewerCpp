#version 450

precision highp float;

uniform vec3 uColor;

void main() {
    vec4 finalColor = vec4(1.0, 1.0, 0.0, 1.0);

    gl_FragColor = finalColor;
}
