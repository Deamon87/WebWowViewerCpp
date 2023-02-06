//
// Created by Deamon on 06.02.23.
//

#ifndef AWEBWOWVIEWERCPP_TEXTUREUPLOADHELPER_H
#define AWEBWOWVIEWERCPP_TEXTUREUPLOADHELPER_H

#include <vector>
#include "../../textures/GTextureVLK.h"

class TextureUploadHelper {
    TextureUploadHelper(CmdBufRecorder &renderCmdBufRecorder, CmdBufRecorder &uploadCmdBufRecorder);
    ~TextureUploadHelper();

private:
    CmdBufRecorder &m_renderCmdBufRecorder;
    CmdBufRecorder &m_uploadCmdBufRecorder;
    std::vector<GTextureVLK> m_texturesToUpload;
};


#endif //AWEBWOWVIEWERCPP_TEXTUREUPLOADHELPER_H
