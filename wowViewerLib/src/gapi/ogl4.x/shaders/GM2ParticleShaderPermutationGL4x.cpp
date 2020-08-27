//
// Created by Deamon on 7/24/2018.
//

#include "GM2ParticleShaderPermutationGL4x.h"
GM2ParticleShaderPermutationGL4x::GM2ParticleShaderPermutationGL4x(std::string &shaderName, IDevice *device) : GShaderPermutationGL4x(shaderName,
                                                                                                          device) {}

void GM2ParticleShaderPermutationGL4x::compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) {
    GShaderPermutationGL4x::compileShader("", "");

    //Init newly created shader
    glUseProgram(this->m_programBuffer);

    glUniform1i(this->getUnf("uTexture"), 0);
    glUniform1i(this->getUnf("uTexture2"), 1);
    glUniform1i(this->getUnf("uTexture3"), 2);
    glUseProgram(0);
}
