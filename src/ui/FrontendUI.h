//
// Created by deamon on 20.12.19.
//

#ifndef AWEBWOWVIEWERCPP_FRONTENDUI_H
#define AWEBWOWVIEWERCPP_FRONTENDUI_H


#include "../../wowViewerLib/src/gapi/interface/IDeviceUI.h"

class FrontendUI : public IDeviceUI {
private:
    std::function <void(std::string cascPath)> openCascCallback = nullptr;
    std::function <void(int fdid)> openSceneByfdid = nullptr;
    std::function <void(float &cameraX,float &cameraY,float &cameraZ)> getCameraPos = nullptr;

    std::function <void(int mapId)> getAdtSelectionMinimap = nullptr;

    std::function <bool(std::array<std::array<HGTexture, 64>, 64> &minimap)> fillAdtSelectionminimap = nullptr;

    std::array<std::array<HGTexture, 64>, 64> adtSelectionMinimap;

    void emptyMinimap() {
        for (int i = 0; i < 64; i++) {
            for (int j = 0; j < 64; j++) {
                adtSelectionMinimap[i][j] = nullptr;
            }
        }
    }
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

#ifdef LINK_VULKAN
    virtual void renderUIVLK(VkCommandBuffer commandBuffer) = 0;
#endif


};


#endif //AWEBWOWVIEWERCPP_FRONTENDUI_H
