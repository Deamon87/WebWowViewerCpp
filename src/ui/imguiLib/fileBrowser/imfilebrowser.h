#pragma once

#include <array>
#include <cstring>
#include <iostream>
//#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <string>

#include "../../../../3rdparty/filesystem_impl/include/ghc/filesystem.hpp"

#ifndef IMGUI_VERSION
#   error "include imgui.h before this header"
#endif

#include "../../../../wowViewerLib/src/include/string_utils.h"
#include "buildDefinition.h"
#include "../../../database/buildInfoParser/buildInfoParser.h"
#include "../../../database/product_db_parser/productDbParser.h"

using ImGuiFileBrowserFlags = int;

enum ImGuiFileBrowserFlags_
{
    ImGuiFileBrowserFlags_SelectDirectory    = 1 << 0, // select directory instead of regular file
    ImGuiFileBrowserFlags_EnterNewFilename   = 1 << 1, // allow user to enter new filename when selecting regular file
    ImGuiFileBrowserFlags_NoModal            = 1 << 2, // file browsing window is modal by default. specify this to use a popup window
    ImGuiFileBrowserFlags_NoTitleBar         = 1 << 3, // hide window title bar
    ImGuiFileBrowserFlags_NoStatusBar        = 1 << 4, // hide status bar at the bottom of browsing window
    ImGuiFileBrowserFlags_CloseOnEsc         = 1 << 5, // close file browser when pressing 'ESC'
    ImGuiFileBrowserFlags_CreateNewDir       = 1 << 6, // allow user to create new directory
};

namespace ImGui
{
    class FileBrowser
    {
    public:

        // pwd is set to current working directory by default
        explicit FileBrowser(ImGuiFileBrowserFlags flags = 0, bool cascOpenMode = false );

        FileBrowser(const FileBrowser &copyFrom);

        FileBrowser &operator=(const FileBrowser &copyFrom);

        // set the window title text
        void SetTitle(std::string title);

        // open the browsing window
        void Open();

        // close the browsing window
        void Close();

        // the browsing window is opened or not
        bool IsOpened() const noexcept;

        // display the browsing window if opened
        void Display();

        // returns true when there is a selected filename and the "ok" button was clicked
        bool HasSelected() const noexcept;

        // set current browsing directory
        bool SetPwd(const ghc::filesystem::path &pwd = ghc::filesystem::current_path());

        // returns selected filename. make sense only when HasSelected returns true
        ghc::filesystem::path GetSelected() const;

        // set selected filename to empty
        void ClearSelected();

        // set file type filters. eg. { ".h", ".cpp", ".hpp", ".cc", ".inl" }
        void SetTypeFilters(const std::vector<const char*> &typeFilters);

        bool isCascOpenMode() { return m_cascOpenMode;};
        BuildDefinition getProductBuild() { return currentBuild;};
    private:

        void loadBuildsFromBuildInfo();

        bool m_cascOpenMode = false;
        ghc::filesystem::path m_last_pwd_forBuildInfo;
        std::vector<BuildDefinition> availableBuilds;
        BuildDefinition currentBuild = {};

        class ScopeGuard
        {
            std::function<void()> func_;

        public:

            template<typename T>
            explicit ScopeGuard(T func) : func_(std::move(func)) { }
            ~ScopeGuard() { func_(); }
        };

        void SetPwdUncatched(const ghc::filesystem::path &pwd);

#ifdef _WIN32
        static std::uint32_t GetDrivesBitMask();
#endif

        ImGuiFileBrowserFlags flags_;

        std::string title_;
        std::string openLabel_;

        bool openFlag_;
        bool closeFlag_;
        bool isOpened_;
        bool ok_;

        std::string statusStr_;

        std::vector<const char*> typeFilters_;
        int typeFilterIndex_;

        ghc::filesystem::path pwd_;
        std::string selectedFilename_;

        struct FileRecord
        {
            bool isDir = false;
            std::string name;
            std::string showName;
            std::string extension;
        };
        std::vector<FileRecord> fileRecords_;

        // IMPROVE: overflow when selectedFilename_.length() > inputNameBuf_.size() - 1
        static constexpr size_t INPUT_NAME_BUF_SIZE = 512;
        std::unique_ptr<std::array<char, INPUT_NAME_BUF_SIZE>> inputNameBuf_;

