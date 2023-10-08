//
// Created by Deamon on 12/1/2022.
//

#include "MapSceneRendererFactory.h"
#include "vulkan/MapSceneRenderForwardVLK.h"
#include "vulkan/MapSceneRenderVisBufferVLK.h"

std::shared_ptr<MapSceneRenderer> MapSceneRendererFactory::createForwardRenderer(const HGDevice &device, Config * config) {
    switch (device->getDeviceType()) {
        case GDeviceType::GVulkan:
            return std::make_shared<MapSceneRenderVisBufferVLK>(std::dynamic_pointer_cast<GDeviceVLK>(device), config);
        default:
            return std::make_shared<MapSceneRenderVisBufferVLK>(std::dynamic_pointer_cast<GDeviceVLK>(device), config);
    }

    return nullptr;
}
