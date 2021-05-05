#version 450

layout (location = 0) in vec2 position;

layout(std140, binding=2) uniform meshWideBlockVS {
    vec4 uWidth_uHeight_uX_uY;
};

layout(location = 0) out vec2 texCoord;

void main() {
    float uWidth = uWidth_uHeight_uX_uY.x;
    float uHeight = uWidth_uHeight_uX_uY.y;
    float uX = uWidth_uHeight_uX_uY.z;
    float uY = uWidth_uHeight_uX_uY.w;

    //texCoord = texture;
    texCoord = position.xy * 0.5 + 0.5;

    //gl_Position = vec4(position, 0.0, 1.0);
    gl_Position = vec4(
        (((position.x + 1.0)/2.0) * uWidth + uX)*2.0 - 1.0,
        (((position.y + 1.0)/2.0) * uHeight + uY)*2.0 - 1.0,
        0.5,
        1.0);
}