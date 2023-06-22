//
// Created by Deamon on 6/22/2023.
//

#include "wheelCapture.h"
#include "imgui_internal.h"
#include "../../../../wowViewerLib/src/engine/algorithms/mathHelper.h"

bool ImGui::CaptureMouseWheel(float &wheel) {
    ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
    if ((IsKeyDown(ImGuiKey_RightCtrl) || IsKeyDown(ImGuiKey_LeftCtrl))) {
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows )) {


            wheel = ImGui::GetIO().MouseWheel;
            return !feq(wheel, 0.0);
        }
    }

    return false;
}