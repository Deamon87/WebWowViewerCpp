struct PSFog
{
    vec4 densityParams;
    vec4 heightPlane;
    vec4 color_and_heightRate;
    vec4 heightDensity_and_endColor;
    vec4 sunAngle_and_sunColor;
    vec4 heightColor_and_endFogDistance;
    vec4 sunPercentage;
};

const vec3 fogColors[14] = vec3[14](
    vec3(0.0, 0.0, 0.0), //0  GxBlend_Opaque
    vec3(0.0, 0.0, 0.0), //1  GxBlend_AlphaKey
    vec3(0.0, 0.0, 0.0), //2  GxBlend_Alpha
    vec3(0.0, 0.0, 0.0), //3  GxBlend_Add
    vec3(1.0, 1.0, 1.0), //4  GxBlend_Mod
    vec3(0.5, 0.5, 0.5), //5  GxBlend_Mod2x
    vec3(0.5, 0.5, 0.5), //6  GxBlend_ModAdd
    vec3(0.0, 0.0, 0.0), //7  GxBlend_InvSrcAlphaAdd
    vec3(0.0, 0.0, 0.0), //8  GxBlend_InvSrcAlphaOpaque
    vec3(0.5, 0.5, 0.5), //9  GxBlend_SrcAlphaOpaque
    vec3(0.0, 0.0, 0.0), //10 GxBlend_NoAlphaAdd
    vec3(0.0, 0.0, 0.0), //11 GxBlend_ConstantAlpha
    vec3(0.0, 0.0, 0.0), //12 GxBlend_Screen
    vec3(0.0, 0.0, 0.0)  //13 GxBlend_BlendAdd
);
const float fogMix[14] = float[14](
    0.0, //0  GxBlend_Opaque
    0.0, //1  GxBlend_AlphaKey
    0.0, //2  GxBlend_Alpha
    1.0, //3  GxBlend_Add
    1.0, //4  GxBlend_Mod
    1.0, //5  GxBlend_Mod2x
    0.0, //6  GxBlend_ModAdd
    0.0, //7  GxBlend_InvSrcAlphaAdd
    0.0, //8  GxBlend_InvSrcAlphaOpaque
    0.0, //9  GxBlend_SrcAlphaOpaque
    1.0, //10 GxBlend_NoAlphaAdd
    0.0, //11 GxBlend_ConstantAlpha
    0.0, //12 GxBlend_Screen
    1.0  //13 GxBlend_BlendAdd
);



vec3 validateFogColor(in vec3 fogColor, int blendMode) {
    return mix(fogColor, fogColors[blendMode], fogMix[blendMode]);
}

vec3 makeFog(const in PSFog fogData, in vec3 final, in vec3 vertexInViewSpace, in vec3 sunDirInViewSpace, in int blendMode) {
    vec4 l_densityParams = fogData.densityParams;
    vec4 l_heightPlane = fogData.heightPlane;
    vec4 l_color_and_heightRate = fogData.color_and_heightRate;
    vec4 l_heightDensity_and_endColor = fogData.heightDensity_and_endColor;
    float start = l_densityParams.x;
    float end = l_densityParams.y;
    float density = l_densityParams.z;
    float bias = l_densityParams.w;
    float vLength = length(vertexInViewSpace);
    float z = (vLength - bias);
    float expMax = max(0.0, (z - start));
    float expFog = 1 / (exp((expMax * density)));
    float expFogHeight = 1 / (exp((expMax * l_heightDensity_and_endColor.x)));
    float height = (dot(l_heightPlane.xyz, vertexInViewSpace) + l_heightPlane.w);
    float heightFog = clamp((height * l_color_and_heightRate.w), 0, 1);
    float finalFog = mix(expFog, expFogHeight, heightFog);
    float endFadeFog = clamp((1.42857146 * (1.0 - (vLength / end))), 0, 1);
    vec3 endColor = validateFogColor(l_heightDensity_and_endColor.yzw, blendMode);
    vec4 l_heightColor_and_endFogDistance = fogData.heightColor_and_endFogDistance;

    float end2 = (vLength / l_heightColor_and_endFogDistance.w);
    float end2_cube = (end2 * (end2 * end2));
    vec3 heightColor = mix(validateFogColor(l_heightColor_and_endFogDistance.xyz, blendMode), endColor, vec3(clamp(end2, 0, 1)));
    vec3 fogFinal = mix(validateFogColor(l_color_and_heightRate.xyz, blendMode), endColor, vec3(clamp(end2_cube, 0, 1)));
    fogFinal = mix(fogFinal, heightColor, vec3(heightFog));

    float nDotSun = dot(normalize(vertexInViewSpace), sunDirInViewSpace.xyz);
    vec3 sunColor = mix(fogFinal, validateFogColor(fogData.sunAngle_and_sunColor.yzw, blendMode), vec3(fogData.sunPercentage.x));
    nDotSun = clamp((nDotSun - fogData.sunAngle_and_sunColor.x), 0, 1);
    if ((nDotSun > 0.0))
    {
        nDotSun = ((nDotSun * nDotSun) * nDotSun);
        fogFinal = mix(fogFinal, sunColor, vec3(nDotSun));
    }

    fogFinal = mix(fogFinal, final.xyz, vec3(min(finalFog, endFadeFog)));

    return fogFinal;
}

