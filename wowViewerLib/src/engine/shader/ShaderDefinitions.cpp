//
// Created by Deamon on 6/26/2018.
//
#include "ShaderDefinitions.h"

std::string loadShader(std::string shaderName) {
    std::ifstream t("./glsl/" + shaderName + ".glsl");
    return std::string((std::istreambuf_iterator<char>(t)),
                       std::istreambuf_iterator<char>());
}