//
// Created by Deamon on 12.12.22.
//

#ifndef AWEBWOWVIEWERCPP_FRONTENDUIRENDERERFACTORY_H
#define AWEBWOWVIEWERCPP_FRONTENDUIRENDERERFACTORY_H

#include <memory>
#include <memory>
#include "FrontendUIRenderer.h"
#include "../../../../wowViewerLib/src/gapi/interface/IDevice.h"

class FrontendUIRendererFactory {
    static std::shared_ptr<FrontendUIRenderer> createForwardRenderer(HGDevice &device);
};


#endif //AWEBWOWVIEWERCPP_FRONTENDUIRENDERERFACTORY_H
