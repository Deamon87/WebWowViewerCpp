//
// Created by deamon on 22.06.17.
//

#ifndef WOWVIEWERLIB_MATHHELPER_H
#define WOWVIEWERLIB_MATHHELPER_H


#include "../persistance/header/commonFileStructs.h"

class MathHelper {
public:
    static CAaBox transformAABBWithMat4(mathfu::mat4 matrix, mathfu::vec4 min, mathfu::vec4 max);
};


#endif //WOWVIEWERLIB_MATHHELPER_H
