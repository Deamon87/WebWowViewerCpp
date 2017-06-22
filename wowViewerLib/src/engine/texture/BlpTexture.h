//
// Created by deamon on 19.06.17.
//

#ifndef WOWVIEWERLIB_BLPTEXTURE_H
#define WOWVIEWERLIB_BLPTEXTURE_H


#include "../persistance/blpFile.h"

class BlpTexture {
public:
    void process(std::vector<unsigned char> &blpFile);
};


#endif //WOWVIEWERLIB_BLPTEXTURE_H
