#ifndef COMMON_LIGHT_FUNCTIONS
#define COMMON_LIGHT_FUNCTIONS

struct LocalLight
{
    vec4 innerColor;
    vec4 outerColor;
    vec4 position;
    vec4 attenuation;
    vec4 blendParams;
};

struct SpotLight
{
//    vec4 rotQuaternion;
    mat4 lightModelMat;
    vec4 spotLightLen;
    vec4 colorAndFalloff;
    vec4 positionAndcosInnerAngle;
    vec4 attenuationAndcosOuterAngle;
    vec4 directionAndcosAngleDiff;
    vec4 interior;
};

struct SceneExteriorLight {
     vec4 uExteriorAmbientColor;
     vec4 uExteriorHorizontAmbientColor;
     vec4 uExteriorGroundAmbientColor;
     vec4 uExteriorDirectColor;
     vec4 uExteriorDirectColorDir;
     vec4 adtSpecMult_fogCount;
 };

struct SceneWideParams {
    mat4 uLookAtMat;
    mat4 uPMatrix;
    mat4 uInvLookAtMat;
    mat4 uInvPMatrix;
    vec4 uViewUpSceneTime;
    vec4 uInteriorSunDir;
    vec4 uSceneSize_DisableLightBuffer;

    vec4 closeRiverColor_shallowAlpha;
    vec4 farRiverColor_deepAlpha;
    vec4 closeOceanColor_shallowAlpha;
    vec4 farOceanColor_deepAlpha;
    SceneExteriorLight extLight;
};

struct InteriorLightParam {
    vec4 uInteriorAmbientColorAndInteriorExteriorBlend;
    vec4 uInteriorGroundAmbientColor;
    vec4 uInteriorHorizontAmbientColor;
    vec4 uInteriorDirectColor;
    vec4 uPersonalInteriorSunDirAndApplyPersonalSunDir;
};

vec3 Slerp(vec3 p0, vec3 p1, float t)
{
    float dotp = dot(normalize(p0), normalize(p1));
    if ((dotp > 0.9999) || (dotp<-0.9999))
    {
        if (t<=0.5)
            return p0;

        return p1;
    }
    float theta = acos(dotp);
    vec3 P = ((p0*sin((1.0-t)*theta) + p1*sin(t*theta)) / sin(theta));

    return P;
}


vec3 applyAndMixAmbients(vec3 ambient, vec3 horizAmbient, vec3 groundAmbient, vec3 precomputedLight, float nDotL, float nDotUp) {
    vec3 currColor = vec3(0.0, 0.0, 0.0);

    vec3 adjAmbient =       (ambient.rgb       + precomputedLight);
    vec3 adjHorizAmbient =  (horizAmbient.rgb  + precomputedLight);
    vec3 adjGroundAmbient = (groundAmbient.rgb + precomputedLight);

    if ((nDotUp >= 0.0))
    {
        currColor = mix(adjHorizAmbient, adjAmbient, vec3(nDotUp));
    }
    else
    {
        currColor= mix(adjHorizAmbient, adjGroundAmbient, vec3(-(nDotUp)));
    }

    vec3 skyColor = (currColor * 1.10000002);
    vec3 groundColor = (currColor* 0.699999988);

    currColor = mix(groundColor, skyColor, (0.5f + vec3(0.5f * nDotL)));

    return currColor;
}

