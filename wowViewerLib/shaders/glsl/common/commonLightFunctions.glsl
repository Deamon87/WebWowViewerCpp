#ifndef COMMON_LIGHT_FUNCTIONS
#define COMMON_LIGHT_FUNCTIONS
struct LocalLight
{
    vec4 color;
    vec4 position;
    vec4 attenuation;
};

struct SceneExteriorLight {
     vec4 uExteriorAmbientColor;
     vec4 uExteriorHorizontAmbientColor;
     vec4 uExteriorGroundAmbientColor;
     vec4 uExteriorDirectColor;
     vec4 uExteriorDirectColorDir;
     vec4 adtSpecMult;
 };

struct SceneWideParams {
    mat4 uLookAtMat;
    mat4 uPMatrix;
    vec4 uViewUpSceneTime;
    vec4 uInteriorSunDir;
    SceneExteriorLight extLight;
};

struct InteriorLightParam {
    vec4 uInteriorAmbientColorAndApplyInteriorLight;
    vec4 uInteriorDirectColorAndApplyExteriorLight;
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
    vec3 P = ((p0*sin((1-t)*theta) + p1*sin(t*theta)) / sin(theta));

    return P;
}

vec3 calcLight(
    const in vec3 matDiffuse,
    const in vec3 vNormal,
    const in bool applyLight,
    const in float interiorExteriorBlend,
    const in SceneWideParams sceneParams,
    const in InteriorLightParam intLight,
    const in vec3 accumLight, const in vec3 precomputedLight, const in vec3 specular,
    const in vec3 emissive) {

    vec3 result = matDiffuse;
    if (applyLight) {
        vec3 currColor = vec3(0.0, 0.0, 0.0);
        vec3 lDiffuse = vec3(0.0, 0.0, 0.0);
        vec3 normalizedN = normalize(vNormal);

        if (intLight.uInteriorDirectColorAndApplyExteriorLight.w > 0) {
            float nDotL = clamp(dot(normalizedN, normalize(-(sceneParams.extLight.uExteriorDirectColorDir.xyz))), 0.0, 1.0);
            float nDotUp = dot(normalizedN, normalize(sceneParams.uViewUpSceneTime.xyz));

            vec3 adjAmbient =       (sceneParams.extLight.uExteriorAmbientColor.rgb          + precomputedLight);
            vec3 adjHorizAmbient =  (sceneParams.extLight.uExteriorHorizontAmbientColor.rgb  + precomputedLight);
            vec3 adjGroundAmbient = (sceneParams.extLight.uExteriorGroundAmbientColor.rgb    + precomputedLight);

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


            lDiffuse = (sceneParams.extLight.uExteriorDirectColor.xyz * nDotL);
            currColor = mix(groundColor, skyColor, (0.5f + vec3(0.5f * nDotL)));
        }
        if (intLight.uInteriorAmbientColorAndApplyInteriorLight.w > 0) {
            float nDotL = clamp(dot(normalizedN, -(sceneParams.uInteriorSunDir.xyz)), 0.0, 1.0);
            vec3 lDiffuseInterior = intLight.uInteriorDirectColorAndApplyExteriorLight.xyz * nDotL;
            vec3 interiorAmbient = intLight.uInteriorAmbientColorAndApplyInteriorLight.xyz + precomputedLight;

            if (intLight.uInteriorDirectColorAndApplyExteriorLight.w > 0) {
                lDiffuse = mix(lDiffuseInterior, lDiffuse, interiorExteriorBlend);
                currColor = mix(interiorAmbient, currColor, interiorExteriorBlend);
            } else {
                lDiffuse = lDiffuseInterior;
                currColor = interiorAmbient;
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

    return result + specular;
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