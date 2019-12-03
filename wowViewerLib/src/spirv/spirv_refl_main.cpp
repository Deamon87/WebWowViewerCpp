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
#include <map>

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

void dumpMetaData(int argc, char **argv) {
    // Read SPIR-V from disk or similar.

    std::cout << "#include  \"shaderMetaData.h\"\n";
    std::cout << "const std::unordered_map<std::string, shaderMetaData> shaderMetaInfo = {";

    for (int i = 1; i < argc; i++) {
        std::string filePath = argv[i];
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


struct fieldDefine {
    std::string name;
    bool isFloat ;
    int columns;
    int vecSize;
    int arraySize;
};
void dumpMembers(spirv_cross::WebGLSLCompiler &glsl, std::vector<fieldDefine> &fieldDefines, spirv_cross::TypeID type, std::string namePrefix, int offset, int currmemberSize) {
    auto memberType = glsl.get_type(type);
    int arraySize = memberType.array.size() > 0 ? memberType.array[0] : 0;
    bool arrayLiteral = memberType.array_size_literal.size() > 0 ? memberType.array_size_literal[0] : 0;
    int vecSize = memberType.vecsize;
    int width = memberType.width;
    int columns = memberType.columns;


    bool isStruct = memberType.basetype == spirv_cross::SPIRType::Struct;

    if (isStruct) {
        auto submemberType = glsl.get_type(memberType.parent_type);
        int structSize = submemberType.vecsize * submemberType.columns*(submemberType.width/8);

        if (arrayLiteral) {
            for (int j = 0; j < arraySize; j++) {
                for (int k = 0; k < memberType.member_types.size(); k++) {
//                auto memberSize = glsl.get_declared_struct_member_size(type,., j);
//                auto offset = glsl.type_struct_member_offset(uboParentType, j);



                    auto memberName = glsl.get_member_name(memberType.parent_type, k);
                    auto memberOffset = glsl.type_struct_member_offset(memberType, k);
                    auto memberSize = glsl.get_declared_struct_member_size(memberType, k);

                    dumpMembers(glsl, fieldDefines, memberType.member_types[k],
                                namePrefix + "[" + std::to_string(j) + "]" + "." + memberName, offset+(j*(currmemberSize/arraySize))+memberOffset, memberSize);
                }
            }
        } else {
            for (int k = 0; k < memberType.member_types.size(); k++) {
                auto memberName = glsl.get_member_name(memberType.parent_type, k);
                auto memberSize = glsl.get_declared_struct_member_size(memberType, k);
                auto memberOffset = glsl.type_struct_member_offset(memberType, k);
                dumpMembers(glsl, fieldDefines, memberType.member_types[k], namePrefix + "." + memberName, offset+memberOffset, memberSize);
            }
        }
    } else {
        bool isFloat = (memberType.basetype == spirv_cross::SPIRType::Float);
//        std::cout << "{\"" <<namePrefix <<"\","<< isFloat << ", " <<columns<<","<<vecSize<<","<<arraySize << "}"<<std::endl;
        fieldDefines.push_back({namePrefix,isFloat ,columns,vecSize,arraySize});

//        std::cout <<
//                  namePrefix <<
//                  ", columns = " << columns <<
//                  ", isFloat = " << (memberType.basetype == spirv_cross::SPIRType::Float) <<
//                  ", memberSize = " << currmemberSize <<
//                  ", vecSize = " << vecSize <<
//                  ", width = " << width <<
//                  ", arraySize = " << arraySize <<
//                  ", arrayLiteral = " << arrayLiteral <<
//                  "  offset = " << offset << std::endl;
    }
}

void dumpShaderUniformOffsets(int argc, char **argv) {

    std::cout << "#include <string>" << std::endl;
    std::cout << "#include <unordered_map>" << std::endl;
    std::cout << "    struct fieldDefine {\n"
                 "        std::string name;\n"
                 "        bool isFloat ;\n"
                 "        int columns;\n"
                 "        int vecSize;\n"
                 "        int arraySize;\n"
                 "    };" << std::endl;

//    std::cout << " std::unordered_map<int, std::vector<fieldDefine>> fieldDefMap = {\n"
//                 "        {\n"
//                 "            0,\n"
//                 "            {\n"
//                 "                {\"test\", false, 1, 1, 1}\n"
//                 "            }\n"
//                 "        }\n"
//                 "    };";


    std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderName;

    for (int i = 1; i < argc; i++) {
        std::string filePath = argv[i];
//        std::cerr << filePath << std::endl << std::flush;

        std::vector<uint32_t> spirv_binary = readFile(filePath);

        std::string fileName = basename(filePath);
        auto tokens = split(fileName, '.');


        //Find or create new record for shader

        auto it = fieldDefMapPerShaderName.find(tokens[0]);
        if (it == fieldDefMapPerShaderName.end()) {
            fieldDefMapPerShaderName[tokens[0]] = {};
        }
        auto &perSetMap = fieldDefMapPerShaderName.at(tokens[0]);


        spirv_cross::WebGLSLCompiler glsl(std::move(spirv_binary));

        // The SPIR-V is now parsed, and we can perform reflection on it.
        spirv_cross::ShaderResources resources = glsl.get_shader_resources();


        for (auto &resource : resources.uniform_buffers) {
            auto uboType = glsl.get_type(resource.type_id);

            auto typeId_size = glsl.get_declared_struct_size(uboType);

            unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
            unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);

            if (perSetMap.find(binding) != perSetMap.end()) {
                perSetMap[binding]={};
            }

            auto &fieldVectorDef = perSetMap[binding];

            for (int j = 0; j < uboType.member_types.size(); j++) {

                auto uboParentType = glsl.get_type(uboType.parent_type);
                auto memberSize = glsl.get_declared_struct_member_size(uboParentType, j);
                auto offset = glsl.type_struct_member_offset(uboParentType, j);
                auto memberName = glsl.get_member_name(uboType.parent_type, j);


                dumpMembers(glsl, fieldVectorDef, uboType.member_types[j], "_" +std::to_string(resource.id) + "."+memberName, offset, memberSize);

            }
        }
    }

    std::cout << "std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderName = {" << std::endl;
    for (auto it = fieldDefMapPerShaderName.begin(); it != fieldDefMapPerShaderName.end(); it++) {
        std::cout << "{\"" << it->first << "\", " << " {" << std::endl;

        for (auto subIt = it->second.begin(); subIt != it->second.end(); subIt++) {
            std::cout << "{" << std::endl;

            std::cout << subIt->first <<", {" << std::endl;

            for (auto &fieldDef : subIt->second) {
                std::cout << "{"
                    << "\"" << fieldDef.name << "\", "
                    << (fieldDef.isFloat ? "true" : "false") << ", "
                    << fieldDef.columns << ", "
                    << fieldDef.vecSize << ", "
                    << fieldDef.arraySize << "}," << std::endl;
            }

            std::cout << "}" << std::endl;

            std::cout << "}," << std::endl;
        }

        std::cout << "}},";


    }

    std::cout << "};" << std::endl;
}

void dumpGLSLText(int argc, char **argv) {

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


int main(int argc, char **argv)
{
//    dumpMetaData(argc, argv);
    dumpShaderUniformOffsets(argc, argv);

//    struct fieldDefine {
//        std::string name;
//        bool isFloat ;
//        int columns;
//        int vecSize;
//        int arraySize;
//    };
//    std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderName = {
//        {
//            0,
//            {
//                {"test", false, 1, 1, 1}
//            }
//        }
//    };


}