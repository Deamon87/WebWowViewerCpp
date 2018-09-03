//
// Created by Deamon on 7/27/2018.
//

#include "GWMOShaderPermutation.h"


GWMOShaderPermutation::GWMOShaderPermutation(std::string &shaderName, IDevice *device) : GShaderPermutation(shaderName,
                                                                                                                          device) {}

void GWMOShaderPermutation::compileShader() {
    GShaderPermutation::compileShader();

    //Init newly created shader
    glUseProgram(this->m_programBuffer);

    glUniform1i(this->getUnf("uTexture"), 0);
    glUniform1i(this->getUnf("uTexture2"), 1);
    glUniform1i(this->getUnf("uTexture3"), 2);
    glUseProgram(0);
}
