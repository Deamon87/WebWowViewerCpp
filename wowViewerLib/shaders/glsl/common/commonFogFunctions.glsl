struct PSFog
{
    vec4 densityParams;
    vec4 classicFogParams;
    vec4 heightPlane;
    vec4 color_and_heightRate;
    vec4 heightDensity_and_endColor;
    vec4 sunAngle_and_sunColor;
    vec4 heightColor_and_endFogDistance;
    vec4 sunPercentage;
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
    vec3 sunColor = mix(fogFinal, validateFogColor(fogData.sunAngle_and_sunColor.yzw, blendMode), vec3(fogData.sunPercentage.x));
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

vec4 makeFog2(const in PSFog mainFogData[8], int fogCount, in vec4 final, in vec3 uViewUp, in vec3 vertexInViewSpaceMy, in vec3 sunDirInViewSpace, in int blendMode) {
    float fogRateScalar = 1.0f; //Unk stuff atm.
    bool t138 = false; //some strange parameter from materials
    vec3 vertexInViewSpace = vertexInViewSpaceMy;
    float vLength = length(vertexInViewSpace);
    float _retVal_739;
    bool _ret0_738;
    float _retVal_715 = 0.0f;
    float _retVal_913 = 0.0f;
    float heightFog_912 = 0.0f;

    vec4 outColor_713 = vec4(0.0); //accumulator


    for (int i = 0; i < fogCount; i++) {
        vec4 classicFogParams = mainFogData[i].classicFogParams;
        float classicFogEnabled = classicFogParams.x;
        float classicEnd = classicFogParams.y;
        float classicEndMinusStartInv = classicFogParams.z;
        float classicRate = classicFogParams.w;
        if (bool(classicFogEnabled))
        {
            // Block 726
            float t_729 = max(0.0f, ((classicEnd - vLength) * classicEndMinusStartInv));
            float pow_730 = pow(t_729, classicRate);
            float _retVal_731 = saturatef(pow_730);
            _retVal_739 = _retVal_731;
            _ret0_738 = true;
        }
        else
        {
            // Block 734
            _retVal_739 = _retVal_715;
            _ret0_738 = false;
        }
        // Block 737
        float _retVal_742 = _retVal_739;
        bool _ret0_741 = _ret0_738;
        if (!(_ret0_741))
        {
            // Block 743
            vec4 densityParams = mainFogData[i].densityParams;
            float start = densityParams.x;
            float end = densityParams.y;
            float density = densityParams.z;
            float bias = densityParams.w;
            float heightRate = mainFogData[i].color_and_heightRate.w;
            float heightDensity = mainFogData[i].heightDensity_and_endColor.x;
            float fogZScalar = mainFogData[i].sunDirection_and_fogZScalar.w;
            vec4 t755 = mainFogData[i].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha;
            float mainFogCurveEndDist = t755.x;
            float mainFogCurveStartDist = t755.y;
            float legacyFogBlend = t755.z;
            vec3 viewSpaceUp = uViewUp;
            float dotVal_763 = dot(viewSpaceUp, vertexInViewSpace);
            vec3 adjustedVpos_777;
            if (dotVal_763 > 0.0f)
            {
                // Block 765
                float clamp_766 = clamp(fogZScalar, 0.0f, dotVal_763);
                vec3 adjustedVpos_768 = (vertexInViewSpace - (viewSpaceUp * clamp_766));
                adjustedVpos_777 = adjustedVpos_768;
            }
            else
            {
                // Block 770
                float clamp_772 = clamp(-(fogZScalar), dotVal_763, 0.0f);
                vec3 adjustedVpos_774 = (vertexInViewSpace - (viewSpaceUp * clamp_772));
                adjustedVpos_777 = adjustedVpos_774;
            }
            // Block 776
            vec3 adjustedVpos_905 = adjustedVpos_777;
            float vLength_778 = length(adjustedVpos_905);
            float z_779 = (vLength_778 - bias);
            float expMax_781 = max(0.0f, (z_779 - start));
            float exp_783 = exp((expMax_781 * heightDensity));
            float legacyExpFogHeight_784 = (1.0 / exp_783);
            vec4 t785 = mainFogData[i].heightPlane;
            float dot_787 = dot(t785.xyz, vertexInViewSpace);
            float height_789 = (dot_787 + t785.w);
            float saturate_791 = saturatef((height_789 * heightRate));
            float heightFog_792 = (1.0f - saturate_791);
            vec4 t793 = mainFogData[i].heightFogCoeff;
            float xSqrd_794 = (heightFog_792 * heightFog_792);
            float xCubed_795 = (xSqrd_794 * heightFog_792);
            float saturate_806 = saturatef(((((t793.x * xCubed_795) + (t793.y * xSqrd_794)) + (t793.z * heightFog_792)) + t793.w));
            float heightFog_807 = (1.0f - saturate_806);
            float normalizedDistance_808 = (vLength_778 / end);
            float exp_810 = exp((expMax_781 * density));
            float legacyFogResult_811 = (1.0 / exp_810);
            float lerp_812 = mix(legacyFogResult_811, legacyExpFogHeight_784, heightFog_807);
            float finalLegacyFog_815 = (1.0f - ((1.0f - lerp_812) * fogRateScalar));
            float endFadeFog_818 = saturatef((1.42857146f * (1.0f - normalizedDistance_808)));
            float finalLegacyFog_819 = min(finalLegacyFog_815, endFadeFog_818);
            float artFogNormalizedDistance_823 = saturatef(((vLength_778 - mainFogCurveStartDist) / (mainFogCurveEndDist - mainFogCurveStartDist)));
            float engineFogNormalizedDistance_825 = saturatef((vLength_778 / mainFogCurveEndDist));
            vec4 t826 = mainFogData[i].mainFogCoeff;
            float xSqrd_827 = (artFogNormalizedDistance_823 * artFogNormalizedDistance_823);
            float xCubed_828 = (xSqrd_827 * artFogNormalizedDistance_823);
            float saturate_839 = saturatef(((((t826.x * xCubed_828) + (t826.y * xSqrd_827)) + (t826.z * artFogNormalizedDistance_823)) + t826.w));
            float fogResult_841 = saturatef((1.0f - saturate_839));
            vec4 t842 = mainFogData[i].heightDensityFogCoeff;
            float xSqrd_843 = (artFogNormalizedDistance_823 * artFogNormalizedDistance_823);
            float xCubed_844 = (xSqrd_843 * artFogNormalizedDistance_823);
            float saturate_855 = saturatef(((((t842.x * xCubed_844) + (t842.y * xSqrd_843)) + (t842.z * artFogNormalizedDistance_823)) + t842.w));
            float heightFogResult_857 = saturatef((1.0f - saturate_855));
            float lerp_858 = mix(fogResult_841, heightFogResult_857, heightFog_807);
            float finalFog_861 = (1.0f - ((1.0f - lerp_858) * fogRateScalar));
            float endPct_863 = saturatef((end / mainFogCurveEndDist));
            float finalFogBeginPct_864 = (endPct_863 - 0.300000012f);
            float xSqrd_865 = (finalFogBeginPct_864 * finalFogBeginPct_864);
            float xCubed_866 = (xSqrd_865 * finalFogBeginPct_864);
            float saturate_877 = saturatef(((((t826.x * xCubed_866) + (t826.y * xSqrd_865)) + (t826.z * finalFogBeginPct_864)) + t826.w));
            float mainFogValueAtBlendStart_878 = saturatef(saturate_877);
            float xSqrd_879 = (finalFogBeginPct_864 * finalFogBeginPct_864);
            float xCubed_880 = (xSqrd_879 * finalFogBeginPct_864);
            float saturate_891 = saturatef(((((t842.x * xCubed_880) + (t842.y * xSqrd_879)) + (t842.z * finalFogBeginPct_864)) + t842.w));
            float heightFogValueAtBlendStart_892 = saturatef(saturate_891);
            float fogValueAtBlendStart_893 = mix(mainFogValueAtBlendStart_878, heightFogValueAtBlendStart_892, heightFog_807);
            float blendValue_896 = saturatef(((engineFogNormalizedDistance_825 - finalFogBeginPct_864) / 0.300000012f));
            float endFogValue_898 = (blendValue_896 * (1.0f - fogValueAtBlendStart_893));
            float endFogValue_899 = (endFogValue_898 + fogValueAtBlendStart_893);
            float endFogValue_901 = saturatef((1.0f - endFogValue_899));
            float lerp_902 = mix(finalFog_861, endFogValue_901, blendValue_896);
            float finalFog_903 = min(finalFog_861, lerp_902);
            float _retVal_904 = mix(finalFog_903, finalLegacyFog_819, legacyFogBlend);
            _retVal_913 = _retVal_904;
            heightFog_912 = heightFog_807;
        }
        else
        {
            // Block 908
            _retVal_913 = _retVal_742;
            heightFog_912 = 0.0f;
        }
        // Block 911
        float _retVal_956 = _retVal_913;
        float heightFog_955 = heightFog_912;
        vec3 normalize_918 = normalize(vertexInViewSpace);
        float t924 = mainFogData[i].sunAngle_and_sunColor.x;
        float alpha_935 = final.w;
        float end2_938 = saturatef(((vLength - mainFogData[i].heightFogEndColor_fogStartOffset.w) / mainFogData[i].heightColor_and_endFogDistance.w));
        float end_940 = (end2_938 * (end2_938 * end2_938));
        vec3 heightColor_942 = mix(validateFogColor(mainFogData[0].heightColor_and_endFogDistance.xyz, blendMode),
            validateFogColor(mainFogData[0].heightFogEndColor_fogStartOffset.xyz, blendMode), vec3(end2_938));
        float saturate_943 = saturatef(end_940);
        vec3 fogFinal_945 = mix(validateFogColor(mainFogData[0].color_and_heightRate.xyz, blendMode),
            validateFogColor(mainFogData[0].heightDensity_and_endColor.yzw, blendMode), vec3(saturate_943));
        vec3 fogFinal_947 = mix(fogFinal_945, heightColor_942, vec3(heightFog_955));
        float dot_948 = dot(normalize_918, mainFogData[i].sunDirection_and_fogZScalar.xyz);
        float nDotSun_949 = saturatef(dot_948);
        vec3 sunColor_951 = mix(fogFinal_947, validateFogColor(mainFogData[0].sunAngle_and_sunColor.yzw, blendMode), vec3(mainFogData[i].sunPercentage.x));
        float nDotSun_953 = saturatef((nDotSun_949 - t924));
        vec3 fogFinal_969;
        if (nDotSun_953 > 0.0f)
        {
            // Block 957
            float nDotSun_960 = ((1.0f / (1.0f - t924)) * nDotSun_953);
            float nDotSun_962 = ((nDotSun_960 * nDotSun_960) * nDotSun_960);
            vec3 fogFinal_964 = mix(fogFinal_947, sunColor_951, vec3(nDotSun_962));
            fogFinal_969 = fogFinal_964;
        }
        else
        {
            // Block 966
            fogFinal_969 = fogFinal_947;
        }
        // Block 968
        vec3 fogFinal_971 = fogFinal_969;
        float alpha_978;
        if (bool(t138))
        {
            // Block 972
            float alpha_973 = (alpha_935 * _retVal_956);
            alpha_978 = alpha_973;
        }
        else
        {
            // Block 975
            alpha_978 = alpha_935;
        }
        // Block 977
        float alpha_988 = alpha_978;
        vec3 lerp_981 = mix(fogFinal_971, final.xyz, vec3(_retVal_956));
        vec4 t982 = vec4(lerp_981, alpha_988);

        vec4 outColor_986 = (outColor_713 + (t982 * mainFogData[i].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha.w));
        outColor_713 = outColor_986;
    }
    return outColor_713;
}

