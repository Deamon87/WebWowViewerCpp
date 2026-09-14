#ifndef COMMON_LIQUID_STRUCTS_SLANG
#define COMMON_LIQUID_STRUCTS_SLANG

import glsl;

struct LiquidInstanceData {
    ivec4 liquidMaterial_liquidType_isInterior_vtxCellOffetWidthHeight;
    vec4 flowSpeed_flowDirection_unused4_unused5;
};

const float ROUNDING_ERROR_f32 = 0.001f;
bool feq(const float a, const float b)
{
    return (a + ROUNDING_ERROR_f32 >= b) && (a - ROUNDING_ERROR_f32 <= b);
}
// Fractional position in the scroll cycle (time in ms). Zero for
// |speed| <= 1e-8 or |speed| >= 1000; negative speeds scroll backwards
// (1 - frac). Guards avoid the int(1000/speed) == 0 div-by-zero.
float GetTexScrollComponent(float time, float speed) {
    if (speed > 0.00000001f && speed < 1000.0f) {
        int period = int(1000.0f / speed);
        return float(int(time) % period) / float(period);
    }
    if (speed < -0.00000001f && speed > -1000.0f) {
        int period = int(-1000.0f / speed);
        return 1.0f - float(int(time) % period) / float(period);
    }
    return 0.0f;
}
vec2 GetTexScrollMtx(float time, vec2 scrollVec) {
    return vec2(GetTexScrollComponent(time, scrollVec.x), GetTexScrollComponent(time, scrollVec.y));
}

struct WaterData {
    vec4 f0_f1_f2_f3;
    vec4 f4_f5_f6_f7;
    vec4 f8_f9_f10_f11;
    vec4 f12_f13_f14_f15;
    vec4 f16_f17_pad_pad;
    vec4 depthTable;
    ivec4 waterType_pad_pad_pad;
    // x/y = precomputed wave PERIODS (LCM of reciprocal scales, GCD/LCM done on CPU),
    // fed to CalculateWaveOffset on GPU as `finalFrequency`. Not offsets despite the name.
    vec4 computedOffset0_computedOffsets1_pad_pad;
};

// Magma liquid data
struct MagmaData {
    vec4 f0_f1_f2_f3;
    vec4 f4_f5_f6_f7;
    vec4 f8_f9_f10_f11;
    vec4 f12_f13_f14_f15;
    vec4 f16_f17_pad_pad;
    vec4 depthTable;
    vec4 colorRGB_pad;
};

// Mercury liquid data
struct MercuryData {
    vec4 f0_f1_f2_f3;
    vec4 f4_f5_f6_f7;
    vec4 f8_f9_f10_f11;
    vec4 f12_f13_f14_f15;
    vec4 f16_pad_pad_pad;
};

// Fog liquid data
struct FogData {
    vec4 f0_f1_f2_f3;      // x: height falloff, y: density factor, w: texMtx[0] scale
    vec4 f4_f5_f6_f7;
    vec4 f8_f9_f10_f11;
    vec4 fogColor;         // rgba (a = 1.0, DB layer has no alpha)
};

// Ley Line liquid data
struct LeyLineData {
    vec4 f0_f1_f2_f3;
    vec4 f4_f5_f6_f7;
    vec4 f8_f9_f10_f11;
    vec4 f12_f13_f14_f15;
    vec4 f16_f17_pad_pad;
    vec4 color0;           // LiquidType Color[0]/255
    vec4 color1;           // LiquidType Color[1]/255
    vec4 depthTable;       // LiquidType.Coefficient[0..3]
};


// Fel liquid data
struct FelData {
    vec4 f0_f1_f2_f3;
    vec4 f4_f5_f6_f7;
    vec4 f8_f9_f10_f11;
    vec4 f12_f13_f14_f15;
    vec4 f16_f17_pad_pad;
    vec4 intParams;        // x: hasDepth (1.0/0.0), y: offset*0.01, z: blend*0.01, w: normal map frame count
    vec4 depthTable;       // LiquidType.Coefficient[0..3]
};

// Swamp liquid data
struct SwampData {
    vec4 f0_f1_f2_f3;
    vec4 f4_f5_f6_f7;
    vec4 f8_f9_f10_f11;
    vec4 f12_f13_f14_f15;
    vec4 f16_f17_pad_pad;
    vec4 depthTable;       // xyz = coefficients from liquidTypeAndMat
    vec4 colorRGB_pad;     // xyz = murky fog color from m_colors[0]
};

// Azerithe liquid data
struct AzeritheData {
    vec4 f0_f1_f2_f3;
    vec4 f4_f5_f6_f7;
    vec4 f8_f9_f10_f11;
    vec4 f12_f13_f14_f15;
    vec4 f16_pad_pad_pad;
    vec4 color0;           // rgb = shallow color, w = normal map (Texture[5]) frame count
};

const vec4 reflectionParams = { 0.1f, 0.55f, 20.0f, 1.0f };

#endif //COMMON_LIQUID_STRUCTS 
