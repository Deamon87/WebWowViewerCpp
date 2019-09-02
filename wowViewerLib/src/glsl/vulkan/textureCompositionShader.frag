#version 450

precision highp float;

varying vec2 vTextCoords;
uniform sampler2D uTexture;

void main() {
    vec4 color = texture2D( uTexture, vTextCoords.xy );
    gl_FragColor = color;
}
