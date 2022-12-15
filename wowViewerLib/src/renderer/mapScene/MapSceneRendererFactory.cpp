//
// Created by Deamon on 12/1/2022.
//

#include "MapSceneRendererFactory.h"
#include "vulkan/MapSceneRenderForwardVLK.h"

std::shared_ptr<MapSceneRenderer> MapSceneRendererFactory::createForwardRenderer(HGDevice &device) {
    switch (device->getDeviceType()) {
        case GDeviceType::GVulkan:
            return std::make_shared<MapSceneRenderForwardVLK>(std::dynamic_pointer_cast<GDeviceVLK>(device));
        default:
            return nullptr;
    }

    return nullptr;
}
