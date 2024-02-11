//
// Created by Deamon on 2/11/2024.
//

#ifndef AWEBWOWVIEWERCPP_IMGUIHASFOCUS_H
#define AWEBWOWVIEWERCPP_IMGUIHASFOCUS_H


namespace ImGui {
    //The standart imgui button ties to user_texture_id as id. Thus it cant detect clicks,
    // when user_texture_id is different in every frame (like when that texture is from framebuffer)
    bool          HasFocus();
}



#endif //AWEBWOWVIEWERCPP_IMGUIHASFOCUS_H
