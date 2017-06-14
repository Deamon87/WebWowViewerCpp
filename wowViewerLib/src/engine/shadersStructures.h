#ifndef WOWMAPVIEWERREVIVED_SHADERSTRUCTURES_H
#define WOWMAPVIEWERREVIVED_SHADERSTRUCTURES_H
#include <string>
#include <vector>
#include <unordered_map>

struct shaderItem {
    constexpr shaderItem(char *variableName, const int number): variableName(variableName), number(number) {

    }
    char *variableName;
    int number;
} ;

struct shaderDefinition{
    constexpr shaderDefinition(const char* const shaderString,
                               const int attributesNum,
                               const shaderItem *attributes/*,
                               const int uniformsNum,
                               const shaderItem *uniforms*/)
            :shaderString(shaderString), attributesNum(attributesNum), attributes(attributes)/*, uniformsNum(uniformsNum), uniforms(uniforms)*/
    {

    }
    const char* const shaderString;
    const int attributesNum;
    const shaderItem *attributes;
//    const int uniformsNum;
//    const shaderItem *uniforms;
};

struct innerType {
    char* shaderName;
    shaderDefinition shaderDefinition1;
};

#endif //WOWMAPVIEWERREVIVED_SHADERSTRUCTURES_H
