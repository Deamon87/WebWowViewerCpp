//
// Created by Deamon on 1/16/2021.
//

#ifndef AWEBWOWVIEWERCPP_DISABLABLEBUTTON_H
#define AWEBWOWVIEWERCPP_DISABLABLEBUTTON_H

#include <imgui.h>
namespace ImGui {
    bool ButtonDisablable(const char* label, bool disabled, const struct ImVec2& size = ImVec2(0,0));
}


#endif //AWEBWOWVIEWERCPP_DISABLABLEBUTTON_H
