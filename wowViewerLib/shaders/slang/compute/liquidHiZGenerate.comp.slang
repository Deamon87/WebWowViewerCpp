import glsl;

// Generates one mip level of the liquid Hi-Z buffer (min-max).
// R channel = min depth (nearest surface in regular Z)
// G channel = max depth (farthest surface in regular Z)
// Handles non-power-of-2 and odd source dimensions.

[vk::push_constant]
ivec4 srcMipSize_dstMipSize; // x=srcWidth, y=srcHeight, z=dstWidth, w=dstHeight

// binding 0: source mip (combined image sampler, nearest filter)
Sampler2D srcMip;
// binding 1: destination mip (storage image, written by compute)
[vk::image_format("rg32f")]
RWTexture2D<float2> dstMip;

float2 sampleSrc(ivec2 coord) {
    int srcW = srcMipSize_dstMipSize.x;
    int srcH = srcMipSize_dstMipSize.y;

    coord = clamp(coord, ivec2(0, 0), ivec2(srcW - 1, srcH - 1));

    vec2 texelSize = vec2(1.0 / float(srcW), 1.0 / float(srcH));
    vec2 uv = (vec2(coord) + 0.5) * texelSize;
    return textureLod(srcMip, uv, 0.0).xy;
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
    float2 s00 = sampleSrc(srcBase + ivec2(0, 0));
    float2 s10 = sampleSrc(srcBase + ivec2(1, 0));
    float2 s01 = sampleSrc(srcBase + ivec2(0, 1));
    float2 s11 = sampleSrc(srcBase + ivec2(1, 1));

    // R = min (nearest), G = max (farthest)
    float minDepth = min(min(s00.x, s10.x), min(s01.x, s11.x));
    float maxDepth = max(max(s00.y, s10.y), max(s01.y, s11.y));

    bool needExtraSampleX = ratio.x > 2;
    bool needExtraSampleY = ratio.y > 2;

    if (needExtraSampleX) {
        float2 sx0 = sampleSrc(srcBase + ivec2(2, 0));
        float2 sx1 = sampleSrc(srcBase + ivec2(2, 1));
        minDepth = min(minDepth, min(sx0.x, sx1.x));
        maxDepth = max(maxDepth, max(sx0.y, sx1.y));
    }
    if (needExtraSampleY) {
        float2 sy0 = sampleSrc(srcBase + ivec2(0, 2));
        float2 sy1 = sampleSrc(srcBase + ivec2(1, 2));
        minDepth = min(minDepth, min(sy0.x, sy1.x));
        maxDepth = max(maxDepth, max(sy0.y, sy1.y));
    }
    if (needExtraSampleX && needExtraSampleY) {
        float2 sxy = sampleSrc(srcBase + ivec2(2, 2));
        minDepth = min(minDepth, sxy.x);
        maxDepth = max(maxDepth, sxy.y);
    }

    dstMip[dstCoord] = float2(minDepth, maxDepth);
    //imageStore(dstMip, dstCoord, vec4(minDepth, maxDepth, 0, 0));
}
