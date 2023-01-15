//
// Created by Deamon on 11.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IRENDERPARAMETERS_H
#define AWEBWOWVIEWERCPP_IRENDERPARAMETERS_H

#include "frame/FrameInputParams.h"

template<typename PlanParams, typename FramePlan>
class IRendererParameters {
    virtual void putIntoQueue(std::shared_ptr<FrameInputParams<PlanParams>> &frameInputParams) = 0;
    virtual std::shared_ptr<FramePlan> getLastPlan() = 0;
};

#endif //AWEBWOWVIEWERCPP_IRENDERPARAMETERS_H
