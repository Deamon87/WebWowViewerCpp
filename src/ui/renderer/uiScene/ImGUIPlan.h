//
// Created by Deamon on 12.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IMGUIPLAN_H
#define AWEBWOWVIEWERCPP_IMGUIPLAN_H

#include "imgui.h"

namespace ImGuiFramePlan {
    class EmptyPlan {
    };

    class ImGUIParam {
    public:
        ~ImGUIParam() {
            if (imData != nullptr) {
                for (int i = 0; i < imData->CmdListsCount; i++ ) {
                    IM_FREE(imData->CmdLists[i]);
                }
            }
        }

    public:
        ImDrawData *imData = nullptr;
    };
}

#endif //AWEBWOWVIEWERCPP_IMGUIPLAN_H
