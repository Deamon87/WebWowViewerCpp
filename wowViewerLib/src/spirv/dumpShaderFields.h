//
// Created by deamon on 04.12.19.
//

#ifndef AWEBWOWVIEWERCPP_DUMPSHADERFIELDS_H
#define AWEBWOWVIEWERCPP_DUMPSHADERFIELDS_H

#include <string>
#include <vector>
#include <iostream>
#include "fileHelpers.h"
#include "webGLSLCompiler.h"

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

void dumpShaderUniformOffsets(std::vector<std::string> &shaderFilePaths) {

    std::cout << "#include <string>" << std::endl;
    std::cout << "#include <unordered_map>" << std::endl;
    std::cout << "    struct fieldDefine {\n"
                 "        std::string name;\n"
                 "        bool isFloat ;\n"
                 "        int columns;\n"
                 "        int vecSize;\n"
                 "        int arraySize;\n"
                 "    };" << std::endl;


    std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderName;

    for (auto &filePath : shaderFilePaths) {

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

#endif //AWEBWOWVIEWERCPP_DUMPSHADERFIELDS_H
