//
// Created by Deamon on 12/1/2022.
//

#include "MapSceneRendererFactory.h"
#ifdef LINK_VULKAN
#include "vulkan/MapSceneRenderForwardVLK.h"
#include "vulkan/MapSceneRenderBindlessVLK.h"
#include "vulkan/MapSceneRenderDeferredVLK.h"
#endif


std::shared_ptr<MapSceneRenderer> MapSceneRendererFactory::createForwardRenderer(const HGDevice &device, bool forceForward, Config * config) {
    switch (device->getDeviceType()) {
        case GDeviceType::GVulkan:
#ifdef LINK_VULKAN
            if (forceForward) {
                return std::make_shared<MapSceneRenderForwardVLK>(std::dynamic_pointer_cast<GDeviceVLK>(device), config);
            } else if (!device->supportsBindless()) {
                return std::make_shared<MapSceneRenderDeferredVLK>(std::dynamic_pointer_cast<GDeviceVLK>(device), config);
            } else {
                return std::make_shared<MapSceneRenderBindlessVLK>(std::dynamic_pointer_cast<GDeviceVLK>(device), config);
            }
#else
            return nullptr;
#endif
        default:
            return nullptr;
    }

    return nullptr;
}
