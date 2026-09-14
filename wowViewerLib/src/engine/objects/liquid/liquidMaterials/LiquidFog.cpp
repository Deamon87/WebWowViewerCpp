#include "../../../../renderer/mapScene/materials/IMaterialStructs.h"
#include "../../../algorithms/mathHelper.h"
#include "LiquidMaterialManager.h"

std::shared_ptr<ILiquidDataBase> LiquidMaterialManager::createFogLiquidData(
    const LiquidTypeAndMat &liquidTypeAndMat,
    const std::vector<LiquidTextureData> &liquidTextureData)
{
    auto result = m_mapBuffCreator->createFogLiquidData();
    result->liquidMaterial = liquidTypeAndMat.materialID;
    result->liquidTypeAndMat = liquidTypeAndMat;

    int waterTypeStub = 0;
    assignLiquidTextures(result, liquidTypeAndMat.texture, liquidTypeAndMat.frameCountTexture, liquidTextureData, waterTypeStub);

    auto &fogData = result->m_liquidData->getObject();

    // Pack material floats into FogData vec4s
    fogData.f0_f1_f2_f3 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[0],
        liquidTypeAndMat.m_floats[1],
        liquidTypeAndMat.m_floats[2],
        liquidTypeAndMat.m_floats[3]
    ));
    fogData.f4_f5_f6_f7 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[4],
        liquidTypeAndMat.m_floats[5],
        liquidTypeAndMat.m_floats[6],
        liquidTypeAndMat.m_floats[7]
    ));
    fogData.f8_f9_f10_f11 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[8],
        liquidTypeAndMat.m_floats[9],
        liquidTypeAndMat.m_floats[10],
        liquidTypeAndMat.m_floats[11]
    ));

    // alpha is not available there, so use 1.0)
    fogData.fogColor = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_colors[0][0],
        liquidTypeAndMat.m_colors[0][1],
        liquidTypeAndMat.m_colors[0][2],
        1.0f
    ));

    return result;
}
