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
    BLPViewer(const HApiContainer &api, const std::shared_ptr<FrontendUIRenderer> &uiRenderer, bool noSearch = false);

    void loadBlp(const std::string &p_blpName);
    bool draw();
private:
    bool m_showWindow = true;
    std::array<char, 128> blpName = {0};\

    bool stretchImage = false;

    HApiContainer m_api;
    std::shared_ptr<FrontendUIRenderer> m_uiRenderer;
    std::shared_ptr<BlpTexture> m_blpTexture;
    HGSamplableTexture m_texture;
    std::shared_ptr<IUIMaterial> material = nullptr;

    bool m_noSearch;
};


#endif //AWEBWOWVIEWERCPP_BLPVIEWER_H
