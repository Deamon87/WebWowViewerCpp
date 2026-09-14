//
// Created by Deamon on 7/26/2026.
//

#include "CustomObjectsWindow.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>

#include "../sceneWindow/SceneWindow.h"
#include "../../../../wowViewerLib/src/engine/algorithms/mathHelper.h"
#include "../../../../wowViewerLib/src/engine/camera/CameraInterface.h"
#include "../../../../wowViewerLib/src/renderer/mapScene/MapScenePlan.h"
#include "../../../../3rdparty/nlohmann/json.hpp"
#include "../../../../3rdparty/filesystem_impl/include/ghc/filesystem.hpp"
#include "ImGuizmo/ImGuizmo.h"
#include "imgui_notify.h"

namespace {
    constexpr const char *kDocumentsDir = "customObjects";

    void mat4ToFloat16(const mathfu::mat4 &m, float *out16) {
        for (int c = 0; c < 4; c++) {
            for (int r = 0; r < 4; r++) {
                out16[c * 4 + r] = m(r, c);
            }
        }
    }

    // Keeps a document name usable as a plain file name (no path traversal, no blanks).
    std::string sanitizeDocumentName(const std::string &name) {
        std::string result;
        for (char ch : name) {
            if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '-' || ch == '_' || ch == ' ') {
                result += ch;
            }
        }
        // trim trailing/leading spaces
        auto begin = result.find_first_not_of(' ');
        auto end = result.find_last_not_of(' ');
        return begin == std::string::npos ? "" : result.substr(begin, end - begin + 1);
    }

    std::string documentPath(const std::string &docName) {
        return std::string(kDocumentsDir) + "/" + docName + ".json";
    }
}

CustomObjectsWindow::CustomObjectsWindow(const HApiContainer &api,
                                         std::function<std::shared_ptr<SceneWindow>()> getMapScene) :
    m_api(api), m_getMapScene(std::move(getMapScene)) {
}

std::shared_ptr<SceneWindow> CustomObjectsWindow::getScene() {
    return m_getMapScene ? m_getMapScene() : nullptr;
}

std::shared_ptr<WorldObjectManager> CustomObjectsWindow::getWorldObjectManager() {
    auto scene = getScene();
    return scene ? scene->getWorldObjectManager() : nullptr;
}

bool CustomObjectsWindow::draw() {
    if (!m_show) {
        ImGuizmo::Enable(false);
        return false;
    }

    auto wom = getWorldObjectManager();

    if (ImGui::Begin("Custom placed objects", &m_show)) {
        if (!wom) {
            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.2f, 1.0f), "No map is currently open.");
            ImGui::TextWrapped("Open a map scene to place custom M2 objects.");
        } else {
            if (!m_api->getConfig()->renderGameObjects) {
                ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.2f, 1.0f),
                                   "Note: 'Render Gameobjects' is off in Settings - custom objects will not be drawn.");
            }

            drawAddSection(wom);
            ImGui::Separator();
            drawObjectList(wom);
            ImGui::Separator();
            drawSelectedEditor(wom);
            ImGui::Separator();
            drawDocumentsSection(wom);
        }
    }
    ImGui::End();

    if (wom) {
        syncSelectionFromScenePick(wom);
        drawGizmo(wom);
    }

    return m_show;
}

void CustomObjectsWindow::drawAddSection(const std::shared_ptr<WorldObjectManager> &wom) {
    if (!ImGui::CollapsingHeader("Add custom object", ImGuiTreeNodeFlags_DefaultOpen)) return;

    ImGui::InputText("Name##add", m_addName, sizeof(m_addName));

    if (ImGui::RadioButton("FileDataID##modelSource", !m_addByFileName)) m_addByFileName = false;
    ImGui::SameLine();
    if (ImGui::RadioButton("Model path##modelSource", m_addByFileName)) m_addByFileName = true;

    if (m_addByFileName) {
        ImGui::InputText("Model##add", m_addModelName, sizeof(m_addModelName));
    } else {
        if (ImGui::InputInt("FileDataID##add", &m_addFileDataId)) {
            if (m_addFileDataId < 0) m_addFileDataId = 0;
        }
    }

    ImGui::DragFloat3("Position##add", m_addPosition, 0.1f);
    ImGui::SameLine();
    if (ImGui::Button("From camera##addpos")) {
        auto scene = getScene();
        auto camera = scene ? scene->getCamera() : nullptr;
        if (camera) {
            auto camMatrices = camera->getCameraMatrices(0, 0, 0, 0);
            m_addPosition[0] = camMatrices->cameraPos.x;
            m_addPosition[1] = camMatrices->cameraPos.y;
            m_addPosition[2] = camMatrices->cameraPos.z;
        }
    }
    ImGui::DragFloat3("Rotation##add", m_addRotation, 0.5f);
    ImGui::DragFloat3("Scale##add", m_addScale, 0.01f, 0.001f, 1000.0f);

    if (ImGui::Button("Add object")) {
        bool inputValid = m_addByFileName ? (std::strlen(m_addModelName) > 0) : (m_addFileDataId > 0);
        if (!inputValid) {
            ImGui::InsertNotification({ImGuiToastType_Error, 3000, "Custom object needs a model path or a FileDataID"});
        } else {
            int newId = wom->addCustomM2(m_addName,
                                         m_addByFileName ? m_addModelName : "",
                                         m_addByFileName ? 0 : m_addFileDataId,
                                         {m_addPosition[0], m_addPosition[1], m_addPosition[2]},
                                         {m_addRotation[0], m_addRotation[1], m_addRotation[2]},
                                         {m_addScale[0], m_addScale[1], m_addScale[2]});
            if (newId >= 0) {
                m_selectedId = newId;
                m_addName[0] = '\0';
            }
        }
    }
}

