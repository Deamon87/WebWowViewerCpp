//
// Created by deamon on 18.10.19.
//

#include "spirv_glsl.hpp"
#include "webGLSLCompiler.h"
#include <vector>
#include <utility>
#include <fstream>
#include <iostream>
#include <spirv.h>

std::string
basename(const std::string &filename)
{
    if (filename.empty()) {
        return {};
    }

    auto len = filename.length();
    auto index = filename.find_last_of("/\\");

    if (index == std::string::npos) {
        return filename;
    }

    if (index + 1 >= len) {

        len--;
        index = filename.substr(0, len).find_last_of("/\\");

        if (len == 0) {
            return filename;
        }

        if (index == 0) {
            return filename.substr(1, len - 1);
        }

        if (index == std::string::npos) {
            return filename.substr(0, len);
        }

        return filename.substr(index + 1, len - index - 1);
    }

    return filename.substr(index + 1, len - index);
}

std::vector<std::string> split(const std::string& s, char seperator)
{
    std::vector<std::string> output;

    std::string::size_type prev_pos = 0, pos = 0;

    while((pos = s.find(seperator, pos)) != std::string::npos)
    {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );

        output.push_back(substring);

        prev_pos = ++pos;
    }

    output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word

    return output;
}

static std::vector<uint32_t> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    file.seekg(0);
    file.read((char *) buffer.data(), fileSize);

    file.close();

    return buffer;
}


int main(int argc, char **argv)
{
    // Read SPIR-V from disk or similar.

    std::cout << "#include  \"shaderMetaData.h\"\n";
    std::cout << "const std::unordered_map<std::string, shaderMetaData> shaderMetaInfo = {";

    for (int i = 1; i < argc; i++) {
        std::string filePath = argv[i];
//        std::cerr << filePath << std::endl << std::flush;

        std::vector<uint32_t> spirv_binary = readFile(filePath);

        std::string fileName = basename(filePath);
        auto tokens = split(fileName, '.');


//        try {

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

//            for (int j = 0; j < res_type.columns; j++) {
//                glsl.set_member_decoration(resource.type_id, j, spv::Decoration::DecorationUniform);
//                std::string newName = "test_"+glsl.get_member_name(resource.type_id, j);
//
//                glsl.set_member_decoration_string(resource.type_id, j, spv::Decoration::DecorationUniform, newName);
//            }

//            for (auto &range : ranges)
//            {
//                const auto &name = glsl.get_member_name(resource.base_type_id, range.index);
//                std::string newName = "test_"+name;
//                glsl.set_member_name(resource.base_type_id, range.index, newName);
//            }

            std::cout << "{" << set << "," << binding << "," << typeId_size << "}," << std::endl;
        }
        std::cout << "}\n";
        std::cout << "}\n},";

        //        } catch (...) {
//            throw std::runtime_error("invalid file "+filePath);
//        }

        // Get all sampled images in the shader.
//        for (auto &resource : resources.uniform_buffers)
//        {
//            unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
//            unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
//            printf("Image %s at set = %u, binding = %u\n", resource.name.c_str(), set, binding);
//
//            // Modify the decoration to prepare it for GLSL.
//            glsl.unset_decoration(resource.id, spv::DecorationDescriptorSet);
//
//            // Some arbitrary remapping if we want.
//            glsl.set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
//        }

//        // Set some options.
//        spirv_cross::WebGLSLCompiler::Options options;
//        options.version = 110;
//        options.es = true;
//        glsl.set_common_options(options);
//
//
//        // Compile to GLSL, ready to give to GL driver.
//        std::string source = glsl.compile();
//        std::cout << source << std::endl;

    }
    std::cout << "};";

//    // Set some options.
//    spirv_cross::CompilerGLSL::Options options;
//    options.version = 310;
//    options.es = true;
//    glsl.set_common_options(options);
//
//    // Compile to GLSL, ready to give to GL driver.
//    std::string source = glsl.compile();



}