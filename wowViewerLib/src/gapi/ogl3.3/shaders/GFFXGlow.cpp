//
// Created by Deamon on 17.05.2020.
//

#include "GFFXGlow.h"

auto GFFXGlow_vertShaderName = std::string("drawQuad");
auto GFFXGlow_fragShaderName = std::string("ffxglow");

GFFXGlow::GFFXGlow(std::string &shaderName, const HGDevice &device) :
    GShaderPermutationGL33(GFFXGlow_vertShaderName, GFFXGlow_fragShaderName, device) {
}

void GFFXGlow::compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) {
    GShaderPermutationGL33::compileShader("", "");

    //Init newly created shader
    glUseProgram(this->m_programBuffer);

    glUniform1i(this->getUnf("screenTex"), 0);
    glUniform1i(this->getUnf("blurTex"), 1);
    glUseProgram(0);
}
