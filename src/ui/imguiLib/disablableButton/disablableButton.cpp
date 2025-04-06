//
// Created by Deamon on 1/16/2021.
//

#include "disablableButton.h"
#include <imgui.h>
#include <imgui_internal.h>

bool ImGui::ButtonDisablable(const char* label, bool disabled, const struct ImVec2& size_arg) {
    bool result;
    if (disabled) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f,0.5f,0.5f,1.0));
        result = ImGui::ButtonEx(label, size_arg, ImGuiItemFlags_Disabled);
        ImGui::PopStyleColor(1);
    } else {
        result = ImGui::Button(label, size_arg);
    }

    return result;
}