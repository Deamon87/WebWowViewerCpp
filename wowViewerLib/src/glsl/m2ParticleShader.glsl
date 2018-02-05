#ifdef COMPILING_VS
attribute vec3 aPosition;
attribute vec4 aColor;
attribute float alpha;
attribute vec2 aTexcoord0;
attribute vec2 aTexcoord1;
attribute vec2 aTexcoord2;


varying vec4 vColor;
varying vec2 vTexcoord0;
varying vec2 vTexcoord1;
varying vec2 vTexcoord2;


uniform mat4 uPMatrix;
void main() {
    vec4 aPositionVec4 = vec4(aPosition, 1);

    vColor = aColor;
    vTexcoord0 = aTexcoord0;
    vTexcoord1 = aTexcoord1;
    vTexcoord2 = aTexcoord2;
    gl_Position = uPMatrix * aPositionVec4;
}
#endif //COMPILING_VS

#ifdef COMPILING_FS
varying vec4 vColor;
varying vec2 vTexcoord0;
varying vec2 vTexcoord1;
varying vec2 vTexcoord2;

uniform float uAlphaTest;

uniform sampler2D uTexture;
uniform sampler2D uTexture2;
uniform sampler2D uTexture3;
void main() {
    vec4 tex = texture2D(uTexture, vTexcoord0).rgba;
    vec4 tex2 = texture2D(uTexture2, vTexcoord1).rgba;
    vec4 tex3 = texture2D(uTexture3, vTexcoord2).rgba;

    vec4 finalColor = vec4((tex ).rgb, tex.a );

    if(finalColor.a < uAlphaTest)
        discard;

    gl_FragColor.rgba = finalColor ;
}
#endif //COMPILING_FS