//
// Created by Deamon on 3/18/2024.
//

#include "CommandPoolThread.h"

CommandPoolThread::CommandPoolThread(const HGDeviceVLK &device, const std::string &name) : m_device(device) {
    m_thread = std::thread([&, l_name= name] {
        setThreadName(l_name.c_str());

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.pNext = NULL;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        poolInfo.queueFamilyIndex = m_device->getQueueFamilyIndices().graphicsFamily.value();

        if (vkCreateCommandPool(m_device->getVkDevice(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics command pool!");
        }

        for (auto &cmdBuff : m_commandBuffers) {
            cmdBuff = std::make_shared<GCommandBuffer>(
                *m_device,
                VK_NULL_HANDLE,
                m_commandPool,
                false
            );
        }
        uint32_t lastFrame = m_device->getFrameNumber() + 10;
        while (!this->m_isTerminating) {
            auto input = poolInput.waitForNewInput();

            uint32_t currentFrame = input.currentProcessingFrame;
            if (lastFrame == currentFrame) {
                std::cerr << "Same pool was called twice in a frame" << std::endl;
                throw "Same pool was called twice in a frame";
            }

            lastFrame = currentFrame;

            if (input.task) {
                (*input.task)();
            }
        }
    });
}

CommandPoolThread::~CommandPoolThread() {
    m_isTerminating = true;

    poolInput.pushInput({});

    m_thread.join();
}

std::shared_future<std::shared_ptr<GCommandBuffer>>
CommandPoolThread::asyncRecordSecondaryBuffer(std::function<void(CmdBufRecorder &)> callback,
                                              const std::shared_ptr<GRenderPassVLK> &renderPass,
                                              const std::array<int32_t, 2> &areaOffset,
                                              const std::array<uint32_t, 2> &areaSize,
                                              uint32_t currentProcessingFrame
                                              ) {

    HCommandTask task = std::make_shared<CommandTask>([&,
                                                       l_renderPass = renderPass,
                                                       l_currentProcessingFrame = currentProcessingFrame,
                                                       l_callback = std::move(callback),
                                                       l_areaOffset = areaOffset,
                                                       l_areaSize = areaSize]() {
        auto commandBuffer = m_commandBuffers[l_currentProcessingFrame % (IDevice::MAX_FRAMES_IN_FLIGHT + 1)];

        auto cmdRecorder = commandBuffer->beginRecord(l_renderPass);
        cmdRecorder.setSecondaryCmdRenderArea(l_areaOffset, l_areaSize);

        l_callback(cmdRecorder);

        return commandBuffer;
    });

    CommandBuffRequest request;
    request.task = task;
    request.currentProcessingFrame = currentProcessingFrame;

    poolInput.pushInput(request);

    return task->get_future();
}