void CustomObjectsWindow::drawObjectList(const std::shared_ptr<WorldObjectManager> &wom) {
    auto &objects = wom->getCustomM2s();

    ImGui::Text("Placed objects (%d)", static_cast<int>(objects.size()));
    if (objects.empty()) {
        ImGui::TextDisabled("No custom objects placed yet.");
        return;
    }

    if (ImGui::BeginChild("CustomObjectsList", ImVec2(0, 150), true)) {
        for (auto &[id, object] : objects) {
            ImGui::PushID(id);

            std::string model = object.modelFileName.empty()
                ? ("fdid:" + std::to_string(object.fileDataId))
                : object.modelFileName;

            if (ImGui::Selectable((object.name + "##select").c_str(), m_selectedId == id)) {
                m_selectedId = id;
            }
            ImGui::SameLine();
            ImGui::TextDisabled("%s", model.c_str());

            ImGui::PopID();
        }
    }
    ImGui::EndChild();
}

void CustomObjectsWindow::drawSelectedEditor(const std::shared_ptr<WorldObjectManager> &wom) {
    auto object = wom->getCustomM2(m_selectedId);
    if (object == nullptr) {
        m_selectedId = -1;
        ImGui::TextDisabled("No object selected. Select one in the list or right-click it in the scene.");
        return;
    }

    ImGui::Text("Selected: %s (id %d)", object->name.c_str(), object->id);

    float position[3] = {object->position.x, object->position.y, object->position.z};
    float rotation[3] = {object->rotationEulerDeg.x, object->rotationEulerDeg.y, object->rotationEulerDeg.z};
    float scale[3] = {object->scale.x, object->scale.y, object->scale.z};

    bool changed = false;
    changed |= ImGui::DragFloat3("Position##edit", position, 0.1f);
    changed |= ImGui::DragFloat3("Rotation##edit", rotation, 0.5f);
    changed |= ImGui::DragFloat3("Scale##edit", scale, 0.01f, 0.001f, 1000.0f);

    if (changed) {
        wom->setCustomM2Transform(m_selectedId,
                                  {position[0], position[1], position[2]},
                                  {rotation[0], rotation[1], rotation[2]},
                                  {scale[0], scale[1], scale[2]});
    }

    if (ImGui::Button("Remove object")) {
        wom->removeCustomM2(m_selectedId);
        m_selectedId = -1;
    }

    ImGui::Separator();

    ImGui::Checkbox("Show gizmo", &m_gizmoEnabled);
    if (m_gizmoEnabled) {
        ImGui::RadioButton("Translate##gizmo", &m_gizmoOperation, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Rotate##gizmo", &m_gizmoOperation, 1);
        ImGui::SameLine();
        ImGui::RadioButton("Scale##gizmo", &m_gizmoOperation, 2);
        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();
        if (ImGui::RadioButton("World##gizmo", m_gizmoWorldMode)) m_gizmoWorldMode = true;
        ImGui::SameLine();
        if (ImGui::RadioButton("Local##gizmo", !m_gizmoWorldMode)) m_gizmoWorldMode = false;

        ImGui::Checkbox("Snap##gizmo", &m_gizmoSnap);
        if (m_gizmoSnap) {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(70);
            ImGui::DragFloat("##snapT", &m_gizmoSnapValues[0], 0.1f, 0.01f, 1000.0f, "T: %.2f");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(70);
            ImGui::DragFloat("##snapR", &m_gizmoSnapValues[1], 0.5f, 0.1f, 180.0f, "R: %.1f");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(70);
            ImGui::DragFloat("##snapS", &m_gizmoSnapValues[2], 0.01f, 0.01f, 10.0f, "S: %.2f");
        }
    }
}

void CustomObjectsWindow::drawGizmo(const std::shared_ptr<WorldObjectManager> &wom) {
    auto object = m_gizmoEnabled ? wom->getCustomM2(m_selectedId) : nullptr;

    auto scene = object ? getScene() : nullptr;
    auto camera = scene ? scene->getCamera() : nullptr;

    // Keep ImGuizmo's global state in sync with whether the gizmo is actually shown:
    // Enable(false) also clears any in-progress drag (which getStopMouse consults).
    ImGuizmo::Enable(camera != nullptr);
    if (!camera) return;

    ImGuiIO &io = ImGui::GetIO();
    float aspect = io.DisplaySize.y > 0.0f ? io.DisplaySize.x / io.DisplaySize.y : 1.0f;
    auto camMatrices = camera->getCameraMatrices(toRadian(m_api->getConfig()->fov), aspect,
                                                 1.0f, m_api->getConfig()->farPlaneForCulling);

    float view[16], projection[16], matrix[16];
    mat4ToFloat16(camMatrices->lookAtMat, view);
    mat4ToFloat16(camMatrices->perspectiveMat, projection);

    float translation[3] = {object->position.x, object->position.y, object->position.z};
    float rotation[3] = {object->rotationEulerDeg.x, object->rotationEulerDeg.y, object->rotationEulerDeg.z};
    float scale[3] = {object->scale.x, object->scale.y, object->scale.z};
    ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, matrix);

    ImGuizmo::OPERATION operation =
        m_gizmoOperation == 1 ? ImGuizmo::ROTATE :
        m_gizmoOperation == 2 ? ImGuizmo::SCALE : ImGuizmo::TRANSLATE;
    ImGuizmo::MODE mode = m_gizmoWorldMode ? ImGuizmo::WORLD : ImGuizmo::LOCAL;

    float snap[3] = {m_gizmoSnapValues[0], m_gizmoSnapValues[1], m_gizmoSnapValues[2]};
    const float *snapPtr = m_gizmoSnap ? snap : nullptr;

    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    if (ImGuizmo::Manipulate(view, projection, operation, mode, matrix, nullptr, snapPtr)) {
        ImGuizmo::DecomposeMatrixToComponents(matrix, translation, rotation, scale);
        wom->setCustomM2Transform(m_selectedId,
                                  {translation[0], translation[1], translation[2]},
                                  {rotation[0], rotation[1], rotation[2]},
                                  {scale[0], scale[1], scale[2]});
    }
}

