//
// Created by deamon on 18.10.19.
//

#include "spirv_glsl.hpp"
#include "webGLSLCompiler.h"
#include "dumpShaderFields.h"
#include "dumpShaderMetaData.h"
#include "dumpGLSLShader.h"
#include <vector>
#include <utility>
#include <fstream>
#include <iostream>
#include <spirv.h>
#include <map>

int main(int argc, char **argv)
{
    std::string mode(argv[1]);
    std::vector<std::string> filePaths;
    for (int i = 2; i < argc; i++) {
        filePaths.push_back(std::string(argv[i]));
    }


    if (mode == "-sf") {
        dumpShaderUniformOffsets(filePaths);
    } else if (mode == "-smd") {
        dumpMetaData(filePaths);
    } else if (mode == "-glsl100") {
        dumpGLSLText(filePaths, 100);
    } else if (mode == "-glsl330") {
        dumpGLSLText(filePaths, 330);
    }
}