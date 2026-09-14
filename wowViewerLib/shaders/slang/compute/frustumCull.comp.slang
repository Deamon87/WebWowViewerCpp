import glsl;

struct AABB {
    float4 min;
    float4 max;
};

struct CullConstants {
    float4x4 viewProj;
    ivec4 objectCount_pad;
};

[vk::push_constant]
CullConstants cullConstants;

StructuredBuffer<AABB, ScalarDataLayout> aabbBuffer;
RWStructuredBuffer<uint, ScalarDataLayout> visibilityBuffer;

// Clip-space frustum culling: transform all 8 AABB corners by viewProj,
// test in clip space without perspective divide.
// For each corner (x, y, z, w), inside if -w <= x <= w, -w <= y <= w, 0 <= z <= w.
// If ALL corners are outside the same side -> culled.
[shader("compute")]
[numthreads(64, 1, 1)]
void main() {
    uint idx = gl_GlobalInvocationID.x;
    if (idx >= cullConstants.objectCount_pad.x) return;

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

    // Track: are ALL corners outside on the same side?
    // 6 planes: left (x < -w), right (x > w), bottom (y < -w), top (y > w), near (z < 0), far (z > w)
    uint allOutside_left   = 0xff;
    uint allOutside_right  = 0xff;
    uint allOutside_bottom = 0xff;
    uint allOutside_top    = 0xff;
    uint allOutside_near   = 0xff;
    uint allOutside_far    = 0xff;
    bool anyBehindCamera   = false;

    for (uint i = 0; i < 8; i++) {
        float4 clip = cullConstants.viewProj * corners[i];

        // If w <= 0, this corner is behind or at the near plane.
        // Clip-space comparisons are invalid with non-positive w.
        // The object straddles the camera — force visible.
        if (clip.w <= 0.0) {
            anyBehindCamera = true;
            break;
        }

        // For each corner, check which sides it's outside of
        // A corner is inside if: -w <= x <= w, -w <= y <= w, 0 <= z <= w
        uint bit = 1u << i;

        if (clip.x >= -clip.w) allOutside_left   &= ~bit;
        if (clip.x <=  clip.w) allOutside_right  &= ~bit;
        if (clip.y >= -clip.w) allOutside_bottom  &= ~bit;
        if (clip.y <=  clip.w) allOutside_top    &= ~bit;
        if (clip.z >= 0.0)     allOutside_near   &= ~bit;
        if (clip.z <=  clip.w) allOutside_far    &= ~bit;
    }

    // If any corner is behind camera, object straddles near plane — definitely visible
    // Otherwise, if ALL 8 corners are outside the same plane -> culled
    bool culled = !anyBehindCamera &&
                  ((allOutside_left   == 0xff) || (allOutside_right == 0xff) ||
                   (allOutside_bottom == 0xff) || (allOutside_top   == 0xff) ||
                   (allOutside_near   == 0xff) || (allOutside_far   == 0xff));

    if (culled && false) {
        printf("obj %u aabb min(%f,%f,%f) max(%f,%f,%f)\n", idx,
               aabbMin.x, aabbMin.y, aabbMin.z, aabbMax.x, aabbMax.y, aabbMax.z);
        if (allOutside_left == 0xff)
            printf("obj %u culled: LEFT\n", idx);
        if (allOutside_right == 0xff)
            printf("obj %u culled: RIGHT\n", idx);
        if (allOutside_bottom == 0xff)
            printf("obj %u culled: BOTTOM\n", idx);
        if (allOutside_top == 0xff)
            printf("obj %u culled: TOP\n", idx);
        if (allOutside_near == 0xff)
            printf("obj %u culled: NEAR\n", idx);
        if (allOutside_far == 0xff)
            printf("obj %u culled: FAR\n", idx);
    }

    // Write visibility: non-zero = visible, 0 = culled
    // This directly feeds VK_EXT_conditional_rendering which checks for non-zero
    visibilityBuffer[idx] = culled ? 0u : 1u;
}
