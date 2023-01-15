//
// Created by Deamon on 12/1/2022.
//

#include "MapSceneRenderForwardVLK.h"

MapSceneRenderForwardVLK::MapSceneRenderForwardVLK(HGDeviceVLK hDevice) : m_device(hDevice) {

}

void MapSceneRenderForwardVLK::putIntoQueue(std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams) {

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

std::shared_ptr<MapRenderPlan> MapSceneRenderForwardVLK::getLastPlan() {
    return std::shared_ptr<MapRenderPlan>();
}
