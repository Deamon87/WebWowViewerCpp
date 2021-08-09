//
// Created by Deamon on 8/3/2021.
//

#ifndef AWEBWOWVIEWERCPP_STATESAVER_H
#define AWEBWOWVIEWERCPP_STATESAVER_H

#include <imgui.h>
void addIniCallback(ImGuiContext* context, const std::string& sectionName,
                    std::function<void(const char* line)> readFunction,
                    std::function<void(ImGuiTextBuffer* buf)> writeFunction
);

#endif //AWEBWOWVIEWERCPP_STATESAVER_H
