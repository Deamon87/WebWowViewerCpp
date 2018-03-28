#ifdef COMPILING_VS
uniform float x;
uniform float y;
uniform float width;
uniform float height;

attribute vec2 aTextCoord;
varying vec2 vTextCoords;
void main() {
    vTextCoords = aTextCoord;


    vec2 pos = vec2(
        //(x + aTextCoord.x*width)/( (1024.0 - 529.0) /1024.0) - 1.0,
        (x + aTextCoord.x*width)*2.0 - 1.0,
        //(y + aTextCoord.y*height)/( (753.0) /1024.0) - 1.0
        (y + aTextCoord.y*height)*2.0 - 1.0
    );

    gl_Position = vec4(pos.x, pos.y, 0, 1);
}
#endif //COMPILING_VS
#ifdef COMPILING_FS
precision highp float;

varying vec2 vTextCoords;
uniform sampler2D uTexture;

void main() {
    vec4 alpha = texture2D( uTexture, vTextCoords.xy );
    gl_FragColor = alpha;
}
#endif //COMPILING_FS