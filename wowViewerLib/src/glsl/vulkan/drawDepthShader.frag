#version 450

precision highp float;
uniform sampler2D diffuse;
uniform int drawDepth;
uniform float uFarPlane;
uniform float uNearPlane;

varying vec2 texCoord;

void main(void) {

    vec4 finalColor;
    if (drawDepth == 1) {
        float f = uFarPlane; //far plane
        float n = uNearPlane; //near plane
        float z = (2.0 * n) / (f + n - texture2D( diffuse, texCoord ).x * (f - n));

        finalColor = vec4(z,z,z, 255);

    } else {
        finalColor = vec4(texture2D( diffuse, texCoord ).rgb, 255);
    }
    gl_FragColor = finalColor;
}
