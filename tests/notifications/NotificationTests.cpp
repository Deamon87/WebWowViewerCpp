#include "../../src/ui/StorageNotificationUI.h"
#include <iostream>
#include <thread>

static void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
int main() {
    auto notifications = std::make_shared<StorageNotifications>();
    auto callback = [notifications](const StorageError& error) { notifications->push(error); };
    std::vector<std::thread> producers;
    for (int i = 0; i < 4; ++i) producers.emplace_back([&, i] {
        for (int j = 0; j < 8; ++j)
            callback({StorageErrorCategory::FileNotFound, std::to_string(i*8+j)});
    });
    for (auto& producer : producers) producer.join();
    require(notifications->drain().size() == 32, "Concurrent producers must not lose errors");
    callback({StorageErrorCategory::FileNotFound, "duplicate"});
    callback({StorageErrorCategory::FileNotFound, "duplicate"});
    require(notifications->drain().size() == 1, "Deduplicate repeated requests");
    notifications->enabled[1] = false;
    callback({StorageErrorCategory::FileNotFound, "disabled file"});
    callback({StorageErrorCategory::MissingKey, "key"});
    auto filtered = notifications->drain();
    require(filtered.size() == 1 && filtered[0].category == StorageErrorCategory::MissingKey,
            "Category switches must be independent");
    for (int i=0; i<1000; ++i) callback({StorageErrorCategory::FileRead, "burst " + std::to_string(i)});
    require(notifications->drain().size() == 64, "Bound queue memory during bursts");

    auto context = ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.DisplaySize = ImVec2(1280, 720);
    io.DeltaTime = 1.0f/60.0f;
    unsigned char* pixels; int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    StorageNotificationUI::registerSettings(context, notifications);
    // Ensure handler names do not depend on the temporary argument string.
    std::vector<std::string> overwrite(100, std::string(128, 'x'));
    const char* ini = "[Notifications][global]\nnotifyStorageErrors=0\nnotifyMissingFiles=1\nnotifyMissingKeys=0\nnotifyFileErrors=1\n\n";
    ImGui::LoadIniSettingsFromMemory(ini);
    require(!notifications->enabled[0] && notifications->enabled[1] &&
            !notifications->enabled[2] && notifications->enabled[3], "Read saved preferences");
    std::string saved = ImGui::SaveIniSettingsToMemory();
    require(saved.find("[Notifications][global]") != std::string::npos &&
            saved.find("notifyMissingKeys=0") != std::string::npos, "Write saved preferences");

    notifications->enabled.fill(true);
    callback({StorageErrorCategory::StorageOpen, "Missing build config (100% test)"});
    callback({StorageErrorCategory::MissingKey, "FileDataId 42: Missing decryption key ABC"});
    ImGui::NewFrame();
    ImGui::Begin("Settings");
    StorageNotificationUI::drawSettings(notifications);
    ImGui::End();
    StorageNotificationUI::render(notifications);
    ImGui::Render();
    require(ImGui::notifications.size() == 2, "Callback events become toasts");
    require(ImGui::notifications[0].get_type() == ImGuiToastType_Error &&
            ImGui::notifications[1].get_type() == ImGuiToastType_Info, "Toast severity mapping");
    require(std::string(ImGui::notifications[0].get_content()).find("100%") != std::string::npos,
            "Messages must be treated as text, not format strings");
    ImGui::notifications.clear();
    ImGui::InsertNotification({ImGuiToastType_Info, 0, "Expired 1"});
    ImGui::InsertNotification({ImGuiToastType_Info, 0, "Expired 2"});
    std::this_thread::sleep_for(std::chrono::milliseconds(350));
    ImGui::InsertNotification({ImGuiToastType_Info, 5000, "Visible"});
    ImGui::NewFrame();
    StorageNotificationUI::render(notifications);
    ImGui::Render();
    require(ImGui::notifications.size()==1, "Removing an expired toast must not skip the next one");
    ImGui::NewFrame();
    StorageNotificationUI::render(notifications);
    ImGui::Render();
    require(ImGui::GetDrawData()->TotalVtxCount > 0, "Toasts must render with the existing font");
    ImGui::notifications.clear();
    ImGui::DestroyContext(context);
    std::cout << "Notification queue, preferences and rendering tests passed\n";
}
