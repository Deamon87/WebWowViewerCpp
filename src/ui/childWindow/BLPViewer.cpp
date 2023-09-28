//
// Created by Deamon on 7/14/2023.
//

#include "BLPViewer.h"
#include "imgui.h"
#include <string>

BLPViewer::BLPViewer(const HApiContainer &api, const std::shared_ptr<FrontendUIRenderer> &uiRenderer) :
    m_api(api), m_uiRenderer(uiRenderer)
{

}

bool BLPViewer::draw() {

    ImGui::Begin("BLP Viewer", &m_showWindow);
    {
        if (ImGui::InputText("BlpName/FileDataId: ", blpName.data(), blpName.size()-1)) {
            blpName[blpName.size()-1] = 0;
        }
        ImGui::SameLine();
        if (ImGui::Button("Load")) {
            int fileDataId = 0;

            try{
                fileDataId = std::stoi(blpName.data());
            } catch (...) {}

            if (fileDataId > 0) {
                m_blpTexture = m_api->cacheStorage->getTextureCache()->getFileId(fileDataId);
                auto textureObj = m_api->hDevice->createBlpTexture(m_blpTexture, false, false);
                material = m_uiRenderer->createUIMaterial({textureObj});
            } else {
                m_blpTexture = m_api->cacheStorage->getTextureCache()->get(blpName.data());
                auto textureObj = m_api->hDevice->createBlpTexture(m_blpTexture, false, false);
                material = m_uiRenderer->createUIMaterial({textureObj});
            }
        }
        if (m_blpTexture && m_blpTexture->getStatus() == FileStatus::FSRejected) {
            ImGui::Text("Failed to load BLP file");
        } else
        if (m_blpTexture && m_blpTexture->getStatus() == FileStatus::FSLoaded && material) {
            float sizeX = 0, sizeY = 0;
            auto windowSize = ImGui::GetContentRegionAvail();
            sizeX = windowSize.x;
            sizeY = windowSize.y;

            ImGui::ImageButton(material, ImVec2(sizeX, sizeY));
        }
    }
    ImGui::End();

    return m_showWindow;
}
