#ifdef COMPILING_VS
precision highp float;
attribute vec3 aPosition;
attribute vec4 aColor;
attribute vec2 aTexcoord0;
attribute vec2 aTexcoord1;
attribute vec2 aTexcoord2;


varying vec4 vColor;
varying vec2 vTexcoord0;
varying vec2 vTexcoord1;
varying vec2 vTexcoord2;

//Whole model
uniform mat4 uPMatrix;
uniform mat4 uViewMatrix;
void main() {
    vec4 aPositionVec4 = vec4(aPosition, 1);

    vColor = aColor;
    vTexcoord0 = aTexcoord0;
    vTexcoord1 = aTexcoord1;
    vTexcoord2 = aTexcoord2;
    gl_Position = uPMatrix * uViewMatrix * aPositionVec4;
}
#endif //COMPILING_VS

#ifdef COMPILING_FS
precision highp float;
varying vec4 vColor;
varying vec2 vTexcoord0;
varying vec2 vTexcoord1;
varying vec2 vTexcoord2;

//Individual meshes
uniform float uAlphaTest;

uniform sampler2D uTexture;
uniform sampler2D uTexture2;
uniform sampler2D uTexture3;
uniform int uPixelShader;
void main() {
    vec4 tex = texture2D(uTexture, vTexcoord0).rgba;
    vec4 tex2 = texture2D(uTexture2, vTexcoord1).rgba;
    vec4 tex3 = texture2D(uTexture3, vTexcoord2).rgba;

    vec4 finalColor = vec4((tex * vColor ).rgb, tex.a*vColor.a );

    if (uPixelShader == 0) {//particle_2colortex_3alphatex

        vec4 textureMod = tex*tex2;
        float texAlpha = (textureMod.w * tex3.w);
        float opacity = texAlpha*vColor.a;

        finalColor = vec4(textureMod.rgb, opacity);
    } else if (uPixelShader == 1) { //particle_3colortex_3alphatex
        vec4 textureMod = tex*tex2*tex3;
        float texAlpha = (textureMod.w);
        float opacity = texAlpha*vColor.a;

        finalColor = vec4(textureMod.rgb, opacity);
    } else if (uPixelShader == 2) { //particle_3colortex_3alphatex_uv
        //TODO: incorrect implementation, because the original shader is too complicated
        vec4 textureMod = tex*tex2*tex3;
        float texAlpha = (textureMod.w);
        float opacity = texAlpha*vColor.a;

        finalColor = vec4(textureMod.rgb, opacity);
    } else if (uPixelShader == 3) { //particle_mod
        vec4 textureMod = tex*tex2*tex3;
        float texAlpha = (textureMod.w);
        float opacity = texAlpha*vColor.a;

        finalColor = vec4(textureMod.rgb, opacity);
    }

    if(finalColor.a < uAlphaTest)
        discard;

    gl_FragColor.rgba = finalColor ;
}
#endif //COMPILING_FS