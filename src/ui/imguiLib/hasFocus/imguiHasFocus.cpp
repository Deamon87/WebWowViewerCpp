//
// Created by Deamon on 2/11/2024.
//

#include "imguiHasFocus.h"
#include "imgui_internal.h"

bool ImGui::HasFocus() {
    auto context = ImGui::GetCurrentContext();
    ImGuiWindow* window = context->NavWindow;
    return window != nullptr;
}