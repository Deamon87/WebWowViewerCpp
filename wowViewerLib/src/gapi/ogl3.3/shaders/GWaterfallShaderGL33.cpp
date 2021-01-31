//
// Created by Deamon on 1/31/2021.
//

#include "GWaterfallShaderGL33.h"

GWaterfallShaderGL33::GWaterfallShaderGL33(std::string &shaderName, IDevice *device) : GShaderPermutationGL33(shaderName,
                                                                                                      device) {

}

void GWaterfallShaderGL33::compileShader(const std::string &vertExtraDefStrings, const std::string &fragExtraDefStrings) {
    GShaderPermutationGL33::compileShader("", "");

    glUseProgram(this->m_programBuffer);

    glUniform1i(this->getUnf("uMask"), 0);
    glUniform1i(this->getUnf("uWhiteWater"), 1);
    glUniform1i(this->getUnf("uNoise"), 2);
    glUniform1i(this->getUnf("uBumpTexture"), 3);
    glUniform1i(this->getUnf("uNormalTex"), 4);
    glUseProgram(0);
};