vec2 posToTexCoord(vec3 cameraPoint, vec3 normal){
//    vec3 normPos = -normalize(cameraPoint.xyz);
//    vec3 normPos = cameraPoint.xyz;
//    vec3 reflection = reflect(normPos, normal);
//    return (normalize(vec3(reflection.r, reflection.g, reflection.b + 1.0)).rg * 0.5) + vec2(0.5);

    vec3 normPos_495 = normalize(cameraPoint.xyz);
    vec3 temp_500 = (normPos_495 - (normal * (2.0 * dot(normPos_495, normal))));
    vec3 temp_657 = vec3(temp_500.x, temp_500.y, (temp_500.z + 1.0));

    return ((normalize(temp_657).xy * 0.5) + vec2(0.5));
}

float edgeScan(vec3 position, vec3 normal){
    float dotProductClamped = clamp(dot(-normalize(position),normal), 0.000000, 1.000000);
    return clamp(2.700000 * dotProductClamped * dotProductClamped - 0.400000, 0.000000, 1.000000);
}

mat3 blizzTranspose(mat4 value) {
    return mat3(
        value[0].xyz,
        value[1].xyz,
        value[2].xyz
    );
}


vec3 calcLight(vec3 matDiffuse, bool applyLight, vec3 accumLight, vec3 precomputedLight) {
//    return matDiffuse;
    vec3 currColor;
    vec3 localDiffuse = accumLight;

    if (!applyLight) {
        return matDiffuse;
    }

    vec3 lDiffuse = vec3(0.0, 0.0, 0.0);
    vec3 normalizedN = normalize(vNormal);
    float nDotL = clamp(dot(normalizedN, -(uExteriorDirectColorDir.xyz)), 0.0, 1.0);
    float nDotUp = dot(normalizedN, uViewUp.xyz);

    vec3 adjAmbient =       (uExteriorAmbientColor.rgb         + precomputedLight);
    vec3 adjHorizAmbient =  (uExteriorHorizontAmbientColor.rgb + precomputedLight);
    vec3 adjGroundAmbient = (uExteriorGroundAmbientColor.rgb   + precomputedLight);

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


    lDiffuse = (uExteriorDirectColor.xyz * nDotL);
    currColor = mix(groundColor, skyColor, vec3((0.5 + (0.5 * nDotL))));

    vec3 gammaDiffTerm = matDiffuse * (currColor + lDiffuse);
    vec3 linearDiffTerm = (matDiffuse * matDiffuse) * localDiffuse;
    //Specular term
    vec3 specTerm = vec3(0,0,0);
    //Emission term
    vec3 emTerm = vec3(0,0,0);


    return sqrt(gammaDiffTerm*gammaDiffTerm + linearDiffTerm) + specTerm + emTerm;
}