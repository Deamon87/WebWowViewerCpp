//
// Created by Deamon on 7/24/2018.
//

#include "CM2ParticleShaderPermutation.h"
CM2ParticleShaderPermutation::CM2ParticleShaderPermutation(std::string &shaderName, GDevice &device) : GShaderPermutation(shaderName,
                                                                                                          device) {}

void CM2ParticleShaderPermutation::compileShader() {
    GShaderPermutation::compileShader();

    //Init newly created shader
    glUseProgram(this->m_programBuffer);

    glUniform1i(this->getUnf("uTexture"), 0);
    glUniform1i(this->getUnf("uTexture2"), 1);
    glUniform1i(this->getUnf("uTexture3"), 2);
    glUseProgram(0);
}
