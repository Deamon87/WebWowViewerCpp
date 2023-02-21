//
// Created by Deamon on 06.02.23.
//

#ifndef AWEBWOWVIEWERCPP_TEXTUREUPLOADHELPER_H
#define AWEBWOWVIEWERCPP_TEXTUREUPLOADHELPER_H

#include <vector>
#include "../../textures/GTextureVLK.h"
#include "CommandBufferRecorder.h"

void textureUploadStrategy(const std::vector<std::weak_ptr<GTextureVLK>> &textures,
                           CmdBufRecorder &renderCmdBufRecorder,
                           CmdBufRecorder &uploadCmdBufRecorder);


#endif //AWEBWOWVIEWERCPP_TEXTUREUPLOADHELPER_H
