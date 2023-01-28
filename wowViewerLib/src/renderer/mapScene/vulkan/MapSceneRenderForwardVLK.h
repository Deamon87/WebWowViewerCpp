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

    std::shared_ptr<MapRenderPlan> processCulling(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams) override;
    void updateAndDraw(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams, const std::shared_ptr<MapRenderPlan> &framePlan) override;

    std::shared_ptr<MapRenderPlan> getLastCreatedPlan() override;

//-------------------------------------
//  Buffer creation
//-------------------------------------

    HGVertexBuffer createM2VertexBuffer(int sizeInBytes) override;
    HGIndexBuffer  createM2IndexBuffer(int sizeInBytes) override;

    HGVertexBuffer createADTVertexBuffer(int sizeInBytes) override;
    HGIndexBuffer  createADTIndexBuffer(int sizeInBytes) override;

    HGVertexBuffer createWMOVertexBuffer(int sizeInBytes) override;
    HGIndexBuffer  createWMOIndexBuffer(int sizeInBytes) override;

    HGVertexBuffer createWaterVertexBuffer(int sizeInBytes) override;
    HGIndexBuffer  createWaterIndexBuffer(int sizeInBytes) override;
private:
    HGDeviceVLK m_device;


};


#endif //AWEBWOWVIEWERCPP_MAPSCENERENDERFORWARDVLK_H
