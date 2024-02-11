//
// Created by Deamon on 2/11/2024.
//

#include "imguiHasFocus.h"
#include "imgui_internal.h"

bool ImGui::HasFocus() {
    return ImGui::GetFocusID() != 0;
}