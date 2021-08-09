#include <imgui.h>
#include <imgui_internal.h>

void addIniCallback(ImGuiContext* context, const std::string& sectionName,
                    std::function<void(const char* line)> readFunction,
                    std::function<void(ImGuiTextBuffer* buf)> writeFunction
                    )
{
    ImGuiContext& g = *context;

    // Add .ini handle for UserData type
    ImGuiSettingsHandler ini_handler;
    ini_handler.TypeName = sectionName.c_str();
    ini_handler.TypeHash = ImHashStr(sectionName.c_str());
    ini_handler.ReadOpenFn = [](ImGuiContext*, ImGuiSettingsHandler*, const char* name) -> void* {
        if (std::string("global").compare(name) != 0 )
            return nullptr;

        return reinterpret_cast<void *>(1);
    };
    ini_handler.ReadLineFn = [readFunction](ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line) -> void {
        readFunction(line);
    };

    ini_handler.WriteAllFn = [writeFunction, sectionName](ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf) -> void {
        buf->appendf("[%s][%s]\n", sectionName.c_str(), "global");
        writeFunction(buf);
//        buf->appendf("Pos=%d,%d\n", settings->Pos.x, settings->Pos.y);
//        buf->appendf("Size=%d,%d\n", settings->Size.x, settings->Size.y);
//        buf->appendf("Collapsed=%d\n", settings->Collapsed);
        buf->append("\n");
    };
    g.SettingsHandlers.push_back(ini_handler);
}