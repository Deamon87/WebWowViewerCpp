#pragma once

#include "StorageNotifications.h"
#include "imguiLib/imgui.h"
#include "imguiLib/imgui_internal.h"
#include "imgui_notify.h"
#include "imguiLib/stateSaver/stateSaver.h"
#include <memory>

namespace StorageNotificationUI {
inline void registerSettings(ImGuiContext* imguiContext, const std::shared_ptr<StorageNotifications>& notifications) {
    addIniCallback(imguiContext, "Notifications",
        [notifications](const char* line) {
            for (size_t i = 0; i < notifications->keys.size(); ++i) {
                const std::string prefix = std::string(notifications->keys[i]) + "=";
                if (std::strncmp(line, prefix.c_str(), prefix.size()) == 0)
                    notifications->enabled[i] = std::strcmp(line + prefix.size(), "0") != 0;
            }
        },
        [notifications](ImGuiTextBuffer* buf) {
            for (size_t i = 0; i < notifications->keys.size(); ++i)
                buf->appendf("%s=%d\n", notifications->keys[i], notifications->enabled[i] ? 1 : 0);
        });
}

inline void drawSettings(const std::shared_ptr<StorageNotifications>& notifications) {
    ImGui::TextUnformatted("Notifications");
    for (size_t i = 0; i < notifications->enabled.size(); ++i) {
        if (ImGui::Checkbox(StorageNotifications::labels[i], &notifications->enabled[i]))
            ImGui::MarkIniSettingsDirty();
    }
    ImGui::Separator();
}

inline void render(const std::shared_ptr<StorageNotifications>& notifications) {
    for (const auto& error : notifications->drain()) {
        const bool info = error.category == StorageErrorCategory::FileNotFound ||
                          error.category == StorageErrorCategory::MissingKey;
        ImGuiToast toast(info ? ImGuiToastType_Info : ImGuiToastType_Error, info ? 5000 : 8000);
        toast.set_title("%s", StorageNotifications::labels.at(static_cast<size_t>(error.category)));
        toast.set_content("%s", error.message.c_str());
        if (ImGui::notifications.size() >= 64) ImGui::RemoveNotification(0);
        ImGui::InsertNotification(toast);
    }
    ImGui::RenderNotifications();
}
}