        std::string openNewDirLabel_;
        std::unique_ptr<std::array<char, INPUT_NAME_BUF_SIZE>> newDirNameBuf_;

#ifdef _WIN32
        uint32_t drives_;
#endif
    };
} // namespace ImGui

inline ImGui::FileBrowser::FileBrowser(ImGuiFileBrowserFlags flags, bool cascOpenMode)
    : flags_(flags),
      openFlag_(false), closeFlag_(false), isOpened_(false), ok_(false),
      m_cascOpenMode(cascOpenMode),
      inputNameBuf_(std::make_unique<std::array<char, INPUT_NAME_BUF_SIZE>>())
{
    if(flags_ & ImGuiFileBrowserFlags_CreateNewDir)
        newDirNameBuf_ = std::make_unique<std::array<char, INPUT_NAME_BUF_SIZE>>();

    inputNameBuf_->at(0) = '\0';
    if (m_cascOpenMode) {
        SetTitle("Select WoW Directory");
    } else {
        SetTitle("file browser");
    }

    SetPwd(ghc::filesystem::current_path());

    typeFilters_.clear();
    typeFilterIndex_ = 0;

#ifdef _WIN32
    drives_ = GetDrivesBitMask();
#endif
}

inline ImGui::FileBrowser::FileBrowser(const FileBrowser &copyFrom)
    : FileBrowser()
{
    *this = copyFrom;
}

inline ImGui::FileBrowser &ImGui::FileBrowser::operator=(const FileBrowser &copyFrom)
{
    flags_ = copyFrom.flags_;
    SetTitle(copyFrom.title_);

    openFlag_  = copyFrom.openFlag_;
    closeFlag_ = copyFrom.closeFlag_;
    isOpened_  = copyFrom.isOpened_;
    ok_        = copyFrom.ok_;

    statusStr_ = "";
    pwd_ = copyFrom.pwd_;
    selectedFilename_ = copyFrom.selectedFilename_;

    fileRecords_ = copyFrom.fileRecords_;

    *inputNameBuf_ = *copyFrom.inputNameBuf_;

    if(flags_ & ImGuiFileBrowserFlags_CreateNewDir)
    {
        newDirNameBuf_ = std::make_unique<std::array<char, INPUT_NAME_BUF_SIZE>>();
        *newDirNameBuf_ = *copyFrom.newDirNameBuf_;
    }

    return *this;
}

inline void ImGui::FileBrowser::SetTitle(std::string title)
{
    title_ = std::move(title);
    openLabel_ = title_ + "##filebrowser_" + std::to_string(reinterpret_cast<size_t>(this));
    openNewDirLabel_ = "new dir##new_dir_" + std::to_string(reinterpret_cast<size_t>(this));
}

inline void ImGui::FileBrowser::Open()
{
    ClearSelected();
    statusStr_ = std::string();
    openFlag_ = true;
    closeFlag_ = false;
}

inline void ImGui::FileBrowser::Close()
{
    ClearSelected();
    statusStr_ = std::string();
    closeFlag_ = true;
    openFlag_ = false;
}

inline bool ImGui::FileBrowser::IsOpened() const noexcept
{
    return isOpened_;
}

inline bool fileExistsNotNull1 (const std::string& name) {
    ghc::filesystem::path p{name};

    return exists(p) && ghc::filesystem::file_size(p) > 10;
}

inline void readWholeFileToString (const std::string& fileName, std::string &output) {
    std::ifstream t(fileName, std::ios::binary );

    //Allocate whole size for string
    t.seekg(0, std::ios::end);
    output.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    //Read and assign
    output.assign((std::istreambuf_iterator<char>(t)),
               std::istreambuf_iterator<char>());

    t.close();
}

