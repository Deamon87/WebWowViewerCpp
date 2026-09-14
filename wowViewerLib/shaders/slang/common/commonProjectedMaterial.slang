#ifndef COMMON_PROJECTED_MATERIAL_SLANG
#define COMMON_PROJECTED_MATERIAL_SLANG

import glsl;

void calcProjectedMaterial(const in int uPixelShader,
    in sampler2D texSampler1,
    in sampler2D texSampler2,
    in sampler2D texSampler3,
    const in vec2 texCoordinates,
    const in mat4 textMat0,
    const in mat4 textMat1,
    const in vec2 gradBase,
    out vec3 matDiffuse,
    out float finalOpacity
) {
    switch (uPixelShader) {
        case 0 : //Opaque_Single_Texture
        {
            vec4 tex0 = textureLod(texSampler1, texCoordinates, 0);
            matDiffuse = tex0.xyz;
            finalOpacity = tex0.a;
        }
        break;
        case 1 : //Two_Texture
        {
            vec2 tc0 = (textMat0 * vec4(texCoordinates.xy, 0, 1)).xy;
            vec2 tc1 = (textMat1 * vec4(texCoordinates.xy, 0, 1)).xy;

            vec4 tex0 = textureGrad(texSampler1, tc0, dFdx(gradBase), dFdy(gradBase));
            vec4 tex1 = textureGrad(texSampler2, tc1, dFdx(gradBase), dFdy(gradBase));
            vec4 textureMod = tex0 * tex1;
            matDiffuse = 2 * textureMod.xyz;
            finalOpacity = 2 * textureMod.a;
        }
        break;
        case 2 : //Three_Texture
        {
            vec2 tc0 = (textMat0 * vec4(texCoordinates.xy, 0, 1)).xy;
            vec2 tc1 = (textMat1 * vec4(texCoordinates.xy, 0, 1)).xy;
            vec2 tc2 = texCoordinates.xy;

            vec4 tex0 = textureGrad(texSampler1, tc0, dFdx(gradBase), dFdy(gradBase));
            vec4 tex1 = textureGrad(texSampler2, tc1, dFdx(gradBase), dFdy(gradBase));
            vec4 tex2 = texture(texSampler3, tc2);
            vec4 textureMod = tex0 * tex1 * tex2;
            matDiffuse = 4 * textureMod.xyz;
            finalOpacity = 4 * textureMod.a;
        }
        break;
        case 4 : //Add_Single_Texture
        {
            vec4 tex0 = texture(texSampler1, texCoordinates);
            matDiffuse = tex0.xyz;
            finalOpacity = tex0.a;
        }
        break;
    }
}

#endif //COMMON_PROJECTED_MATERIAL_SLANG
