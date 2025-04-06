//
// Created by Deamon on 1/9/2021.
//

#ifndef AWEBWOWVIEWERCPP_GROUPPANEL_H
#define AWEBWOWVIEWERCPP_GROUPPANEL_H

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS 1
#endif
#include <imgui.h>

namespace ImGui {
    void BeginGroupPanel(const char *name, const ImVec2 &size = ImVec2(0.0f, 0.0f));

    void EndGroupPanel();
}

#endif //AWEBWOWVIEWERCPP_GROUPPANEL_H
