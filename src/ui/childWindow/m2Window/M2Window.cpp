//
// Created by Deamon on 2/7/2024.
//

#include "M2Window.h"
#include "imgui.h"

M2Window::M2Window(HApiContainer api, const std::shared_ptr<FrontendUIRenderer> &renderer) : SceneWindow(api, false), m_uiRenderer(renderer) {

    openWMOSceneByfdid(113992);
}

M2Window::~M2Window() {
    if (m_renderView) {
        m_renderView->eraseOnUpdate(iteratorUnique);
    }
}

bool M2Window::draw() {
    if (iteratorUnique == nullptr && m_renderView ) {
        auto l_weak = this->weak_from_this();
        iteratorUnique = m_renderView->addOnUpdate([l_weak] {
            auto shared = l_weak.lock();
            if (shared) {
                shared->createMaterials();
            }
        });
    }

    if (ImGui::Begin("M2Window", &m_showWindow))
    {
        auto currentFrame = m_api->hDevice->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

        float sizeX = 0, sizeY = 0;
        auto windowSize = ImGui::GetContentRegionAvail();
        sizeX = windowSize.x;
        sizeY = windowSize.y;

        if (m_width != sizeX || m_height != sizeY) {
            m_width = sizeX > 0 ? sizeX : 1;
            m_height = sizeY > 0 ? sizeY : 1;
        }

        auto const &currentMaterial = materials[currentFrame];
        if (currentMaterial) {
            ImGui::Image(currentMaterial->uniqueId, {sizeX, sizeY});
        }

        ImGui::End();
    }

    return m_showWindow;
}

void M2Window::createMaterials() {
    m_renderView->iterateOverOutputTextures([&](const std::array<std::shared_ptr<ISamplableTexture>, IDevice::MAX_FRAMES_IN_FLIGHT> &textures,
                                                const std::string &name, ITextureFormat textureFormat) {
        if (textureFormat == ITextureFormat::itRGBA) {
            for (int i = 0; i < IDevice::MAX_FRAMES_IN_FLIGHT; i++) {
                materials[i] = m_uiRenderer->createUIMaterial(textures[i]);
            }
        }
    });
}

void M2Window::render(double deltaTime, const HFrameScenario &scenario,
                      const std::function<uint32_t()> &updateFrameNumberLambda) {

    ViewPortDimensions dimensions = {
        {0,0},
        {m_width, m_height}
    };

    SceneWindow::render(deltaTime, 60, scenario, dimensions, updateFrameNumberLambda, nullptr, 0,0);
}
