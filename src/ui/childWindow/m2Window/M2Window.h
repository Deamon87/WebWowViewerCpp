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
    M2Window(HApiContainer api, const std::shared_ptr<FrontendUIRenderer> &renderer);
    ~M2Window();
    bool draw();

    void render(double deltaTime,
                const HFrameScenario &scenario,
                const std::function<uint32_t()> &updateFrameNumberLambda);
private:
    void createMaterials();
private:
    bool m_showWindow = true;

    uint16_t m_width = 640;
    uint16_t m_height = 480;

    std::shared_ptr<FrontendUIRenderer> m_uiRenderer;

    typedef std::list<std::function<void()>>::const_iterator OnUpdateIter;
    std::unique_ptr<OnUpdateIter> iteratorUnique = nullptr;

    std::array<std::shared_ptr<IUIMaterial>, IDevice::MAX_FRAMES_IN_FLIGHT> materials = {nullptr};
};


#endif //AWEBWOWVIEWERCPP_M2WINDOW_H
