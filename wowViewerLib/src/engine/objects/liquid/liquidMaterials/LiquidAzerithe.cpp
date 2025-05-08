#include "../../../../renderer/mapScene/materials/IMaterialStructs.h"
#include "../../../algorithms/mathHelper.h"
#include "LiquidMaterialManager.h"
#include <cmath>

// Azerithe: the normal map (LiquidType Texture[5]) is an animated texture —
// slot 5 = frame k, slot 7 = frame (k+1)%N. The GPU crossfades with the
// fractional frame position.
template<>
void LiquidData<Liquid::AzeritheData>::resolveAnimatedTextures(
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

std::shared_ptr<ILiquidDataBase> LiquidMaterialManager::createAzeritheLiquidData(
    const LiquidTypeAndMat &liquidTypeAndMat,
    const std::vector<LiquidTextureData> &liquidTextureData)
{
    auto result = m_mapBuffCreator->createAzeritheLiquidData();
    result->liquidMaterial = liquidTypeAndMat.materialID;
    result->liquidTypeAndMat = liquidTypeAndMat;

    int waterTypeStub = 0;
    assignLiquidTextures(result, liquidTypeAndMat.texture, liquidTypeAndMat.frameCountTexture, liquidTextureData, waterTypeStub);

    auto &azeritheData = result->m_liquidData->getObject();

    // Pack material floats into AzeritheData vec4s
    azeritheData.f0_f1_f2_f3 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[0],
        liquidTypeAndMat.m_floats[1],
        liquidTypeAndMat.m_floats[2],
        liquidTypeAndMat.m_floats[3]
    ));
    azeritheData.f4_f5_f6_f7 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[4],
        liquidTypeAndMat.m_floats[5],
        liquidTypeAndMat.m_floats[6],
        liquidTypeAndMat.m_floats[7]
    ));
    azeritheData.f8_f9_f10_f11 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[8],
        liquidTypeAndMat.m_floats[9],
        liquidTypeAndMat.m_floats[10],
        liquidTypeAndMat.m_floats[11]
    ));
    azeritheData.f12_f13_f14_f15 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[12],
        liquidTypeAndMat.m_floats[13],
        liquidTypeAndMat.m_floats[14],
        liquidTypeAndMat.m_floats[15]
    ));
    azeritheData.f16_pad_pad_pad = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[16],
        0.0f,
        0.0f,
        0.0f
    ));

    // w = normal map (Texture[5]) frame count, used by the GPU crossfade
    azeritheData.color0 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_colors[0][0],
        liquidTypeAndMat.m_colors[0][1],
        liquidTypeAndMat.m_colors[0][2],
        static_cast<float>(result->usedTextures[5].size())
    ));

    // Engine-global textures (env map and foam)
    {
        auto envMapFile = m_api->cacheStorage->getTextureCache()->getFileId(1797551);
        result->extraTexture2 = m_api->hDevice->createBlpTexture(envMapFile, true, true);

        auto foamFile = m_api->cacheStorage->getTextureCache()->getFileId(1844666);
        result->extraTexture3 = m_api->hDevice->createBlpTexture(foamFile, true, true);
    }

    return result;
}
