//
// Created by Deamon on 9/26/2023.
//

#ifndef AWEBWOWVIEWERCPP_DEBUGRENDERERWINDOW_H
#define AWEBWOWVIEWERCPP_DEBUGRENDERERWINDOW_H

#include "../../renderer/uiScene/FrontendUIRenderer.h"
#include "../../../../wowViewerLib/src/engine/ApiContainer.h"
#include "../../../../wowViewerLib/src/renderer/mapScene/MapSceneParams.h"

class DebugRendererWindow : public std::enable_shared_from_this<DebugRendererWindow> {
public:
    DebugRendererWindow(const HApiContainer &api, const std::shared_ptr<FrontendUIRenderer> &renderer,
                        const std::shared_ptr<IRenderView> &renderView);
    ~DebugRendererWindow();

    void draw();
    void createMaterials();

    uint16_t getWidth() { return m_width;}
    uint16_t getHeight() { return m_height;}
private:
    HApiContainer m_api;
    std::shared_ptr<IRenderView> m_renderView;

    struct SelectionRecord {
        std::string name;
        std::array<std::shared_ptr<IUIMaterial>, IDevice::MAX_FRAMES_IN_FLIGHT> materials;
    };

    int currentSelectionIndex = 0;
    std::vector<SelectionRecord> m_selections;
    std::shared_ptr<FrontendUIRenderer> m_renderer;

    typedef std::list<std::function<void()>>::const_iterator OnUpdateIter;
    std::unique_ptr<OnUpdateIter> iteratorUnique;

    uint16_t m_width = 640;
    uint16_t m_height = 480;

    void drawSelection();
};


#endif //AWEBWOWVIEWERCPP_DEBUGRENDERERWINDOW_H
