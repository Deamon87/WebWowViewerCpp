//
// Created by Deamon on 12.12.22.
//

#include "FrontendUIRendererFactory.h"
#ifdef LINK_VULKAN
#include "vulkan/FrontendUIRenderForwardVLK.h"
#endif

std::shared_ptr<FrontendUIRenderer> FrontendUIRendererFactory::createForwardRenderer(HGDevice &device) {
    switch (device->getDeviceType()) {
        case GDeviceType::GVulkan:
#ifdef LINK_VULKAN
            return std::make_shared<FrontendUIRenderForwardVLK>(std::dynamic_pointer_cast<GDeviceVLK>(device));
#else
            return nullptr;
#endif
        default:
            return nullptr;
    }

}
