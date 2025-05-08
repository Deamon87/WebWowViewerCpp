#include "../../../../renderer/mapScene/materials/IMaterialStructs.h"
#include "../../../algorithms/mathHelper.h"
#include "LiquidMaterialManager.h"

unsigned long long ComputeGCD(unsigned long long a, unsigned long long b) {
    while (b != 0) {
        unsigned long long temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

float calculateWaveOffset(animTime_t time, float frequency, float finalFrequency)
{
    // Convert time to scaled time value (multiply by 100)
    float scaledTime = (float)(100 * time);


    // Calculate phase accumulator
    float phaseFloat = scaledTime * frequency;

    // Calculate period in scaled units (finalFrequency * 100000)
    float periodFloat = finalFrequency * 100000.0f;

    // Perform modulo operation: phase % period
    float moduloResult = phaseFloat - (roundf(phaseFloat / periodFloat) * periodFloat);

    // Scale result back down and return as __m128
    float finalResult = moduloResult * 0.000010000001f; // Inverse of 100000

    return finalResult;
}


float CalculateWavePeriod(float waveScale, float waveAmplitude) {
    unsigned int periodCount = 0;
    unsigned long long periods[2] = {0, 0};

    // Calculate period from inverse scale
    float invScale = 1.0f / waveScale;
    if (invScale != 0.0f) {
        periods[periodCount++] = (unsigned long long)fabs(invScale * 10000.0f);
    }

    // Calculate period from normalized scale with amplitude
    float normalizedScale = 1.0f / ((waveScale * 32.0f) * waveAmplitude);
    if (normalizedScale != 0.0f) {
        periods[periodCount++] = (unsigned long long)fabs(normalizedScale * 10000.0f);
    }

    float finalPeriod = 0.0f;
    if (periodCount > 0) {
        // Calculate LCM (Least Common Multiple) of all periods
        unsigned long long lcm = periods[0];
        for (int i = 1; i < periodCount; ++i) {
            unsigned long long period = periods[i];
            if (period != 0) {
                unsigned long long gcd = ComputeGCD(lcm, period);
                lcm = (lcm * period) / gcd;
            }
        }

        finalPeriod = lcm / 10000.0f;
    }

    return finalPeriod;
}

std::shared_ptr<ILiquidDataBase> LiquidMaterialManager::createWaterLiquidData(
    const LiquidTypeAndMat &liquidTypeAndMat,
    const std::vector<LiquidTextureData> &liquidTextureData)
{

    auto result = m_mapBuffCreator->createWaterLiquidData();
    result->liquidMaterial = liquidTypeAndMat.materialID;
    result->liquidTypeAndMat = liquidTypeAndMat;

    int waterType = 0;

    assignLiquidTextures(result, liquidTypeAndMat.texture, liquidTypeAndMat.frameCountTexture, liquidTextureData, waterType);

    auto &waterData = result->m_liquidData->getObject();

    // High detail - multiple textures and wave effects
    float waveScale0 = liquidTypeAndMat.m_floats[3];
    float waveScale1 = liquidTypeAndMat.m_floats[4];
    float waveAmplitude = liquidTypeAndMat.m_floats[8];

    float waveSpeed = liquidTypeAndMat.m_floats[16] * 0.0018750001f;

    float computedOffsets[2] = {0.0f, 0.0f};
    if (waveSpeed != 0.0f)
    {
        computedOffsets[0] = CalculateWavePeriod(waveScale0, waveAmplitude);
        computedOffsets[1] = CalculateWavePeriod(waveScale1, waveAmplitude);
    }

    // Pack material floats into WaterData vec4s
    waterData.f0_f1_f2_f3 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[0],
        liquidTypeAndMat.m_floats[1],
        liquidTypeAndMat.m_floats[2],
        liquidTypeAndMat.m_floats[3]
    ));
    waterData.f4_f5_f6_f7 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[4],
        liquidTypeAndMat.m_floats[5],
        liquidTypeAndMat.m_floats[6],
        liquidTypeAndMat.m_floats[7]
    ));
    waterData.f8_f9_f10_f11 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[8],
        liquidTypeAndMat.m_floats[9],
        liquidTypeAndMat.m_floats[10],
        liquidTypeAndMat.m_floats[11]
    ));
    waterData.f12_f13_f14_f15 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[12],
        liquidTypeAndMat.m_floats[13],
        liquidTypeAndMat.m_floats[14],
        liquidTypeAndMat.m_floats[15]
    ));
    waterData.f16_f17_pad_pad = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[16],
        liquidTypeAndMat.m_floats[17],
        0.0f,
        0.0f
    ));

    // Precomputed wave periods (CalculateWavePeriod with GCD/LCM, too expensive for GPU)
    waterData.computedOffset0_computedOffsets1_pad_pad = mathfu::vec4_packed(mathfu::vec4(
        computedOffsets[0],
        computedOffsets[1],
        0.0f,
        0.0f
    ));

    waterData.depthTable = mathfu::vec4_packed(mathfu::vec4(
      liquidTypeAndMat.coefficient[0],
      liquidTypeAndMat.coefficient[1],
      liquidTypeAndMat.coefficient[2],
      liquidTypeAndMat.coefficient[3]
    ));

    waterData.waterType_pad_pad_pad.x = waterType;

    return result;
}
