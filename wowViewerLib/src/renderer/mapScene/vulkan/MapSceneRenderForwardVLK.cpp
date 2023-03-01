//
// Created by Deamon on 12/1/2022.
//

#include "MapSceneRenderForwardVLK.h"
#include "../../vulkan/IRenderFunctionVLK.h"
#include "../../../engine/objects/scenes/map.h"

MapSceneRenderForwardVLK::MapSceneRenderForwardVLK(HGDeviceVLK hDevice) : m_device(hDevice) {

}
// ------------------
// Buffer creation
// ------------------

HGVertexBufferBindings MapSceneRenderForwardVLK::createWmoVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto wmoVAO = m_device->createVertexBufferBindings();
    wmoVAO->addVertexBufferBinding(vertexBuffer, std::vector(staticWMOBindings.begin(), staticWMOBindings.end()));
    wmoVAO->setIndexBuffer(indexBuffer);

    return wmoVAO;
}
HGVertexBufferBindings MapSceneRenderForwardVLK::createM2VAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto m2VAO = m_device->createVertexBufferBindings();
    m2VAO->addVertexBufferBinding(vertexBuffer, std::vector(staticM2Bindings.begin(), staticM2Bindings.end()));
    m2VAO->setIndexBuffer(indexBuffer);

    return m2VAO;
}
HGVertexBufferBindings MapSceneRenderForwardVLK::createWaterVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto waterVAO = m_device->createVertexBufferBindings();
    waterVAO->addVertexBufferBinding(vertexBuffer, std::vector(staticWaterBindings.begin(), staticWaterBindings.end()));
    waterVAO->setIndexBuffer(indexBuffer);

    return waterVAO;
};

HGVertexBuffer MapSceneRenderForwardVLK::createM2VertexBuffer(int sizeInBytes) {
    return HGVertexBuffer();
}

HGIndexBuffer MapSceneRenderForwardVLK::createM2IndexBuffer(int sizeInBytes) {
    return HGIndexBuffer();
}

HGVertexBuffer MapSceneRenderForwardVLK::createADTVertexBuffer(int sizeInBytes) {
    return HGVertexBuffer();
}

HGIndexBuffer MapSceneRenderForwardVLK::createADTIndexBuffer(int sizeInBytes) {
    return HGIndexBuffer();
}

HGVertexBuffer MapSceneRenderForwardVLK::createWMOVertexBuffer(int sizeInBytes) {
    return HGVertexBuffer();
}

HGIndexBuffer MapSceneRenderForwardVLK::createWMOIndexBuffer(int sizeInBytes) {
    return HGIndexBuffer();
}

HGVertexBuffer MapSceneRenderForwardVLK::createWaterVertexBuffer(int sizeInBytes) {
    return HGVertexBuffer();
}

HGIndexBuffer MapSceneRenderForwardVLK::createWaterIndexBuffer(int sizeInBytes) {
    return HGIndexBuffer();
}

std::unique_ptr<IRenderFunction> MapSceneRenderForwardVLK::update(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams,
                                             const std::shared_ptr<MapRenderPlan> &framePlan) {

    auto hthis = std::dynamic_pointer_cast<MapSceneRenderForwardVLK>(this->shared_from_this());
    auto mapScene = std::dynamic_pointer_cast<Map>(frameInputParams->frameParameters->scene);

    mapScene->doPostLoad(hthis, framePlan);
    mapScene->update(framePlan);


    return createRenderFuncVLK([](CmdBufRecorder &uploadCmd, CmdBufRecorder &frameBufCmd, CmdBufRecorder &swapChainCmd) -> void {

    });
}

std::shared_ptr<MapRenderPlan> MapSceneRenderForwardVLK::getLastCreatedPlan() {
    return nullptr;
}

