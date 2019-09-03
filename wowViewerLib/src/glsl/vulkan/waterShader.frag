#version 450

precision highp float;

layout(location=0) in vec3 vPosition;

layout(binding=3) uniform sampler2D uTexture;

layout(location=1) out vec4 outputColor;

layout(std140, binding=0) uniform sceneWideBlockVSPS {
    mat4 uLookAtMat;
    mat4 uPMatrix;
};

//Individual meshes
layout(std140, binding=2) uniform meshWideBlockPS {
    int waterType;
};

void main() {
    if (waterType == 13) { // LIQUID_WMO_Water
        outputColor = vec4(0.0, 0, 0.3, 0.5);
    } else if (waterType == 14) { //LIQUID_WMO_Ocean
        outputColor = vec4(0, 0, 0.8, 0.8);
    } else if (waterType == 19) { //LIQUID_WMO_Magma
        outputColor = vec4(0.3, 0, 0, 0.5);
    } else if (waterType == 20) { //LIQUID_WMO_Slime
        outputColor = vec4(0.0, 0.5, 0, 0.5);
    } else {
        outputColor = vec4(0.5, 0.5, 0.5, 0.5);
    }
}
