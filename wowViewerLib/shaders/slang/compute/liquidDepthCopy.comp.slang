import glsl;

// Copies a liquid depth texture to mip 0 of the liquid Hi-Z buffer (RG32F storage image).
// R channel = min depth, G channel = max depth.
// At mip 0, both are initialized to the same value.

[vk::push_constant]
ivec4 dimensions_pad; // x=width, y=height

Sampler2D depthSrc;

[vk::image_format("rg32f")]
RWTexture2D<float2> hiZDst;

[shader("compute")]
[numthreads(8, 8, 1)]
void main() {
    ivec2 coord = ivec2(gl_GlobalInvocationID.xy);

    if (coord.x >= dimensions_pad.x || coord.y >= dimensions_pad.y) return;

    vec2 texelSize = vec2(1.0 / float(dimensions_pad.x), 1.0 / float(dimensions_pad.y));
    vec2 uv = (vec2(coord) + 0.5) * texelSize;

    float depth = textureLod(depthSrc, uv, 0.0).x;
    hiZDst[coord] = float2(depth, depth);
    //imageStore(hiZDst, coord, vec4(depth, depth, 0, 0));
}
