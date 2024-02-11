//
// Created by Deamon on 2/7/2024.
//

#include "M2Window.h"
#include "imgui.h"
#include "imgui_internal.h"

M2Window::M2Window(HApiContainer api, const std::shared_ptr<FrontendUIRenderer> &renderer, const std::string &nameSuffix) : SceneWindow(api, false), m_uiRenderer(renderer) {
    m_windowName = "M2Window##" + nameSuffix;

    openWMOSceneByfdid(113992);
}

M2Window::~M2Window() {
    if (m_renderView) {
        m_renderView->eraseOnUpdate(iteratorUnique);
    }
}

bool M2Window::draw() {
    this->m_isActive = false;

    if (iteratorUnique == nullptr && m_renderView ) {
        auto l_weak = this->weak_from_this();
        iteratorUnique = m_renderView->addOnUpdate([l_weak] {
            auto shared = l_weak.lock();
            if (shared) {
                shared->m_needToUpdateMaterials = true;
            }
        });
    }

    if (m_needToUpdateMaterials) {
        this->createMaterials();
        m_needToUpdateMaterials = false;
    }

    if (ImGui::Begin(m_windowName.c_str(), &m_showWindow))
    {
        auto currentFrame = m_api->hDevice->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

        auto imguiContent = ImGui::GetCurrentContext();
        ImGui::Text("FocusID == %d LastActiveId == %d GetActiveID == %d", ImGui::GetFocusID(), imguiContent->LastActiveId, ImGui::GetActiveID());
//        std::cout
//            << " FocusID == " << ImGui::GetFocusID()
//            << " LastActiveId == " << imguiContent->LastActiveId
//            << " GetActiveID == " << ImGui::GetActiveID()
//            << std::endl;




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
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,1.0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0,0,0,1.0));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0,0,0,1.0));

            ImGui::ImageButton("ModelScene", currentMaterial->uniqueId, {sizeX, sizeY});

            auto modelSceneId = ImGui::GetCurrentWindow()->GetID("ModelScene");


            if (modelSceneId == ImGui::GetFocusID() &&
                modelSceneId == imguiContent->LastActiveId &&
                (modelSceneId == ImGui::GetActiveID() || 0 == ImGui::GetActiveID())) {
                ImGui::SetNextFrameWantCaptureKeyboard(false);
                ImGui::SetNextFrameWantCaptureMouse(false);
//                static uint32_t itsActive = 0;
//                std::cout << "it's active" << itsActive++ << std::endl;
                this->m_isActive = true;
            }
            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar(3);
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

    m_dimension = {
        {0,0},
        {m_width, m_height}
    };

    SceneWindow::render(deltaTime, 60, scenario, nullptr, updateFrameNumberLambda);
}
