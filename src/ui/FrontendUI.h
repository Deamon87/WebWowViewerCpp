//
// Created by deamon on 20.12.19.
//

#ifndef AWEBWOWVIEWERCPP_FRONTENDUI_H
#define AWEBWOWVIEWERCPP_FRONTENDUI_H


#include "../../wowViewerLib/src/gapi/interface/IDeviceUI.h"

class FrontendUI : public IDeviceUI {

public:
    void initImgui(GLFWwindow* window);

    void composeUI();
    void newFrame();
    void renderUI() override;

    bool getStopMouse();
    bool getStopKeyboard();

    void setOpenCascStorageCallback(std::function <void(std::string cascPath)> callback);

#ifdef LINK_VULKAN
    virtual void renderUIVLK(VkCommandBuffer commandBuffer) = 0;
#endif


};


#endif //AWEBWOWVIEWERCPP_FRONTENDUI_H
