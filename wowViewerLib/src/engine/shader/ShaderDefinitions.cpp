//
// Created by Deamon on 6/26/2018.
//
#include <vector>
#include "ShaderDefinitions.h"

#ifdef __ANDROID_API__
#include "../androidLogSupport.h"
#endif

std::string loadShader(std::string shaderName) {
#ifdef __ANDROID_API__
    //TODO: pass this stuff here
    AAssetManager *mgr = g_assetMgr;
    if (g_assetMgr == nullptr) {
        std::cout << "g_assetMgr == nullptr";
    }
    std::string filename = "glsl/" + shaderName + ".glsl";

    std::cout << "AAssetManager_open" << std::endl;
    AAsset* asset = AAssetManager_open(mgr, filename.c_str(), AASSET_MODE_STREAMING);
    std::cout << "AAssetManager_opened" << std::endl;
    if (asset == nullptr) {
        std::cout << "asset == nullptr" << std::endl;
    }

    char buf[BUFSIZ];
    std::vector<char> outBuf;

    std::cout << "buffers created" << std::endl;
    int nb_read = 0;
    while ((nb_read = AAsset_read(asset, buf, BUFSIZ)) > 0) {
        std::cout << "file read" << std::endl;
        for (int i = 0; i < nb_read; i++) {
            outBuf.push_back(buf[i]);
        }
    }
        //std::copy(&buf[0], &buf[nb_read-1]+1, outPutIterator);
    std::cout << "file fully read" << std::endl;
    AAsset_close(asset);
    std::cout << "asset closed" << std::endl;

    return std::string(outBuf.begin(), outBuf.end());

#else
    std::ifstream t("./glsl/" + shaderName + ".glsl");
    return std::string((std::istreambuf_iterator<char>(t)),
                       std::istreambuf_iterator<char>());
#endif

}