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

    virtual void execute(CmdBufRecorder &uploadCmd, CmdBufRecorder &frameBufCmd, CmdBufRecorder &swapChainCmd) = 0;
};

template<typename T>
class TemplateIRenderFunctionVLK : public IRenderFunctionVLK {
public:
    TemplateIRenderFunctionVLK(T a) : m_a(std::move(a)){

    };
    void execute(CmdBufRecorder &uploadCmd, CmdBufRecorder &frameBufCmd, CmdBufRecorder &swapChainCmd) override {
        m_a(uploadCmd, frameBufCmd, swapChainCmd);
    };
private:
    T m_a;
};

template<typename T>
std::unique_ptr<IRenderFunctionVLK> createRenderFuncVLK(T a) {
    return std::make_unique<TemplateIRenderFunctionVLK<T>>(std::move(a));
}


#endif //AWEBWOWVIEWERCPP_IRENDERFUNCTIONVLK_H
