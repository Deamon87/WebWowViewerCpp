//
// Created by Deamon on 17.05.2020.
//

#include "GFFXgauss4.h"

auto vertShaderName = std::string("drawQuad");
auto fragShaderName = std::string("ffxgauss4");

GFFXgauss4::GFFXgauss4(std::string &shaderName, IDevice *device) :
    GShaderPermutationGL33(vertShaderName, fragShaderName, device) {


}

void GFFXgauss4::compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) {
    GShaderPermutationGL33::compileShader("", "");

    //Init newly created shader
    glUseProgram(this->m_programBuffer);

    glUniform1i(this->getUnf("texture0"), 0);
    glUseProgram(0);
}
