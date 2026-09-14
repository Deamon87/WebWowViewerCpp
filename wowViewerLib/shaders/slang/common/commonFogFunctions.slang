#ifndef COMMON_FOG_FUNCTIONS_SLANG
#define COMMON_FOG_FUNCTIONS_SLANG

import glsl;

struct PSFog
{
    vec4 densityParams;
    vec4 classicFogParams;
    vec4 heightPlane;
    vec4 color_and_heightRate;
    vec4 heightDensity_and_endColor;
    vec4 sunAngle_and_sunColor;
    vec4 heightColor_and_endFogDistance;
    vec4 sunPercentage_sunFogStrength;
    vec4 sunDirection_and_fogZScalar;
    vec4 heightFogCoeff;
    vec4 mainFogCoeff;
    vec4 heightDensityFogCoeff;
    vec4 mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha;
    vec4 heightFogEndColor_fogStartOffset;
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

vec4 makeFog(const in PSFog fogData, in vec4 final, in vec3 vertexInViewSpace, in vec3 sunDirInViewSpace, in int blendMode) {
//    //The best solution so far is to not apply fog to blendmode GxBlend_BlendAdd

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
    float expFog = 1.0 / (exp((expMax * density)));
    float expFogHeight = 1.0 / (exp((expMax * l_heightDensity_and_endColor.x)));
    float height = (dot(l_heightPlane.xyz, vertexInViewSpace) + l_heightPlane.w);
    float heightFog = clamp((height * l_color_and_heightRate.w), 0.0, 1.0);
    float finalFog = mix(expFog, expFogHeight, heightFog);
    float endFadeFog = clamp((1.42857146 * (1.0 - (vLength / end))), 0.0, 1.0);

    float alpha = 1.0;
    if (blendMode == 13) {
        alpha = min(finalFog, endFadeFog);
    }

    vec3 endColor = validateFogColor(l_heightDensity_and_endColor.yzw, blendMode);
    vec4 l_heightColor_and_endFogDistance = fogData.heightColor_and_endFogDistance;

    float end2 = (vLength / l_heightColor_and_endFogDistance.w);
    float end2_cube = (end2 * (end2 * end2));
    vec3 heightColor = mix(validateFogColor(l_heightColor_and_endFogDistance.xyz, blendMode), endColor, vec3(clamp(end2, 0.0, 1.0)));
    vec3 fogFinal = mix(validateFogColor(l_color_and_heightRate.xyz, blendMode), endColor, vec3(clamp(end2_cube, 0.0, 1.0)));
    fogFinal = mix(fogFinal, heightColor, vec3(heightFog));

    float nDotSun = dot(normalize(vertexInViewSpace), sunDirInViewSpace.xyz);
    vec3 sunColor = mix(fogFinal, validateFogColor(fogData.sunAngle_and_sunColor.yzw, blendMode), vec3(fogData.sunPercentage_sunFogStrength.x));
    nDotSun = clamp((nDotSun - fogData.sunAngle_and_sunColor.x), 0.0, 1.0);
    if ((nDotSun > 0.0))
    {
        nDotSun = ((nDotSun * nDotSun) * nDotSun);
        fogFinal = mix(fogFinal, sunColor, vec3(nDotSun));
    }

    fogFinal = mix(fogFinal, final.xyz, vec3(min(finalFog, endFadeFog)));

    return vec4(fogFinal, final.a * alpha);
}

float saturatef(float x) {
    return clamp(x, 0.0f, 1.0f);
}

float computeFogPoly(vec4 coeff, float x) {
    float x2 = x * x;
    float x3 = x * x * x;
    return saturatef(
        coeff.x * x3 +
        coeff.y * x2 +
        coeff.z * x +
        coeff.w);
}

// Calculates classic fog (simple distance-based fog)
float calculateClassicFog(in PSFog fogData, float viewDistance) {
    vec4 params = fogData.classicFogParams;
    float enabled = params.x;
    float end = params.y;
    float endMinusStartInv = params.z;
    float rate = params.w;

    if (bool(enabled)) {
        float t = max(0.0f, (end - viewDistance) * endMinusStartInv);
        return saturatef(pow(t, rate));
    }

    return 0.0f;
}

// Calculates height-adjusted vertex position for fog calculations
vec3 calculateAdjustedViewPosition(in vec3 vertexInViewSpace, in vec3 viewSpaceUp, float fogZScalar) {
    float dotVal = dot(viewSpaceUp, vertexInViewSpace);

    if (dotVal > 0.0f) {
        float clampVal = clamp(fogZScalar, 0.0f, dotVal);
        return vertexInViewSpace - (viewSpaceUp * clampVal);
    } else {
        float clampVal = clamp(-fogZScalar, dotVal, 0.0f);
        return vertexInViewSpace - (viewSpaceUp * clampVal);
    }
}

// Calculates height fog factor
float calculateHeightFogFactor(in PSFog fogData, in vec3 vertexInViewSpace) {
    vec4 heightPlane = fogData.heightPlane;
    float heightRate = fogData.color_and_heightRate.w;

    float height = dot(heightPlane.xyz, vertexInViewSpace) + heightPlane.w;
    float heightFactor = 1.0f - saturatef(height * heightRate);

    // Apply height fog coefficient polynomial
    return 1.0f - computeFogPoly(fogData.heightFogCoeff, heightFactor);
}

// Calculates legacy exponential fog
float calculateLegacyFog(in PSFog fogData, float viewDistance, float heightFogFactor, float fogRateScalar) {
    vec4 densityParams = fogData.densityParams;
    float start = densityParams.x;
    float end = densityParams.y;
    float density = densityParams.z;
    float bias = densityParams.w;
    float heightDensity = fogData.heightDensity_and_endColor.x;

    // Calculate base fog
    float z = viewDistance - bias;
    float expMax = max(0.0f, z - start);

    // Calculate standard exponential fog
    float legacyFogResult = 1.0 / exp(expMax * density);

    // Calculate height-adjusted exponential fog
    float legacyExpFogHeight = 1.0 / exp(expMax * heightDensity);

    // Blend between standard and height-adjusted fog
    float blendedFog = mix(legacyFogResult, legacyExpFogHeight, heightFogFactor);

    // Apply fog rate scalar and distance fade
    float finalLegacyFog = 1.0f - ((1.0f - blendedFog) * fogRateScalar);
    float normalizedDistance = viewDistance / end;
    float endFadeFog = saturatef(1.42857146f * (1.0f - normalizedDistance));

    return min(finalLegacyFog, endFadeFog);
}

// Calculates artistic fog using polynomial curves
float calculateArtisticFog(in PSFog fogData, float viewDistance, float heightFogFactor, float fogRateScalar) {
    float mainFogCurveStartDist = fogData.mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha.y;
    float mainFogCurveEndDist = fogData.mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha.x;

    // Calculate normalized distances
    float artFogNormalizedDistance = saturatef(
        (viewDistance - mainFogCurveStartDist) / (mainFogCurveEndDist - mainFogCurveStartDist)
    );
    float engineFogNormalizedDistance = saturatef(viewDistance / mainFogCurveEndDist);

    // Calculate main fog using polynomial
    float mainFogResult = 1.0f - computeFogPoly(fogData.mainFogCoeff, artFogNormalizedDistance);

    // Calculate height-adjusted fog using polynomial
    float heightFogResult = 1.0f - computeFogPoly(fogData.heightDensityFogCoeff, artFogNormalizedDistance);

    // Blend between main and height-adjusted fog
    float blendedFog = mix(mainFogResult, heightFogResult, heightFogFactor);

    // Apply fog rate scalar
    float finalFog = 1.0f - ((1.0f - blendedFog) * fogRateScalar);

    // Calculate end distance blending
    float endPct = saturatef(fogData.densityParams.y / mainFogCurveEndDist);
    float finalFogBeginPct = endPct - 0.3;

    // Calculate fog values at blend boundaries
    float mainFogValueAtBlendStart = computeFogPoly(fogData.mainFogCoeff, finalFogBeginPct);
    float heightFogValueAtBlendStart = computeFogPoly(fogData.heightDensityFogCoeff, finalFogBeginPct);
    float fogValueAtBlendStart = mix(mainFogValueAtBlendStart, heightFogValueAtBlendStart, heightFogFactor);

    // Calculate blend factor and apply it
    float blendValue = saturatef((engineFogNormalizedDistance - finalFogBeginPct) / 0.3);
    float endFogValue = blendValue * (1.0f - fogValueAtBlendStart) + fogValueAtBlendStart;
    float endFogValue3 = saturatef(1.0f - endFogValue);

    float lerpedFog = mix(finalFog, endFogValue3, blendValue);
    return min(finalFog, lerpedFog);
}

// Calculate fog color including sun effects
vec3 calculateFogColor(in PSFog fogData, in vec3 viewDirection, float heightFogFactor, float viewDistance, int blendMode) {
    // Calculate distance-based color transition
    float distanceFactor = saturatef(
        (viewDistance - fogData.heightFogEndColor_fogStartOffset.w) /
        fogData.heightColor_and_endFogDistance.w
    );
    float cubicDistanceFactor = distanceFactor * distanceFactor * distanceFactor;

    // Calculate base fog color
    vec3 baseFogColor = mix(
        validateFogColor(fogData.color_and_heightRate.xyz, blendMode),
        validateFogColor(fogData.heightDensity_and_endColor.yzw, blendMode),
        vec3(saturatef(cubicDistanceFactor))
    );

    // Calculate height-based fog color
    vec3 heightFogColor = mix(
        validateFogColor(fogData.heightColor_and_endFogDistance.xyz, blendMode),
        validateFogColor(fogData.heightFogEndColor_fogStartOffset.xyz, blendMode),
        vec3(distanceFactor)
    );

    // Blend base and height fog colors
    vec3 blendedFogColor = mix(baseFogColor, heightFogColor, vec3(heightFogFactor));

    // Apply sun scattering effects
    float sunAngle = fogData.sunAngle_and_sunColor.x;
    float nDotSun = saturatef(dot(viewDirection, fogData.sunDirection_and_fogZScalar.xyz));

    // Calculate sun-affected color
    vec3 sunColor = mix(
        blendedFogColor,
        validateFogColor(fogData.sunAngle_and_sunColor.yzw, blendMode),
        vec3(fogData.sunPercentage_sunFogStrength.x)
    );

    // Apply sun angle effect
    float sunFactor = saturatef(nDotSun - sunAngle);
    if (sunFactor > 0.0f) {
        float normalizedSunFactor = ((1.0f / (1.0f - sunAngle)) * sunFactor);
        float cubicSunFactor = normalizedSunFactor * normalizedSunFactor * normalizedSunFactor;
        return mix(blendedFogColor, sunColor, vec3(cubicSunFactor));
    }

    return blendedFogColor;
}

// Main fog calculation function
vec4 makeFog2(const in PSFog fogData, in vec4 final, in vec3 uViewUp, in vec3 vertexInViewSpace,
              in vec3 sunDirInViewSpace, in int blendMode) {
    float fogRateScalar = 1.0f;
    bool t138 = false; // Strange parameter from materials

    // Calculate view distance
    float viewDistance = length(vertexInViewSpace);

    // First try classic fog calculation
    float fogFactor = calculateClassicFog(fogData, viewDistance);
    bool usingClassicFog = bool(fogData.classicFogParams.x);

    // If classic fog is not enabled, use advanced fog calculation
    float heightFogFactor = 0.0f;
    if (!usingClassicFog) {
        // Calculate adjusted view position for height-based fog
        vec3 adjustedViewPos = calculateAdjustedViewPosition(
            vertexInViewSpace,
            uViewUp,
            fogData.sunDirection_and_fogZScalar.w
        );

        float adjustedViewDistance = length(adjustedViewPos);

        // Calculate height fog factor
        heightFogFactor = calculateHeightFogFactor(fogData, vertexInViewSpace);

        float legacyFog = calculateLegacyFog(fogData, adjustedViewDistance, heightFogFactor, fogRateScalar);
        float artisticFog = calculateArtisticFog(fogData, adjustedViewDistance, heightFogFactor, fogRateScalar);

        // Blend between legacy and artistic fog
        float legacyFogBlend = fogData.mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha.z;
        fogFactor = mix(artisticFog, legacyFog, legacyFogBlend);
    }

    // Calculate view direction
    vec3 viewDirection = normalize(vertexInViewSpace);

    vec3 fogColor = calculateFogColor(fogData, viewDirection, heightFogFactor, viewDistance, blendMode);

    vec3 finalColor = mix(fogColor, final.xyz, vec3(fogFactor));
    float finalAlpha = t138 ? final.w * fogFactor : final.w;

    // Blend with fog intensity
    float fogBlendAlpha = fogData.mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha.w;
    return vec4(finalColor, finalAlpha) * fogBlendAlpha;
}



#endif
