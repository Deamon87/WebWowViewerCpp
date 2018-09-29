//
// Created by Deamon on 7/8/2018.
//

#include "GM2ShaderPermutationGL33.h"
#include <string>

GM2ShaderPermutationGL33::GM2ShaderPermutationGL33(std::string &shaderName, IDevice *device, M2ShaderCacheRecord &permutation) :
                        GShaderPermutationGL33(shaderName, device) , permutation(permutation) {}

void GM2ShaderPermutationGL33::compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) {
    std::string fragmentExtraDefines = "";
    std::string vertexExtraDefines = "";

    vertexExtraDefines += "#define VERTEXSHADER " + std::to_string(permutation.vertexShader)+"\n";
    vertexExtraDefines += "#define UNLIT " + std::to_string(permutation.unlit)+"\n";

    fragmentExtraDefines += "#define FRAGMENTSHADER " + std::to_string(permutation.pixelShader)+"\n";
    fragmentExtraDefines += "#define UNLIT " + std::to_string(permutation.unlit ? 1 : 0)+"\n";
    fragmentExtraDefines += "#define ALPHATEST_ENABLED " + std::to_string(permutation.alphaTestOn ? 1 : 0)+"\n";
    fragmentExtraDefines += "#define UNFOGGED " + std::to_string(permutation.unFogged ? 1 : 0)+"\n";
    fragmentExtraDefines += "#define UNSHADOWED " + std::to_string(permutation.unShadowed ? 1 : 0)+"\n";


    GShaderPermutationGL33::compileShader(vertexExtraDefines, fragmentExtraDefines);

    //Init newly created shader
    glUseProgram(this->m_programBuffer);

    if (hasUnf("uTexture")) {
        glUniform1i(this->getUnf("uTexture"), 0);
    }
    if (hasUnf("uTexture2")) {
        glUniform1i(this->getUnf("uTexture2"), 1);
    }
    if (hasUnf("uTexture3")) {
        glUniform1i(this->getUnf("uTexture3"), 2);
    }
    if (hasUnf("uTexture4")) {
        glUniform1i(this->getUnf("uTexture4"), 2);
    }

    glUseProgram(0);
}
