//
// Created by Deamon on 1/23/2021.
//

#include "compactColorPicker.h"

void ImGui::CompactColorPicker(const std::string colorName, mathfu::vec4 &color) {
    ImVec4 col = ImVec4(color.x, color.y, color.z, 1.0);
    if (ImGui::ColorButton((colorName+"##3b").c_str(), col)) {
        ImGui::OpenPopup((colorName+"picker").c_str());
    }
    ImGui::SameLine();
    ImGui::Text("%s", colorName.c_str());

    if (ImGui::BeginPopup((colorName + "picker").c_str())) {
        if (ImGui::ColorPicker3(colorName.c_str(), color.data_)) {

        }
        ImGui::EndPopup();
    }
}