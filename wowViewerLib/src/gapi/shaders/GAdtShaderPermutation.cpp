//
// Created by Deamon on 7/27/2018.
//

#include "GAdtShaderPermutation.h"

GAdtShaderPermutation::GAdtShaderPermutation(std::string &shaderName, GDevice &device) : GShaderPermutation(shaderName,
                                                                                                            device) {}

void GAdtShaderPermutation::compileShader() {
    GShaderPermutation::compileShader();

    //Init newly created shader
    glUseProgram(this->m_programBuffer);

    glUniform1i(this->getUnf("uLayer0"), 0);
    glUniform1i(this->getUnf("uLayer1"), 1);
    glUniform1i(this->getUnf("uLayer2"), 2);
    glUniform1i(this->getUnf("uLayer3"), 3);
    glUniform1i(this->getUnf("uAlphaTexture"), 4);
    glUniform1i(this->getUnf("uLayerHeight0"), 5);
    glUniform1i(this->getUnf("uLayerHeight1"), 6);
    glUniform1i(this->getUnf("uLayerHeight2"), 7);
    glUniform1i(this->getUnf("uLayerHeight3"), 8);
    glUseProgram(0);
}
