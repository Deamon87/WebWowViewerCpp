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
        explicit ImGUIParam(ImDrawData *imData) {
            //Do copy of imData into local copy
            if (imData != nullptr) {
                m_imData = *imData;
                m_imData.CmdLists = new ImDrawList *[imData->CmdListsCount];

                for (int i = 0; i < m_imData.CmdListsCount; i++) {
                    m_imData.CmdLists[i] = imData->CmdLists[i]->CloneOutput();
                }
            } else {
                m_imData.CmdLists = nullptr;
            }

        }
        ~ImGUIParam() {
            if (m_imData.CmdLists != nullptr) {
                for (int i = 0; i < m_imData.CmdListsCount; i++) {
                    IM_FREE(m_imData.CmdLists[i]);
                }
                delete m_imData.CmdLists;
            }
        }

        const ImDrawData * const getImData() { return &m_imData; };
    private:
        ImDrawData m_imData;
    };
}

#endif //AWEBWOWVIEWERCPP_IMGUIPLAN_H
