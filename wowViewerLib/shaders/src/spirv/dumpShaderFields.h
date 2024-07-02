//
// Created by deamon on 04.12.19.
//

#ifndef AWEBWOWVIEWERCPP_DUMPSHADERFIELDS_H
#define AWEBWOWVIEWERCPP_DUMPSHADERFIELDS_H

#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <sstream>
#include "fileHelpers.h"
#include "webGLSLCompiler.h"
#include "../../../src/include/string_utils.h"

constexpr const int MAX_SHADER_DESC_SETS = 8;

enum class ShaderStage {
    Unk, Vertex, Fragment, RayGenerate, RayAnyHit, RayClosestHit, RayMiss
};

#define printStage(stage) case stage: return #stage; break;
std::string ShaderStageToStr(ShaderStage stage) {
    switch (stage) {
        printStage(ShaderStage::Unk)
        printStage(ShaderStage::Vertex)
        printStage(ShaderStage::Fragment)
        printStage(ShaderStage::RayGenerate)
        printStage(ShaderStage::RayAnyHit)
        printStage(ShaderStage::RayClosestHit)
        printStage(ShaderStage::RayMiss)
    }
}

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
struct ssboBindingData {
    unsigned int set;
    unsigned int binding;
    unsigned long long size;
};

struct imageBindingData {
    unsigned int set;
    unsigned int binding;
    std::string imageName;
};

struct bindingAmountData {
    unsigned int start = 999;
    unsigned int end = 0;
    unsigned int length = 0;
};

struct shaderMetaData {
    ShaderStage stage;
    std::vector<uboBindingData> m_uboBindings;
    std::vector<ssboBindingData> m_ssboBindingData;
    std::array<bindingAmountData, MAX_SHADER_DESC_SETS> uboBindingAmountsPerSet;

    std::vector<imageBindingData> imageBindings;
    std::array<bindingAmountData, MAX_SHADER_DESC_SETS> imageBindingAmountsPerSet;
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

void dumpShaderUniformOffsets(const std::string &basePath, const std::vector<std::string> &shaderFilePaths) {
    std::cout << "#ifndef WOWMAPVIEWERREVIVED_SHADERDEFINITIONS_H\n"
                 "#define WOWMAPVIEWERREVIVED_SHADERDEFINITIONS_H\n"
                 "\n"
                 "#include <string>\n"
                 "#include <iostream>\n"
                 "#include <fstream>\n"
                 "#include <vector>\n"
                 "#include <array>\n"
                 "#include <unordered_map>\n"
                 "#include <cstdint>\n"
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

    std::cout <<
    R"===(
    constexpr const int MAX_SHADER_DESC_SETS = 8;

    enum class ShaderStage {
        Unk, Vertex, Fragment, RayGenerate, RayAnyHit, RayClosestHit, RayMiss
    };


    struct uboBindingData {
        unsigned int set;
        unsigned int binding;
        unsigned long long size;
    };
    struct ssboBindingData {
        unsigned int set;
        unsigned int binding;
        unsigned long long size;
    };
    struct imageBindingData {
        unsigned int set;
        unsigned int binding;
        std::string imageName;
    };

    struct bindingAmountData {
        unsigned int start = 999;
        unsigned int end = 0;
        unsigned int length = 0;
    };

    struct shaderMetaData {
        ShaderStage stage;

        std::vector<uboBindingData> uboBindings;
        std::array<bindingAmountData, MAX_SHADER_DESC_SETS> uboBindingAmountsPerSet;

        std::vector<ssboBindingData> m_ssboBindings;

        std::vector<imageBindingData> imageBindings;
        std::array<bindingAmountData, MAX_SHADER_DESC_SETS> imageBindingAmountsPerSet;
    };

    //Per file
    extern const std::unordered_map<std::string, shaderMetaData> shaderMetaInfo;
    extern const std::unordered_map<std::string, std::vector<attributeDefine>> attributesPerShaderName;

    extern const std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameVert;

    extern const std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameFrag;
)===" << std::endl;


    std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameVert;
    std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameFrag;
    std::unordered_map<std::string, std::vector<attributeDefine>> attributesPerShaderName;

