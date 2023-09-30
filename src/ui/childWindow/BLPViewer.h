//
// Created by Deamon on 7/14/2023.
//

#ifndef AWEBWOWVIEWERCPP_BLPVIEWER_H
#define AWEBWOWVIEWERCPP_BLPVIEWER_H

#include <array>
#include "../../../wowViewerLib/src/gapi/interface/materials/IMaterial.h"
#include "../../../wowViewerLib/src/engine/ApiContainer.h"
#include "../renderer/uiScene/FrontendUIRenderer.h"

class BLPViewer {
public:
    BLPViewer(const HApiContainer &api, const std::shared_ptr<FrontendUIRenderer> &uiRenderer);

    bool draw();
private:
    bool m_showWindow = true;
    std::array<char, 128> blpName = {0};\

    HApiContainer m_api;
    std::shared_ptr<FrontendUIRenderer> m_uiRenderer;
    std::shared_ptr<BlpTexture> m_blpTexture;
    std::shared_ptr<IUIMaterial> material = nullptr;
};


#endif //AWEBWOWVIEWERCPP_BLPVIEWER_H
