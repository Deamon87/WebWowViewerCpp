import glsl;

// Per-object classification against liquid Hi-Z.
// Tests each object's AABB against the liquid min-max depth pyramid to determine
// whether the object is above, below, or intersecting the liquid plane.
//
// Output classification bits per object:
//   bit 0 (value 1) = has geometry below liquid
//   bit 1 (value 2) = has geometry above liquid
//   value 3 = intersects liquid plane (both bits set)

struct AABB {
    float4 min;
    float4 max;
};

struct LiquidClassifyConstants {
    float4x4 viewProj;
    uint objectCount;
    uint hiZMipCount;
    uint hiZWidth;
    uint hiZHeight;
    float viewportDepthMin;
    float viewportDepthRange;
    float pad0;
    float pad1;
};

[vk::push_constant]
LiquidClassifyConstants constants;

StructuredBuffer<AABB, ScalarDataLayout> aabbBuffer;
RWStructuredBuffer<uint, ScalarDataLayout> classificationBuffer;
Sampler2D liquidHiZMap;  // RG32_SFLOAT: R=min depth (nearest surface, regular Z), G=max depth (farthest)

[shader("compute")]
[numthreads(64, 1, 1)]
void main() {
    uint idx = gl_GlobalInvocationID.x;
    if (idx >= constants.objectCount) return;

    float3 aabbMin = aabbBuffer[idx].min.xyz;
    float3 aabbMax = aabbBuffer[idx].max.xyz;

    // Generate 8 AABB corners
    float4 corners[8];
    corners[0] = float4(aabbMin.x, aabbMin.y, aabbMin.z, 1.0);
    corners[1] = float4(aabbMax.x, aabbMin.y, aabbMin.z, 1.0);
    corners[2] = float4(aabbMin.x, aabbMax.y, aabbMin.z, 1.0);
    corners[3] = float4(aabbMax.x, aabbMax.y, aabbMin.z, 1.0);
    corners[4] = float4(aabbMin.x, aabbMin.y, aabbMax.z, 1.0);
    corners[5] = float4(aabbMax.x, aabbMin.y, aabbMax.z, 1.0);
    corners[6] = float4(aabbMin.x, aabbMax.y, aabbMax.z, 1.0);
    corners[7] = float4(aabbMax.x, aabbMax.y, aabbMax.z, 1.0);

    // Project all corners, track screen bounds and depth range
    float2 ndcMin = float2(1e30, 1e30);
    float2 ndcMax = float2(-1e30, -1e30);
    float nearestDepth = 1.0;   // regular Z: smallest value = nearest
    float farthestDepth = 0.0;  // regular Z: largest value = farthest
    bool anyBehindCamera = false;

    for (uint i = 0; i < 8; i++) {
        float4 clip = constants.viewProj * corners[i];

        if (clip.w <= 0.0) {
            anyBehindCamera = true;
            break;
        }

        float3 ndc = clip.xyz / clip.w;
        ndcMin = min(ndcMin, ndc.xy);
        ndcMax = max(ndcMax, ndc.xy);

        float depthBufValue = constants.viewportDepthMin + ndc.z * constants.viewportDepthRange;
        nearestDepth = min(nearestDepth, depthBufValue);   // regular Z: nearest = smallest
        farthestDepth = max(farthestDepth, depthBufValue); // regular Z: farthest = largest
    }

    // Objects behind camera or straddling near plane: conservatively mark as intersecting
    if (anyBehindCamera) {
        classificationBuffer[idx] = 3u; // both above and below
        return;
    }

    // Clamp and convert to UV
    ndcMin = clamp(ndcMin, float2(-1.0, -1.0), float2(1.0, 1.0));
    ndcMax = clamp(ndcMax, float2(-1.0, -1.0), float2(1.0, 1.0));
    float2 uvMin = ndcMin * 0.5 + 0.5;
    float2 uvMax = ndcMax * 0.5 + 0.5;

    // Select mip level based on screen-space size
    float2 screenSize = float2(constants.hiZWidth, constants.hiZHeight);
    float2 pixelSize = (uvMax - uvMin) * screenSize;
    float maxPixelDim = max(pixelSize.x, pixelSize.y) / 2;

    int mipLevel;
    if (maxPixelDim < 1.0) {
        mipLevel = 0;
    } else {
        float mipF = log2(maxPixelDim) + 1;
        mipLevel = clamp(int(ceil(mipF)), 0, int(constants.hiZMipCount) - 1);
    }

    // Sample liquid HiZ at 4 corners of the AABB's screen-space footprint
    // R = min depth (farthest liquid surface), G = max depth (nearest liquid surface)
    float2 h00 = textureLod(liquidHiZMap, uvMin, float(mipLevel)).xy;
    float2 h10 = textureLod(liquidHiZMap, float2(uvMax.x, uvMin.y), float(mipLevel)).xy;
    float2 h01 = textureLod(liquidHiZMap, float2(uvMin.x, uvMax.y), float(mipLevel)).xy;
    float2 h11 = textureLod(liquidHiZMap, uvMax, float(mipLevel)).xy;

    // Conservative liquid bounds across the footprint
    // Regular Z: R (min) = nearest liquid surface, G (max) = farthest liquid surface
    float liquidMinDepth = min(min(h00.x, h10.x), min(h01.x, h11.x)); // nearest liquid
    float liquidMaxDepth = max(max(h00.y, h10.y), max(h01.y, h11.y)); // farthest liquid

    // If liquid depth is still at the clear value (1.0 in regular Z), no liquid present.
    // In that case, object is entirely above liquid
    if (liquidMinDepth >= 0.9999) {
        classificationBuffer[idx] = 2u; // above only
        return;
    }

    // Classify (regular Z: smaller depth = nearer to camera)
    // Object is entirely above liquid if its farthest point is still nearer than the nearest liquid
    // Object is entirely below liquid if its nearest point is still farther than the farthest liquid
    uint classification = 0u;

    if (farthestDepth < liquidMinDepth) {
        // Object's farthest point < liquid's nearest surface: entirely above liquid
        classification = 2u;
    } else if (nearestDepth > liquidMaxDepth) {
        // Object's nearest point > liquid's farthest surface: entirely below liquid
        classification = 1u;
    } else {
        // Object straddles the liquid plane
        classification = 3u;
    }

    classificationBuffer[idx] = classification;
}
