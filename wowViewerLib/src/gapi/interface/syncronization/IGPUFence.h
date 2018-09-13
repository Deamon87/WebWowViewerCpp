//
// Created by deamon on 13.09.18.
//

#ifndef AWEBWOWVIEWERCPP_IGPUFENCE_H
#define AWEBWOWVIEWERCPP_IGPUFENCE_H

class IGPUFence {
public:
    virtual void wait() = 0;
    virtual void setGpuFence() = 0;
};

#endif //AWEBWOWVIEWERCPP_IGPUFENCE_H
