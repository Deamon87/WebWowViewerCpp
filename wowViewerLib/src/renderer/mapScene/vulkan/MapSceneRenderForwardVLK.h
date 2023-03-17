//
// Created by Deamon on 12/1/2022.
//

#ifndef AWEBWOWVIEWERCPP_MAPSCENERENDERFORWARDVLK_H
#define AWEBWOWVIEWERCPP_MAPSCENERENDERFORWARDVLK_H


#include "../MapSceneRenderer.h"
#include "../../../gapi/vulkan/GDeviceVulkan.h"
#include "../materials/IMaterialStructs.h"

class MapSceneRenderForwardVLK : public MapSceneRenderer {
public:
    explicit MapSceneRenderForwardVLK(const HGDeviceVLK &hDevice, Config *config);
    ~MapSceneRenderForwardVLK() override = default;

    std::unique_ptr<IRenderFunction> update(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams, const std::shared_ptr<MapRenderPlan> &framePlan) override;
    inline static void drawMesh(CmdBufRecorder &cmdBuf, const HGMesh &mesh);

    std::shared_ptr<MapRenderPlan> getLastCreatedPlan() override;

//-------------------------------------
//  Buffer creation
//-------------------------------------
    HGVertexBufferBindings createWmoVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override;
    HGVertexBufferBindings createM2VAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override;
    HGVertexBufferBindings createWaterVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override;
    HGVertexBufferBindings createSkyVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) override;

    HGVertexBuffer createM2VertexBuffer(int sizeInBytes) override;
    HGIndexBuffer  createM2IndexBuffer(int sizeInBytes) override;

    HGVertexBuffer createADTVertexBuffer(int sizeInBytes) override;
    HGIndexBuffer  createADTIndexBuffer(int sizeInBytes) override;

    HGVertexBuffer createWMOVertexBuffer(int sizeInBytes) override;
    HGIndexBuffer  createWMOIndexBuffer(int sizeInBytes) override;

    HGVertexBuffer createWaterVertexBuffer(int sizeInBytes) override;
    HGIndexBuffer  createWaterIndexBuffer(int sizeInBytes) override;

    HGVertexBuffer createSkyVertexBuffer(int sizeInBytes) override;
    HGIndexBuffer  createSkyIndexBuffer(int sizeInBytes) override;

    std::shared_ptr<IM2ModelData> createM2ModelMat(int bonesCount) override;
    std::shared_ptr<IM2Material> createM2Material(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                  const PipelineTemplate &pipelineTemplate,
                                                  const M2MaterialTemplate &m2MaterialTemplate) override;
    std::shared_ptr<ISkyMeshMaterial> createSkyMeshMaterial(const PipelineTemplate &pipelineTemplate) override;

    HGMesh createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) override;
    HGM2Mesh createM2Mesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2Material> &material, int layer, int priorityPlane) override;
private:
    HGDeviceVLK m_device;

    int m_width = 640;
    int m_height = 480;

    HGBufferVLK vboM2Buffer;
    HGBufferVLK vboAdtBuffer;
    HGBufferVLK vboWMOBuffer;
    HGBufferVLK vboWaterBuffer;
    HGBufferVLK vboSkyBuffer;

    HGBufferVLK iboBuffer;
    HGBufferVLK uboBuffer;

    std::shared_ptr<IBufferChunk<sceneWideBlockVSPS>> sceneWideChunk;


    std::shared_ptr<GRenderPassVLK> m_renderPass;
    std::array<std::shared_ptr<GFrameBufferVLK>, IDevice::MAX_FRAMES_IN_FLIGHT> m_colorFrameBuffers;


    HGVertexBufferBindings m_emptyM2VAO = nullptr;
    HGVertexBufferBindings m_emptySkyVAO = nullptr;

    void assignFFXGlowUBOConsts();
    void createFrameBuffers();

    void createFFXGlowMats();
};


#endif //AWEBWOWVIEWERCPP_MAPSCENERENDERFORWARDVLK_H
