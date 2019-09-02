//
// Created by Deamon on 7/8/2018.
//

#include "GM2ShaderPermutationVLK.h"
#include <string>
#include <fstream>




GM2ShaderPermutationVLK::GM2ShaderPermutationVLK(std::string &shaderName, IDevice *device, M2ShaderCacheRecord &permutation) :
                        GShaderPermutationVLK(shaderName, device) , permutation(permutation) {}

void GM2ShaderPermutationVLK::compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) {
    GShaderPermutationVLK::compileShader(vertExtraDef, fragExtraDef);

//    std::string fragmentExtraDefines = "";
//    std::string vertexExtraDefines = "";
//
//    vertexExtraDefines += "#define VERTEXSHADER " + std::to_string(permutation.vertexShader)+"\n";
//    vertexExtraDefines += "#define UNLIT " + std::to_string(permutation.unlit)+"\n";
//    vertexExtraDefines += "#define BONEINFLUENCES " + std::to_string(permutation.boneInfluences)+"\n";
//
//    fragmentExtraDefines += "#define FRAGMENTSHADER " + std::to_string(permutation.pixelShader)+"\n";
//    fragmentExtraDefines += "#define UNLIT " + std::to_string(permutation.unlit ? 1 : 0)+"\n";
//    fragmentExtraDefines += "#define ALPHATEST_ENABLED " + std::to_string(permutation.alphaTestOn ? 1 : 0)+"\n";
//    fragmentExtraDefines += "#define UNFOGGED " + std::to_string(permutation.unFogged ? 1 : 0)+"\n";
//    fragmentExtraDefines += "#define UNSHADOWED " + std::to_string(permutation.unShadowed ? 1 : 0)+"\n";
//
//
//
//    GM2ShaderPermutationVLK::compileShader(vertexExtraDefines, fragmentExtraDefines);


}
