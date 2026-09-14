//
// Created by Claude on 2026.
//

#include "CascStorageDialog.h"
#include "../../imguiLib/imgui.h"
#include "../../../../3rdparty/nlohmann/json.hpp"
#include "../../../../3rdparty/filesystem_impl/include/ghc/filesystem.hpp"
#include <fstream>
#include <iostream>
#include <utility>

using json = nlohmann::json;

CascStorageDialog::CascStorageDialog(const HApiContainer& api,
                                     StorageOpenCallback openCallback)
    : m_api(api), m_openCallback(openCallback)
{
    loadStorages();
    resetNewStorageFields();
}

void CascStorageDialog::show() {
    m_showDialog = true;
}

void CascStorageDialog::hide() {
    m_showDialog = false;
    m_showNewStorageDialog = false;
}

bool CascStorageDialog::draw() {
    if (!m_showDialog) return true;

    m_fileDialog.Display();
    if (m_fileDialog.HasSelected()) {
        std::cout << "Selected folder " << m_fileDialog.GetSelected().string() << std::endl;
        std::string cascPath = m_fileDialog.GetSelected().string();
        BuildDefinition buildDef = m_fileDialog.getProductBuild();

        // Save the last browsed folder
        m_lastBrowsedFolder = cascPath;
        saveStorages();

        this->onFolderSelected(cascPath, buildDef);

        m_fileDialog.ClearSelected();
    }

    drawMainDialog();

    if (m_showNewStorageDialog) {
        drawNewStorageDialog();
    }

    return m_showDialog;
}

void CascStorageDialog::drawMainDialog() {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("CASC Storage Manager", &m_showDialog)) {
        // Header with instructions
        ImGui::TextWrapped("Select a previously opened storage or add a new one.");
        ImGui::Separator();

        // Storage list
        drawStorageList();

        ImGui::Separator();

        // Buttons at the bottom
        float buttonWidth = 150.0f;

        // Open button (disabled if nothing selected)
        bool canOpen = m_selectedStorageIndex >= 0 && m_selectedStorageIndex < (int)m_storages.size();
        if (!canOpen) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Open Selected", ImVec2(buttonWidth, 0))) {
            if (canOpen) {
                if (tryOpenStorage(m_storages[m_selectedStorageIndex])) {
                    m_showDialog = false;
                }
            }
        }
        if (!canOpen) {
            ImGui::EndDisabled();
        }

        ImGui::SameLine();

        // Add new storage button
        if (ImGui::Button("Add New Storage...", ImVec2(buttonWidth, 0))) {
            resetNewStorageFields();
            m_editMode = false;
            m_editingIndex = -1;
            m_showNewStorageDialog = true;
        }

        ImGui::SameLine();

        // Remove button (disabled if nothing selected)
        if (!canOpen) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Remove Selected", ImVec2(buttonWidth, 0))) {
            if (canOpen) {
                m_storages.erase(m_storages.begin() + m_selectedStorageIndex);
                m_selectedStorageIndex = -1;
                saveStorages();
            }
        }
        if (!canOpen) {
            ImGui::EndDisabled();
        }

        // Second row: edit and reorder
        // Edit button (disabled if nothing selected)
        if (!canOpen) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Edit Selected", ImVec2(buttonWidth, 0))) {
            if (canOpen) {
                m_editMode = true;
                m_editingIndex = m_selectedStorageIndex;
                populateNewStorageFields(m_storages[m_selectedStorageIndex]);
                m_showNewStorageDialog = true;
            }
        }
        if (!canOpen) {
            ImGui::EndDisabled();
        }

        ImGui::SameLine();

        bool canMoveUp = canOpen && m_selectedStorageIndex > 0;
        if (!canMoveUp) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Move Up", ImVec2(buttonWidth * 0.5f, 0))) {
            if (canMoveUp) {
                moveStorageUp(m_selectedStorageIndex);
            }
        }
        if (!canMoveUp) {
            ImGui::EndDisabled();
        }

        ImGui::SameLine();

        bool canMoveDown = canOpen && m_selectedStorageIndex < (int)m_storages.size() - 1;
        if (!canMoveDown) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Move Down", ImVec2(buttonWidth * 0.5f, 0))) {
            if (canMoveDown) {
                moveStorageDown(m_selectedStorageIndex);
            }
        }
        if (!canMoveDown) {
            ImGui::EndDisabled();
        }
    }
    ImGui::End();
}

