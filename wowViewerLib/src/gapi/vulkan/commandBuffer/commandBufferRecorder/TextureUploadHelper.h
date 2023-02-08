//
// Created by Deamon on 06.02.23.
//

#ifndef AWEBWOWVIEWERCPP_TEXTUREUPLOADHELPER_H
#define AWEBWOWVIEWERCPP_TEXTUREUPLOADHELPER_H

#include <vector>
#include "../../textures/GTextureVLK.h"

void textureUploadStrategy(std::vector<GTextureVLK> &textures, CmdBufRecorder &renderCmdBufRecorder, CmdBufRecorder &uploadCmdBufRecorder);


#endif //AWEBWOWVIEWERCPP_TEXTUREUPLOADHELPER_H
