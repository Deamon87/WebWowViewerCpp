//
// Created by Deamon on 11.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IRENDERPARAMETERS_H
#define AWEBWOWVIEWERCPP_IRENDERPARAMETERS_H

#include "frame/FrameInputParams.h"

template<typename FramePlan, typename AdditionalParam>
class IRendererParameters {
    virtual void putIntoQueue(std::shared_ptr<FrameInputParams<FramePlan, AdditionalParam>> &frameInputParams) = 0;

};

#endif //AWEBWOWVIEWERCPP_IRENDERPARAMETERS_H
