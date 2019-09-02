#version 450

precision highp float;

uniform vec3 uColor;

void main() {
    vec4 finalColor = vec4(1.0, 1.0, 1.0, 1.0);

    finalColor.a = 1.0; //do I really need it now?
    gl_FragColor = finalColor;
}
