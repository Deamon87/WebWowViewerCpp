//
// Created by Deamon on 3/18/2024.
//

#ifndef AWEBWOWVIEWERCPP_COMMANDPOOLTHREAD_H
#define AWEBWOWVIEWERCPP_COMMANDPOOLTHREAD_H


#include "../../IDeviceVulkan.h"
#include "../../../../renderer/frame/prodConsumerChain/ProdConsumerIOConnector.h"
#include "../CommandBuffer.h"
#include <thread>
#include <future>

class CommandPoolThread {
public:
    CommandPoolThread(const HGDeviceVLK &device, const std::string &name);
    ~CommandPoolThread();

    std::shared_future<std::shared_ptr<GCommandBuffer>> asyncRecordSecondaryBuffer(
        std::function<void(CmdBufRecorder &recorder)> callback,
        const std::shared_ptr<GRenderPassVLK> &renderPass,
        const std::array<int32_t, 2> &areaOffset,
        const std::array<uint32_t, 2> &areaSize,
        uint32_t currentProcessingFrame
    );
private:
    typedef std::packaged_task<
        std::shared_ptr<GCommandBuffer>()
    > CommandTask;
    typedef std::shared_ptr<CommandTask> HCommandTask;

    struct CommandBuffRequest {
        HCommandTask task;
        uint32_t currentProcessingFrame;
    };

    std::thread m_thread;
    HGDeviceVLK m_device;

    VkCommandPool m_commandPool;

    bool m_isTerminating = false;
    ProdConsumerIOConnector<CommandBuffRequest> poolInput = {m_isTerminating};

    std::array<std::shared_ptr<GCommandBuffer>, (IDevice::MAX_FRAMES_IN_FLIGHT + 1)> m_commandBuffers;
};


#endif //AWEBWOWVIEWERCPP_COMMANDPOOLTHREAD_H
