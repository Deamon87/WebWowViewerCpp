//
// Created by Deamon on 12/1/2022.
//

#include "MapSceneRenderForwardVLK.h"

MapSceneRenderForwardVLK::MapSceneRenderForwardVLK(HGDeviceVLK hDevice) : m_device(hDevice) {

}

//Buffer creation
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

    return nullptr;
}

std::shared_ptr<MapRenderPlan> MapSceneRenderForwardVLK::getLastCreatedPlan() {
    return nullptr;
}

std::shared_ptr<MapRenderPlan>
MapSceneRenderForwardVLK::processCulling(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams) {
    return nullptr;
}
