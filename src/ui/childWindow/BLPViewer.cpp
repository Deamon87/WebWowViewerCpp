//
// Created by Deamon on 7/14/2023.
//

#include "BLPViewer.h"
#include "imgui.h"
#include <string>

BLPViewer::BLPViewer(const HApiContainer &api, const std::shared_ptr<FrontendUIRenderer> &uiRenderer, bool noSearch) :
    m_api(api), m_uiRenderer(uiRenderer), m_noSearch(noSearch), windowName(std::string("BLP Viewer")+(noSearch ? "##no_search" : ""))
{

}

bool BLPViewer::draw() {

    ImGui::Begin(windowName.c_str(), &m_showWindow);
    {
        if (!m_noSearch) {
            if (ImGui::InputText("BlpName/FileDataId: ", blpName.data(), blpName.size() - 1)) {
                blpName[blpName.size() - 1] = 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("Load")) {
                std::string blpNameStr = blpName.data();
                loadBlp(blpNameStr);
            }
        }

        if (m_blpTexture && m_blpTexture->getStatus() == FileStatus::FSRejected) {
            ImGui::Text("Failed to load BLP file");
        } else
        if (m_blpTexture && m_blpTexture->getStatus() == FileStatus::FSLoaded && material) {
            float sizeX = 0, sizeY = 0;

            sizeX = (m_texture && m_texture->getTexture()) ? m_texture->getTexture()->getWidth() : 0;
            sizeY = (m_texture && m_texture->getTexture()) ? m_texture->getTexture()->getHeight() : 0;

            ImGui::Text("Width = %0.0f, Height = %0.0f", sizeX, sizeY);
            ImGui::Checkbox("Stretch image", &stretchImage);


            if (sizeX > 0 && sizeY > 0 && stretchImage) {
                auto windowSize = ImGui::GetContentRegionAvail();

                // Calculate resize ratios for resizing
                float ratioW = windowSize.x / sizeX;
                float ratioH = windowSize.y / sizeY;

                // smaller ratio will ensure that the image fits in the view
                float ratio = ratioW < ratioH ? ratioW : ratioH;

                sizeX = sizeX * ratio;
                sizeY = sizeY * ratio;
            }

            ImGui::Image(material->uniqueId, ImVec2(sizeX, sizeY));
        }
    }
    ImGui::End();

    return m_showWindow;
}

void BLPViewer::loadBlp(const std::string &p_blpName) {
    int fileDataId = 0;

    try{
        fileDataId = std::stoi(p_blpName);
    } catch (...) {}

    if (fileDataId > 0) {
        m_blpTexture = m_api->cacheStorage->getTextureCache()->getFileId(fileDataId);
        m_texture = m_api->hDevice->createBlpTexture(m_blpTexture, false, false);
        material = m_uiRenderer->createUIMaterial({m_texture}, true);
    } else {
        m_blpTexture = m_api->cacheStorage->getTextureCache()->get(blpName.data());
        m_texture = m_api->hDevice->createBlpTexture(m_blpTexture, false, false);
        material = m_uiRenderer->createUIMaterial({m_texture}, true);
    }
}