void CustomObjectsWindow::syncSelectionFromScenePick(const std::shared_ptr<WorldObjectManager> &wom) {
    auto scene = getScene();
    if (!scene) return;

    auto plan = scene->getLastPlan();
    if (!plan || !plan->hasSelectedM2) {
        m_hasLastSyncedPick = false;
        return;
    }

    if (m_hasLastSyncedPick && plan->selectedM2 == m_lastSyncedPick) return;

    m_lastSyncedPick = plan->selectedM2;
    m_hasLastSyncedPick = true;

    int customId = wom->getCustomM2IdByM2ObjId(plan->selectedM2);
    if (customId >= 0) {
        m_selectedId = customId;
    }
}

void CustomObjectsWindow::refreshDocumentList() {
    m_documents.clear();

    std::error_code ec;
    if (ghc::filesystem::exists(kDocumentsDir, ec)) {
        for (auto &entry : ghc::filesystem::directory_iterator(kDocumentsDir, ec)) {
            if (entry.path().extension() == ".json") {
                m_documents.push_back(entry.path().stem().string());
            }
        }
    }
    std::sort(m_documents.begin(), m_documents.end());
}

bool CustomObjectsWindow::saveDocument(const std::shared_ptr<WorldObjectManager> &wom, const std::string &docName) {
    nlohmann::json document;
    document["name"] = docName;

    auto scene = getScene();
    document["mapId"] = scene ? scene->getMapId() : -1;

    auto &objectsJson = document["objects"];
    objectsJson = nlohmann::json::array();
    for (auto &[id, object] : wom->getCustomM2s()) {
        nlohmann::json objectJson;
        objectJson["name"] = object.name;
        objectJson["fileDataId"] = object.fileDataId;
        objectJson["modelFileName"] = object.modelFileName;
        objectJson["position"] = {object.position.x, object.position.y, object.position.z};
        objectJson["rotation"] = {object.rotationEulerDeg.x, object.rotationEulerDeg.y, object.rotationEulerDeg.z};
        objectJson["scale"] = {object.scale.x, object.scale.y, object.scale.z};
        objectsJson.push_back(objectJson);
    }

    std::error_code ec;
    ghc::filesystem::create_directories(kDocumentsDir, ec);

    std::ofstream out(documentPath(docName), std::ios::trunc);
    if (!out) return false;

    out << document.dump(2);
    return out.good();
}

