#include "../../../../renderer/mapScene/materials/IMaterialStructs.h"
#include "../../../algorithms/mathHelper.h"
#include "LiquidMaterialManager.h"
#include <cmath>

// Swamp: the normal map (LiquidType Texture[4]) is an animated texture —
// slot 4 = frame k, slot 7 = frame (k+1)%N. The GPU crossfades with the
// fractional frame position.
template<>
void LiquidData<Liquid::SwampData>::resolveAnimatedTextures(
    std::array<HGSamplableTexture, MAX_LIQUID_TYPE_TEXTURES + 1> &textures, float currentTime) {
    auto &frames = this->usedTextures[4];
    if (frames.empty()) {
        // Fall back to whatever slot 4 resolved to (black pixel)
        textures[MAX_LIQUID_TYPE_TEXTURES] = textures[4];
        return;
    }

    double interval = (double)this->liquidTypeAndMat.m_floats[5] * 1000.0;
    size_t k = 0;
    if (interval > 0.0 && frames.size() > 1) {
        double framePos = (double)frames.size() * (std::fmod((double)currentTime, interval) / interval);
        k = (size_t)framePos;
    }
    textures[4] = frames[k % frames.size()];
    textures[MAX_LIQUID_TYPE_TEXTURES] = frames[(k + 1) % frames.size()];
}

std::shared_ptr<ILiquidDataBase> LiquidMaterialManager::createSwampLiquidData(
    const LiquidTypeAndMat &liquidTypeAndMat,
    const std::vector<LiquidTextureData> &liquidTextureData)
{
    auto result = m_mapBuffCreator->createSwampLiquidData();
    result->liquidMaterial = liquidTypeAndMat.materialID;
    result->liquidTypeAndMat = liquidTypeAndMat;

    int swampType = 0;
    assignLiquidTextures(result, liquidTypeAndMat.texture, liquidTypeAndMat.frameCountTexture, liquidTextureData, swampType);

    auto &swampData = result->m_liquidData->getObject();

    // Pack material floats into SwampData vec4s
    swampData.f0_f1_f2_f3 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[0],
        liquidTypeAndMat.m_floats[1],
        liquidTypeAndMat.m_floats[2],
        liquidTypeAndMat.m_floats[3]
    ));
    swampData.f4_f5_f6_f7 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[4],
        liquidTypeAndMat.m_floats[5],
        liquidTypeAndMat.m_floats[6],
        liquidTypeAndMat.m_floats[7]
    ));
    swampData.f8_f9_f10_f11 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[8],
        liquidTypeAndMat.m_floats[9],
        liquidTypeAndMat.m_floats[10],
        liquidTypeAndMat.m_floats[11]
    ));
    swampData.f12_f13_f14_f15 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[12],
        liquidTypeAndMat.m_floats[13],
        liquidTypeAndMat.m_floats[14],
        liquidTypeAndMat.m_floats[15]
    ));
    swampData.f16_f17_pad_pad = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[16],
        liquidTypeAndMat.m_floats[17],
        0.0f,
        0.0f
    ));

    // Depth table from coefficients (same as water)
    swampData.depthTable = mathfu::vec4_packed(mathfu::vec4(
      liquidTypeAndMat.coefficient[0],
      liquidTypeAndMat.coefficient[1],
      liquidTypeAndMat.coefficient[2],
      liquidTypeAndMat.coefficient[3]
    ));

    swampData.colorRGB_pad = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_colors[0][2],
        liquidTypeAndMat.m_colors[0][1],
        liquidTypeAndMat.m_colors[0][0],
        static_cast<float>(result->usedTextures[4].size())
    ));

    return result;
}
