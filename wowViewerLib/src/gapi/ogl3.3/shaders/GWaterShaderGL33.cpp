//
// Created by Deamon on 9/13/2020.
//

#include "GWaterShaderGL33.h"

GWaterShaderGL33::GWaterShaderGL33(std::string &shaderName, IDevice *device) : GShaderPermutationGL33(shaderName,
                                                                                                      device) {

}

void GWaterShaderGL33::compileShader(const std::string &vertExtraDefStrings, const std::string &fragExtraDefStrings) {
    GShaderPermutationGL33::compileShader("", "");

    glUseProgram(this->m_programBuffer);

    glUniform1i(this->getUnf("uTexture"), 0);
    glUseProgram(0);
};