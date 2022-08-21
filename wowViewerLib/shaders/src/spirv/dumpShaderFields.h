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

struct attributeDefine {
    std::string name;
    unsigned int location;
};

struct fieldDefine {
    std::string name;
    bool isFloat ;
    int offset;
    int columns;
    int vecSize;
    int arraySize;
};

struct uboBindingData {
    unsigned int set;
    unsigned int binding;
    unsigned long long size;
};

struct shaderMetaData {
    std::vector<uboBindingData> uboBindings;
};

//Per file
std::unordered_map<std::string, shaderMetaData> shaderMetaInfo;


void dumpMembers(spirv_cross::WebGLSLCompiler &glsl, std::vector<fieldDefine> &fieldDefines, spirv_cross::TypeID type, std::string namePrefix, int offset, int currmemberSize) {
    auto memberType = glsl.get_type(type);
    int arraySize = memberType.array.size() > 0 ? memberType.array[0] : 0;
    bool arrayLiteral = memberType.array_size_literal.size() > 0 ? memberType.array_size_literal[0] : 0;
    int vecSize = memberType.vecsize;
    int width = memberType.width;
    int columns = memberType.columns;


    bool isStruct = memberType.basetype == spirv_cross::SPIRType::Struct;

    if (isStruct) {
        auto parentTypeId = memberType.parent_type;
        if (parentTypeId == spirv_cross::TypeID(0)) {
            parentTypeId = memberType.type_alias;
        }
        if (parentTypeId == spirv_cross::TypeID(0)) {
            parentTypeId = memberType.self;
        }
//
//        auto submemberType = glsl.get_type(submemberTypeId);
//        int structSize = submemberType.vecsize * submemberType.columns*(submemberType.width/8);

        if (arrayLiteral) {
            for (int j = 0; j < arraySize; j++) {
                for (int k = 0; k < memberType.member_types.size(); k++) {
                    auto memberName = glsl.get_member_name(parentTypeId, k);
                    auto memberOffset = glsl.type_struct_member_offset(memberType, k);
                    auto memberSize = glsl.get_declared_struct_member_size(memberType, k);

                    dumpMembers(glsl, fieldDefines, memberType.member_types[k],
                                namePrefix + "[" + std::to_string(j) + "]" + "." + memberName, offset+(j*(currmemberSize/arraySize))+memberOffset, memberSize);
                }
            }
        } else {
            for (int k = 0; k < memberType.member_types.size(); k++) {
                auto memberName = glsl.get_member_name(parentTypeId, k);
                auto memberSize = glsl.get_declared_struct_member_size(memberType, k);
                auto memberOffset = glsl.type_struct_member_offset(memberType, k);
                dumpMembers(glsl, fieldDefines, memberType.member_types[k], namePrefix + "_" + memberName, offset+memberOffset, memberSize);
            }
        }
    } else {
        bool isFloat = (memberType.basetype == spirv_cross::SPIRType::Float);
//        std::cout << "{\"" <<namePrefix <<"\","<< isFloat << ", " <<columns<<","<<vecSize<<","<<arraySize << "}"<<std::endl;
        fieldDefines.push_back({namePrefix, isFloat, offset, columns,vecSize,arraySize});

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
    std::cout << "#ifndef WOWMAPVIEWERREVIVED_SHADERDEFINITIONS_H\n"
                 "#define WOWMAPVIEWERREVIVED_SHADERDEFINITIONS_H\n"
                 "\n"
                 "#include <string>\n"
                 "#include <iostream>\n"
                 "#include <fstream>\n"
                 "#include <vector>\n"
                 "#include <unordered_map>\n"
                 "\n"
                 "template <typename T>\n"
                 "inline constexpr const uint32_t operator+ (T const val) { return static_cast<const uint32_t>(val); };"
              << std::endl;


    std::cout << "struct fieldDefine {\n"
                 "    std::string name;\n"
                 "    bool isFloat ;\n"
                 "    int offset;\n"
                 "    int columns;\n"
                 "    int vecSize;\n"
                 "    int arraySize;\n"
                 "};" << std::endl;

    std::cout << "struct attributeDefine {\n"
                 "    std::string name;\n"
                 "    int location;\n"
                 "};" << std::endl;

    std::cout << "struct uboBindingData {\n"
                 "    int set;\n"
                 "    int binding;\n"
                 "    int size;\n"
                 "};\n"
                 "\n"
                 "struct shaderMetaData {\n"
                 "    std::vector<uboBindingData> uboBindings;\n"
                 "};\n"
                 "\n"
                 "//Per file\n"
                 "extern const std::unordered_map<std::string, shaderMetaData> shaderMetaInfo;" << std::endl <<
              "extern const std::unordered_map<std::string, std::vector<attributeDefine>> attributesPerShaderName;"
              << std::endl <<
              "extern const std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameVert;"
              << std::endl <<
              "extern const std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameFrag;"
              << std::endl;


    std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameVert;
    std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameFrag;
    std::unordered_map<std::string, std::vector<attributeDefine>> attributesPerShaderName;

    //1. Collect data
    for (auto &filePath : shaderFilePaths) {

//        std::cerr << filePath << std::endl << std::flush;

        std::vector<uint32_t> spirv_binary = readFile(filePath);

        std::string fileName = basename(filePath);
        auto tokens = split(fileName, '.');

        spirv_cross::WebGLSLCompiler glsl(std::move(spirv_binary));

        auto fieldDefMapPerShaderNameRef = std::ref(fieldDefMapPerShaderNameVert);
        if (glsl.get_entry_points_and_stages()[0].execution_model == spv::ExecutionModel::ExecutionModelFragment) {
            fieldDefMapPerShaderNameRef = std::ref(fieldDefMapPerShaderNameFrag);
        }
        auto &fieldDefMapPerShaderName = fieldDefMapPerShaderNameRef.get();


        //Find or create new record for shader
        {
            auto it = fieldDefMapPerShaderName.find(tokens[0]);
            if (it == fieldDefMapPerShaderName.end()) {
                fieldDefMapPerShaderName[tokens[0]] = {};
            }
        }

        {
            auto it = shaderMetaInfo.find(fileName);
            if (it == shaderMetaInfo.end()) {
                shaderMetaInfo[fileName] = {};
            }
        }
        auto &perSetMap = fieldDefMapPerShaderName.at(tokens[0]);
        auto &metaInfo = shaderMetaInfo.at(fileName);

        if (glsl.get_entry_points_and_stages()[0].execution_model == spv::ExecutionModel::ExecutionModelVertex) {
            auto it = attributesPerShaderName.find(tokens[0]);
            if (it == attributesPerShaderName.end()) {
                attributesPerShaderName[tokens[0]] = {};
            }
            auto &shaderAttributeVector = attributesPerShaderName.at(tokens[0]);


            auto inputAttributes = glsl.get_shader_resources();
            for (auto &attribute : inputAttributes.stage_inputs) {
                //Create a record if it didnt exist yet

                auto location = glsl.get_decoration(attribute.id, spv::DecorationLocation);

                shaderAttributeVector.push_back({attribute.name, location});
            }

            std::sort(shaderAttributeVector.begin(), shaderAttributeVector.end(),
                      [](const attributeDefine &a, const attributeDefine &b) -> bool {
                          return a.location < b.location;
                      });
        }

        // The SPIR-V is now parsed, and we can perform reflection on it.
        spirv_cross::ShaderResources resources = glsl.get_shader_resources();
        for (auto &resource : resources.uniform_buffers) {
            auto uboType = glsl.get_type(resource.type_id);

            auto typeId_size = glsl.get_declared_struct_size(uboType);

            unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
            unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);

            metaInfo.uboBindings.push_back({set, binding, typeId_size});

            if (perSetMap.find(binding) != perSetMap.end()) {
                perSetMap[binding] = {};
            }

            auto &fieldVectorDef = perSetMap[binding];

            for (int j = 0; j < uboType.member_types.size(); j++) {

                auto uboParentType = glsl.get_type(uboType.parent_type);
                auto memberSize = glsl.get_declared_struct_member_size(uboParentType, j);
                auto offset = glsl.type_struct_member_offset(uboParentType, j);
                auto memberName = glsl.get_member_name(uboType.parent_type, j);


                dumpMembers(glsl, fieldVectorDef, uboType.member_types[j],
                            "_" + std::to_string(resource.id) + "_" + memberName, offset, memberSize);

            }
        }
    }

    //2.1 Create attribute enums
    for (auto it = attributesPerShaderName.begin(); it != attributesPerShaderName.end(); it++) {
        std::cout << "struct " << it->first << " {\n"
                                               "    enum class Attribute {" << std::endl;

        std::cout << "        ";
        for (auto &attributeInfo : it->second) {
            std::cout << "" << attributeInfo.name << " = " << attributeInfo.location << ", ";
        }

        std::cout << it->first << "AttributeEnd" << std::endl;
        std::cout << "    };\n"
                     "};" << std::endl << std::endl;
    }
    std::cout << "std::string loadShader(std::string shaderName);" << std::endl;

    //3.1 cpp only data

    std::cout << "#ifdef SHADERDATACPP" << std::endl;

    //3.2 Dump attribute info
    std::cout << "const std::unordered_map<std::string, std::vector<attributeDefine>> attributesPerShaderName = {"
              << std::endl;

    for (auto it = attributesPerShaderName.begin(); it != attributesPerShaderName.end(); it++) {
        std::cout << "{\"" << it->first << "\", " << " {" << std::endl;

        for (auto &attributeInfo : it->second) {
            std::cout << "{\"" << attributeInfo.name << "\", " << attributeInfo.location << "}," << std::endl;
        }

        std::cout << "}},";
    }
    std::cout << "};" << std::endl << std::endl;

    //Add shader meta
    std::cout << "const std::unordered_map<std::string, shaderMetaData> shaderMetaInfo = {";
    for (auto it = shaderMetaInfo.begin(); it != shaderMetaInfo.end(); it++) {
        std::cout << "{ \"" << it->first << "\", {\n";
        std::cout << "{\n";

        for (auto subIt = it->second.uboBindings.begin(); subIt != it->second.uboBindings.end(); subIt++) {
            std::cout << "{" << subIt->set << "," << subIt->binding << "," << subIt->size << "}," << std::endl;
        }

        std::cout << "}\n";
        std::cout << "}\n},";


    }
    std::cout << "};" << std::endl << std::endl;




    //Dump unfform bufffers info
    auto dumpLambda = [](auto fieldDefMapPerShaderName) {
        for (auto it = fieldDefMapPerShaderName.begin(); it != fieldDefMapPerShaderName.end(); it++) {
            std::cout << "  {\"" << it->first << "\", " << " {" << std::endl;

            for (auto subIt = it->second.begin(); subIt != it->second.end(); subIt++) {
                std::cout << "    {" << std::endl;

                std::cout << "      " << subIt->first << ", {" << std::endl;

                for (auto &fieldDef : subIt->second) {
                    std::cout << "        {"
                              << "\"" << fieldDef.name << ((fieldDef.arraySize > 0) ? "[0]" : "") << "\", "
                              << (fieldDef.isFloat ? "true" : "false") << ", "
                              << fieldDef.offset << ", "
                              << fieldDef.columns << ", "
                              << fieldDef.vecSize << ", "
                              << fieldDef.arraySize << "}," << std::endl;
                }

                std::cout << "      }" << std::endl;

                std::cout << "    }," << std::endl;
            }

            std::cout << "  }}," << std::endl;
        }
    };
    std::cout
        << "const  std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameVert = {"
        << std::endl;
    dumpLambda(fieldDefMapPerShaderNameVert);
    std::cout << "};" << std::endl;

    std::cout
        << "const  std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameFrag = {"
        << std::endl;
    dumpLambda(fieldDefMapPerShaderNameFrag);
    std::cout << "};" << std::endl;

    std::cout << "#endif" << std::endl << std::endl;


    std::cout << std::endl << "#endif" << std::endl;
}

#endif //AWEBWOWVIEWERCPP_DUMPSHADERFIELDS_H
