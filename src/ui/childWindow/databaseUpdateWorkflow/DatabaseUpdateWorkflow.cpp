//
// Created by Deamon on 8/27/2022.
//

#include "DatabaseUpdateWorkflow.h"
#include "imgui.h"
#include "imgui_internal.h"

static const std::string POPUP_UPDATE_DBD = "Update DBD files";

DatabaseUpdateWorkflow::DatabaseUpdateWorkflow() {

}

void DatabaseUpdateWorkflow::render() {
    if (m_showDBDPrompt) {
        ImGui::OpenPopup(POPUP_UPDATE_DBD.c_str());
    } else {

    }


    defineDialogs();
}

void DatabaseUpdateWorkflow::defineDialogs() {
    if (ImGui::BeginPopupModal(POPUP_UPDATE_DBD.c_str()))
    {
        ImGui::Text("Do you want to download DBD from github?");
        ImGui::Text("Warning: current files will be overwritten");

        if (ImGui::Button("Yes", ImVec2(-1, 23))) {
            m_showDBDPrompt = false;
            m_needToUpdateDBD = true;
        }
        if (ImGui::Button("No", ImVec2(-1, 23))) {
            m_showDBDPrompt = false;
            m_needToUpdateDBD = false;
        }

        ImGui::EndPopup();
    }
}
