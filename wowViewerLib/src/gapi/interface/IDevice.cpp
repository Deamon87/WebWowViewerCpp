//
// Created by Deamon on 8/25/2018.
//

#include <iostream>
#include <algorithm>
#include "IDevice.h"


//inline bool IDevice::sortMeshes(const HGMesh &a, const HGMesh &b) {
//
//}

int compressedTexturesSupported = -1;
int anisFiltrationSupported = -1;

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

/*
bool IDevice::getIsDTXCompressedTexturesSupported() {
#ifdef __EMSCRIPTEN__
    if (compressedTexturesSupported == -1){
        auto res = emscripten_webgl_enable_extension(emscripten_webgl_get_current_context(), "WEBGL_compressed_texture_s3tc");
        std::cout << "texture_s3tc returned " << res << std::endl;
        if (res == EM_TRUE) {
            compressedTexturesSupported = 1;
        } else {
            compressedTexturesSupported = 0;
        }
    }
    return (compressedTexturesSupported == 1);

#elif (WITH_GLESv2)
    return false;
#else
    return true;
#endif
}
*/

bool IDevice::getIsAnisFiltrationSupported() {
#ifdef __EMSCRIPTEN__
    if (anisFiltrationSupported == -1){
        if (emscripten_webgl_enable_extension(emscripten_webgl_get_current_context(), "EXT_texture_filter_anisotropic ") == EM_TRUE) {
            anisFiltrationSupported = 1;
        } else {
            anisFiltrationSupported = 0;
        }
    }
      return anisFiltrationSupported == 1;
#else
    return true;
#endif
}

std::string IDevice::insertAfterVersion(std::string &glslShaderString, std::string stringToPaste) {
    auto start = glslShaderString.find("#version");
    if (start != std::string::npos) {
        auto end = glslShaderString.find("\n", start+1);
        return glslShaderString.insert(end+1, stringToPaste);
    } else {
        return glslShaderString.insert(0, stringToPaste);
    }
}


