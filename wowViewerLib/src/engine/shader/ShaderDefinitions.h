#ifndef WOWMAPVIEWERREVIVED_SHADERDEFINITIONS_H
#define WOWMAPVIEWERREVIVED_SHADERDEFINITIONS_H

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

template <typename T>
inline constexpr const uint32_t operator+ (T const val) { return static_cast<const uint32_t>(val); };
struct fieldDefine {
    std::string name;
    bool isFloat ;
    int offset;
    int columns;
    int vecSize;
    int arraySize;
};
struct attributeDefine {
    std::string name;
    int location;
};
struct uboBindingData {
    int set;
    int binding;
    int size;
};

struct shaderMetaData {
    std::vector<uboBindingData> uboBindings;
};

//Per file
extern const std::unordered_map<std::string, shaderMetaData> shaderMetaInfo;
extern const std::unordered_map<std::string, std::vector<attributeDefine>> attributesPerShaderName;
extern const std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderName;
