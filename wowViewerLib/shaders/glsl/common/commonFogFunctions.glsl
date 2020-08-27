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

vec3 makeFog(const in PSFog fogData, in vec3 final, in vec3 vertexInViewSpace, in vec3 sunDirInViewSpace) {
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
    vec3 endColor = l_heightDensity_and_endColor.yzw;
    vec4 l_heightColor_and_endFogDistance = fogData.heightColor_and_endFogDistance;

    float end2 = (vLength / l_heightColor_and_endFogDistance.w);
    float end2_cube = (end2 * (end2 * end2));
    vec3 heightColor = mix(l_heightColor_and_endFogDistance.xyz, endColor, vec3(clamp(end2, 0, 1)));
    vec3 fogFinal = mix(l_color_and_heightRate.xyz, endColor, vec3(clamp(end2_cube, 0, 1)));
    fogFinal = mix(fogFinal, heightColor, vec3(heightFog));

    float nDotSun = dot(normalize(vertexInViewSpace), sunDirInViewSpace.xyz);
    vec3 sunColor = mix(fogFinal, fogData.sunAngle_and_sunColor.yzw, vec3(fogData.sunPercentage.x));
    nDotSun = clamp((nDotSun - fogData.sunAngle_and_sunColor.x), 0, 1);
    if ((nDotSun > 0.0))
    {
        nDotSun = ((nDotSun * nDotSun) * nDotSun);
        fogFinal = mix(fogFinal, sunColor, vec3(nDotSun));
    }

    fogFinal = mix(fogFinal, final.xyz, vec3(min(finalFog, endFadeFog)));

    return fogFinal;
}

