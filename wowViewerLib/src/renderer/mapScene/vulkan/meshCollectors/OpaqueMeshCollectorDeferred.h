//
// Created for deferred renderer: ADT meshes are collected separately
// since M2/WMO go through the indirect draw path.
//

#ifndef WOWSTUDIO_OPAQUEMESHCOLLECTORDEFERRED_H
#define WOWSTUDIO_OPAQUEMESHCOLLECTORDEFERRED_H

#include "../../../../gapi/interface/IDevice.h"
#include "../../../../gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder.h"

class COpaqueMeshCollectorDeferredVLK : public COpaqueMeshCollector {
public:
    COpaqueMeshCollectorDeferredVLK() {
        adtMeshes.reserve(1000);
        waterMeshes.reserve(1000);
    }
private:
    std::vector<HGMesh> adtMeshes;
    std::vector<HGMesh> waterMeshes;
    std::vector<HGMesh> projectiveMeshes;
public:
    // M2 and WMO are handled by the indirect draw path — no-op here
    void addM2Mesh(const HGM2Mesh &mesh) override {};
    void addWMOMesh(const HGMesh &mesh) override {};

    void addWaterMesh(const HGMesh &mesh) override {
        waterMeshes.push_back(mesh);
    };
    void addADTMesh(const HGMesh &mesh) override {
        adtMeshes.push_back(mesh);
    };

    void addMesh(const HGMesh &mesh) override {
        adtMeshes.push_back(mesh);
    };

    void addProjectiveMesh(const HGMesh &mesh) override {
        projectiveMeshes.push_back(mesh);
    };

    void renderADT(CmdBufRecorder &cmdBuf, CmdBufRecorder::ViewportType viewPortType) {
        for (auto const &mesh : adtMeshes) {
            cmdBuf.drawMesh(mesh, viewPortType);
        }
    }
    void renderWater(CmdBufRecorder &cmdBuf, CmdBufRecorder::ViewportType viewPortType) {
        for (auto const &mesh : waterMeshes) {
            cmdBuf.drawMesh(mesh, viewPortType);
        }
    }
    void renderProjective(CmdBufRecorder &cmdBuf, CmdBufRecorder::ViewportType viewPortType) {
        for (auto const &mesh : projectiveMeshes) {
            cmdBuf.drawMesh(mesh, viewPortType);
        }
    }
    COpaqueMeshCollector* clone() override {
        return new COpaqueMeshCollectorDeferredVLK();
    }
    void merge(COpaqueMeshCollector& collector) override {
        auto &l_collector = static_cast<COpaqueMeshCollectorDeferredVLK &>(collector);

        adtMeshes.insert(adtMeshes.end(), l_collector.adtMeshes.begin(), l_collector.adtMeshes.end());
        waterMeshes.insert(waterMeshes.end(), l_collector.waterMeshes.begin(), l_collector.waterMeshes.end());
        projectiveMeshes.insert(projectiveMeshes.end(), l_collector.projectiveMeshes.begin(), l_collector.projectiveMeshes.end());
    }
};

#endif //WOWSTUDIO_OPAQUEMESHCOLLECTORDEFERRED_H
