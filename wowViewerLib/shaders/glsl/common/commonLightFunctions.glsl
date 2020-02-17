struct SceneExteriorLight {
     vec4 uExteriorAmbientColor;
     vec4 uExteriorHorizontAmbientColor;
     vec4 uExteriorGroundAmbientColor;
     vec4 uExteriorDirectColor;
     vec4 uExteriorDirectColorDir;
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
    readonly SceneWideParams sceneParams,
    readonly InteriorLightParam intLight,
    readonly vec3 accumLight, const vec3 precomputedLight) {

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
        vec3 lDiffuseInterior = intLight.uInteriorAmbientColorAndApplyInteriorLight.xyz * nDotL;

        if (intLight.uInteriorDirectColorAndApplyExteriorLight.w > 0) {
            lDiffuse = mix(lDiffuse, lDiffuseInterior, intLight.interiorExteriorBlend.x);
        } else {
            lDiffuse = lDiffuseInterior;
            currColor = intLight.uInteriorDirectColorAndApplyExteriorLight.xyz;
        }
    }

    vec3 gammaDiffTerm = matDiffuse * (currColor + lDiffuse);
    vec3 linearDiffTerm = (matDiffuse * matDiffuse) * localDiffuse;

    //Specular term
    vec3 specTerm = vec3(0,0,0);
    //Emission term
    vec3 emTerm = vec3(0,0,0);


    return sqrt(gammaDiffTerm*gammaDiffTerm + linearDiffTerm) + specTerm + emTerm;
}