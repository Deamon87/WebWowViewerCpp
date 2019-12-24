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
    std::function <void(std::string cascPath)> openCascCallback = nullptr;
    std::function <void(int fdid)> openSceneByfdid = nullptr;
    std::function <void(float &cameraX,float &cameraY,float &cameraZ)> getCameraPos = nullptr;

    std::function <void(int wdtFileDataId)> getAdtSelectionMinimap = nullptr;
    std::function <void(std::vector<MapRecord> &mapList)> getMapList = nullptr;

    std::function <bool(std::array<std::array<HGTexture, 64>, 64> &minimap)> fillAdtSelectionminimap = nullptr;

    std::array<std::array<HGTexture, 64>, 64> adtSelectionMinimap;

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
//  c bool showWorldPosTooltip = false;

    float minimapZoom = 1;
    float prevMinimapZoom = 1;
    int prevMapId = -1;


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

    void setOpenCascStorageCallback(std::function <void(std::string cascPath)> callback);
    void setOpenSceneByfdidCallback(std::function <void(int fdid)> callback);
    void setGetCameraPos( std::function <void(float &cameraX,float &cameraY,float &cameraZ)> callback);

    void setGetAdtSelectionMinimap( std::function <void(int mapId)> callback);
    void setFillAdtSelectionMinimap( std::function <bool(std::array<std::array<HGTexture, 64>, 64> &minimap)> callback);
    void setGetMapList( std::function <void(std::vector<MapRecord> &mapList)> callback);

#ifdef LINK_VULKAN
    virtual void renderUIVLK(VkCommandBuffer commandBuffer) = 0;
#endif


    void showMainMenu();

    void showMapSelectionDialog();

    void showAdtSelectionMinimap();
};


#endif //AWEBWOWVIEWERCPP_FRONTENDUI_H
