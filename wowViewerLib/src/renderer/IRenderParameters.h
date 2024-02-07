//
// Created by Deamon on 11.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IRENDERPARAMETERS_H
#define AWEBWOWVIEWERCPP_IRENDERPARAMETERS_H

#include <memory>
#include <functional>
#include <utility>
#include "frame/FrameInputParams.h"
#include "frame/FrameProfile.h"

class IRenderFunction {
public:
    virtual ~IRenderFunction() = default;

    void setProcessingFrame(unsigned int frame) { currentProcessingFrame = frame; };
    unsigned int getProcessingFrame() const { return currentProcessingFrame; }
private:
    unsigned int currentProcessingFrame = 0;
};
typedef std::function<std::unique_ptr<IRenderFunction>()> SceneUpdateLambda;
typedef std::function<SceneUpdateLambda()> CullLambda;

template<typename PlanParams, typename FramePlan>
class IRendererParameters : public std::enable_shared_from_this<IRendererParameters<PlanParams, FramePlan>> {
public:
    virtual ~IRendererParameters() = default;

    virtual std::shared_ptr<FramePlan> processCulling(const std::shared_ptr<FrameInputParams<PlanParams>> &frameInputParams) = 0;
    virtual std::unique_ptr<IRenderFunction> update(const std::shared_ptr<FrameInputParams<PlanParams>> &frameInputParams, const std::shared_ptr<FramePlan> &framePlan) = 0;

    //This function is to be used to display data in UI
    virtual std::shared_ptr<FramePlan> getLastCreatedPlan() = 0;

    CullLambda createCullUpdateRenderChain(const std::shared_ptr<FrameInputParams<PlanParams>> &frameInputParams,
                                           std::function<uint32_t()> updateProcessingFrame) {
        auto this_s = this->shared_from_this();
        auto l_updateProcessingFrame = std::move(updateProcessingFrame);

        return [frameInputParams, this_s, l_updateProcessingFrame]() -> SceneUpdateLambda {
            auto l_currentFrame = l_updateProcessingFrame();
            TracyMessageStr(("Culling stage frame = " + std::to_string(l_currentFrame)));

            std::shared_ptr<FramePlan> framePlan = this_s->processCulling(frameInputParams);

            return [framePlan, frameInputParams, this_s, l_updateProcessingFrame]() -> std::unique_ptr<IRenderFunction> {
                auto l_currentFrame = l_updateProcessingFrame();

                auto renderFunc = this_s->update(frameInputParams, framePlan);
                renderFunc->setProcessingFrame(l_currentFrame);
                return renderFunc;
            };
        };
    }
};

#endif //AWEBWOWVIEWERCPP_IRENDERPARAMETERS_H
