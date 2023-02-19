//
// Created by Deamon on 11.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IRENDERPARAMETERS_H
#define AWEBWOWVIEWERCPP_IRENDERPARAMETERS_H

#include <memory>
#include <functional>
#include "frame/FrameInputParams.h"

class IRenderFunction {
public:
    virtual ~IRenderFunction() = default;
};
typedef std::function<std::unique_ptr<IRenderFunction>()> SceneUpdateLambda;
typedef std::function<SceneUpdateLambda()> CullLambda;

template<typename PlanParams, typename FramePlan>
class IRendererParameters : public std::enable_shared_from_this<IRendererParameters<PlanParams, FramePlan>> {
public:
    virtual std::shared_ptr<FramePlan> processCulling(const std::shared_ptr<FrameInputParams<PlanParams>> &frameInputParams) = 0;
    virtual std::unique_ptr<IRenderFunction> update(const std::shared_ptr<FrameInputParams<PlanParams>> &frameInputParams, const std::shared_ptr<FramePlan> &framePlan) = 0;

    //This function is to be used to display data in UI
    virtual std::shared_ptr<FramePlan> getLastCreatedPlan() = 0;

    CullLambda createPlan(const std::shared_ptr<FrameInputParams<PlanParams>> &frameInputParams) {
        auto this_s = this->shared_from_this();

        return [frameInputParams, this_s]() -> SceneUpdateLambda {
            std::shared_ptr<FramePlan> framePlan = this_s->processCulling(frameInputParams);

            return [framePlan, frameInputParams, this_s]() -> std::unique_ptr<IRenderFunction> {
                return this_s->update(frameInputParams, framePlan);
            };
        };
    }
};

#endif //AWEBWOWVIEWERCPP_IRENDERPARAMETERS_H
