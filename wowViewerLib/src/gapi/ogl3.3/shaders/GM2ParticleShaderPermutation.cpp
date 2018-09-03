//
// Created by Deamon on 7/24/2018.
//

#include "GM2ParticleShaderPermutation.h"
GM2ParticleShaderPermutation::GM2ParticleShaderPermutation(std::string &shaderName, IDevice *device) : GShaderPermutation(shaderName,
                                                                                                          device) {}

void GM2ParticleShaderPermutation::compileShader() {
    GShaderPermutation::compileShader();

    //Init newly created shader
    glUseProgram(this->m_programBuffer);

    glUniform1i(this->getUnf("uTexture"), 0);
    glUniform1i(this->getUnf("uTexture2"), 1);
    glUniform1i(this->getUnf("uTexture3"), 2);
    glUseProgram(0);
}
