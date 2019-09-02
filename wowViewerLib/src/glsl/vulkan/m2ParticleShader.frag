#version 450

precision highp float;
in vec4 vColor;
in vec2 vTexcoord0;
in vec2 vTexcoord1;
in vec2 vTexcoord2;

//Individual meshes
layout(std140) uniform meshWideBlockPS {
    vec4 uAlphaTestv;
    ivec4 uPixelShaderv;
};

uniform sampler2D uTexture;
uniform sampler2D uTexture2;
uniform sampler2D uTexture3;

out vec4 outputColor;

void main() {
    vec4 tex = texture(uTexture, vTexcoord0).rgba;
    vec4 tex2 = texture(uTexture2, vTexcoord1).rgba;
    vec4 tex3 = texture(uTexture3, vTexcoord2).rgba;

    float uAlphaTest = uAlphaTestv.x;

    if(tex.a < uAlphaTest)
        discard;

    vec4 finalColor = vec4((tex * vColor ).rgb, tex.a*vColor.a );
    int uPixelShader = uPixelShaderv.x;
    if (uPixelShader == 0) { //particle_mod
        vec3 matDiffuse = vColor.xyz * tex.rgb;

        finalColor = vec4(matDiffuse.rgb, tex.a*vColor.a);
    } else if (uPixelShader == 1) {//particle_2colortex_3alphatex
        vec4 textureMod = tex*tex2;
        float texAlpha = (textureMod.w * tex3.w);
        float opacity = texAlpha*vColor.a;


        vec3 matDiffuse = vColor.xyz * textureMod.rgb;
        finalColor = vec4(matDiffuse.rgb, opacity);
    } else if (uPixelShader == 2) { //particle_3colortex_3alphatex
        vec4 textureMod = tex*tex2*tex3;
        float texAlpha = (textureMod.w);
        float opacity = texAlpha*vColor.a;


        vec3 matDiffuse = vColor.xyz * textureMod.rgb;
        finalColor = vec4(matDiffuse.rgb, opacity);
    } else if (uPixelShader == 3) { //Particle_3ColorTex_3AlphaTex_UV
        //TODO: incorrect implementation, because the original shader is too complicated
        vec4 textureMod = tex*tex2*tex3;
        float texAlpha = (textureMod.w);
        float opacity = texAlpha*vColor.a;

        vec3 matDiffuse = vColor.xyz * textureMod.rgb;
        finalColor = vec4(matDiffuse.rgb, opacity);
    } else if (uPixelShader == 4) { //Refraction
        //TODO:!
    }

    if(finalColor.a < uAlphaTest)
        discard;

    outputColor.rgba = finalColor ;
}
