//
// Created by Deamon on 9/26/2023.
//

#include "DebugRendererWindow.h"
#include "../../../../wowViewerLib/src/renderer/mapScene/MapSceneParams.h"

DebugRendererWindow::DebugRendererWindow(const HApiContainer &api,
                                         const std::shared_ptr<FrontendUIRenderer> &renderer,
                                         const std::shared_ptr<IRenderView> &renderView) :
                                         m_renderView(renderView), m_api(api), m_renderer(renderer) {

    createMaterials();
}

DebugRendererWindow::~DebugRendererWindow() {
    m_renderView->eraseOnUpdate(iteratorUnique);
}

void DebugRendererWindow::draw() {
    if (iteratorUnique == nullptr) {
        auto l_weak = this->weak_from_this();
        iteratorUnique = m_renderView->addOnUpdate([l_weak] {
            auto shared = l_weak.lock();
            if (shared) {
                shared->createMaterials();
            }
        });
    }

    auto currentFrame = m_api->hDevice->getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

    ImGui::Begin("Debug Render Window");
    {
//        drawSelection();

        float sizeX = 0, sizeY = 0;
        auto windowSize = ImGui::GetContentRegionAvail();
        sizeX = windowSize.x;
        sizeY = windowSize.y;

        if (m_width != sizeX || m_height != sizeY) {
            m_width = sizeX > 0 ? sizeX : 1;
            m_height = sizeY > 0 ? sizeY : 1;
        }

        if (currentSelectionIndex < m_selections.size()) {
            auto const &currentMaterial = m_selections[currentSelectionIndex].materials[currentFrame];
            if (currentMaterial) {
                ImGui::Image(currentMaterial->uniqueId, {sizeX, sizeY});
            }
        }
    }
    ImGui::End();
}

void DebugRendererWindow::createMaterials() {
    this->m_selections.clear();
    m_renderView->iterateOverOutputTextures([&](const std::array<std::shared_ptr<ISamplableTexture>, IDevice::MAX_FRAMES_IN_FLIGHT> &textures,
                                               const std::string &name, ITextureFormat textureFormat) {
        SelectionRecord &record = this->m_selections.emplace_back();
        record.name = name;

        if (textureFormat == ITextureFormat::itRGBA) {
            for (int i = 0; i < IDevice::MAX_FRAMES_IN_FLIGHT; i++) {
                record.materials[i] = this->m_renderer->createUIMaterial(textures[i]);
            }
        }
//        else if (textureFormat == ITextureFormat::itDepth32) {
//            for (int i = 0; i < IDevice::MAX_FRAMES_IN_FLIGHT; i++) {
//                if (textures[i]->getTexture()->getIsSamplable()) {
//                    record.materials[i] = this->m_renderer->createUIMaterial(textures[i], true);
//                }
//            }
//        }
    });
    if (currentSelectionIndex > m_selections.size())
        currentSelectionIndex = 0;
}

void DebugRendererWindow::drawSelection() {
    std::string comboName =
        (currentSelectionIndex < m_selections.size()) ?
        m_selections[currentSelectionIndex].name :
        "";

    if (ImGui::BeginCombo("##combo", comboName.c_str())) // The second parameter is the label previewed before opening the combo.
    {
        for (int n = 0; n < m_selections.size(); n++)
        {
            bool is_selected = (currentSelectionIndex == n); // You can store your selection however you want, outside or inside your objects
            std::string selectionName =  m_selections[n].name;
            if (ImGui::Selectable(selectionName.c_str(), is_selected)) {
                currentSelectionIndex = n;
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
        }
        ImGui::EndCombo();
    }
}
