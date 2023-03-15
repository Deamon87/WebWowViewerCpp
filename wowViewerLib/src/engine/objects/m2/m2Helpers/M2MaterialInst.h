//
// Created by deamon on 23.06.17.
//

#ifndef WOWVIEWERLIB_M2MATERIALINST_H
#define WOWVIEWERLIB_M2MATERIALINST_H

class M2MaterialInst;
#include <string>
#include "../../../wowCommonClasses.h"
#include "../../../persistance/header/blpFileHeader.h"
#include "../../../texture/BlpTexture.h"
#include "../../../../gapi/interface/IDevice.h"
//#include "../../../../gapi/GTexture.h"

class M2MaterialInst {
public:
    int layer = 0;
    int renderFlagIndex = -1;
    int batchIndex = -1;
    int flags = 0;
    int priorityPlane = 0;
    int vertexShader;
    int pixelShader;
};


#endif //WOWVIEWERLIB_M2MATERIALINST_H
