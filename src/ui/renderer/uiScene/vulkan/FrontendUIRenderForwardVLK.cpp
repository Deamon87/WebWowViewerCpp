//
// Created by Deamon on 12.12.22.
//

#include "FrontendUIRenderForwardVLK.h"
#include "../../../../../wowViewerLib/src/gapi/UniformBufferStructures.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/materials/ISimpleMaterialVLK.h"

FrontendUIRenderForwardVLK::FrontendUIRenderForwardVLK(HGDeviceVLK hDevice) : FrontendUIRenderer(
    hDevice), m_device(hDevice) {
}

void FrontendUIRenderForwardVLK::update(VkCommandBuffer transferQueueCMD, VkCommandBuffer renderQueueCMD) {

}

void FrontendUIRenderForwardVLK::createBuffers() {
    vboBuffer = m_device->createIndexBuffer(1024*1024);
    iboBuffer = m_device->createVertexBuffer(1024*1024);
    uboBuffer = m_device->createUniformBuffer(sizeof(ImgUI::modelWideBlockVS)*IDevice::MAX_FRAMES_IN_FLIGHT);

    m_imguiUbo = uboBuffer->getSubBuffer()
}

HGVertexBuffer FrontendUIRenderForwardVLK::createVertexBuffer(int sizeInBytes) {
    return vboBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer FrontendUIRenderForwardVLK::createIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}

HMaterial FrontendUIRenderForwardVLK::createUIMaterial(const UIMaterialTemplate &materialTemplate) {
    auto i = m_materialCache.find(materialTemplate);
    if (i != m_materialCache.end()) {
        if (!i->second.expired()) {
            return i->second.lock();
        } else {
            m_materialCache.erase(i);
        }
    }

    std::vector<std::shared_ptr<IBufferVLK>> ubos = {};
    std::vector<HGTextureVLK> texturesVLK = {std::dynamic_pointer_cast<GTextureVLK>(materialTemplate.texture)};
    auto material = std::make_shared<ISimpleMaterialVLK>(m_device,
                                  "imguiShader", "imguiShader",
                                  ubos,
                                  texturesVLK);

    std::weak_ptr<ISimpleMaterialVLK> weakPtr(material);
    m_materialCache[materialTemplate] = weakPtr;

    return material;
}

HGMesh FrontendUIRenderForwardVLK::createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) {
    //TODO:
    return nullptr;
}

void FrontendUIRenderForwardVLK::updateAndDraw(
    const std::shared_ptr<FrameInputParams<ImGuiFramePlan::ImGUIParam>> &frameInputParams,
    const std::shared_ptr<ImGuiFramePlan::EmptyPlan> &framePlan) {

    this->consumeFrameInput(frameInputParams);

    //Record commands to update buffer and draw
}
