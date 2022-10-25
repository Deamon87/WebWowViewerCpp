vec2 posToTexCoord(vec3 cameraPoint, vec3 normal){
//    vec3 normPos = -normalize(cameraPoint.xyz);
//    vec3 normPos = cameraPoint.xyz;
//    vec3 reflection = reflect(normPos, normal);
//    return (normalize(vec3(reflection.r, reflection.g, reflection.b + 1.0)).rg * 0.5) + vec2(0.5);

    vec3 viewVecNormalized = -normalize(cameraPoint.xyz);
    vec3 reflection = reflect(viewVecNormalized, normal);
    vec3 temp_657 = vec3(reflection.x, reflection.y, (reflection.z + 1.0));

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

