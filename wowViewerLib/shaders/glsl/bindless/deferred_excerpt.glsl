#ifndef DEFERRED_EXCERPT
#define DEFERRED_EXCERPT

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outSpecular;
layout(location = 2) out vec4 outNormal;

void writeGBuffer(vec3 albedo, vec3 normal, vec3 specular) {
    outAlbedo = vec4(albedo, 0.0);
    outNormal = vec4(normal, 0.0);
    outSpecular = vec4(specular, 0.0);
}

#endif //DEFERRED_EXCERPT