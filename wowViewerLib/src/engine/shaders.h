
#ifndef WOWMAPVIEWERREVIVED_SHADERS_H
#define WOWMAPVIEWERREVIVED_SHADERS_H

#include "shadersStructures.h"
#include "shaderDefinitions.h"
#include <stdexcept>

constexpr bool strings_equal(char const * a, char const * b) {
    return *a == *b && (*a == '\0' || strings_equal(a + 1, b + 1));
}
constexpr const shaderDefinition *getShaderDef(const char *shaderName) {
    int arraySize = 12;
    const shaderDefinition *result = nullptr;
    for (int i = 0; i < arraySize; i++) {
        if (strings_equal(map[i].shaderName,shaderName)) {
            result = &map[i].shaderDefinition1;
            break;
        }
    }
    return (result != nullptr) ? result : throw std::invalid_argument(std::string("Cannot Find Shader"));
}

constexpr const int getShaderAttribute(const char *shaderName, const char *attributeName) {
    const shaderDefinition * shaderDef = getShaderDef(shaderName);
    int result = -1;
    bool found = false;
    for (int i = 0; i < shaderDef->attributesNum; i++) {
        if (strings_equal(shaderDef->attributes[i].variableName,attributeName)) {
            result = shaderDef->attributes[i].number;
            found = true;
            break;
        }
    }
    return (found) ? result : throw std::invalid_argument(std::string("Cannot Find Attribute"));
}
//constexpr const int getShaderUniform(const char *shaderName, const char *uniformName) {
//    const shaderDefinition * shaderDef = getShaderDef(shaderName);
//    int result = -1;
//    bool found = false;
//    for (int i = 0; i < shaderDef->uniformsNum; i++) {
//        if (strings_equal(shaderDef->uniforms[i].variableName,uniformName)) {
//            result = shaderDef->uniforms[i].number;
//            found = true;
//            break;
//        }
//    }
//    return (found) ? result : throw std::invalid_argument(std::string("Cannot Find Uniform"));
//}

#endif //WOWMAPVIEWERREVIVED_SHADERS_H
