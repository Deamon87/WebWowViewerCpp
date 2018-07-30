//
// Created by Deamon on 7/8/2018.
//

#include "GM2ShaderPermutation.h"

GM2ShaderPermutation::GM2ShaderPermutation(std::string &shaderName, GDevice &device) : GShaderPermutation(shaderName,
                                                                                                          device) {}

void GM2ShaderPermutation::compileShader() {
    GShaderPermutation::compileShader();

    //Init newly created shader
    glUseProgram(this->m_programBuffer);

    glUniform1i(this->getUnf("uTexture"), 0);
//    glUniform1i(this->getUnf("uTexture2"), 1);
//    glUniform1i(this->getUnf("uTexture3"), 2);
//    glUniform1i(this->getUnf("uTexture4"), 3);
    glUseProgram(0);
}
