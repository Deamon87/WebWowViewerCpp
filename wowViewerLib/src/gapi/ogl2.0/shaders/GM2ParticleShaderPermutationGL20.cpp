//
// Created by Deamon on 7/24/2018.
//

#include "GM2ParticleShaderPermutationGL20.h"
GM2ParticleShaderPermutationGL20::GM2ParticleShaderPermutationGL20(std::string &shaderName, IDevice *device) : GShaderPermutationGL20(shaderName,
                                                                                                          device) {}

void GM2ParticleShaderPermutationGL20::compileShader(const std::string &vertExtraDefStrings, const std::string &fragExtraDefStrings) {
    GShaderPermutationGL20::compileShader("", "");

    //Init newly created shader
    glUseProgram(this->m_programBuffer);

    glUniform1i(this->getUnf("uTexture"), 0);
    glUniform1i(this->getUnf("uTexture2"), 1);
    glUniform1i(this->getUnf("uTexture3"), 2);
    glUseProgram(0);
}
