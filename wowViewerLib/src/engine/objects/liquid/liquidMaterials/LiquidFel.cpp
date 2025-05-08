#include "../../../../renderer/mapScene/materials/IMaterialStructs.h"
#include "../../../algorithms/mathHelper.h"
#include "LiquidMaterialManager.h"
#include <cmath>

// Fel: the normal map (LiquidType Texture[5]) is an animated texture —
// slot 5 = frame k, slot 7 = frame (k+1)%N. The GPU crossfades with the
// fractional frame position.
template<>
void LiquidData<Liquid::FelData>::resolveAnimatedTextures(
    std::array<HGSamplableTexture, MAX_LIQUID_TYPE_TEXTURES + 1> &textures, float currentTime) {
    auto &frames = this->usedTextures[5];
    if (frames.empty()) {
        // Fall back to whatever slot 5 resolved to (black pixel)
        textures[MAX_LIQUID_TYPE_TEXTURES] = textures[5];
        return;
    }

    double interval = (double)this->liquidTypeAndMat.m_floats[12] * 1000.0;
    size_t k = 0;
    if (interval > 0.0 && frames.size() > 1) {
        double framePos = (double)frames.size() * (std::fmod((double)currentTime, interval) / interval);
        k = (size_t)framePos;
    }
    textures[5] = frames[k % frames.size()];
    textures[MAX_LIQUID_TYPE_TEXTURES] = frames[(k + 1) % frames.size()];
}

std::shared_ptr<ILiquidDataBase> LiquidMaterialManager::createFelLiquidData(
    const LiquidTypeAndMat &liquidTypeAndMat,
    const std::vector<LiquidTextureData> &liquidTextureData)
{
    auto result = m_mapBuffCreator->createFelLiquidData();
    result->liquidMaterial = liquidTypeAndMat.materialID;
    result->liquidTypeAndMat = liquidTypeAndMat;

    int waterTypeStub = 0;
    assignLiquidTextures(result, liquidTypeAndMat.texture, liquidTypeAndMat.frameCountTexture, liquidTextureData, waterTypeStub);

    auto &felData = result->m_liquidData->getObject();

    // Pack material floats into FelData vec4s
    felData.f0_f1_f2_f3 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[0],
        liquidTypeAndMat.m_floats[1],
        liquidTypeAndMat.m_floats[2],
        liquidTypeAndMat.m_floats[3]
    ));

    felData.f4_f5_f6_f7 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[4],
        liquidTypeAndMat.m_floats[5],
        liquidTypeAndMat.m_floats[6],
        liquidTypeAndMat.m_floats[7]
    ));

    felData.f8_f9_f10_f11 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[8],
        liquidTypeAndMat.m_floats[9],
        liquidTypeAndMat.m_floats[10],
        liquidTypeAndMat.m_floats[11]
    ));

    felData.f12_f13_f14_f15 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[12],
        liquidTypeAndMat.m_floats[13],
        liquidTypeAndMat.m_floats[14],
        liquidTypeAndMat.m_floats[15]
    ));

    felData.f16_f17_pad_pad = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[16],
        liquidTypeAndMat.m_floats[17],
        0.0f,
        0.0f
    ));

    // w: normal map (Texture[5]) frame count, used by the GPU crossfade
    float hasDepth = (liquidTypeAndMat.m_ints[1] != 0) ? 1.0f : 0.0f;
    float intOffset = static_cast<float>(liquidTypeAndMat.m_ints[2]) * 0.01f;
    float intBlend = static_cast<float>(liquidTypeAndMat.m_ints[3]) * 0.01f;
    float normalFrameCount = static_cast<float>(result->usedTextures[5].size());
    felData.intParams = mathfu::vec4_packed(mathfu::vec4(hasDepth, intOffset, intBlend, normalFrameCount));

    // Depth table from coefficients (used by CalcDepthPercent in the shader)
    felData.depthTable = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.coefficient[0],
        liquidTypeAndMat.coefficient[1],
        liquidTypeAndMat.coefficient[2],
        liquidTypeAndMat.coefficient[3]
    ));

    return result;
}
