#ifndef DEFERRED_EXCERPT
#define DEFERRED_EXCERPT

//layout(location = 0) out vec4 outAlbedo;
//layout(location = 1) out vec4 outSpecular;
layout(location = 0) out vec4 outNormal;
layout(location = 1) out float outDepth;

void writeGBuffer(vec3 albedo, vec3 normal, vec3 specular) {
//    outAlbedo = vec4(albedo, 0.0);
//    outNormal = vec4(normal, 0.0);
//    outSpecular = vec4(specular, 0.0);
    outNormal = vec4(normal * 0.5 + 0.5, 0.0);
    outDepth = gl_FragCoord.z;
}

#endif //DEFERRED_EXCERPT