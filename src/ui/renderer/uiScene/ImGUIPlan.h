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
        explicit ImGUIParam(ImDrawData *imData, const ViewPortDimensions &dimensions) : m_dimensions(dimensions) {
            //Do copy of imData into local copy
            if (imData != nullptr) {
                m_imData = *imData;
                m_imData.CmdLists.resize(imData->CmdListsCount); //ImDrawList *[imData->CmdListsCount];

                for (int i = 0; i < m_imData.CmdListsCount; i++) {
                    m_imData.CmdLists[i] = imData->CmdLists[i]->CloneOutput();
                }
            } else {
                m_imData.CmdLists.clear();
            }

        }
        ~ImGUIParam() {
            if (!m_imData.CmdLists.empty()) {
                for (int i = 0; i < m_imData.CmdListsCount; i++) {
                    m_imData.CmdLists[i]->~ImDrawList();
                    IM_FREE(m_imData.CmdLists[i]);
                }
            }
        }

        const ImDrawData * const getImData() { return &m_imData; };
        const ViewPortDimensions & getDimensions() const { return m_dimensions; };
    private:
        ImDrawData m_imData;
        ViewPortDimensions m_dimensions;
    };
}

#endif //AWEBWOWVIEWERCPP_IMGUIPLAN_H