void CascStorageDialog::drawStorageList() {
    ImGui::BeginChild("StorageList", ImVec2(0, -50), true);

    if (m_storages.empty()) {
        ImGui::TextDisabled("No saved storages. Click 'Add New Storage...' to add one.");
    } else {
        for (int i = 0; i < (int)m_storages.size(); i++) {
            const auto& entry = m_storages[i];
            std::string displayName = entry.getDisplayName();

            bool isSelected = (m_selectedStorageIndex == i);
            if (ImGui::Selectable(displayName.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
                m_selectedStorageIndex = i;

                // Double-click to open
                if (ImGui::IsMouseDoubleClicked(0)) {
                    if (tryOpenStorage(entry)) {
                        m_showDialog = false;
                    }
                }
            }

            // Tooltip with details
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Product: %s", entry.buildDef.productName.c_str());
                ImGui::Text("Version: %s", entry.buildDef.buildVersion.c_str());
                ImGui::Text("Build Config: %s", entry.buildDef.buildConfigHash.c_str());
                ImGui::Text("CDN Config: %s", entry.buildDef.cdnConfigHash.c_str());
                if (!entry.buildDef.cdnServersOverride.empty()) {
                    ImGui::Text("CDN Servers Override: %s", entry.buildDef.cdnServersOverride.c_str());
                }
                ImGui::Text("Path: %s", entry.cascLocalFolder.c_str());
                ImGui::Text("Local Tact: %s", entry.buildDef.useTactLocal ? "Yes" : "No");
                if (entry.buildDef.useTactLocal) {
                    ImGui::Text("Local path: %s", entry.buildDef.localTactPath.c_str());
                }
                ImGui::Text("Region: %s", entry.buildDef.region.c_str());
                ImGui::Text("Online Download: %s", entry.buildDef.allowOnlineDownload ? "Yes" : "No");
                ImGui::Text("Is classic: %s", entry.buildDef.isClassic ? "Yes" : "No");
                ImGui::EndTooltip();
            }
        }
    }

    ImGui::EndChild();
}

