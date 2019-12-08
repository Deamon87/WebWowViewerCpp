//
// Created by deamon on 04.12.19.
//

#ifndef AWEBWOWVIEWERCPP_DUMPSHADERMETADATA_H
#define AWEBWOWVIEWERCPP_DUMPSHADERMETADATA_H

#include <string>
#include <vector>
#include <iostream>
#include "fileHelpers.h"
#include "webGLSLCompiler.h"

void dumpMetaData(std::vector<std::string> &shaderFilePaths) {
    // Read SPIR-V from disk or similar.

    std::cout << "#include  \"shaderMetaData.h\"\n";
    std::cout << "const std::unordered_map<std::string, shaderMetaData> shaderMetaInfo = {";

    for (auto &filePath : shaderFilePaths) {
//        std::cerr << filePath << std::endl << std::flush;

        std::vector<uint32_t> spirv_binary = readFile(filePath);

        std::string fileName = basename(filePath);
        auto tokens = split(fileName, '.');

        spirv_cross::WebGLSLCompiler glsl(std::move(spirv_binary));

        // The SPIR-V is now parsed, and we can perform reflection on it.
        spirv_cross::ShaderResources resources = glsl.get_shader_resources();

        // Get all sampled images in the shader.
        std::cout << "{ \"" << fileName << "\", {\n";

        std::cout << "{\n";
        for (auto &resource : resources.uniform_buffers) {
            auto typeId_size = glsl.get_declared_struct_size(glsl.get_type(resource.type_id));

            unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
            unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);

            std::cout << "{" << set << "," << binding << "," << typeId_size << "}," << std::endl;
        }
        std::cout << "}\n";
        std::cout << "}\n},";


    }
    std::cout << "};";
}

#endif //AWEBWOWVIEWERCPP_DUMPSHADERMETADATA_H
