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
    vec4 uViewUp;
    vec4 uInteriorSunDir;
    SceneExteriorLight extLight;
};

struct InteriorLightParam {
    vec4 uInteriorAmbientColorAndApplyInteriorLight;
    vec4 uInteriorDirectColorAndApplyExteriorLight;
};

vec3 calcLight(
    vec3 matDiffuse,
    vec3 vNormal,
    bool applyLight,
    float interiorExteriorBlend,
    readonly SceneWideParams sceneParams,
    readonly InteriorLightParam intLight,
    readonly vec3 accumLight, const vec3 precomputedLight, const vec3 specular,
    readonly const vec3 emissive) {

    vec3 currColor;
    vec3 localDiffuse = accumLight;

    if (!applyLight) {
        return matDiffuse;
    }

    vec3 lDiffuse = vec3(0.0, 0.0, 0.0);
    vec3 normalizedN = normalize(vNormal);

    if (intLight.uInteriorDirectColorAndApplyExteriorLight.w > 0) {
        float nDotL = clamp(dot(normalizedN, -(sceneParams.extLight.uExteriorDirectColorDir.xyz)), 0.0, 1.0);
        float nDotUp = dot(normalizedN, sceneParams.uViewUp.xyz);

        vec3 adjAmbient =       (sceneParams.extLight.uExteriorAmbientColor.rgb         + precomputedLight);
        vec3 adjHorizAmbient =  (sceneParams.extLight.uExteriorHorizontAmbientColor.rgb + precomputedLight);
        vec3 adjGroundAmbient = (sceneParams.extLight.uExteriorGroundAmbientColor.rgb   + precomputedLight);

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
        currColor = mix(groundColor, skyColor, vec3((0.5 + (0.5 * nDotL))));
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
    vec3 linearDiffTerm = (matDiffuse * matDiffuse) * localDiffuse;

    //Specular term
    vec3 specTerm = specular;
    //Emission term
    vec3 emTerm = emissive;


    return sqrt(gammaDiffTerm*gammaDiffTerm + linearDiffTerm) + specTerm + emTerm;


//    vec3 normalDirection = normalize(vNormal);
//
//    vec3 viewDirection = normalize(_WorldSpaceCameraPos - vec3(position));
//    vec3 lightDirection;
//    float attenuation;
//
//    if (0.0 == _WorldSpaceLightPos0.w) // directional light?
//    {
//        attenuation = 1.0; // no attenuation
//        lightDirection = normalize(vec3(_WorldSpaceLightPos0));
//    }
//    else // point or spot light
//    {
//        vec3 vertexToLightSource =
//        vec3(_WorldSpaceLightPos0 - position);
//        float distance = length(vertexToLightSource);
//        attenuation = 1.0 / distance; // linear attenuation
//        lightDirection = normalize(vertexToLightSource);
//    }
//
//    vec4 fragmentColor = vec4(0.0, 0.0, 0.0, 0.0);
//    if (dot(normalDirection, lightDirection) > 0.0
//    // light source on the right side?
//        && attenuation *  pow(max(0.0, dot(reflect(-lightDirection, normalDirection), viewDirection)), _Shininess) > 0.5)
//    // more than half highlight intensity?
//    {
//        fragmentColor = vec4(_LightColor0.rgb, 1.0) * _SpecColor;
//    }

}