//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GTEXTURE_H
#define WEBWOWVIEWERCPP_GTEXTURE_H


#include "../engine/wowCommonClasses.h"

class GTexture {
    GTexture(HBlpTexture &texture) : m_texture(texture) {

    }
private:
    HBlpTexture m_texture;
};


#endif //WEBWOWVIEWERCPP_GTEXTURE_H
