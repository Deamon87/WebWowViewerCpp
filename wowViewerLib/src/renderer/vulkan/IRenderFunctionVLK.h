//
// Created by Deamon on 11.02.23.
//

#ifndef AWEBWOWVIEWERCPP_IRENDERFUNCTIONVLK_H
#define AWEBWOWVIEWERCPP_IRENDERFUNCTIONVLK_H

#include "../IRenderParameters.h"
#include "../../gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder.h"

class IRenderFunctionVLK : public IRenderFunction {
public:
    ~IRenderFunctionVLK() override = default;
public:
    virtual void executeUpload(IDevice &device, CmdBufRecorder &uploadCmd) = 0;
    virtual void executeRender(IDevice &device, CmdBufRecorder &frameBufCmd, CmdBufRecorder &swapChainCmd) = 0;
};

template<typename REND, typename UP >
class TemplateIRenderFunctionVLK : public IRenderFunctionVLK {
public:
    TemplateIRenderFunctionVLK(UP a, REND b) : m_render(std::move(b)), m_upload(std::move(a)) {

    };
    void executeUpload(IDevice &device, CmdBufRecorder &uploadCmd) override {
        //Hack through currently processing frame
        device.setCurrentProcessingFrameNumber(getProcessingFrame());
        m_upload(uploadCmd);
    };
    void executeRender(IDevice &device, CmdBufRecorder &frameBufCmd, CmdBufRecorder &swapChainCmd) override {
        //Hack through currently processing frame
        device.setCurrentProcessingFrameNumber(getProcessingFrame());
        m_render(frameBufCmd, swapChainCmd);
    };
private:
    REND m_render;
    UP m_upload;
};

template<typename REND, typename UP>
std::unique_ptr<IRenderFunctionVLK> createRenderFuncVLK(UP a, REND b) {
    return std::make_unique<TemplateIRenderFunctionVLK<REND, UP>>(std::move(a), std::move(b));
}


#endif //AWEBWOWVIEWERCPP_IRENDERFUNCTIONVLK_H
