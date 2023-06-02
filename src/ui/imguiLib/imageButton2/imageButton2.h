//
// Created by Deamon on 1/27/2021.
//

#ifndef AWEBWOWVIEWERCPP_IMAGEBUTTON2_H
#define AWEBWOWVIEWERCPP_IMAGEBUTTON2_H

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS 1
#endif
#include <imgui.h>

namespace ImGui {
    //The standart imgui button ties to user_texture_id as id. Thus it cant detect clicks,
    // when user_texture_id is different in every frame (like when that texture is from framebuffer)
    IMGUI_API bool          ImageButton2(ImTextureID user_texture_id, char *idText, const ImVec2& size, const ImVec2& uv0 = ImVec2(0,0),  const ImVec2& uv1 = ImVec2(1,1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1));    // <0 frame_padding uses default frame padding settings. 0 for no padding
}

#endif //AWEBWOWVIEWERCPP_IMAGEBUTTON2_H
