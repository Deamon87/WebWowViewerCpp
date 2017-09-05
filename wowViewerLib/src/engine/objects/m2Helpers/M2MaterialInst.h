//
// Created by deamon on 23.06.17.
//

#ifndef WOWVIEWERLIB_M2MATERIALINST_H
#define WOWVIEWERLIB_M2MATERIALINST_H

#include <string>
#include "../../persistance/header/blpFileHeader.h"
#include "../../texture/BlpTexture.h"

class M2MaterialInst {
public:
    bool isRendered= false;
    bool isTransparent= false;
    bool isEnviromentMapping= false;
    int meshIndex= -1;


    int texUnit1TexIndex = 01;
    int mdxTextureIndex1 = 0;
    bool xWrapTex1 = false;
    bool yWrapTex1 = false;
    std::string textureUnit1TexName = "";
    BlpTexture *texUnit1Texture = nullptr;

    int texUnit2TexIndex = -1;
    int mdxTextureIndex2 = 0;
    bool xWrapTex2 = false;
    bool yWrapTex2 = false;
    std::string textureUnit2TexName = "";
    BlpTexture *texUnit2Texture = nullptr;

    int texUnit3TexIndex = -1;
    int mdxTextureIndex3 = 0;
    bool xWrapTex3 = false;
    bool yWrapTex3 = false;
    std::string textureUnit3TexName = "";
    BlpTexture *texUnit3Texture = nullptr;

    int layer = 0;
    int renderFlagIndex = -1;
    int flags = 0;
    int pixelShader;
};


#endif //WOWVIEWERLIB_M2MATERIALINST_H
