//
// Created by deamon on 03.07.17.
//

#ifndef WEBWOWVIEWERCPP_WMOOBJECT_H
#define WEBWOWVIEWERCPP_WMOOBJECT_H

#include <string>
#include "m2WmoObject.h"

class WmoObject {
    WmoObject(){

    }
public:
    std::string getTextureName(int index);

    M2WmoObject& getDoodad(int index);
};


#endif //WEBWOWVIEWERCPP_WMOOBJECT_H
