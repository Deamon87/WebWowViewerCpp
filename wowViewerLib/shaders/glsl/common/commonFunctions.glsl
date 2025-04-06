#ifndef COMMON_FUNCTION_GLSL
#define COMMON_FUNCTION_GLSL

vec2 posToTexCoord(const vec3 vertexPosInView, const vec3 normal){
    //Blizz seems to have vertex in view space as vector from "vertex to eye", while in this implementation, it's
    //vector from "eye to vertex". So the minus here is not needed
    vec3 viewVecNormalized = normalize(vertexPosInView.xyz);
//    vec3 viewVecNormalized = normalize(vertexPosInView.xyz);
    vec3 reflection = reflect(viewVecNormalized, normalize(normal));
    vec3 temp_657 = vec3(reflection.x, reflection.y, (reflection.z + 1.0));

    return (-(normalize(temp_657).xy * 0.5) + vec2(0.5));
}

float edgeScan(const vec3 position, const vec3 normal){
    //Blizz seems to have vertex in view space as vector from "vertex to eye", while in this implementation, it's
    //vector from "eye to vertex". So the minus here is not needed
//    float dotProductClamped = clamp(dot(-normalize(position),normal), 0.000000, 1.000000);
    float dotProductClamped = clamp(dot(normalize(position),normal), 0.000000, 1.000000);
    return clamp(2.700000 * dotProductClamped * dotProductClamped - 0.400000, 0.000000, 1.000000);
}

mat3 blizzTranspose(const mat4 value) {
    return mat3(
        value[0].xyz,
        value[1].xyz,
        value[2].xyz
    );
}

#ifdef FRAGMENT_SHADER
//From: https://pdfslide.tips/technology/shaderx5-26normalmappingwithoutprecomputedtangents-130318-1.html?page=14
mat3 contangent_frame(vec3 N, vec3 p, vec2 uv)
{
    // Get edge vectors of the pixel triangle
    vec3 dp1 = dFdx(p);
    vec3 dp2 = dFdy(p);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);
    // Solve the linear system
    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
    // Construct a scale-invariant frame
    float invmax = inversesqrt(max(dot(T,T), dot(B,B)));
    return mat3(T * invmax, B * invmax, N);
}
#else
//Temp implementation.
//TODO: add implementation for raytracing case
mat3 contangent_frame(vec3 N, vec3 p, vec2 uv) {
    return mat3(1.0);
}

#endif


#endif

