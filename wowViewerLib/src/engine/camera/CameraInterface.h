//
// Created by deamon on 15.06.17.
//

#ifndef WOWVIEWERLIB_CAMERAINTERFACE_H
#define WOWVIEWERLIB_CAMERAINTERFACE_H
#include <mathfu/vector.h>
#include "../../include/controllable.h"

class ICamera : public IControllable {
public:
    virtual mathfu::vec3 getCameraPosition() = 0;
    virtual mathfu::vec3 getCameraLookAt() = 0;


};

#endif //WOWVIEWERLIB_CAMERAINTERFACE_H
