//
// Created by Deamon on 12/1/2022.
//

#include "MapSceneRenderForwardVLK.h"
#include "../../vulkan/IRenderFunctionVLK.h"
#include "../../../engine/objects/scenes/map.h"
#include "../../../gapi/vulkan/materials/MaterialBuilderVLK.h"
#include "../../../gapi/vulkan/meshes/GMeshVLK.h"
#include "../../../gapi/vulkan/buffers/IBufferChunkVLK.h"
#include "materials/IMaterialInstance.h"

MapSceneRenderForwardVLK::MapSceneRenderForwardVLK(const HGDeviceVLK &hDevice, Config *config) :
    m_device(hDevice), MapSceneRenderer(config) {
    iboBuffer   = m_device->createIndexBuffer(1024*1024);

    vboM2Buffer     = m_device->createVertexBuffer(1024*1024);
    vboAdtBuffer    = m_device->createVertexBuffer(1024*1024);
    vboWMOBuffer    = m_device->createVertexBuffer(1024*1024);
    vboWaterBuffer  = m_device->createVertexBuffer(1024*1024);
    vboSkyBuffer    = m_device->createVertexBuffer(1024*1024);

    uboBuffer = m_device->createUniformBuffer(sizeof(ImgUI::modelWideBlockVS)*IDevice::MAX_FRAMES_IN_FLIGHT);

    m_emptyM2VAO = createM2VAO(nullptr, nullptr);

    //Framebuffers for rendering
    auto const dataFormat = { ITextureFormat::itRGBA };

    m_renderPass = hDevice->getRenderPass(dataFormat, ITextureFormat::itDepth32,
                                          sampleCountToVkSampleCountFlagBits(hDevice->getMaxSamplesCnt()),
                                          true, false);

    for (auto & colorFrameBuffer : m_colorFrameBuffers) {
        colorFrameBuffer = std::make_shared<GFrameBufferVLK>(
            *hDevice,
            dataFormat,
            ITextureFormat::itDepth32,
            hDevice->getMaxSamplesCnt(),
            640, 480
        );
    }

    sceneWideChunk = std::make_shared<CBufferChunkVLK<sceneWideBlockVSPS>>(uboBuffer);
}

// ------------------
// Buffer creation
// ------------------

HGVertexBufferBindings MapSceneRenderForwardVLK::createWmoVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto wmoVAO = m_device->createVertexBufferBindings();
    wmoVAO->addVertexBufferBinding(vertexBuffer, std::vector(staticWMOBindings.begin(), staticWMOBindings.end()));
    wmoVAO->setIndexBuffer(indexBuffer);

    return wmoVAO;
}
HGVertexBufferBindings MapSceneRenderForwardVLK::createM2VAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto m2VAO = m_device->createVertexBufferBindings();
    m2VAO->addVertexBufferBinding(vertexBuffer, std::vector(staticM2Bindings.begin(), staticM2Bindings.end()));
    m2VAO->setIndexBuffer(indexBuffer);

    return m2VAO;
}
HGVertexBufferBindings MapSceneRenderForwardVLK::createWaterVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto waterVAO = m_device->createVertexBufferBindings();
    waterVAO->addVertexBufferBinding(vertexBuffer, std::vector(staticWaterBindings.begin(), staticWaterBindings.end()));
    waterVAO->setIndexBuffer(indexBuffer);

    return waterVAO;
};

HGVertexBufferBindings MapSceneRenderForwardVLK::createSkyVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto skyVAO = m_device->createVertexBufferBindings();
    skyVAO->addVertexBufferBinding(vertexBuffer, std::vector(skyConusBinding.begin(), skyConusBinding.end()));
    skyVAO->setIndexBuffer(indexBuffer);

    return skyVAO;
}

HGVertexBuffer MapSceneRenderForwardVLK::createM2VertexBuffer(int sizeInBytes) {
    return vboM2Buffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer MapSceneRenderForwardVLK::createM2IndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}

HGVertexBuffer MapSceneRenderForwardVLK::createADTVertexBuffer(int sizeInBytes) {
    return vboAdtBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer MapSceneRenderForwardVLK::createADTIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}

HGVertexBuffer MapSceneRenderForwardVLK::createWMOVertexBuffer(int sizeInBytes) {
    return vboWMOBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer MapSceneRenderForwardVLK::createWMOIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}

HGVertexBuffer MapSceneRenderForwardVLK::createWaterVertexBuffer(int sizeInBytes) {
    return vboWaterBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer MapSceneRenderForwardVLK::createWaterIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}

HGVertexBuffer MapSceneRenderForwardVLK::createSkyVertexBuffer(int sizeInBytes) {
    return vboSkyBuffer->getSubBuffer(sizeInBytes);;
};
HGIndexBuffer  MapSceneRenderForwardVLK::createSkyIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}

