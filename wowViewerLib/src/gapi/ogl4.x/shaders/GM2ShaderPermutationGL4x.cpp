//
// Created by Deamon on 7/8/2018.
//

#include "GM2ShaderPermutationGL4x.h"

GM2ShaderPermutationGL4x::GM2ShaderPermutationGL4x(std::string &shaderName, IDevice *device) : GShaderPermutationGL4x(shaderName,
                                                                                                          device) {}

void GM2ShaderPermutationGL4x::compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) {
    GShaderPermutationGL4x::compileShader("", "");

    //Init newly created shader
    glUseProgram(this->m_programBuffer);

    glUniform1i(this->getUnf("uTexture"), 0);
    glUniform1i(this->getUnf("uTexture2"), 1);
    glUniform1i(this->getUnf("uTexture3"), 2);
    glUniform1i(this->getUnf("uTexture4"), 3);
    glUseProgram(0);
}
