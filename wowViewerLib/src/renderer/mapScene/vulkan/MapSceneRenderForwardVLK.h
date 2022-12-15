//
// Created by Deamon on 12/1/2022.
//

#ifndef AWEBWOWVIEWERCPP_MAPSCENERENDERFORWARDVLK_H
#define AWEBWOWVIEWERCPP_MAPSCENERENDERFORWARDVLK_H


#include "../MapSceneRenderer.h"
#include "../../../gapi/vulkan/GDeviceVulkan.h"

class MapSceneRenderForwardVLK : public MapSceneRenderer {
public:
    explicit MapSceneRenderForwardVLK(HGDeviceVLK hDevice);

    void putIntoQueue(FrameInputParams<MapRenderPlan> &frameInputParams) override;
private:
    HGDeviceVLK m_device;


};


#endif //AWEBWOWVIEWERCPP_MAPSCENERENDERFORWARDVLK_H
