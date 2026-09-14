import glsl;

// Copies a depth texture (sampled as float) to mip 0 of the Hi-Z buffer (storage image).

[vk::push_constant]
ivec4 dimensions_pad; // x=width, y=height

Sampler2D depthSrc;

[vk::image_format("r32f")]
RWTexture2D<float> hiZDst;

[shader("compute")]
[numthreads(8, 8, 1)]
void main() {
    ivec2 coord = ivec2(gl_GlobalInvocationID.xy);

    if (coord.x >= dimensions_pad.x || coord.y >= dimensions_pad.y) return;

    vec2 texelSize = vec2(1.0 / float(dimensions_pad.x), 1.0 / float(dimensions_pad.y));
    vec2 uv = (vec2(coord) + 0.5) * texelSize;

    float depth = textureLod(depthSrc, uv, 0.0).x;
    imageStore(hiZDst, coord, vec4(depth, 0, 0, 0));
}
