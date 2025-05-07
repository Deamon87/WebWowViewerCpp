//
// Created by Deamon on 5/6/2025.
//

#include "compactFloatEditor.h"

void ImGui::CompactFloatEditor(const std::string &name, float &f, bool columnMode) {
    if (columnMode) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
    }

    ImGui::Text("%s", name.c_str());

    if (columnMode) {
        ImGui::TableNextColumn();
    } else {
        ImGui::SameLine();
    }


    // if (ImGui::ColorButton((colorName+"##3b").c_str(), col)) {
    //     ImGui::OpenPopup((colorName+"picker").c_str());
    // }
    //
    // if (ImGui::BeginPopup((colorName + "picker").c_str())) {
    //     if (ImGui::ColorPicker3(colorName.c_str(), color.data_)) {
    //
    //     }
    //     ImGui::EndPopup();
    // }
}

