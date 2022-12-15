//
// Created by Deamon on 12.12.22.
//

#include "FrontendUIRendererFactory.h"
#include "vulkan/FrontendUIRenderForwardVLK.h"

std::shared_ptr<FrontendUIRenderer> FrontendUIRendererFactory::createForwardRenderer(HGDevice &device) {
    switch (device->getDeviceType()) {
        case GDeviceType::GVulkan:
            return std::make_shared<FrontendUIRenderForwardVLK>(std::dynamic_pointer_cast<GDeviceVLK>(device));
        default:
            return nullptr;
    }

}
