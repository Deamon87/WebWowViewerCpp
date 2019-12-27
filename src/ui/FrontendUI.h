//
// Created by deamon on 20.12.19.
//

#ifndef AWEBWOWVIEWERCPP_FRONTENDUI_H
#define AWEBWOWVIEWERCPP_FRONTENDUI_H


#include "imguiLib/imgui.h"
#include <fileBrowser/imfilebrowser.h>
#include "../../wowViewerLib/src/gapi/interface/IDeviceUI.h"
#include "../database/dbStructs.h"

class FrontendUI : public IDeviceUI {
private:
    std::function <bool(std::string cascPath)> openCascCallback = nullptr;
    std::function <void(int mapId, int wdtFileId, float x, float y, float z)> openSceneByfdid = nullptr;
    std::function <void()> openWMOMap = nullptr;
    std::function <void(float &cameraX,float &cameraY,float &cameraZ)> getCameraPos = nullptr;

    std::function <void(int wdtFileDataId)> getAdtSelectionMinimap = nullptr;
    std::function <void(std::vector<MapRecord> &mapList)> getMapList = nullptr;

    std::function <bool(std::array<std::array<HGTexture, 64>, 64> &minimap, bool &isWMOMap)> fillAdtSelectionminimap = nullptr;

    std::array<std::array<HGTexture, 64>, 64> adtSelectionMinimap;
    std::function<void(float farPlane)> setFarPlane;
    std::function<void(float farPlane)> setMovementSpeed;
    std::function<void(int value)> setThreadCount;
    std::function<void(int value)> setQuicksortCutoff;

    void emptyMinimap() {
        for (int i = 0; i < 64; i++) {
            for (int j = 0; j < 64; j++) {
                adtSelectionMinimap[i][j] = nullptr;
            }
        }
    }

    ImGui::FileBrowser fileDialog = ImGui::FileBrowser(ImGuiFileBrowserFlags_SelectDirectory);

    bool show_demo_window = true;
    bool show_another_window = true;
    bool showCurrentStats = true;
    bool showSelectMap = false;
    bool showSettings = false;
//  c bool showWorldPosTooltip = false;

    float minimapZoom = 1;
    float farPlane = 200;
    float movementSpeed = 1;
    int  threadCount = 4;
    int  quickSortCutoff = 10;
    float prevMinimapZoom = 1;
    int prevMapId = -1;
    bool isWmoMap = false;
    MapRecord prevMapRec;


    float worldPosX = 0;
    float worldPosY = 0;

    std::vector<MapRecord> mapList = {};
    std::vector<std::vector<std::string>> mapListStringMap = {};

public:
    void initImgui(GLFWwindow* window);


    void composeUI();
    void newFrame();
    void renderUI() override;

    bool getStopMouse();
    bool getStopKeyboard();

    void setOpenCascStorageCallback(std::function <bool(std::string cascPath)> callback);
    void setOpenSceneByfdidCallback(std::function <void(int mapId, int wdtFileId, float x, float y, float z)> callback);
    void setOpenWMOMapCallback(std::function <void()> callback);
    void setGetCameraPos( std::function <void(float &cameraX,float &cameraY,float &cameraZ)> callback);

    void setFarPlaneChangeCallback(std::function<void(float farPlane)> callback);
    void setSpeedCallback(std::function<void(float speed)> callback);


    void setThreadCountCallback(std::function<void(int value)> callback);
    void setQuicksortCutoffCallback(std::function<void(int value)> callback);

    void setGetAdtSelectionMinimap( std::function <void(int mapId)> callback);
    void setFillAdtSelectionMinimap( std::function <bool(std::array<std::array<HGTexture, 64>, 64> &minimap, bool &isWMOMap)> callback);
    void setGetMapList( std::function <void(std::vector<MapRecord> &mapList)> callback);

#ifdef LINK_VULKAN
    virtual void renderUIVLK(VkCommandBuffer commandBuffer) = 0;
#endif


    void showMainMenu();

    void showMapSelectionDialog();

    void showAdtSelectionMinimap();
    void showSettingsDialog();
};


#endif //AWEBWOWVIEWERCPP_FRONTENDUI_H
