//
// Created by Deamon on 12.12.22.
//

#include "FrontendUIRenderForwardVLK.h"
#include "../../../../../wowViewerLib/src/gapi/UniformBufferStructures.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/materials/ISimpleMaterialVLK.h"

FrontendUIRenderForwardVLK::FrontendUIRenderForwardVLK(HGDeviceVLK hDevice) : m_device(hDevice) {
}

void FrontendUIRenderForwardVLK::putIntoQueue(std::shared_ptr<FrontendUIInputParams> &frameInputParams) {
    std::lock_guard<std::mutex> guard(m_inputParamsMtx);

    m_inputParams.push(frameInputParams);
}

void FrontendUIRenderForwardVLK::update(VkCommandBuffer transferQueueCMD, VkCommandBuffer renderQueueCMD) {

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

HMaterial FrontendUIRenderForwardVLK::createUIMaterial(const UIMaterialTemplate &materialTemplate) {
    std::vector<std::shared_ptr<IBufferVLK>> ubos = {std::dynamic_pointer_cast<IBufferVLK>(materialTemplate.uiUBO)};
    std::vector<HGTextureVLK> texturesVLK = {std::dynamic_pointer_cast<GTextureVLK>(materialTemplate.texture)};
    return std::make_shared<ISimpleMaterialVLK>(m_device,
                                  "imguiShader", "imguiShader",
                                  ubos,
                                  texturesVLK);
}

HGMesh FrontendUIRenderForwardVLK::createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) {
    //TODO:
    return nullptr;
}
