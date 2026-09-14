#include "../../../../renderer/mapScene/materials/IMaterialStructs.h"
#include "../../../algorithms/mathHelper.h"
#include "LiquidMaterialManager.h"

std::shared_ptr<ILiquidDataBase> LiquidMaterialManager::createMagmaLiquidData(
    const LiquidTypeAndMat &liquidTypeAndMat,
    const std::vector<LiquidTextureData> &liquidTextureData)
{
    auto result = m_mapBuffCreator->createMagmaLiquidData();
    result->liquidMaterial = liquidTypeAndMat.materialID;
    result->liquidTypeAndMat = liquidTypeAndMat;

    int waterTypeStub = 0;
    assignLiquidTextures(result, liquidTypeAndMat.texture, liquidTypeAndMat.frameCountTexture, liquidTextureData, waterTypeStub);

    auto &magmaData = result->m_liquidData->getObject();

    magmaData.f0_f1_f2_f3 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[0],
        liquidTypeAndMat.m_floats[1],
        liquidTypeAndMat.m_floats[2],
        liquidTypeAndMat.m_floats[3]
    ));

    magmaData.f4_f5_f6_f7 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[4],
        liquidTypeAndMat.m_floats[5],
        liquidTypeAndMat.m_floats[6],
        liquidTypeAndMat.m_floats[7]
    ));

    magmaData.f8_f9_f10_f11 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[8],
        liquidTypeAndMat.m_floats[9],
        liquidTypeAndMat.m_floats[10],
        liquidTypeAndMat.m_floats[11]
    ));

    magmaData.f12_f13_f14_f15 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[12],
        liquidTypeAndMat.m_floats[13],
        liquidTypeAndMat.m_floats[14],
        liquidTypeAndMat.m_floats[15]
    ));

    magmaData.f16_f17_pad_pad = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[16],
        liquidTypeAndMat.m_floats[17],
        0.0f,
        0.0f
    ));

    magmaData.depthTable = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.coefficient[0],
        liquidTypeAndMat.coefficient[1],
        liquidTypeAndMat.coefficient[2],
        liquidTypeAndMat.coefficient[3]
    ));

    // Precomputed color from m_ints[2]: packed BGR int with a 0x01000000 bias
    uint32_t colorInt = liquidTypeAndMat.m_ints[2];
    if (colorInt >= 0x01000000) {
        colorInt -= 0x01000000;
    }
    float r = static_cast<float>((colorInt >> 16) & 0xFF) / 255.0f;
    float g = static_cast<float>((colorInt >> 8) & 0xFF) / 255.0f;
    float b = static_cast<float>(colorInt & 0xFF) / 255.0f;
    magmaData.colorRGB_pad = mathfu::vec4_packed(mathfu::vec4(r, g, b, 0.0f));

    // Magma noise volume atlas (bound at slot 7 via resolveAnimatedTextures); content uploads async
    result->extraTexture = m_magmaNoiseTex;

    return result;
}
