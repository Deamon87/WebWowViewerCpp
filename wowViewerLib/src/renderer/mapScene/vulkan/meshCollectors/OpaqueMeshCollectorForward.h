//
// Created by Deamon on 1/10/2026.
//

#ifndef WOWSTUDIO_OPAQUEMESHCOLLECTORFORWARD_H
#define WOWSTUDIO_OPAQUEMESHCOLLECTORFORWARD_H

#include "../../../../gapi/interface/IDevice.h"
#include "../../../../gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder.h"
#include "../MapSceneRenderForwardVLK.h"

class COpaqueMeshCollectorForwardVLK : public COpaqueMeshCollector {
public:
    COpaqueMeshCollectorForwardVLK() {
        commonMeshes.reserve(10000);
        waterMeshes.reserve(1000);
    }
private:
    std::vector<HGMesh> commonMeshes;
    std::vector<HGMesh> projectiveMeshes;
    std::vector<HGMesh> waterMeshes;
public:
    void addM2Mesh(const HGM2Mesh &mesh) override {
        commonMeshes.push_back(mesh);
    };
    void addWMOMesh(const HGMesh &mesh) override {
        commonMeshes.push_back(mesh);
    } ;
    void addWaterMesh(const HGMesh &mesh) override {
        waterMeshes.push_back(mesh);
    } ;
    void addADTMesh(const HGMesh &mesh) override {
        commonMeshes.push_back(mesh);
    } ;

    void addMesh(const HGMesh &mesh) override {
        commonMeshes.push_back(mesh);
    };

    void addProjectiveMesh(const HGMesh &mesh) override {
        projectiveMeshes.push_back(mesh);
    };


    void render(CmdBufRecorder &cmdBuf, CmdBufRecorder::ViewportType viewPortType) {
         //Render commonMeshes
        for (auto const &mesh : commonMeshes ) {
            cmdBuf.drawMesh(mesh, viewPortType);
        }
    }
    void renderWater(CmdBufRecorder &cmdBuf, CmdBufRecorder::ViewportType viewPortType) {
        for (auto const &mesh : waterMeshes ) {
            cmdBuf.drawMesh(mesh, viewPortType);
        }
    }
    COpaqueMeshCollector* clone() override {
        return new COpaqueMeshCollectorForwardVLK();
    }
    void merge(COpaqueMeshCollector& collector) override {
        auto l_collector = (COpaqueMeshCollectorForwardVLK &) collector;

        commonMeshes.insert(commonMeshes.end(), l_collector.commonMeshes.begin(), l_collector.commonMeshes.end());
        waterMeshes.insert(waterMeshes.end(), l_collector.waterMeshes.begin(), l_collector.waterMeshes.end());
    }
};

#endif //WOWSTUDIO_OPAQUEMESHCOLLECTORFORWARD_H
