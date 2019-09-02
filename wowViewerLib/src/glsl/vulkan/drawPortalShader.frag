#version 450

precision highp float;

//Individual mesh
uniform vec4 uColor;

void main() {
    vec4 finalColor = uColor ;

    gl_FragColor = finalColor;
}
