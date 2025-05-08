#include "../../../../renderer/mapScene/materials/IMaterialStructs.h"
#include "../../../algorithms/mathHelper.h"
#include "LiquidMaterialManager.h"

std::shared_ptr<ILiquidDataBase> LiquidMaterialManager::createLeyLineLiquidData(
    const LiquidTypeAndMat &liquidTypeAndMat,
    const std::vector<LiquidTextureData> &liquidTextureData)
{
    auto result = m_mapBuffCreator->createLeyLineLiquidData();
    result->liquidMaterial = liquidTypeAndMat.materialID;
    result->liquidTypeAndMat = liquidTypeAndMat;

    int waterTypeStub = 0;
    assignLiquidTextures(result, liquidTypeAndMat.texture, liquidTypeAndMat.frameCountTexture, liquidTextureData, waterTypeStub);

    auto &leyLineData = result->m_liquidData->getObject();

    // Pack material floats into LeyLineData vec4s
    leyLineData.f0_f1_f2_f3 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[0],
        liquidTypeAndMat.m_floats[1],
        liquidTypeAndMat.m_floats[2],
        liquidTypeAndMat.m_floats[3]
    ));

    leyLineData.f4_f5_f6_f7 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[4],
        liquidTypeAndMat.m_floats[5],
        liquidTypeAndMat.m_floats[6],
        liquidTypeAndMat.m_floats[7]
    ));

    leyLineData.f8_f9_f10_f11 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[8],
        liquidTypeAndMat.m_floats[9],
        liquidTypeAndMat.m_floats[10],
        liquidTypeAndMat.m_floats[11]
    ));

    leyLineData.f12_f13_f14_f15 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[12],
        liquidTypeAndMat.m_floats[13],
        liquidTypeAndMat.m_floats[14],
        liquidTypeAndMat.m_floats[15]
    ));

    leyLineData.f16_f17_pad_pad = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_floats[16],
        liquidTypeAndMat.m_floats[17],
        0.0f,
        0.0f
    ));

    // Colors from LiquidType Color[0]/Color[1] (stored BGR-ordered)
    leyLineData.color0 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_colors[0][2],
        liquidTypeAndMat.m_colors[0][1],
        liquidTypeAndMat.m_colors[0][0],
        0.0f
    ));
    leyLineData.color1 = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.m_colors[1][2],
        liquidTypeAndMat.m_colors[1][1],
        liquidTypeAndMat.m_colors[1][0],
        0.0f
    ));

    leyLineData.depthTable = mathfu::vec4_packed(mathfu::vec4(
        liquidTypeAndMat.coefficient[0],
        liquidTypeAndMat.coefficient[1],
        liquidTypeAndMat.coefficient[2],
        liquidTypeAndMat.coefficient[3]
    ));

    return result;
}
