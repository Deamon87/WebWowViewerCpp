import glsl;

// Generates one mip level of the Hi-Z buffer.
// Handles non-power-of-2 and odd source dimensions by including orphaned
// edge texels when the source dimension doesn't divide evenly by 2.

[vk::push_constant]
ivec4 srcMipSize_dstMipSize; // x=srcWidth, y=srcHeight, z=dstWidth, w=dstHeight

// binding 0: source mip (combined image sampler, nearest filter)
Sampler2D srcMip;
// binding 1: destination mip (storage image, written by compute)
[vk::image_format("r32f")]
RWTexture2D<float> dstMip;

float sampleSrc(ivec2 coord) {
    int srcW = srcMipSize_dstMipSize.x;
    int srcH = srcMipSize_dstMipSize.y;

    // Clamp to source dimensions
    coord = clamp(coord, ivec2(0, 0), ivec2(srcW - 1, srcH - 1));

    vec2 texelSize = vec2(1.0 / float(srcW), 1.0 / float(srcH));
    vec2 uv = (vec2(coord) + 0.5) * texelSize;
    return textureLod(srcMip, uv, 0.0).x;
}

[shader("compute")]
[numthreads(8, 8, 1)]
void main() {
    ivec2 dstCoord = ivec2(gl_GlobalInvocationID.xy);
    int srcW = srcMipSize_dstMipSize.x;
    int srcH = srcMipSize_dstMipSize.y;
    int dstW = srcMipSize_dstMipSize.z;
    int dstH = srcMipSize_dstMipSize.w;

    if (dstCoord.x >= dstW || dstCoord.y >= dstH) return;

	float2 ratio = vec2(srcW, srcH) / vec2(dstW, dstH);

    ivec2 srcBase = dstCoord * 2;

    // Standard 2x2 block
    float result = sampleSrc(srcBase + ivec2(0, 0));
    result = min(result, sampleSrc(srcBase + ivec2(1, 0)));
    result = min(result, sampleSrc(srcBase + ivec2(0, 1)));
    result = min(result, sampleSrc(srcBase + ivec2(1, 1)));

    bool needExtraSampleX = ratio.x>2;
    bool needExtraSampleY = ratio.y>2;

    result = needExtraSampleX ? min(result, min(sampleSrc(srcBase + uint2(2,0)).x, sampleSrc(srcBase + uint2(2,1)).x)) : result;
    result = needExtraSampleY ? min(result, min(sampleSrc(srcBase + uint2(0,2)).x, sampleSrc(srcBase + uint2(1,2)).x)) : result;
    result = (needExtraSampleX && needExtraSampleY) ? min(result, sampleSrc(srcBase + uint2(2,2)).x) : result;


    imageStore(dstMip, dstCoord, vec4(result, 0, 0, 0));
}
