#ifndef DEFERRED_EXCERPT
#define DEFERRED_EXCERPT

//layout(location = 0) out vec4 outAlbedo;
//layout(location = 1) out vec4 outSpecular;
layout(location = 0) out vec4 outNormal;
layout(location = 1) out float outDepth;
//layout(location = 2) out vec4 outVPos;

vec4 pack_depth(const in float depth)
{
    const vec4 bit_shift = vec4(256.0*256.0*256.0, 256.0*256.0, 256.0, 1.0);
    const vec4 bit_mask  = vec4(0.0, 1.0/256.0, 1.0/256.0, 1.0/256.0);
    vec4 res = fract(depth * bit_shift);
    res -= res.xxyz * bit_mask;
    return res;
}

void writeGBuffer(vec3 albedo, vec3 normal, vec3 specular, vec3 vPos) {
//    outAlbedo = vec4(albedo, 0.0);
//    outNormal = vec4(normal, 0.0);
//    outSpecular = vec4(specular, 0.0);
    outNormal = vec4(normal * 0.5 + 0.5, 0.0);
    outDepth = gl_FragCoord.z;
//    outVPos = vec4(vPos, 0.0);
//    outDepth = pack_depth(scene.uPMatrix[3].z/(gl_FragCoord.z * -2.0 + 1.0 - scene.uPMatrix[2].z));
}

#endif //DEFERRED_EXCERPT