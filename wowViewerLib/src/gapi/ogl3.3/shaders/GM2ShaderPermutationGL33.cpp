//
// Created by Deamon on 7/8/2018.
//

#include "GM2ShaderPermutationGL33.h"

GM2ShaderPermutationGL33::GM2ShaderPermutationGL33(std::string &shaderName, IDevice *device) : GShaderPermutationGL33(shaderName,
                                                                                                          device) {}

void GM2ShaderPermutationGL33::compileShader() {
    GShaderPermutationGL33::compileShader();

    //Init newly created shader
    glUseProgram(this->m_programBuffer);

    glUniform1i(this->getUnf("uTexture"), 0);
    glUniform1i(this->getUnf("uTexture2"), 1);
    glUniform1i(this->getUnf("uTexture3"), 2);
    glUniform1i(this->getUnf("uTexture4"), 3);
    glUseProgram(0);
}