void CascStorageDialog::drawNewStorageDialog() {
    ImGui::SetNextWindowSize(ImVec2(500, 450), ImGuiCond_FirstUseEver);

    const char* windowTitle = m_editMode ? "Edit Storage" : "Add New Storage";
    if (ImGui::Begin(windowTitle, &m_showNewStorageDialog)) {
        // Choose folder button
        if (ImGui::Button("Choose Local Folder...", ImVec2(-1, 0))) {
            m_waitingForFolderSelection = true;
            m_fileDialog.Open();
        }

        ImGui::Separator();
        ImGui::Text("Storage Settings:");
        ImGui::Spacing();

        // Local folder path
        ImGui::InputText("Local Folder", m_cascLocalFolderBuf, sizeof(m_cascLocalFolderBuf));

        ImGui::Separator();
        ImGui::Text("Build Definition:");
        ImGui::Spacing();

        // Build definition fields
        if (ImGui::InputText("Product Name", m_productNameBuf, sizeof(m_productNameBuf))) {
            if (contains(std::string(m_productNameBuf), "classic")) {
                m_newStorageEntry.buildDef.isClassic = true;
            }
        };
        ImGui::InputText("Build Version", m_buildVersionBuf, sizeof(m_buildVersionBuf));
        ImGui::InputText("Build Config Hash", m_buildConfigHashBuf, sizeof(m_buildConfigHashBuf));
        ImGui::InputText("CDN Config Hash", m_cdnConfigHashBuf, sizeof(m_cdnConfigHashBuf));
        ImGui::InputText("CDN Path", m_cdnPathBuf, sizeof(m_cdnPathBuf));
        ImGui::InputText("CDN Servers Override", m_cdnServersOverrideBuf, sizeof(m_cdnServersOverrideBuf));
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Optional. Space-separated list of CDN server hostnames "
                              "(e.g. \"level3.blizzard.com\"). Leave empty to auto-discover "
                              "CDN servers from the battle.net patch service.");
        }
        ImGui::InputText("Region", m_regionBuf, sizeof(m_regionBuf));
        ImGui::InputText("Installed Language", m_installedLanguageBuf, sizeof(m_installedLanguageBuf));
        ImGui::InputText("Armadillo Key", m_armadilloKeyBuf, sizeof(m_armadilloKeyBuf));

        ImGui::Separator();
        ImGui::Text("Options:");
        ImGui::Spacing();

        ImGui::Checkbox("Allow Online Download", &m_newStorageEntry.buildDef.allowOnlineDownload);
        ImGui::Checkbox("Prefer Low Violence", &m_newStorageEntry.buildDef.preferLowViolence);
        ImGui::Checkbox("Prefer Hi-Res Textures", &m_newStorageEntry.buildDef.preferHiResTextures);

        ImGui::Checkbox("Is classic", &m_newStorageEntry.buildDef.isClassic);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("This controls which set of DB2 tables gets loaded when DB is updated.");
        }

        ImGui::Checkbox("Use Local TACT Mirror", &m_newStorageEntry.buildDef.useTactLocal);
        if (!m_newStorageEntry.buildDef.useTactLocal) ImGui::BeginDisabled();
        ImGui::InputText("Local TACT Path", m_localTactPathBuf, sizeof(m_localTactPathBuf));
        if (!m_newStorageEntry.buildDef.useTactLocal) ImGui::EndDisabled();

        ImGui::Separator();

        // Buttons
        float buttonWidth = 120.0f;

        bool canEditInPlace = m_editMode && m_editingIndex >= 0 && m_editingIndex < (int)m_storages.size();

        if (m_editMode) {
            if (ImGui::Button("Save & Open", ImVec2(buttonWidth, 0))) {
                copyBuffersToNewStorageEntry();

                if (canEditInPlace && tryOpenStorage(m_newStorageEntry)) {
                    m_storages[m_editingIndex] = m_newStorageEntry;
                    saveStorages();
                    m_showNewStorageDialog = false;
                    m_showDialog = false;
                    m_editMode = false;
                    m_editingIndex = -1;
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Save Only", ImVec2(buttonWidth, 0))) {
                copyBuffersToNewStorageEntry();

                if (canEditInPlace) {
                    m_storages[m_editingIndex] = m_newStorageEntry;
                    saveStorages();
                }
                m_showNewStorageDialog = false;
                m_editMode = false;
                m_editingIndex = -1;
            }
        } else {
            if (ImGui::Button("Add & Open", ImVec2(buttonWidth, 0))) {
                copyBuffersToNewStorageEntry();

                if (tryOpenStorage(m_newStorageEntry)) {
                    addStorage(m_newStorageEntry);
                    m_showNewStorageDialog = false;
                    m_showDialog = false;
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Add Only", ImVec2(buttonWidth, 0))) {
                copyBuffersToNewStorageEntry();

                addStorage(m_newStorageEntry);
                m_showNewStorageDialog = false;
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
            m_showNewStorageDialog = false;
            m_waitingForFolderSelection = false;
            m_editMode = false;
            m_editingIndex = -1;
        }
    }
    ImGui::End();
}

void CascStorageDialog::onFolderSelected(const std::string& path, const BuildDefinition& buildDef) {
    if (!m_waitingForFolderSelection) return;

    m_waitingForFolderSelection = false;

    // Populate the new storage fields with data from fileDialog
    m_newStorageEntry.cascLocalFolder = path;
    m_newStorageEntry.buildDef = buildDef;

    populateNewStorageFields(m_newStorageEntry);
}

void CascStorageDialog::resetNewStorageFields() {
    m_newStorageEntry = CascStorageEntry();
    m_newStorageEntry.buildDef.region = "us";
    m_newStorageEntry.buildDef.preferHiResTextures = true;

    memset(m_buildVersionBuf, 0, sizeof(m_buildVersionBuf));
    memset(m_buildConfigHashBuf, 0, sizeof(m_buildConfigHashBuf));
    memset(m_cdnConfigHashBuf, 0, sizeof(m_cdnConfigHashBuf));
    memset(m_productNameBuf, 0, sizeof(m_productNameBuf));
    memset(m_cdnPathBuf, 0, sizeof(m_cdnPathBuf));
    memset(m_cdnServersOverrideBuf, 0, sizeof(m_cdnServersOverrideBuf));
    memset(m_regionBuf, 0, sizeof(m_regionBuf));
    memset(m_installedLanguageBuf, 0, sizeof(m_installedLanguageBuf));
    memset(m_armadilloKeyBuf, 0, sizeof(m_armadilloKeyBuf));
    memset(m_cascLocalFolderBuf, 0, sizeof(m_cascLocalFolderBuf));
    memset(m_localTactPathBuf, 0, sizeof(m_localTactPathBuf));

    strncpy(m_regionBuf, "us", sizeof(m_regionBuf) - 1);
}

void CascStorageDialog::populateNewStorageFields(const CascStorageEntry& entry) {
    strncpy(m_cascLocalFolderBuf, entry.cascLocalFolder.c_str(), sizeof(m_cascLocalFolderBuf) - 1);
    strncpy(m_productNameBuf, entry.buildDef.productName.c_str(), sizeof(m_productNameBuf) - 1);
    strncpy(m_buildVersionBuf, entry.buildDef.buildVersion.c_str(), sizeof(m_buildVersionBuf) - 1);
    strncpy(m_buildConfigHashBuf, entry.buildDef.buildConfigHash.c_str(), sizeof(m_buildConfigHashBuf) - 1);
    strncpy(m_cdnConfigHashBuf, entry.buildDef.cdnConfigHash.c_str(), sizeof(m_cdnConfigHashBuf) - 1);
    strncpy(m_cdnPathBuf, entry.buildDef.cdnPath.c_str(), sizeof(m_cdnPathBuf) - 1);
    strncpy(m_cdnServersOverrideBuf, entry.buildDef.cdnServersOverride.c_str(), sizeof(m_cdnServersOverrideBuf) - 1);
    strncpy(m_regionBuf, entry.buildDef.region.c_str(), sizeof(m_regionBuf) - 1);
    strncpy(m_installedLanguageBuf, entry.buildDef.installedLanguage.c_str(), sizeof(m_installedLanguageBuf) - 1);
    strncpy(m_armadilloKeyBuf, entry.buildDef.armadilloKey.c_str(), sizeof(m_armadilloKeyBuf) - 1);
    strncpy(m_localTactPathBuf, entry.buildDef.localTactPath.c_str(), sizeof(m_localTactPathBuf) - 1);

    m_newStorageEntry.buildDef.allowOnlineDownload = entry.buildDef.allowOnlineDownload;
    m_newStorageEntry.buildDef.preferLowViolence = entry.buildDef.preferLowViolence;
    m_newStorageEntry.buildDef.preferHiResTextures = entry.buildDef.preferHiResTextures;
    m_newStorageEntry.buildDef.useTactLocal = entry.buildDef.useTactLocal;
    m_newStorageEntry.buildDef.isClassic = entry.buildDef.isClassic;
}

void CascStorageDialog::copyBuffersToNewStorageEntry() {
    m_newStorageEntry.cascLocalFolder = m_cascLocalFolderBuf;

    auto &buildDef = m_newStorageEntry.buildDef;
    buildDef.productName = m_productNameBuf;
    buildDef.buildVersion = m_buildVersionBuf;
    buildDef.buildConfigHash = m_buildConfigHashBuf;
    buildDef.cdnConfigHash = m_cdnConfigHashBuf;
    buildDef.cdnPath = m_cdnPathBuf;
    buildDef.cdnServersOverride = m_cdnServersOverrideBuf;
    buildDef.region = m_regionBuf;
    buildDef.installedLanguage = m_installedLanguageBuf;
    buildDef.armadilloKey = m_armadilloKeyBuf;
    buildDef.localTactPath = m_localTactPathBuf;
}

std::string CascStorageDialog::getStoragesFilePath() const {
    return "./cascStorages.json";
}

void CascStorageDialog::loadStorages() {
    m_storages.clear();

    std::string filePath = getStoragesFilePath();
    if (!ghc::filesystem::exists(filePath)) {
        return;
    }

    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open cascStorages.json for reading" << std::endl;
            return;
        }

        json j;
        file >> j;

        if (j.contains("storages") && j["storages"].is_array()) {
            for (const auto& item : j["storages"]) {
                CascStorageEntry entry;
                from_json(item, entry);
                m_storages.push_back(entry);
            }
        }

        // Load last browsed folder and set it if it still exists
        if (j.contains("lastBrowsedFolder") && j["lastBrowsedFolder"].is_string()) {
            m_lastBrowsedFolder = j["lastBrowsedFolder"].get<std::string>();
            if (!m_lastBrowsedFolder.empty() && ghc::filesystem::exists(m_lastBrowsedFolder)) {
                m_fileDialog.SetPwd(m_lastBrowsedFolder);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading cascStorages.json: " << e.what() << std::endl;
    }
}

void CascStorageDialog::saveStorages() {
    try {
        json j;
        j["storages"] = json::array();

        for (const auto& entry : m_storages) {
            json entryJson;
            to_json(entryJson, entry);
            j["storages"].push_back(entryJson);
        }

        // Save last browsed folder
        j["lastBrowsedFolder"] = m_lastBrowsedFolder;

        std::ofstream file(getStoragesFilePath());
        if (!file.is_open()) {
            std::cerr << "Failed to open cascStorages.json for writing" << std::endl;
            return;
        }

        file << j.dump(2);  // Pretty print with 2-space indent
    } catch (const std::exception& e) {
        std::cerr << "Error saving cascStorages.json: " << e.what() << std::endl;
    }
}

void CascStorageDialog::addStorage(const CascStorageEntry& entry) {
    //Add new entry at the beginning (most recent first)
    m_storages.insert(m_storages.begin(), entry);
    saveStorages();
}

void CascStorageDialog::moveStorageUp(int index) {
    if (index <= 0 || index >= (int)m_storages.size()) return;

    std::swap(m_storages[index], m_storages[index - 1]);
    m_selectedStorageIndex = index - 1;
    saveStorages();
}

void CascStorageDialog::moveStorageDown(int index) {
    if (index < 0 || index >= (int)m_storages.size() - 1) return;

    std::swap(m_storages[index], m_storages[index + 1]);
    m_selectedStorageIndex = index + 1;
    saveStorages();
}

bool CascStorageDialog::tryOpenStorage(const CascStorageEntry& entry) {
    if (m_openCallback) {
        BuildDefinition buildDef = entry.buildDef;
        std::string path = entry.cascLocalFolder;
        return m_openCallback(path, buildDef);
    }
    return false;
}
