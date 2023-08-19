//
// Created by deamon on 18.10.19.
//

#include "spirv_glsl.hpp"
#include "webGLSLCompiler.h"
#include "dumpShaderFields.h"
#include "dumpGLSLShader.h"
#include <vector>
#include <utility>
#include <fstream>
#include <iostream>
#include <spirv.h>
#include <map>
#include <csignal>
#include <exception>

extern "C" void my_function_to_handle_aborts(int signal_number)
{
    /*Your code goes here. You can output debugging info.
      If you return from this function, and it was called
      because abort() was called, your program will exit or crash anyway
      (with a dialog box on Windows).
     */

    std::cout << "HELLO" << std::endl;
}


#ifdef _WIN32
#include <windows.h>
void beforeCrash() {
    std::cout << "HELLO" << std::endl;
    //__asm("int3");
}

static LONG WINAPI
windows_exception_handler(EXCEPTION_POINTERS
                          * ExceptionInfo)
{
    switch(ExceptionInfo->ExceptionRecord->ExceptionCode)
    {
        case EXCEPTION_ACCESS_VIOLATION:
            fputs("Error: EXCEPTION_ACCESS_VIOLATION\n", stderr);
            break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            fputs("Error: EXCEPTION_ARRAY_BOUNDS_EXCEEDED\n", stderr);
            break;
        case EXCEPTION_BREAKPOINT:
            fputs("Error: EXCEPTION_BREAKPOINT\n", stderr);
            break;
    }
    return 0;
}
#endif

int main(int argc, char **argv)
{
#ifdef _WIN32
    SetUnhandledExceptionFilter(windows_exception_handler);
    const bool SET_TERMINATE = std::set_terminate(beforeCrash);
//    const bool SET_TERMINATE_UNEXP = std::set_unexpected(beforeCrash);
#endif
    signal(SIGABRT, &my_function_to_handle_aborts);

    std::string mode(argv[1]);
    std::vector<std::string> filePaths;
    for (int i = 2; i < argc; i++) {
        filePaths.push_back(std::string(argv[i]));
    }


    if (mode == "-sf") {
        dumpShaderUniformOffsets(filePaths[0], decltype(filePaths)(filePaths.begin()+1, filePaths.end()));
    }  else if (mode == "-glsl100") {
        dumpGLSLText(filePaths, 100, false);
    } else if (mode == "-glsl120Es") {
        dumpGLSLText(filePaths, 120, true);
    } else if (mode == "-glsl120") {
        dumpGLSLText(filePaths, 120, false);
    } else if (mode == "-glsl330") {
        dumpGLSLText(filePaths, 330, false);
    } else if (mode == "-glslEs310") {
        dumpGLSLText(filePaths, 310, true);
    } else if (mode == "-glslEs300") {
        dumpGLSLText(filePaths, 300, true);
    }



    return 0;
}