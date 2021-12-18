//
// Created by Deamon on 7/24/2018.
//

#include "GM2ParticleShaderPermutationGL33.h"
GM2ParticleShaderPermutationGL33::GM2ParticleShaderPermutationGL33(std::string &shaderName, const HGDevice &device) : GShaderPermutationGL33(shaderName,
                                                                                                          device) {}

void GM2ParticleShaderPermutationGL33::compileShader(const std::string &vertExtraDefStrings, const std::string &fragExtraDefStrings) {
    GShaderPermutationGL33::compileShader("", "");

    //Init newly created shader
    glUseProgram(this->m_programBuffer);

    glUniform1i(this->getUnf("uTexture"), 0);
    glUniform1i(this->getUnf("uTexture2"), 1);
    glUniform1i(this->getUnf("uTexture3"), 2);
    glUseProgram(0);
}
