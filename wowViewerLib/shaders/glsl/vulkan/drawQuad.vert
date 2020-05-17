#version 450

//Taken from http://media.tojicode.com/webgl-samples/depth-texture.html
layout(location = 0) in vec2 position;
layout(location = 0) out vec2 texCoord;

layout(std140, binding=2) uniform meshWideBlockVS {
    float uWidth;
    float uHeight;
    float uX;
    float uY;
};


void main(void) {
    //texCoord = texture;
    texCoord = position.xy * 0.5 + 0.5;

    //gl_Position = vec4(position, 0.0, 1.0);
    gl_Position = vec4(
        (((position.x + 1.0)/2.0) * uWidth + uX)*2.0 - 1.0,
        (((position.y + 1.0)/2.0) * uHeight + uY)*2.0 - 1.0,
        0.0,
        1.0)  ;
}