inline void ImGui::FileBrowser::loadBuildsFromBuildInfo() {
    availableBuilds.clear();
    currentBuild = {};

    SetOfBuildDefs buildDefs;

    std::string buildFile = GetSelected() / ".build.info";
    if (fileExistsNotNull1(buildFile)) {
        std::string buildFileContent;
        readWholeFileToString(buildFile, buildFileContent);

        try {
            if (!buildFileContent.empty()) {
                buildDefs.merge(BuildInfoParser::parseFileContent(buildFileContent));
            }
        } catch (...) {
            std::cout << "error while trying to read and parse .build.info";
        }
    }

    /*
    std::string productDBFile = GetSelected() / ".product.db";
    if (fileExistsNotNull1(productDBFile)) {
        std::string productFileContent;
        readWholeFileToString(productDBFile, productFileContent);

        try {
            if (!productFileContent.empty()) {
                buildDefs.merge(ProductDbParser::parseFileContent(productFileContent));
            }
        } catch (...) {
            std::cout << "error while trying to read and parse .product.db";
        }
    }
    */


    availableBuilds = std::vector<BuildDefinition>(buildDefs.begin(), buildDefs.end());

    std::sort(availableBuilds.begin(), availableBuilds.end(), [](const BuildDefinition &a, const BuildDefinition &b) {
        int compareRes = a.productName.compare(b.productName);
        if (compareRes != 0) return compareRes < 0;

        compareRes = a.buildVersion.compare(b.buildVersion);
        if (compareRes != 0) return compareRes < 0;

        compareRes = a.buildConfig.compare(b.buildConfig);
        if (compareRes != 0) return compareRes < 0;

        return false;
    });

    if (availableBuilds.size()>0) {
        currentBuild = availableBuilds[0];
    }
}

