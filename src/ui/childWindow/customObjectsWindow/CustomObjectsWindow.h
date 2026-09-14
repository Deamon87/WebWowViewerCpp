//
// Created by Deamon on 7/26/2026.
//

#ifndef AWEBWOWVIEWERCPP_CUSTOMOBJECTSWINDOW_H
#define AWEBWOWVIEWERCPP_CUSTOMOBJECTSWINDOW_H

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "../../../../wowViewerLib/src/engine/ApiContainer.h"
#include "../../../../wowViewerLib/src/engine/objects/worldObject/WorldObjectManager.h"
#include "imgui.h"

class SceneWindow;

// ImGui dialog for managing user-placed custom M2 objects of the currently open map
// (the ones held by the map's WorldObjectManager). Supports adding (by FileDataID or
// model path), removing, numeric and gizmo-based moving/rotating/scaling in 3D, and
// persisting the current set to named JSON documents ("./customObjects/<name>.json").
class CustomObjectsWindow {
public:
    // getMapScene must return the SceneWindow hosting a Map (i.e. one with a
    // WorldObjectManager), or nullptr when no map is currently open.
    explicit CustomObjectsWindow(const HApiContainer &api,
                                 std::function<std::shared_ptr<SceneWindow>()> getMapScene);

    void show() { m_show = true; }
    bool isVisible() const { return m_show; }

    // Returns false once the window was closed (owner should release it then).
    bool draw();

private:
    std::shared_ptr<WorldObjectManager> getWorldObjectManager();
    std::shared_ptr<SceneWindow> getScene();

    void drawAddSection(const std::shared_ptr<WorldObjectManager> &wom);
    void drawObjectList(const std::shared_ptr<WorldObjectManager> &wom);
    void drawSelectedEditor(const std::shared_ptr<WorldObjectManager> &wom);
    void drawGizmo(const std::shared_ptr<WorldObjectManager> &wom);
    void drawDocumentsSection(const std::shared_ptr<WorldObjectManager> &wom);

    void syncSelectionFromScenePick(const std::shared_ptr<WorldObjectManager> &wom);

    void refreshDocumentList();
    bool saveDocument(const std::shared_ptr<WorldObjectManager> &wom, const std::string &docName);
    bool loadDocument(const std::shared_ptr<WorldObjectManager> &wom, const std::string &docName);

    HApiContainer m_api;
    std::function<std::shared_ptr<SceneWindow>()> m_getMapScene;
    bool m_show = true;

    // Add-new-object form
    char m_addName[128] = "";
    int m_addFileDataId = 0;
    char m_addModelName[256] = "";
    bool m_addByFileName = false;
    float m_addPosition[3] = {0, 0, 0};
    float m_addRotation[3] = {0, 0, 0};
    float m_addScale[3] = {1, 1, 1};

    int m_selectedId = -1;

    // Gizmo state
    bool m_gizmoEnabled = true;
    int m_gizmoOperation = 0; // 0 = translate, 1 = rotate, 2 = scale
    bool m_gizmoWorldMode = true;
    bool m_gizmoSnap = false;
    float m_gizmoSnapValues[3] = {1.0f, 5.0f, 0.1f}; // translate units, rotate degrees, scale factor

    // Scene-pick -> list selection sync
    bool m_hasLastSyncedPick = false;
    M2ObjId m_lastSyncedPick = M2ObjId(0);

    // JSON documents
    char m_docName[128] = "";
    bool m_loadReplacesCurrent = true;
    std::vector<std::string> m_documents;
    bool m_documentsScanned = false;
    std::string m_docStatusMessage;
    bool m_docStatusIsError = false;
};

#endif //AWEBWOWVIEWERCPP_CUSTOMOBJECTSWINDOW_H