struct BufferChunkHelper {
public:
    template<typename T>
    static const inline std::shared_ptr<CBufferChunkVLK<T>> cast(const std::shared_ptr<IBufferChunk<T>> &chunk) {
        return std::dynamic_pointer_cast<CBufferChunkVLK<T>>(chunk);
    }

    template<typename T>
    static const inline void create(const HGBufferVLK &parentBuffer, std::shared_ptr<IBufferChunk<T>> &chunk, int realSize = -1) {
        chunk = std::make_shared<CBufferChunkVLK<T>>(parentBuffer, realSize);
    }
};


std::shared_ptr<IM2Material>
MapSceneRenderForwardVLK::createM2Material(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                           const PipelineTemplate &pipelineTemplate,
                                           const M2MaterialTemplate &m2MaterialTemplate) {

    auto &l_sceneWideChunk = sceneWideChunk;
    auto vertexData = std::make_shared<CBufferChunkVLK<M2::meshWideBlockVS>>(uboBuffer);
    auto fragmentData = std::make_shared<CBufferChunkVLK<M2::meshWideBlockPS>>(uboBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"m2Shader", "m2Shader"})
        .createPipeline(m_emptyM2VAO, m_renderPass, pipelineTemplate)
        .createDescriptorSet(0, [&m2ModelData, &vertexData, &fragmentData, &l_sceneWideChunk](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelper::cast(l_sceneWideChunk)->getSubBuffer())
                .ubo(1, BufferChunkHelper::cast(m2ModelData->m_placementMatrix)->getSubBuffer())
                .ubo(2, BufferChunkHelper::cast(m2ModelData->m_bonesData)->getSubBuffer())
                .ubo(3, BufferChunkHelper::cast(m2ModelData->m_modelFragmentData)->getSubBuffer())
                .ubo(4, vertexData->getSubBuffer())
                .ubo(5, fragmentData->getSubBuffer());
        })
        .createDescriptorSet(1, [&m2MaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(6, std::dynamic_pointer_cast<GTextureVLK>(m2MaterialTemplate.texture0))
                .texture(7, std::dynamic_pointer_cast<GTextureVLK>(m2MaterialTemplate.texture1))
                .texture(8, std::dynamic_pointer_cast<GTextureVLK>(m2MaterialTemplate.texture2));
        })
        .toMaterial<IM2Material>([&vertexData, &fragmentData](IM2Material *instance) -> void {
            instance->m_fragmentData = fragmentData;
            instance->m_vertexData = vertexData;
        });

    return material;
}

std::shared_ptr<ISkyMeshMaterial> MapSceneRenderForwardVLK::createSkyMeshMaterial(const PipelineTemplate &pipelineTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto skyColors = std::make_shared<CBufferChunkVLK<DnSky::meshWideBlockVS>>(uboBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"skyConus", "skyConus"})
        .createPipeline(m_emptyM2VAO, m_renderPass, pipelineTemplate)
        .createDescriptorSet(0, [&skyColors, &l_sceneWideChunk](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelper::cast(l_sceneWideChunk)->getSubBuffer())
                .ubo(1, skyColors->getSubBuffer());
        })
        .toMaterial<ISkyMeshMaterial>([&skyColors](ISkyMeshMaterial *instance) -> void {
            instance->m_skyColors = skyColors;
        });

    return material;
}

std::unique_ptr<IRenderFunction> MapSceneRenderForwardVLK::update(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams,
                                             const std::shared_ptr<MapRenderPlan> &framePlan) {

    auto hthis = std::dynamic_pointer_cast<MapSceneRenderForwardVLK>(this->shared_from_this());
    auto mapScene = std::dynamic_pointer_cast<Map>(frameInputParams->frameParameters->scene);

    mapScene->doPostLoad(hthis, framePlan);
    mapScene->update(framePlan);



    return createRenderFuncVLK([](CmdBufRecorder &uploadCmd, CmdBufRecorder &frameBufCmd, CmdBufRecorder &swapChainCmd) -> void {

    });
}

std::shared_ptr<MapRenderPlan> MapSceneRenderForwardVLK::getLastCreatedPlan() {
    return nullptr;
}

std::shared_ptr<IM2ModelData> MapSceneRenderForwardVLK::createM2ModelMat(int bonesCount) {
    auto result = std::make_shared<IM2ModelData>();

    BufferChunkHelper::create(uboBuffer, result->m_placementMatrix);
    BufferChunkHelper::create(uboBuffer, result->m_bonesData, sizeof(mathfu::mat4) * bonesCount);
    BufferChunkHelper::create(uboBuffer, result->m_modelFragmentData);

    return result;
}

HGMesh MapSceneRenderForwardVLK::createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) {
    return std::make_shared<GMeshVLK>(*m_device, meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material));
}

