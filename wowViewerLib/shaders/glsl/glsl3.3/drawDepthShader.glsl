//Taken from http://media.tojicode.com/webgl-samples/depth-texture.html

#ifdef COMPILING_VS
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
#endif //COMPILING_VS

#ifdef COMPILING_FS

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

#endif //COMPILING_FS