#version 450

precision highp float;
layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vTexcoord0;
layout(location = 2) in vec2 vTexcoord1;
layout(location = 3) in vec2 vTexcoord2;

//Individual meshes
layout(std140, binding=4) uniform meshWideBlockPS {
    vec4 uAlphaTestv;
    ivec4 uPixelShaderv;
};

layout(set=1,binding=5) uniform sampler2D uTexture;
layout(set=1,binding=6) uniform sampler2D uTexture2;
layout(set=1,binding=7) uniform sampler2D uTexture3;

layout(location = 0) out vec4 outputColor;

void main() {
    vec4 tex = texture(uTexture, vTexcoord0).rgba;
    vec4 tex2 = texture(uTexture2, vTexcoord1).rgba;
    vec4 tex3 = texture(uTexture3, vTexcoord2).rgba;

    float uAlphaTest = uAlphaTestv.x;

    if(tex.a < uAlphaTest)
        discard;

    vec4 finalColor = vec4((tex * vColor ).rgb, tex.a*vColor.a );
    int uNonOptPixelShader = uPixelShaderv.x;
    if (uNonOptPixelShader == 0) { //particle_mod
        vec3 matDiffuse = vColor.xyz * tex.rgb;

        finalColor = vec4(matDiffuse.rgb, tex.a*vColor.a);
    } else if (uNonOptPixelShader == 1) {//particle_2colortex_3alphatex
        vec4 textureMod = tex*tex2;
        float texAlpha = (textureMod.w * tex3.w);
        float opacity = texAlpha*vColor.a;


        vec3 matDiffuse = vColor.xyz * textureMod.rgb;
        finalColor = vec4(matDiffuse.rgb, opacity);
    } else if (uNonOptPixelShader == 2) { //particle_3colortex_3alphatex
        vec4 textureMod = tex*tex2*tex3;
        float texAlpha = (textureMod.w);
        float opacity = texAlpha*vColor.a;


        vec3 matDiffuse = vColor.xyz * textureMod.rgb;
        finalColor = vec4(matDiffuse.rgb, opacity);
    } else if (uNonOptPixelShader == 3) { //Particle_3ColorTex_3AlphaTex_UV
        //TODO: incorrect implementation, because the original shader is too complicated
        vec4 textureMod = tex*tex2*tex3;
        float texAlpha = (textureMod.w);
        float opacity = texAlpha*vColor.a;

        vec3 matDiffuse = vColor.xyz * textureMod.rgb;
        finalColor = vec4(matDiffuse.rgb, opacity);
    } else if (uNonOptPixelShader == 4) { //Refraction
        //TODO:!
    }

    if(finalColor.a < uAlphaTest)
        discard;

    outputColor.rgba = finalColor ;
}