    //1. Collect data
    for (auto &filePath : shaderFilePaths) {

//        std::cerr << filePath << std::endl << std::flush;

        std::vector<uint32_t> spirv_binary = readFile(filePath);


        std::string fileName = startsWith(filePath, basePath) ?
                               filePath.substr(basePath.size()+1, filePath.size()-basePath.size()) :
                               basename(filePath);

        std::string shaderName = basename(filePath);
        auto tokens = split(shaderName, '.');
        shaderName = tokens[0];



        spirv_cross::WebGLSLCompiler glsl(std::move(spirv_binary));

        auto fieldDefMapPerShaderNameRef = std::ref(fieldDefMapPerShaderNameVert);
        if (glsl.get_entry_points_and_stages()[0].execution_model == spv::ExecutionModel::ExecutionModelFragment) {
            fieldDefMapPerShaderNameRef = std::ref(fieldDefMapPerShaderNameFrag);
        }
        auto &fieldDefMapPerShaderName = fieldDefMapPerShaderNameRef.get();


        //Find or create new record for shader
        {
            auto it = fieldDefMapPerShaderName.find(shaderName);
            if (it == fieldDefMapPerShaderName.end()) {
                fieldDefMapPerShaderName[shaderName] = {};
            }
        }

        {
            auto it = shaderMetaInfo.find(fileName);
            if (it == shaderMetaInfo.end()) {
                shaderMetaInfo[fileName] = {};
            }
        }
        auto &perSetMap = fieldDefMapPerShaderName.at(shaderName);
        auto &metaInfo = shaderMetaInfo.at(fileName);


        auto execModel = glsl.get_entry_points_and_stages()[0].execution_model;
        metaInfo.stage = [&execModel]() -> ShaderStage {
            switch (execModel) {
                case(spv::ExecutionModel::ExecutionModelVertex):
                    return ShaderStage::Vertex;

                case(spv::ExecutionModel::ExecutionModelFragment):
                    return ShaderStage::Fragment;

                case(spv::ExecutionModel::ExecutionModelAnyHitKHR):
                    return ShaderStage::RayAnyHit;

                case(spv::ExecutionModel::ExecutionModelClosestHitKHR):
                    return ShaderStage::RayClosestHit;

                case(spv::ExecutionModel::ExecutionModelRayGenerationKHR):
                    return ShaderStage::RayGenerate;

                case(spv::ExecutionModel::ExecutionModelMissKHR):
                    return ShaderStage::RayMiss;

                default:
                    return ShaderStage::Unk;
            }
        }();

        if (glsl.get_entry_points_and_stages()[0].execution_model == spv::ExecutionModel::ExecutionModelVertex) {
            auto it = attributesPerShaderName.find(shaderName);
            if (it == attributesPerShaderName.end()) {
                attributesPerShaderName[shaderName] = {};
            }
            auto &shaderAttributeVector = attributesPerShaderName.at(shaderName);


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
            shaderAttributeVector.erase(std::unique(shaderAttributeVector.begin(), shaderAttributeVector.end(), [](attributeDefine &a, attributeDefine &b) -> bool {
                return a.name == b.name && a.location == b.location;
            }), shaderAttributeVector.end());
        }

        // The SPIR-V is now parsed, and we can perform reflection on it.
        spirv_cross::ShaderResources resources = glsl.get_shader_resources();

        //Record data for UBO
        for (auto &resource : resources.uniform_buffers) {
            auto uboType = glsl.get_type(resource.type_id);

            auto typeId_size = glsl.get_declared_struct_size(uboType);

            unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
            unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);

            metaInfo.m_uboBindings.push_back({set, binding, typeId_size});

            if (set >= 0) {
                metaInfo.uboBindingAmountsPerSet[set].start =
                    std::min<unsigned int>(metaInfo.uboBindingAmountsPerSet[set].start, binding);
                metaInfo.uboBindingAmountsPerSet[set].end =
                    std::max<unsigned int>(metaInfo.uboBindingAmountsPerSet[set].end, binding);
            }

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
//                            "_" + std::to_string(resource.id) + "_" + memberName, offset, memberSize);
                glsl.to_name(resource.id) + "_" + memberName, offset, memberSize);
            }
        }
        for (auto &data: metaInfo.uboBindingAmountsPerSet ) {
            if (data.start < 100) {
                data.length = data.end - data.start + 1;
            } else {
                data.start = 0;
            }
        }

        //Record data for SSBO
        for (auto &resource : resources.storage_buffers) {
            auto ssboType = glsl.get_type(resource.type_id);

            auto typeId_size = glsl.get_declared_struct_size(ssboType);

            unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
            unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);

            metaInfo.m_ssboBindingData.push_back({set, binding, typeId_size});