inline void ImGui::FileBrowser::Display()
{
    PushID(this);
    ScopeGuard exitThis([this] { openFlag_ = false; closeFlag_ = false; PopID(); });

    if(openFlag_)
        OpenPopup(openLabel_.c_str());
    isOpened_ = false;

    // open the popup window

    if(openFlag_ && (flags_ & ImGuiFileBrowserFlags_NoModal))
        SetNextWindowSize(ImVec2(700, 450));
    else
        SetNextWindowSize(ImVec2(700, 450), ImGuiCond_FirstUseEver);
    if(flags_ & ImGuiFileBrowserFlags_NoModal)
    {
        if(!BeginPopup(openLabel_.c_str()))
            return;
    }
    else if(!BeginPopupModal(openLabel_.c_str(), nullptr,
        flags_ & ImGuiFileBrowserFlags_NoTitleBar ? ImGuiWindowFlags_NoTitleBar : 0))
    {
        return;
    }
    isOpened_ = true;
    ScopeGuard endPopup([] { EndPopup(); });

    // display elements in pwd

#ifdef _WIN32
    char currentDrive = static_cast<char>(pwd_.c_str()[0]);
    char driveStr[] = { currentDrive, ':', '\0' };

    PushItemWidth(4 * GetFontSize());
    if(BeginCombo("##select_drive", driveStr))
    {
        ScopeGuard guard([&] { ImGui::EndCombo(); });
        for(int i = 0; i < 26; ++i)
        {
            if(!(drives_ & (1 << i)))
                continue;
            char driveCh = static_cast<char>('A' + i);
            char selectableStr[] = { driveCh, ':', '\0' };
            bool selected = currentDrive == driveCh;
            if(Selectable(selectableStr, selected) && !selected)
            {
                char newPwd[] = { driveCh, ':', '\\', '\0' };
                SetPwd(newPwd);
            }
        }
    }
    PopItemWidth();

    SameLine();
#endif

    int secIdx = 0, newPwdLastSecIdx = -1;
    for(auto &sec : pwd_)
    {
#ifdef _WIN32
        if(secIdx == 1)
        {
            ++secIdx;
            continue;
        }
#endif
        PushID(secIdx);
        if(secIdx > 0)
            SameLine();
        if(SmallButton(sec.u8string().c_str()))
            newPwdLastSecIdx = secIdx;
        PopID();
        ++secIdx;
    }

    if(newPwdLastSecIdx >= 0)
    {
        int i = 0;
        ghc::filesystem::path newPwd;
        for(auto &sec : pwd_)
        {
            if(i++ > newPwdLastSecIdx)
                break;
            newPwd /= sec;
        }
#ifdef _WIN32
        if(newPwdLastSecIdx == 0)
            newPwd /= "\\";
#endif
        SetPwd(newPwd);
    }

    SameLine();

    if(SmallButton("*"))
        SetPwd(pwd_);

    if(newDirNameBuf_)
    {
        SameLine();
        if(SmallButton("+"))
        {
            OpenPopup(openNewDirLabel_.c_str());
            (*newDirNameBuf_)[0] = '\0';
        }

        if(BeginPopup(openNewDirLabel_.c_str()))
        {
            ScopeGuard endNewDirPopup([] { EndPopup(); });

            InputText("name", newDirNameBuf_->data(), newDirNameBuf_->size()); SameLine();
            if(Button("ok") && (*newDirNameBuf_)[0] != '\0')
            {
                ScopeGuard closeNewDirPopup([] { CloseCurrentPopup(); });
                if(create_directory(pwd_ / newDirNameBuf_->data()))
                    SetPwd(pwd_);
                else
                    statusStr_ = "failed to create " + std::string(newDirNameBuf_->data());
            }
        }
    }

    // browse files in a child window

    float reserveHeight = GetFrameHeightWithSpacing();
    ghc::filesystem::path newPwd; bool setNewPwd = false;
    if(!(flags_ & ImGuiFileBrowserFlags_SelectDirectory) && (flags_ & ImGuiFileBrowserFlags_EnterNewFilename))
        reserveHeight += GetFrameHeightWithSpacing();
    {
        BeginChild("ch", ImVec2(0, -reserveHeight), true,
            (flags_ & ImGuiFileBrowserFlags_NoModal) ? ImGuiWindowFlags_AlwaysHorizontalScrollbar : 0);
        ScopeGuard endChild([] { EndChild(); });

        for(auto &rsc : fileRecords_)
        {
            if (!rsc.isDir && typeFilters_.size() > 0 &&
                typeFilterIndex_ < typeFilters_.size() &&
                !(rsc.extension == typeFilters_[typeFilterIndex_]))
                continue;

            if(!rsc.name.empty() && rsc.name[0] == '$')
                continue;

            const bool selected = selectedFilename_ == rsc.name;
            if(Selectable(rsc.showName.c_str(), selected, ImGuiSelectableFlags_DontClosePopups))
            {
                if(selected)
                {
                    selectedFilename_ = std::string();
                    (*inputNameBuf_)[0] = '\0';
                }
                else if(rsc.name != "..")
                {
                    if((rsc.isDir && (flags_ & ImGuiFileBrowserFlags_SelectDirectory)) ||
                       (!rsc.isDir && !(flags_ & ImGuiFileBrowserFlags_SelectDirectory)))
                    {
                        selectedFilename_ = rsc.name;
                        if(!(flags_ & ImGuiFileBrowserFlags_SelectDirectory))
                            std::strncpy(inputNameBuf_->data(), selectedFilename_.c_str(),
                                         std::min<int>(std::strlen(selectedFilename_.c_str()), INPUT_NAME_BUF_SIZE));
                    }
                }
            }

            if(IsItemClicked(0) && IsMouseDoubleClicked(0) && rsc.isDir)
            {
                setNewPwd = true;
                newPwd = (rsc.name != "..") ? (pwd_ / rsc.name) : pwd_.parent_path();
            }
        }
    }

    if(setNewPwd)
        SetPwd(newPwd);

    if(!(flags_ & ImGuiFileBrowserFlags_SelectDirectory) && (flags_ & ImGuiFileBrowserFlags_EnterNewFilename))
    {
        PushID(this);
        ScopeGuard popTextID([] { PopID(); });

        PushItemWidth(-1);
        if(InputText("", inputNameBuf_->data(), inputNameBuf_->size()))
            selectedFilename_ = inputNameBuf_->data();
        PopItemWidth();
    }

    if(!(flags_ & ImGuiFileBrowserFlags_SelectDirectory))
    {
        if(Button(" ok ") && !selectedFilename_.empty())
        {
            ok_ = true;
            CloseCurrentPopup();
        }
    }
    else
    {
        if(selectedFilename_.empty())
        {
            if(Button(" ok "))
            {
                ok_ = true;
                CloseCurrentPopup();
            }
        }
        else if(Button("open"))
            SetPwd(pwd_ / selectedFilename_);
    }

    SameLine();

    if(Button("cancel") || closeFlag_ ||
        ((flags_ & ImGuiFileBrowserFlags_CloseOnEsc) && IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && IsKeyPressed(ImGuiKey::ImGuiKey_Escape)))
        CloseCurrentPopup();

    if (m_cascOpenMode) {
        if (pwd_ != m_last_pwd_forBuildInfo) {
            loadBuildsFromBuildInfo();
            m_last_pwd_forBuildInfo = pwd_;
        }

        SameLine();
        ImGui::Text("Select product build:");
        SameLine();
        if (ImGui::BeginCombo("##buildSelect",
                              currentBuild.productName.c_str())) // The second parameter is the label previewed before opening the combo.
        {
            if (availableBuilds.empty()) {
                ImGui::Selectable("No builds found", true);
            }
            for (int n = 0; n < availableBuilds.size(); n++)
            {
                auto &buildDef = availableBuilds[n];

                bool is_selected = (buildDef == currentBuild);
                std::string label = buildDef.productName + " - " +buildDef.buildVersion + " - " + buildDef.buildConfig.substr(0,5);

                if (ImGui::Selectable(label.c_str(), is_selected)) {
                    currentBuild = buildDef;
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
            }
            ImGui::EndCombo();
        }
    }

    if(!statusStr_.empty() && !(flags_ & ImGuiFileBrowserFlags_NoStatusBar))
    {
        SameLine();
        Text("%s", statusStr_.c_str());
    }

    if (!typeFilters_.empty())
    {
        SameLine();
        PushItemWidth(8 * GetFontSize());
        Combo("##type_filters", &typeFilterIndex_, typeFilters_.data(), int(typeFilters_.size()));
        PopItemWidth();
    }
}

inline bool ImGui::FileBrowser::HasSelected() const noexcept
{
    return ok_;
}

inline bool ImGui::FileBrowser::SetPwd(const ghc::filesystem::path &pwd)
{
    try
    {
        SetPwdUncatched(pwd);
        return true;
    }
    catch(const std::exception &err)
    {
        statusStr_ = std::string("last error: ") + err.what();
    }
    catch(...)
    {
        statusStr_ = "last error: unknown";
    }

    SetPwdUncatched(ghc::filesystem::current_path());
    return false;
}

inline ghc::filesystem::path ImGui::FileBrowser::GetSelected() const
{
    return pwd_ / selectedFilename_;
}

inline void ImGui::FileBrowser::ClearSelected()
{
    selectedFilename_ = std::string();
    (*inputNameBuf_)[0] = '\0';
    ok_ = false;
}

inline void ImGui::FileBrowser::SetTypeFilters(const std::vector<const char*> &typeFilters)
{
    typeFilters_ = typeFilters;
    typeFilterIndex_ = 0;
}

inline void ImGui::FileBrowser::SetPwdUncatched(const ghc::filesystem::path &pwd)
{
    fileRecords_ = { FileRecord{ true, "..", "[D] .." } };

    for(auto &p : ghc::filesystem::directory_iterator(pwd))
    {
        FileRecord rcd;

        if(p.is_regular_file())
            rcd.isDir = false;
        else if(p.is_directory())
            rcd.isDir = true;
        else
            continue;

        rcd.name = p.path().filename().string();
        if(rcd.name.empty())
            continue;

        rcd.extension = p.path().filename().extension().string();

        rcd.showName = (rcd.isDir ? "[D] " : "[F] ") + p.path().filename().u8string();
        fileRecords_.push_back(rcd);
    }

    std::sort(fileRecords_.begin(), fileRecords_.end(),
        [](const FileRecord &L, const FileRecord &R)
    {
        return (L.isDir ^ R.isDir) ? L.isDir : (L.name < R.name);
    });

    pwd_ = absolute(pwd);
    selectedFilename_ = std::string();
    (*inputNameBuf_)[0] = '\0';
}

#ifdef _WIN32

#include <Windows.h>

inline std::uint32_t ImGui::FileBrowser::GetDrivesBitMask()
{
    DWORD mask = GetLogicalDrives();
    uint32_t ret = 0;
    for(int i = 0; i < 26; ++i)
    {
        if(!(mask & (1 << i)))
            continue;
        char rootName[4] = { static_cast<char>('A' + i), ':', '\\', '\0' };
        UINT type = GetDriveTypeA(rootName);
        if(type == DRIVE_REMOVABLE || type == DRIVE_FIXED)
            ret |= (1 << i);
    }
    return ret;
}

#endif
