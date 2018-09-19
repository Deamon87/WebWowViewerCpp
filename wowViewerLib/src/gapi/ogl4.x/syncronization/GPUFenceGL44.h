//
// Created by deamon on 13.09.18.
//

#ifndef AWEBWOWVIEWERCPP_GPUFENCEGL44_H
#define AWEBWOWVIEWERCPP_GPUFENCEGL44_H

#include "../../../engine/opengl/header.h"
#include "../../interface/syncronization/IGPUFence.h"

class GPUFenceGL44 : public IGPUFence {
public:
    void setGpuFence() override;;
    void wait() override;;
private:
    GLsync m_sync = 0;

};


#endif //AWEBWOWVIEWERCPP_GPUFENCEGL44_H
