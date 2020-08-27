//
// Created by Deamon on 17.05.2020.
//

#include "GSkyConus.h"

auto GSkyConus_vertShaderName = std::string("skyConus");
auto GSkyConus_fragShaderName = std::string("skyConus");

GSkyConus::GSkyConus(std::string &shaderName, IDevice *device) :
    GShaderPermutationGL33(GSkyConus_vertShaderName, GSkyConus_fragShaderName, device) {


}

void GSkyConus::compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) {
    GShaderPermutationGL33::compileShader("", "");

//    //Init newly created shader
//    glUseProgram(this->m_programBuffer);
//
//    glUniform1i(this->getUnf("texture0"), 0);
//    glUseProgram(0);
}
