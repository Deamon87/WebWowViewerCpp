//
// Created by deamon on 23.06.17.
//

#ifndef WOWVIEWERLIB_M2MATERIALINST_H
#define WOWVIEWERLIB_M2MATERIALINST_H

#include <string>
#include "../../../wowCommonClasses.h"
#include "../../../persistance/header/blpFileHeader.h"
#include "../../../texture/BlpTexture.h"

class M2MaterialInst {
public:
    bool isRendered= false;
    bool isTransparent= false;
    int meshIndex= -1;

    int textureCount;

    int texUnitTexIndex = -1;
    struct textureParams {
        int m2TextureIndex = -1;
        bool xWrapTex = false;
        bool yWrapTex = false;
        HBlpTexture texUnitTexture = nullptr;
    } textures[4];

    int layer = 0;
    int renderFlagIndex = -1;
    int flags = 0;
    int priorityPlane = 0;
    int vertexShader;
    int pixelShader;
};


#endif //WOWVIEWERLIB_M2MATERIALINST_H
