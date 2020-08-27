#version 450

layout (location = 0) in vec2 position;

layout(std140, binding=2) uniform meshWideBlockVS {
    float uWidth;
    float uHeight;
    float uX;
    float uY;
};

layout(location = 0) out vec2 texCoord;

void main() {
    //texCoord = texture;
    texCoord = position.xy * 0.5 + 0.5;

    //gl_Position = vec4(position, 0.0, 1.0);
    gl_Position = vec4(
        (((position.x + 1.0)/2.0) * uWidth + uX)*2.0 - 1.0,
        (((position.y + 1.0)/2.0) * uHeight + uY)*2.0 - 1.0,
        0.5,
        1.0);
}