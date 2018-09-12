//
// Created by Deamon on 7/27/2018.
//

#include "GWMOShaderPermutationGL33.h"


GWMOShaderPermutationGL33::GWMOShaderPermutationGL33(std::string &shaderName, IDevice *device) : GShaderPermutationGL33(shaderName,
                                                                                                                          device) {}

void GWMOShaderPermutationGL33::compileShader() {
    GShaderPermutationGL33::compileShader();

    //Init newly created shader
    glUseProgram(this->m_programBuffer);

    glUniform1i(this->getUnf("uTexture"), 0);
    glUniform1i(this->getUnf("uTexture2"), 1);
    glUniform1i(this->getUnf("uTexture3"), 2);
    glUseProgram(0);
}
