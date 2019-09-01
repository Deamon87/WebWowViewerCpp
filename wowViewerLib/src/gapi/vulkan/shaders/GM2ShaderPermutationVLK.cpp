//
// Created by Deamon on 7/8/2018.
//

#include "GM2ShaderPermutationVLK.h"
#include <string>
#include <fstream>


static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

GM2ShaderPermutationVLK::GM2ShaderPermutationVLK(std::string &shaderName, IDevice *device, M2ShaderCacheRecord &permutation) :
                        GShaderPermutationVLK(shaderName, device) , permutation(permutation) {}

void GM2ShaderPermutationVLK::compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) {
    auto vertShaderCode = readFile("D:\\Sources\\WebWowViewerCpp\\wowViewerLib\\src\\glsl\\vulkan\\m2Shader.vert.spv");
    auto fragShaderCode = readFile("D:\\Sources\\WebWowViewerCpp\\wowViewerLib\\src\\glsl\\vulkan\\m2Shader.frag.spv");

    vertShaderModule = createShaderModule(vertShaderCode);
    fragShaderModule = createShaderModule(fragShaderCode);

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
