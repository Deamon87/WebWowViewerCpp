//
// Created by Deamon on 12.12.22.
//

#include "FrontendUIRenderForwardVLK.h"
#include "../../../../../wowViewerLib/src/gapi/UniformBufferStructures.h"

FrontendUIRenderForwardVLK::FrontendUIRenderForwardVLK(HGDeviceVLK hDevice) : m_device(hDevice) {
}

void FrontendUIRenderForwardVLK::putIntoQueue(std::shared_ptr<FrontendUIInputParams> &frameInputParams) {
    std::lock_guard<std::mutex> guard(m_inputParamsMtx);

    m_inputParams.push(frameInputParams);
}

void FrontendUIRenderForwardVLK::update(VkCommandBuffer updateBuffer, VkCommandBuffer swapChainDraw) {

}

void FrontendUIRenderForwardVLK::createBuffers() {
    vboBuffer = m_device->createIndexBuffer(1024*1024);
    iboBuffer = m_device->createVertexBuffer(1024*1024);
    uboBuffer = m_device->createUniformBuffer(sizeof(ImgUI::modelWideBlockVS)*IDevice::MAX_FRAMES_IN_FLIGHT);
}

HGVertexBuffer FrontendUIRenderForwardVLK::createVertexBuffer(int sizeInBytes) {
    return vboBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer FrontendUIRenderForwardVLK::createIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}

HGMesh FrontendUIRenderForwardVLK::createUIMesh(gMeshTemplate &meshTemplate, IBufferChunk<ImgUI::modelWideBlockVS>) {
    //TODO:
    return nullptr;
}