bool CustomObjectsWindow::loadDocument(const std::shared_ptr<WorldObjectManager> &wom, const std::string &docName) {
    std::ifstream in(documentPath(docName));
    if (!in) {
        m_docStatusMessage = "Could not open document '" + docName + "'";
        m_docStatusIsError = true;
        return false;
    }

    nlohmann::json document;
    try {
        in >> document;
    } catch (const std::exception &e) {
        m_docStatusMessage = std::string("Failed to parse document: ") + e.what();
        m_docStatusIsError = true;
        return false;
    }

    if (!document.contains("objects") || !document["objects"].is_array()) {
        m_docStatusMessage = "Document '" + docName + "' has no objects array";
        m_docStatusIsError = true;
        return false;
    }

    if (m_loadReplacesCurrent) {
        wom->clearCustomM2s();
        m_selectedId = -1;
    }

    int loadedCount = 0;
    for (auto &objectJson : document["objects"]) {
        try {
            std::string name = objectJson.value("name", "");
            std::string modelFileName = objectJson.value("modelFileName", "");
            int fileDataId = objectJson.value("fileDataId", 0);

            auto readVec3 = [](const nlohmann::json &j, const char *key, const mathfu::vec3 &fallback) {
                if (!j.contains(key) || !j[key].is_array() || j[key].size() != 3) return fallback;
                return mathfu::vec3(j[key][0].get<float>(), j[key][1].get<float>(), j[key][2].get<float>());
            };

            auto position = readVec3(objectJson, "position", mathfu::vec3(0, 0, 0));
            auto rotation = readVec3(objectJson, "rotation", mathfu::vec3(0, 0, 0));
            auto scale = readVec3(objectJson, "scale", mathfu::vec3(1, 1, 1));

            if (wom->addCustomM2(name, modelFileName, fileDataId, position, rotation, scale) >= 0) {
                loadedCount++;
            }
        } catch (const std::exception &e) {
            m_docStatusMessage = std::string("Skipped malformed object: ") + e.what();
            m_docStatusIsError = true;
        }
    }

    int storedMapId = document.value("mapId", -1);
    m_docStatusMessage = "Loaded " + std::to_string(loadedCount) + " object(s) from '" + docName + "'";
    auto scene = getScene();
    if (scene && storedMapId >= 0 && storedMapId != scene->getMapId()) {
        m_docStatusMessage += " (note: document was saved for map id " + std::to_string(storedMapId) + ")";
    }
    m_docStatusIsError = false;

    return true;
}

void CustomObjectsWindow::drawDocumentsSection(const std::shared_ptr<WorldObjectManager> &wom) {
    if (!ImGui::CollapsingHeader("Documents", ImGuiTreeNodeFlags_DefaultOpen)) return;

    if (!m_documentsScanned) {
        refreshDocumentList();
        m_documentsScanned = true;
    }

    ImGui::InputText("Document name", m_docName, sizeof(m_docName));

    std::string docName = sanitizeDocumentName(m_docName);

    if (ImGui::Button("Save")) {
        if (docName.empty()) {
            m_docStatusMessage = "Enter a document name first";
            m_docStatusIsError = true;
        } else if (saveDocument(wom, docName)) {
            m_docStatusMessage = "Saved '" + docName + "'";
            m_docStatusIsError = false;
            refreshDocumentList();
            ImGui::InsertNotification({ImGuiToastType_Success, 3000, "Custom objects saved to '%s'", docName.c_str()});
        } else {
            m_docStatusMessage = "Failed to save '" + docName + "'";
            m_docStatusIsError = true;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Load")) {
        if (loadDocument(wom, docName)) {
            ImGui::InsertNotification({ImGuiToastType_Success, 3000, "Custom objects loaded from '%s'", docName.c_str()});
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete")) {
        std::error_code ec;
        if (!docName.empty() && ghc::filesystem::remove(documentPath(docName), ec)) {
            m_docStatusMessage = "Deleted '" + docName + "'";
            m_docStatusIsError = false;
            refreshDocumentList();
        } else {
            m_docStatusMessage = "Could not delete '" + docName + "'";
            m_docStatusIsError = true;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        refreshDocumentList();
    }

    ImGui::Checkbox("Replace current objects on load", &m_loadReplacesCurrent);

    if (!m_documents.empty()) {
        if (ImGui::BeginChild("CustomObjectsDocuments", ImVec2(0, 100), true)) {
            for (auto &document : m_documents) {
                if (ImGui::Selectable(document.c_str(), docName == document)) {
                    std::strncpy(m_docName, document.c_str(), sizeof(m_docName) - 1);
                    m_docName[sizeof(m_docName) - 1] = '\0';
                }
            }
        }
        ImGui::EndChild();
    }

    if (!m_docStatusMessage.empty()) {
        ImGui::TextColored(m_docStatusIsError ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) : ImVec4(0.4f, 1.0f, 0.4f, 1.0f),
                           "%s", m_docStatusMessage.c_str());
    }
}
