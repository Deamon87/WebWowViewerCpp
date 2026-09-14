//
// Created by Deamon on 11.02.23.
//

#ifndef AWEBWOWVIEWERCPP_IRENDERFUNCTIONVLK_H
#define AWEBWOWVIEWERCPP_IRENDERFUNCTIONVLK_H

#include <functional>
#include "../IRenderParameters.h"
#include "../../gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder.h"
#include "../../gapi/interface/FrameContext.h"

class IRenderFunctionVLK : public IRenderFunction {
public:
    ~IRenderFunctionVLK() override = default;
public:
    virtual void executeUpload(IDevice &device, CmdBufRecorder &uploadCmd) = 0;
    virtual void executeRender(IDevice &device, CmdBufRecorder &frameBufCmd, CmdBufRecorder &swapChainCmd) = 0;
    virtual void executePostUpdate(IDevice &device) = 0;
    virtual void executeWithinContext(std::function<void()> callback) = 0;
};

//REND = rendering, UP = upload, PU = post update
template<typename REND, typename UP, typename PU>
class TemplateIRenderFunctionVLK : public IRenderFunctionVLK {
public:
    TemplateIRenderFunctionVLK(PU a, UP b, REND c) : m_render(std::move(c)), m_upload(std::move(b)), m_postUpdate(std::move(a)) {

    };
    void executeUpload(IDevice &device, CmdBufRecorder &uploadCmd) override {
        //Hack through currently processing frame
        FrameContext::setCurrentProcessingFrameNumber(getProcessingFrame());
        m_upload(uploadCmd);
    };
    void executeRender(IDevice &device, CmdBufRecorder &frameBufCmd, CmdBufRecorder &swapChainCmd) override {
        //Hack through currently processing frame
        FrameContext::setCurrentProcessingFrameNumber(getProcessingFrame());
        m_render(frameBufCmd, swapChainCmd);
    };
    void executePostUpdate(IDevice &device) override {
        //Hack through currently processing frame
        FrameContext::setCurrentProcessingFrameNumber(getProcessingFrame());
        m_postUpdate();
    };
    void executeWithinContext(std::function<void()> callback) override {
        FrameContext::setCurrentProcessingFrameNumber(getProcessingFrame());
        callback();
    }
private:
    REND m_render;
    UP m_upload;
    PU m_postUpdate;
};

template<typename REND, typename UP, typename PU>
std::unique_ptr<IRenderFunctionVLK> createRenderFuncVLK(PU a, UP b, REND c) {
    return std::make_unique<TemplateIRenderFunctionVLK<REND, UP, PU>>(std::move(a), std::move(b), std::move(c));
}


#endif //AWEBWOWVIEWERCPP_IRENDERFUNCTIONVLK_H
