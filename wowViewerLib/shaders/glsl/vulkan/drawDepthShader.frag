#version 450

precision highp float;
layout(binding=3) uniform sampler2D diffuse;

layout(std140, binding=2) uniform meshWideBlockPS {
    int drawDepth;
    float uFarPlane;
    float uNearPlane;
};

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

void main(void) {

    vec4 finalColor;
    if (drawDepth == 1) {
        float f = uFarPlane; //far plane
        float n = uNearPlane; //near plane
        float z = (2.0 * n) / (f + n - texture( diffuse, texCoord ).x * (f - n));

        finalColor = vec4(z,z,z, 255);

    } else {
        finalColor = vec4(texture( diffuse, texCoord ).rgb, 255);
    }
    fragColor = finalColor;
}
