//
// Created by Claude on 2026.
//

#ifndef AWEBWOWVIEWERCPP_CASCSTORAGEDIALOG_H
#define AWEBWOWVIEWERCPP_CASCSTORAGEDIALOG_H

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "../../../../wowViewerLib/src/engine/ApiContainer.h"
#include "../../../database/buildInfoParser/buildDefinition.h"
#include "CascStorageEntry.h"
#include "imgui.h"
#include "../../imguiLib/fileBrowser/imfilebrowser.h"

// Callback type for when a storage is selected/opened
using StorageOpenCallback = std::function<bool(const std::string& cascPath, BuildDefinition& buildDef)>;

class CascStorageDialog {
public:
    CascStorageDialog(const HApiContainer& api,
                      StorageOpenCallback openCallback);
    ~CascStorageDialog() {
        m_fileDialog.Close();
    };

    // Show/hide the dialog
    void show();
    void hide();
    bool isVisible() const { return m_showDialog; }

    // Draw the dialog - returns false if dialog was closed
    bool draw();

    // Load/save storage list from/to JSON file
    void loadStorages();
    void saveStorages();

    // Add a storage entry to the list
    void addStorage(const CascStorageEntry& entry);

    // Check if there are any saved storages
    bool hasSavedStorages() const { return !m_storages.empty(); }

    // Called when fileDialog has selected a folder (from FrontendUI)
    void onFolderSelected(const std::string& path, const BuildDefinition& buildDef);

private:
    HApiContainer m_api;
    ImGui::FileBrowser m_fileDialog = ImGui::FileBrowser(ImGuiFileBrowserFlags_SelectDirectory, true);
    StorageOpenCallback m_openCallback;

    bool m_showDialog = false;
    bool m_showNewStorageDialog = false;
    bool m_waitingForFolderSelection = false;

    std::vector<CascStorageEntry> m_storages;
    int m_selectedStorageIndex = -1;
    std::string m_lastBrowsedFolder;

    // Editing state: when true, the "new storage" dialog is being used to edit
    // m_storages[m_editingIndex] in place rather than appending a new entry.
    bool m_editMode = false;
    int m_editingIndex = -1;

    // New storage dialog state
    CascStorageEntry m_newStorageEntry;
    char m_buildVersionBuf[256] = {0};
    char m_buildConfigHashBuf[256] = {0};
    char m_cdnConfigHashBuf[256] = {0};
    char m_productNameBuf[256] = {0};
    char m_cdnPathBuf[256] = {0};
    char m_cdnServersOverrideBuf[256] = {0};
    char m_regionBuf[64] = {0};
    char m_installedLanguageBuf[64] = {0};
    char m_armadilloKeyBuf[256] = {0};
    char m_cascLocalFolderBuf[512] = {0};
    char m_localTactPathBuf[512] = {0};

    // Draw methods
    void drawMainDialog();
    void drawNewStorageDialog();
    void drawStorageList();

    // Reset new storage dialog fields
    void resetNewStorageFields();
    void populateNewStorageFields(const CascStorageEntry& entry);
    void copyBuffersToNewStorageEntry();

    // Reorder entries in the list
    void moveStorageUp(int index);
    void moveStorageDown(int index);

    // Get JSON file path
    std::string getStoragesFilePath() const;

    // Try to open a storage
    bool tryOpenStorage(const CascStorageEntry& entry);
};

#endif //AWEBWOWVIEWERCPP_CASCSTORAGEDIALOG_H