//            if (set >= 0) {
//                metaInfo.uboBindingAmountsPerSet[set].start =
//                    std::min<unsigned int>(metaInfo.uboBindingAmountsPerSet[set].start, binding);
//                metaInfo.uboBindingAmountsPerSet[set].end =
//                    std::max<unsigned int>(metaInfo.uboBindingAmountsPerSet[set].end, binding);
//            }

            if (perSetMap.find(binding) != perSetMap.end()) {
                perSetMap[binding] = {};
            }

            auto &fieldVectorDef = perSetMap[binding];

            for (int j = 0; j < ssboType.member_types.size(); j++) {

                auto uboParentType = glsl.get_type(ssboType.parent_type);
                auto memberSize = glsl.get_declared_struct_member_size(uboParentType, j);
                auto offset = glsl.type_struct_member_offset(uboParentType, j);
                auto memberName = glsl.get_member_name(ssboType.parent_type, j);


                dumpMembers(glsl, fieldVectorDef, ssboType.member_types[j],
//                            "_" + std::to_string(resource.id) + "_" + memberName, offset, memberSize);
                            glsl.to_name(resource.id) + "_" + memberName, offset, memberSize);
            }
        }

        //Record data for images
        for (auto &resource : resources.sampled_images) {
            unsigned int set = 255;
            unsigned int binding = 255;
            if (glsl.has_decoration(resource.id, spv::DecorationDescriptorSet) &&
                glsl.has_decoration(resource.id, spv::DecorationBinding))
            {
                set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
                binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
            }

            metaInfo.imageBindings.push_back({set, binding, resource.name});
            if (set < 255) {
                metaInfo.imageBindingAmountsPerSet[set].start =
                    std::min<unsigned int>(metaInfo.imageBindingAmountsPerSet[set].start, binding);
                metaInfo.imageBindingAmountsPerSet[set].end =
                    std::max<unsigned int>(metaInfo.imageBindingAmountsPerSet[set].end, binding);
            }

//            std::cout << "set = " << set << std::endl;
//            std::cout << "binding = " << binding << std::endl;
//            std::cout << resource.name << std::endl;
//            std::cout << glsl.to_name(resource.base_type_id) << std::endl;
        }
        for (auto &data: metaInfo.imageBindingAmountsPerSet ) {
            if (data.start < 100) {
                data.length = data.end - data.start + 1;
            } else {
                data.start = 0;
            }
        }

    }

    //2.1 Create attribute enums
    for (auto it = attributesPerShaderName.begin(); it != attributesPerShaderName.end(); it++) {
        std::cout << "struct " << it->first << " {\n"
                                               "    enum class Attribute {" << std::endl;

        std::cout << "        ";
        auto array = it->second;
        std::sort(array.begin(), array.end(), [](const attributeDefine &a, const attributeDefine &b) -> bool {
            return a.location < b.location;
        });
        array.erase(std::unique(array.begin(), array.end(), [](attributeDefine &a, attributeDefine &b) -> bool {
            return a.name == b.name && a.location == b.location;
        }), array.end());
        for (auto &attributeInfo : array) {
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
        std::cout << "{ \"" << it->first << "\",\n" <<
            "  {" << std::endl;

        for (auto &attributeInfo : it->second) {
            std::cout << "    { \"" << attributeInfo.name << "\", " << attributeInfo.location << "}," << std::endl;
        }

        std::cout << "  }\n"<< "}," << std::endl;
    }
    std::cout << "};" << std::endl << std::endl;

    //Add shader meta
    std::cout << "const std::unordered_map<std::string, shaderMetaData> shaderMetaInfo = { \n";
    for (auto it = shaderMetaInfo.begin(); it != shaderMetaInfo.end(); it++) {
        std::cout << "{ \"" << it->first << "\", \n"<<
            "  {\n";

        //Dump stage
        std::cout << "    " << ShaderStageToStr(it->second.stage) << "," << std::endl;
        //Dump UBO Bindings per shader
        std::cout << "    {\n";
        for (auto subIt = it->second.m_uboBindings.begin(); subIt != it->second.m_uboBindings.end(); subIt++) {
            std::cout << "      {" << subIt->set << "," << subIt->binding << "," << subIt->size << "}," << std::endl;
        }
        std::cout << "    },\n";
        //UBO Bindings dump end

        std::cout << "    {\n";
        std::cout << "      {\n";
        for (auto &bindingAmount : it->second.uboBindingAmountsPerSet) {
            if (bindingAmount.length > 0) {
                std::cout << "        {" << bindingAmount.start << "," << bindingAmount.end << "," << bindingAmount.length << "},"
                          << std::endl;
            } else {
                std::cout << "        {0,0,0}," << std::endl;
            }
        }
        std::cout << "      }\n";
        std::cout << "    },\n";

        //Dump SSBO Bindings per shader
        std::cout << "    {\n";
        for (auto subIt = it->second.m_ssboBindingData.begin(); subIt != it->second.m_ssboBindingData.end(); subIt++) {
            std::cout << "      {" << subIt->set << "," << subIt->binding << "," << subIt->size << "}," << std::endl;
        }
        std::cout << "    },\n";
        //SSBO Bindings dump end

        std::cout << "    {\n";
        for (auto &binding : it->second.imageBindings) {
            std::cout << "      {" << binding.set << "," << binding.binding << ", \"" << binding.imageName << "\"}," << std::endl;
        }
        std::cout << "    },\n";

        std::cout << "    {\n";
        std::cout << "      {\n";
        for (auto &bindingAmount : it->second.imageBindingAmountsPerSet) {
            if (bindingAmount.length > 0) {
                std::cout << "        {" << bindingAmount.start << "," << bindingAmount.end << "," << bindingAmount.length << "},"
                          << std::endl;
            } else {
                std::cout << "        {0,0,0}," << std::endl;
            }
        }
        std::cout << "      }\n";
        std::cout << "    }\n";

        std::cout << "  }\n},\n";

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