vec3 calcLight(
    const in vec3 matDiffuse,
    const in vec3 vNormal,
    const in bool applyLight,
    const in SceneWideParams sceneParams,
    const in InteriorLightParam intLight,
    const in vec3 accumLight, const in vec3 precomputedLight, const in vec3 specular,
    const in vec3 emissive,
    const in float ao
) {

    vec3 result = matDiffuse;
    if (applyLight) {
        vec3 currColor = vec3(0.0, 0.0, 0.0);
        vec3 lDiffuse = vec3(0.0, 0.0, 0.0);
        vec3 normalizedN = normalize(vNormal);
        float nDotUp = dot(normalizedN, normalize(sceneParams.uViewUpSceneTime.xyz));


        float interiorExteriorBlend = intLight.uInteriorAmbientColorAndInteriorExteriorBlend.w;
        if (interiorExteriorBlend > 0) {
            float nDotL = clamp(dot(normalizedN, normalize(-(sceneParams.extLight.uExteriorDirectColorDir.xyz))), 0.0, 1.0);

            currColor = applyAndMixAmbients(
                sceneParams.extLight.uExteriorAmbientColor.rgb,
                sceneParams.extLight.uExteriorHorizontAmbientColor.rgb,
                sceneParams.extLight.uExteriorGroundAmbientColor.rgb,
                precomputedLight,
                nDotL,
                nDotUp
            );

            lDiffuse = (sceneParams.extLight.uExteriorDirectColor.xyz * nDotL);
        }
        if (interiorExteriorBlend < 1.0) {
            vec3 interiorSunDir = mix(
                sceneParams.uInteriorSunDir.xyz,
                intLight.uPersonalInteriorSunDirAndApplyPersonalSunDir.xyz,
                intLight.uPersonalInteriorSunDirAndApplyPersonalSunDir.w);

            float nDotL = clamp(dot(normalizedN, interiorSunDir), 0.0, 1.0);

            vec3 interiorCurrColor = applyAndMixAmbients(
                intLight.uInteriorAmbientColorAndInteriorExteriorBlend.rgb,
                intLight.uInteriorHorizontAmbientColor.rgb,
                intLight.uInteriorGroundAmbientColor.rgb,
                precomputedLight,
                nDotL,
                nDotUp
            );

            vec3 lDiffuseInterior = intLight.uInteriorDirectColor.xyz * nDotL;

            if (interiorExteriorBlend > 0) {
                lDiffuse = mix(lDiffuseInterior, lDiffuse, interiorExteriorBlend);
                currColor = mix(interiorCurrColor, currColor, interiorExteriorBlend);
            } else {
                lDiffuse = lDiffuseInterior;
                currColor = interiorCurrColor;
            }
        }

        vec3 gammaDiffTerm = matDiffuse * (currColor + lDiffuse);
        vec3 linearDiffTerm = (matDiffuse * matDiffuse) * accumLight;

//        //Specular term
//        vec3 specTerm = specular;
//        //Emission term
        const vec3 emTerm = emissive;
        result = sqrt(gammaDiffTerm*gammaDiffTerm + linearDiffTerm)  + emTerm;
    }

    return (result + specular) * ao;
}



vec3 calcSpec(const in float texAlpha) {
    return vec3(0.0f);
}

/*
//TODO: Restore this function some other time
vec3 calcSpec(const in float texAlpha,
              const in vec3 vertexPosInView,
              const in vec3 normalInView,
              const in SceneWideParams sceneParams,
              const in InteriorLightParam intLight,
              float exteriorInteriorBlend
    ) {
    vec3 normal = normalize(normalInView);

    vec3 sunDir = vec3(0);
    vec3 sunColor = vec3(0);

    if (intLight.uInteriorDirectColorAndApplyExteriorLight.w > 0) {
        sunDir = -(sceneParams.extLight.uExteriorDirectColorDir.xyz);
        sunColor = sceneParams.extLight.uExteriorDirectColor.xyz;
    }

    if (intLight.uInteriorAmbientColorAndApplyInteriorLight.w > 0) {
        sunDir = -(sceneParams.uInteriorSunDir.xyz);
        sunColor = intLight.uInteriorDirectColorAndApplyExteriorLight.xyz;

        if (intLight.uInteriorDirectColorAndApplyExteriorLight.w > 0) {
            sunDir = Slerp(sunDir, -(sceneParams.extLight.uExteriorDirectColorDir.xyz), exteriorInteriorBlend);
            sunColor = mix(sunColor, sceneParams.extLight.uExteriorDirectColor.xyz, exteriorInteriorBlend);
        }
    }

    vec3 t849 = normalize((sunDir + normalize(-(vertexPosInView.xyz))));
    float dirAtten_956 = clamp(dot(normal, sunDir), 0, 1);
    float spec = (1.25 * pow(clamp(dot(normal, t849), 0, 1), 8.0));
    vec3 specTerm = ((((vec3(mix(pow((1.0 - clamp(dot(sunDir, t849), 0, 1)), 5.0), 1.0, texAlpha)) * spec) * sunColor) * dirAtten_956));
    //    float distFade = clamp(((vPosition * pc_miscParams.z) + pc_miscParams.w), 0, 1);
    float distFade = 1.0;
    specTerm = (specTerm * distFade);
    return specTerm;
}
*/
#endif