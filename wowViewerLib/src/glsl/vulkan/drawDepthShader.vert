#version 450

//Taken from http://media.tojicode.com/webgl-samples/depth-texture.html
attribute vec2 position;
attribute vec2 texture;
varying vec2 texCoord;

uniform float uWidth;
uniform float uHeight;
uniform float uX;
uniform float uY;


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