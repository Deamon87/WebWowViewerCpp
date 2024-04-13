//
// Created by Deamon on 2/7/2024.
//

#ifndef AWEBWOWVIEWERCPP_M2WINDOW_H
#define AWEBWOWVIEWERCPP_M2WINDOW_H


#include "../sceneWindow/SceneWindow.h"
#include "../../renderer/uiScene/materials/UIMaterial.h"
#include "../../renderer/uiScene/FrontendUIRenderer.h"

class M2Window : public SceneWindow, public std::enable_shared_from_this<M2Window> {
public:
    M2Window(HApiContainer api, const std::shared_ptr<FrontendUIRenderer> &renderer, const std::string &nameSuffix = "");
    ~M2Window();
    bool draw();

    void render(double deltaTime,
                const HFrameScenario &scenario,
                const std::function<uint32_t()> &updateFrameNumberLambda);

    bool isActive() const;;
    void setSelectedMat(uint8_t matIndex) override;
    std::vector<std::tuple<std::string, std::shared_ptr<IUIMaterial>>> getMaterials() override;
private:
    std::string m_windowName = "";
    bool m_showWindow = true;
    bool m_isActive = false;
    uint8_t m_selectedMat = 0;

    uint16_t m_width = 640;
    uint16_t m_height = 480;
};


#endif //AWEBWOWVIEWERCPP_M2WINDOW_H
