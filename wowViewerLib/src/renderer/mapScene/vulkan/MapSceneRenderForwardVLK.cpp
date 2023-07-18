//
// Created by Deamon on 12/1/2022.
//

#include "MapSceneRenderForwardVLK.h"
#include "../../vulkan/IRenderFunctionVLK.h"
#include "../../../engine/objects/scenes/map.h"
#include "../../../gapi/vulkan/materials/MaterialBuilderVLK.h"
#include "../../../gapi/vulkan/meshes/GMeshVLK.h"
#include "../../../gapi/vulkan/buffers/CBufferChunkVLK.h"
#include "../../../gapi/vulkan/meshes/GM2MeshVLK.h"
#include "materials/IMaterialInstance.h"
#include "../../../gapi/vulkan/meshes/GSortableMeshVLK.h"
#include "../../../gapi/vulkan/buffers/GBufferChunkDynamicVLK.h"
#include "../../frame/FrameProfile.h"
#include <future>

MapSceneRenderForwardVLK::MapSceneRenderForwardVLK(const HGDeviceVLK &hDevice, Config *config) :
    m_device(hDevice), MapSceneRenderer(config) {
    iboBuffer   = m_device->createIndexBuffer(1024*1024);

    vboM2Buffer         = m_device->createVertexBuffer(1024*1024);
    vboPortalBuffer     = m_device->createVertexBuffer(1024*1024);
    vboM2ParticleBuffer = m_device->createVertexBuffer(1024*1024);
    vboM2RibbonBuffer   = m_device->createVertexBuffer(1024*1024);
    vboAdtBuffer        = m_device->createVertexBuffer(3*1024*1024);
    vboWMOBuffer        = m_device->createVertexBuffer(1024*1024);
    vboWaterBuffer      = m_device->createVertexBuffer(1024*1024);
    vboSkyBuffer        = m_device->createVertexBuffer(1024*1024);
    vboWMOGroupAmbient  = m_device->createVertexBuffer(16*200);

    {
        const float epsilon = 0.f;
        std::array<mathfu::vec2_packed, 4> vertexBuffer = {
            mathfu::vec2_packed(mathfu::vec2(-1.0f + epsilon, -1.0f + epsilon)),
            mathfu::vec2_packed(mathfu::vec2(-1.0f + epsilon, 1.0f - epsilon)),
            mathfu::vec2_packed(mathfu::vec2(1.0f - epsilon, -1.0f + epsilon)),
            mathfu::vec2_packed(mathfu::vec2(1.0f - epsilon, 1.f - epsilon))
        };
        std::vector<uint16_t> indexBuffer = {
            0, 1, 2,
            2, 1, 3
        };
        m_vboQuad = m_device->createVertexBuffer(vertexBuffer.size() * sizeof(mathfu::vec2_packed));
        m_iboQuad = m_device->createIndexBuffer(indexBuffer.size() * sizeof(uint16_t));
        m_vboQuad->uploadData(vertexBuffer.data(), vertexBuffer.size() * sizeof(mathfu::vec2_packed));
        m_iboQuad->uploadData(indexBuffer.data(), indexBuffer.size() * sizeof(uint16_t));

        m_drawQuadVao = m_device->createVertexBufferBindings();
        m_drawQuadVao->addVertexBufferBinding(m_vboQuad, std::vector(fullScreenQuad.begin(), fullScreenQuad.end()));
        m_drawQuadVao->setIndexBuffer(m_iboQuad);
        m_drawQuadVao->save();
    }

    uboBuffer = m_device->createUniformBuffer(1024*1024);
    uboStaticBuffer = m_device->createUniformBuffer(1024*1024);

    uboM2BoneMatrixBuffer = m_device->createUniformBuffer(5000*64);

    m_emptyADTVAO = createADTVAO(nullptr, nullptr);
    m_emptyM2VAO = createM2VAO(nullptr, nullptr);
    m_emptyM2ParticleVAO = createM2ParticleVAO(nullptr, nullptr);
    m_emptyM2RibbonVAO = createM2RibbonVAO(nullptr, nullptr);
    m_emptySkyVAO = createSkyVAO(nullptr, nullptr);
    m_emptyWMOVAO = createWmoVAO(nullptr, nullptr, mathfu::vec4(0,0,0,0));
    m_emptyWaterVAO = createWaterVAO(nullptr, nullptr);
    m_emptyPortalVAO = createPortalVAO(nullptr, nullptr);

    //Framebuffers for rendering
    auto const dataFormat = { ITextureFormat::itRGBA};

    m_renderPass = m_device->getRenderPass(dataFormat, ITextureFormat::itDepth32,
//                                          VK_SAMPLE_COUNT_1_BIT,
                                          sampleCountToVkSampleCountFlagBits(m_device->getMaxSamplesCnt()),
                                          true, false);

    glowPass = std::make_unique<FFXGlowPassVLK>(hDevice, uboBuffer, m_drawQuadVao);

    createFrameBuffers();

    sceneWideChunk = std::make_shared<GBufferChunkDynamicVLK<sceneWideBlockVSPS>>(hDevice, uboBuffer);
}

// ------------------
// Buffer creation
// ------------------

HGVertexBufferBindings MapSceneRenderForwardVLK::createADTVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto adtVAO = m_device->createVertexBufferBindings();
    adtVAO->addVertexBufferBinding(vertexBuffer, adtVertexBufferBinding);
    adtVAO->setIndexBuffer(indexBuffer);

    return adtVAO;
};

HGVertexBufferBindings MapSceneRenderForwardVLK::createWmoVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer, mathfu::vec4 localAmbient) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto wmoVAO = m_device->createVertexBufferBindings();

    HGVertexBuffer ambientBuffer = nullptr;
    if (vertexBuffer != nullptr) {
        ambientBuffer = vboWMOGroupAmbient->getSubBuffer(sizeof(mathfu::vec4_packed));
        auto packedAmbient = mathfu::vec4_packed(localAmbient);
        static_assert(sizeof(packedAmbient) == 16); static_assert(sizeof(mathfu::vec4_packed) == 16);

        ambientBuffer->uploadData(&packedAmbient, sizeof(mathfu::vec4_packed));
    }
    wmoVAO->addVertexBufferBinding(vertexBuffer, std::vector(staticWMOBindings.begin(), staticWMOBindings.end()));
    wmoVAO->addVertexBufferBinding(ambientBuffer, std::vector(staticWmoGroupAmbient.begin(), staticWmoGroupAmbient.end()), true);
    wmoVAO->setIndexBuffer(indexBuffer);

    return wmoVAO;
}
HGVertexBufferBindings MapSceneRenderForwardVLK::createM2VAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto m2VAO = m_device->createVertexBufferBindings();
    m2VAO->addVertexBufferBinding(vertexBuffer, staticM2Bindings);
    m2VAO->setIndexBuffer(indexBuffer);

    return m2VAO;
}
HGVertexBufferBindings MapSceneRenderForwardVLK::createM2ParticleVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto m2ParticleVAO = m_device->createVertexBufferBindings();
    m2ParticleVAO->addVertexBufferBinding(vertexBuffer, staticM2ParticleBindings);
    m2ParticleVAO->setIndexBuffer(indexBuffer);

    return m2ParticleVAO;
}

HGVertexBufferBindings MapSceneRenderForwardVLK::createM2RibbonVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto m2RibbonVAO = m_device->createVertexBufferBindings();
    m2RibbonVAO->addVertexBufferBinding(vertexBuffer, staticM2RibbonBindings);
    m2RibbonVAO->setIndexBuffer(indexBuffer);

    return m2RibbonVAO;
};

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

HGVertexBufferBindings MapSceneRenderForwardVLK::createPortalVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto portalVAO = m_device->createVertexBufferBindings();
    portalVAO->addVertexBufferBinding(vertexBuffer, std::vector(drawPortalBindings.begin(), drawPortalBindings.end()));
    portalVAO->setIndexBuffer(indexBuffer);

    return portalVAO;
};

HGVertexBuffer MapSceneRenderForwardVLK::createPortalVertexBuffer(int sizeInBytes) {
    return vboPortalBuffer->getSubBuffer(sizeInBytes);
};
HGIndexBuffer  MapSceneRenderForwardVLK::createPortalIndexBuffer(int sizeInBytes){
    return iboBuffer->getSubBuffer(sizeInBytes);
};

HGVertexBuffer MapSceneRenderForwardVLK::createM2VertexBuffer(int sizeInBytes) {
    return vboM2Buffer->getSubBuffer(sizeInBytes);
}

HGVertexBuffer MapSceneRenderForwardVLK::createM2ParticleVertexBuffer(int sizeInBytes) {
    return vboM2ParticleBuffer->getSubBuffer(sizeInBytes);
}
HGVertexBuffer MapSceneRenderForwardVLK::createM2RibbonVertexBuffer(int sizeInBytes) {
    return vboM2RibbonBuffer->getSubBuffer(sizeInBytes);
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

static const ShaderConfig forwardShaderConfig = {{{0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC}}};
static const ShaderConfig m2ForwardShaderConfig = {{
    {0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC},
    {1, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC}
}};

std::shared_ptr<IADTMaterial>
MapSceneRenderForwardVLK::createAdtMaterial(const PipelineTemplate &pipelineTemplate, const ADTMaterialTemplate &adtMaterialTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto vertexFragmentData = std::make_shared<CBufferChunkVLK<ADT::meshWideBlockVSPS>>(uboStaticBuffer);
    auto fragmentData = std::make_shared<CBufferChunkVLK<ADT::meshWideBlockPS>>(uboBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"adtShader", "adtShader"}, forwardShaderConfig)
        .createPipeline(m_emptyADTVAO, m_renderPass, pipelineTemplate)
        .createDescriptorSet(0, [&vertexFragmentData, &fragmentData, &l_sceneWideChunk](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo_dynamic(0, DynamicBufferChunkHelperVLK::cast(l_sceneWideChunk))
                .ubo(1, vertexFragmentData->getSubBuffer())
                .ubo(2, fragmentData->getSubBuffer());
        })
        .createDescriptorSet(1, [&adtMaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(5, adtMaterialTemplate.textures[0])
                .texture(6, adtMaterialTemplate.textures[1])
                .texture(7, adtMaterialTemplate.textures[2])
                .texture(8, adtMaterialTemplate.textures[3])
                .texture(9, adtMaterialTemplate.textures[4])
                .texture(10, adtMaterialTemplate.textures[5])
                .texture(11, adtMaterialTemplate.textures[6])
                .texture(12, adtMaterialTemplate.textures[7])
                .texture(13, adtMaterialTemplate.textures[8]);
        })
        .toMaterial<IADTMaterial>([&vertexFragmentData, &fragmentData](IADTMaterial *instance) -> void {
            instance->m_materialVSPS = vertexFragmentData;
            instance->m_materialPS = fragmentData;
        });

    return material;
}

std::shared_ptr<IM2Material>
MapSceneRenderForwardVLK::createM2Material(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                           const PipelineTemplate &pipelineTemplate,
                                           const M2MaterialTemplate &m2MaterialTemplate) {

    auto &l_sceneWideChunk = sceneWideChunk;
    auto vertexFragmentData = std::make_shared<CBufferChunkVLK<M2::meshWideBlockVSPS>>(uboStaticBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"m2Shader", "m2Shader"}, m2ForwardShaderConfig)
        .createPipeline(m_emptyM2VAO, m_renderPass, pipelineTemplate)
        .createDescriptorSet(0, [&m2ModelData, &vertexFragmentData, &l_sceneWideChunk](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo_dynamic(0, DynamicBufferChunkHelperVLK::cast(l_sceneWideChunk))
                .ubo_dynamic(1, DynamicBufferChunkHelperVLK::cast(m2ModelData->m_placementMatrix))
                .ubo(2, BufferChunkHelperVLK::cast(m2ModelData->m_modelFragmentData)->getSubBuffer())
                .ubo(3, BufferChunkHelperVLK::cast(m2ModelData->m_bonesData)->getSubBuffer())
                .ubo(4, BufferChunkHelperVLK::cast(m2ModelData->m_colors)->getSubBuffer())
                .ubo(5, BufferChunkHelperVLK::cast(m2ModelData->m_textureWeights)->getSubBuffer())
                .ubo(6, BufferChunkHelperVLK::cast(m2ModelData->m_textureMatrices)->getSubBuffer())
                .ubo(7, vertexFragmentData->getSubBuffer());
        })
        .createDescriptorSet(1, [&m2MaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(6, m2MaterialTemplate.textures[0])
                .texture(7, m2MaterialTemplate.textures[1])
                .texture(8, m2MaterialTemplate.textures[2])
                .texture(9, m2MaterialTemplate.textures[3]);
        })
        .toMaterial<IM2Material>([&vertexFragmentData](IM2Material *instance) -> void {
            instance->m_vertexFragmentData = vertexFragmentData;
        });

    material->blendMode = pipelineTemplate.blendMode;
    material->batchIndex = m2MaterialTemplate.batchIndex;
    material->vertexShader = m2MaterialTemplate.vertexShader;
    material->pixelShader = m2MaterialTemplate.pixelShader;


    return material;
}

std::shared_ptr<IM2WaterFallMaterial> MapSceneRenderForwardVLK::createM2WaterfallMaterial(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                                const PipelineTemplate &pipelineTemplate,
                                                                const M2WaterfallMaterialTemplate &m2MaterialTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto vertexData = std::make_shared<CBufferChunkVLK<M2::WaterfallData::meshWideBlockVS>>(uboStaticBuffer);
    auto fragmentData = std::make_shared<CBufferChunkVLK<M2::WaterfallData::meshWideBlockPS>>(uboStaticBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"waterfallShader", "waterfallShader"}, m2ForwardShaderConfig)
        .createPipeline(m_emptyM2VAO, m_renderPass, pipelineTemplate)
        .createDescriptorSet(0, [&m2ModelData, &vertexData, &fragmentData, &l_sceneWideChunk](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo_dynamic(0, DynamicBufferChunkHelperVLK::cast(l_sceneWideChunk))
                .ubo_dynamic(1, DynamicBufferChunkHelperVLK::cast(m2ModelData->m_placementMatrix))
                .ubo(2, BufferChunkHelperVLK::cast(m2ModelData->m_bonesData)->getSubBuffer())
                .ubo(3, BufferChunkHelperVLK::cast(m2ModelData->m_textureMatrices)->getSubBuffer())
                .ubo(4, vertexData->getSubBuffer())
                .ubo(5, fragmentData->getSubBuffer());
        })
        .createDescriptorSet(1, [&m2MaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(6, m2MaterialTemplate.textures[0])
                .texture(7, m2MaterialTemplate.textures[1])
                .texture(8, m2MaterialTemplate.textures[2])
                .texture(9, m2MaterialTemplate.textures[3])
                .texture(10, m2MaterialTemplate.textures[4]);
        })
        .toMaterial<IM2WaterFallMaterial>([&vertexData, fragmentData](IM2WaterFallMaterial *instance) -> void {
            instance->m_vertexData = vertexData;
            instance->m_fragmentData = fragmentData;
        });

    return material;
}

std::shared_ptr<IM2ParticleMaterial> MapSceneRenderForwardVLK::createM2ParticleMaterial(
    const PipelineTemplate &pipelineTemplate,
    const M2ParticleMaterialTemplate &m2ParticleMatTemplate) {

    auto &l_sceneWideChunk = sceneWideChunk;
    auto l_fragmentData = std::make_shared<CBufferChunkVLK<Particle::meshParticleWideBlockPS>>(uboBuffer); ;

    auto material = MaterialBuilderVLK::fromShader(m_device, {"m2ParticleShader", "m2ParticleShader"}, forwardShaderConfig)
        .createPipeline(m_emptyM2ParticleVAO, m_renderPass, pipelineTemplate)
        .createDescriptorSet(0, [&l_sceneWideChunk, l_fragmentData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo_dynamic(0, DynamicBufferChunkHelperVLK::cast(l_sceneWideChunk))
                .ubo(4, l_fragmentData->getSubBuffer());
        })
        .createDescriptorSet(1, [&m2ParticleMatTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(5, m2ParticleMatTemplate.textures[0])
                .texture(6, m2ParticleMatTemplate.textures[1])
                .texture(7, m2ParticleMatTemplate.textures[2]);
        })
        .toMaterial<IM2ParticleMaterial>([l_fragmentData](IM2ParticleMaterial *instance) -> void {
            instance->m_fragmentData = l_fragmentData;
        });

    return material;
}

std::shared_ptr<IM2RibbonMaterial> MapSceneRenderForwardVLK::createM2RibbonMaterial(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                                                    const PipelineTemplate &pipelineTemplate,
                                                                                    const M2RibbonMaterialTemplate &m2RibbonMaterialTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto l_fragmentData = std::make_shared<CBufferChunkVLK<Ribbon::meshRibbonWideBlockPS>>(uboBuffer); ;

    auto material = MaterialBuilderVLK::fromShader(m_device, {"ribbonShader", "ribbonShader"}, forwardShaderConfig)
        .createPipeline(m_emptyM2RibbonVAO, m_renderPass, pipelineTemplate)
        .createDescriptorSet(0, [&l_sceneWideChunk, l_fragmentData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo_dynamic(0, DynamicBufferChunkHelperVLK::cast(l_sceneWideChunk))
                .ubo(4, l_fragmentData->getSubBuffer());
        })
        .createDescriptorSet(1, [&m2RibbonMaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(5, m2RibbonMaterialTemplate.textures[0]);
        })
        .toMaterial<IM2RibbonMaterial>([l_fragmentData](IM2RibbonMaterial *instance) -> void {
            instance->m_fragmentData = l_fragmentData;
        });

    return material;
};

std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> MapSceneRenderForwardVLK::createWMOWideChunk() {
    return std::make_shared<CBufferChunkVLK<WMO::modelWideBlockVS>>(uboBuffer);
}

std::shared_ptr<IWMOMaterial> MapSceneRenderForwardVLK::createWMOMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
                                                                          const PipelineTemplate &pipelineTemplate,
                                                                          const WMOMaterialTemplate &wmoMaterialTemplate) {
    auto l_vertexData = std::make_shared<CBufferChunkVLK<WMO::meshWideBlockVS>>(uboStaticBuffer); ;
    auto l_fragmentData = std::make_shared<CBufferChunkVLK<WMO::meshWideBlockPS>>(uboStaticBuffer); ;

    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"wmoShader", "wmoShader"}, forwardShaderConfig)
        .createPipeline(m_emptyWMOVAO, m_renderPass, pipelineTemplate)
        .createDescriptorSet(0, [l_sceneWideChunk, &modelWide, l_vertexData, l_fragmentData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo_dynamic(0, DynamicBufferChunkHelperVLK::cast(l_sceneWideChunk))
                .ubo(1, BufferChunkHelperVLK::cast(modelWide)->getSubBuffer())
                .ubo(2, l_vertexData->getSubBuffer())
                .ubo(4, l_fragmentData->getSubBuffer());
        })
        .createDescriptorSet(1, [&wmoMaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(5, wmoMaterialTemplate.textures[0])
                .texture(6, wmoMaterialTemplate.textures[1])
                .texture(7, wmoMaterialTemplate.textures[2])
                .texture(8, wmoMaterialTemplate.textures[3])
                .texture(9, wmoMaterialTemplate.textures[4])
                .texture(10, wmoMaterialTemplate.textures[5])
                .texture(11, wmoMaterialTemplate.textures[6])
                .texture(12, wmoMaterialTemplate.textures[7])
                .texture(13, wmoMaterialTemplate.textures[8]);
        })
        .toMaterial<IWMOMaterial>([&l_vertexData, &l_fragmentData](IWMOMaterial *instance) -> void {
            instance->m_materialVS = l_vertexData;
            instance->m_materialPS = l_fragmentData;
        });

    return material;
}

std::shared_ptr<IWaterMaterial> MapSceneRenderForwardVLK::createWaterMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
                                                const PipelineTemplate &pipelineTemplate,
                                                const WaterMaterialTemplate &waterMaterialTemplate) {
    auto l_fragmentData = std::make_shared<CBufferChunkVLK<Water::meshWideBlockPS>>(uboStaticBuffer); ;

    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"waterShader", "waterShader"}, forwardShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_renderPass, pipelineTemplate)
        .createDescriptorSet(0, [l_sceneWideChunk, &modelWide, l_fragmentData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo_dynamic(0, DynamicBufferChunkHelperVLK::cast(l_sceneWideChunk))
                .ubo(1, BufferChunkHelperVLK::cast(modelWide)->getSubBuffer())
                .ubo(4, l_fragmentData->getSubBuffer());
        })
        .createDescriptorSet(1, [&waterMaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(5, waterMaterialTemplate.texture);
        })
        .toMaterial<IWaterMaterial>([&l_fragmentData](IWaterMaterial *instance) -> void {
            instance->m_materialPS = l_fragmentData;
        });

    material->color = waterMaterialTemplate.color;
    material->liquidFlags = waterMaterialTemplate.liquidFlags;
    material->materialId = waterMaterialTemplate.liquidMaterialId;

    return material;
}



std::shared_ptr<ISkyMeshMaterial> MapSceneRenderForwardVLK::createSkyMeshMaterial(const PipelineTemplate &pipelineTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto skyColors = std::make_shared<CBufferChunkVLK<DnSky::meshWideBlockVS>>(uboBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"skyConus", "skyConus"}, forwardShaderConfig)
        .createPipeline(m_emptySkyVAO, m_renderPass, pipelineTemplate)
        .createDescriptorSet(0, [&skyColors, &l_sceneWideChunk](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo_dynamic(0, DynamicBufferChunkHelperVLK::cast(l_sceneWideChunk))
                .ubo(1, skyColors->getSubBuffer());
        })
        .toMaterial<ISkyMeshMaterial>([&skyColors](ISkyMeshMaterial *instance) -> void {
            instance->m_skyColors = skyColors;
        });

    return material;
}

std::shared_ptr<IPortalMaterial> MapSceneRenderForwardVLK::createPortalMaterial(const PipelineTemplate &pipelineTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto materialPS = std::make_shared<CBufferChunkVLK<DrawPortalShader::meshWideBlockPS>>(uboBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"drawPortalShader", "drawPortalShader"}, forwardShaderConfig)
        .createPipeline(m_emptyPortalVAO, m_renderPass, pipelineTemplate)
        .createDescriptorSet(0, [&materialPS, &l_sceneWideChunk](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo_dynamic(0, DynamicBufferChunkHelperVLK::cast(l_sceneWideChunk))
                .ubo(1, materialPS->getSubBuffer());
        })
        .toMaterial<IPortalMaterial>([&materialPS](IPortalMaterial *instance) -> void {
            instance->m_materialPS = materialPS;
        });

    return material;
}

std::shared_ptr<IM2ModelData> MapSceneRenderForwardVLK::createM2ModelMat(int bonesCount, int m2ColorsCount, int textureWeightsCount, int textureMatricesCount) {
    auto result = std::make_shared<IM2ModelData>();

    DynamicBufferChunkHelperVLK::create(m_device, uboBuffer, result->m_placementMatrix);
    BufferChunkHelperVLK::create(uboM2BoneMatrixBuffer, result->m_bonesData, sizeof(mathfu::mat4) * bonesCount);
    BufferChunkHelperVLK::create(uboBuffer, result->m_colors, sizeof(mathfu::vec4_packed) * m2ColorsCount);
    BufferChunkHelperVLK::create(uboBuffer, result->m_textureWeights, sizeof(float) * textureWeightsCount);
    BufferChunkHelperVLK::create(uboBuffer, result->m_textureMatrices, sizeof(mathfu::mat4) * textureMatricesCount);
    BufferChunkHelperVLK::create(uboBuffer, result->m_modelFragmentData);

    return result;
}

inline void MapSceneRenderForwardVLK::drawMesh(CmdBufRecorder &cmdBuf, const HGMesh &mesh, CmdBufRecorder::ViewportType viewportType ) {
    if (mesh == nullptr) return;

    const auto &meshVlk = std::dynamic_pointer_cast<GMeshVLK>(mesh);
    auto vulkanBindings = std::dynamic_pointer_cast<GVertexBufferBindingsVLK>(mesh->bindings());

    //1. Bind VBOs
    cmdBuf.bindVertexBuffers(vulkanBindings->getVertexBuffers());

    //2. Bind IBOs
    cmdBuf.bindIndexBuffer(vulkanBindings->getIndexBuffer());

    //3. Bind pipeline
    auto material = meshVlk->material();
    cmdBuf.bindPipeline(material->getPipeline());

    //4. Bind Descriptor sets
    auto const &descSets = material->getDescriptorSets();
    for (int i = 0; i < descSets.size(); i++) {
        if (descSets[i] != nullptr) {
            cmdBuf.bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, i, descSets[i]);
        }
    }

    //5. Set view port
    cmdBuf.setViewPort(viewportType);

    //6. Set scissors
    if (meshVlk->scissorEnabled()) {
        cmdBuf.setScissors(meshVlk->scissorOffset(), meshVlk->scissorSize());
    } else {
        cmdBuf.setDefaultScissors();
    }

    //7. Draw the mesh
    cmdBuf.drawIndexed(meshVlk->end(), 1, meshVlk->start()/2, 0);
}

static inline std::array<float,3> vec4ToArr3(const mathfu::vec4 &vec) {
    return {vec[0], vec[1], vec[2]};
}

std::unique_ptr<IRenderFunction> MapSceneRenderForwardVLK::update(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams,
                                             const std::shared_ptr<MapRenderPlan> &framePlan) {

    ZoneScoped;

    auto l_this = std::dynamic_pointer_cast<MapSceneRenderForwardVLK>(this->shared_from_this());
    auto mapScene = std::dynamic_pointer_cast<Map>(frameInputParams->frameParameters->scene);

    if (frameInputParams->viewPortDimensions.maxs[0] != m_width ||
        frameInputParams->viewPortDimensions.maxs[1] != m_height) {
        m_width = frameInputParams->viewPortDimensions.maxs[0];
        m_height = frameInputParams->viewPortDimensions.maxs[1];

        createFrameBuffers();

        {
            std::vector<std::shared_ptr<ISamplableTexture>> inputColorTextures;
            for (int i = 0; i < m_colorFrameBuffers.size(); i++) {
                inputColorTextures.emplace_back(m_colorFrameBuffers[i]->getAttachment(0));
            }

            glowPass->updateDimensions(m_width, m_height,
                                       inputColorTextures,
                                       m_device->getSwapChainRenderPass());
        }

    }


    //Create meshes
    auto opaqueMeshes = std::make_shared<std::vector<HGMesh>>();
    auto transparentMeshes = std::make_shared<std::vector<HGSortableMesh>>();

    auto skyOpaqueMeshes = std::make_shared<std::vector<HGMesh>>();
    auto skyTransparentMeshes = std::make_shared<std::vector<HGSortableMesh>>();
    framePlan->m2Array.lock();
    framePlan->wmoArray.lock();
    framePlan->wmoGroupArray.lock();

    //The portal meshes are created here. Need to call doPostLoad before CollectMeshes
    mapScene->doPostLoad(l_this, framePlan);

//    TracyMessageL("collect meshes created");
//    std::future<void> collectMeshAsync = std::async(std::launch::async,
//                                                    [&]() {
                                                        collectMeshes(framePlan, opaqueMeshes, transparentMeshes,
                                                                      skyOpaqueMeshes, skyTransparentMeshes);
//                                                    }
//    );

    mapScene->update(framePlan);
    mapScene->updateBuffers(framePlan);
    glowPass->assignFFXGlowUBOConsts(framePlan->frameDependentData->currentGlow);

    updateSceneWideChunk(sceneWideChunk,
                         framePlan->renderingMatrices,
                         framePlan->frameDependentData,
                         true,
                         mapScene->getCurrentSceneTime());


//    {
//        ZoneScopedN("collect meshes wait");
//        collectMeshAsync.wait();
//    }

    bool renderSky = framePlan->renderSky;
    auto skyMesh = framePlan->skyMesh;
    auto skyMesh0x4 = framePlan->skyMesh0x4;
    return createRenderFuncVLK([opaqueMeshes, transparentMeshes,
                                skyOpaqueMeshes, skyTransparentMeshes,
                                renderSky,
                                skyMesh,
                                skyMesh0x4,
                                l_this, frameInputParams](CmdBufRecorder &uploadCmd, CmdBufRecorder &frameBufCmd, CmdBufRecorder &swapChainCmd) -> void {
        // ---------------------
        // Upload stuff
        // ---------------------
        {
            {
                std::string debugMess = "sceneWideChunk = " + std::to_string(
                    DynamicBufferChunkHelperVLK::cast(l_this->sceneWideChunk)->getOffset());
                auto debugLabel = uploadCmd.beginDebugLabel(debugMess, {1.0, 0, 0, 1.0});
            }
            ZoneScopedN("submit buffers");
            uploadCmd.submitBufferUploads(l_this->uboBuffer);
            uploadCmd.submitBufferUploads(l_this->uboStaticBuffer);

            uploadCmd.submitBufferUploads(l_this->uboM2BoneMatrixBuffer);

            uploadCmd.submitBufferUploads(l_this->vboM2Buffer);
            uploadCmd.submitBufferUploads(l_this->vboPortalBuffer);
            uploadCmd.submitBufferUploads(l_this->vboM2ParticleBuffer);
            uploadCmd.submitBufferUploads(l_this->vboM2RibbonBuffer);
            uploadCmd.submitBufferUploads(l_this->vboAdtBuffer);
            uploadCmd.submitBufferUploads(l_this->vboWMOBuffer);
            uploadCmd.submitBufferUploads(l_this->vboWMOGroupAmbient);
            uploadCmd.submitBufferUploads(l_this->vboWaterBuffer);
            uploadCmd.submitBufferUploads(l_this->vboSkyBuffer);

            uploadCmd.submitBufferUploads(l_this->iboBuffer);
            uploadCmd.submitBufferUploads(l_this->m_vboQuad);
            uploadCmd.submitBufferUploads(l_this->m_iboQuad);
        }

        // ----------------------
        // Draw meshes
        // ----------------------
        {
            auto passHelper = frameBufCmd.beginRenderPass(false,
                                                          l_this->m_renderPass,
                                                          l_this->m_colorFrameBuffers[l_this->m_device->getDrawFrameNumber()],
                                                          frameInputParams->viewPortDimensions.mins,
                                                          frameInputParams->viewPortDimensions.maxs,
                                                          vec4ToArr3(frameInputParams->frameParameters->clearColor),
                                                          true
            );

            {
                ZoneScopedN("submit opaque");
                for (auto const &mesh: *opaqueMeshes) {
                    MapSceneRenderForwardVLK::drawMesh(frameBufCmd, mesh, CmdBufRecorder::ViewportType::vp_usual);
                }
            }
            if (true) {
                if (renderSky && skyMesh)
                    MapSceneRenderForwardVLK::drawMesh(frameBufCmd, skyMesh, CmdBufRecorder::ViewportType::vp_skyBox);

                for (auto const &mesh: *skyOpaqueMeshes) {
                    MapSceneRenderForwardVLK::drawMesh(frameBufCmd, mesh, CmdBufRecorder::ViewportType::vp_skyBox);
                }
                for (auto const &mesh: *skyTransparentMeshes) {
//                    std::string debugMess =
//                        "Drawing mesh "
//                        " meshType = " + std::to_string((int)mesh->getMeshType()) +
//                        " priorityPlane = " + std::to_string(mesh->priorityPlane()) +
//                        " sortDistance = " + std::to_string(mesh->getSortDistance()) +
//                        " blendMode = " + std::to_string((int)mesh->getGxBlendMode());
//
//                    auto debugLabel = frameBufCmd.beginDebugLabel(debugMess, {1.0, 0, 0, 1.0});
                    
                    MapSceneRenderForwardVLK::drawMesh(frameBufCmd, mesh, CmdBufRecorder::ViewportType::vp_skyBox);
                }
                if (renderSky && skyMesh0x4)
                    MapSceneRenderForwardVLK::drawMesh(frameBufCmd, skyMesh0x4, CmdBufRecorder::ViewportType::vp_skyBox);
            }
            {
                ZoneScopedN("submit transparent");
                for (auto const &mesh: *transparentMeshes) {
//
//                std::string debugMess =
//                    "Drawing mesh "
//                    " meshType = " + std::to_string((int)mesh->getMeshType()) +
//                    " priorityPlane = " + std::to_string(mesh->priorityPlane()) +
//                    " sortDistance = " + std::to_string(mesh->getSortDistance()) +
//                    " blendMode = " + std::to_string((int)mesh->getGxBlendMode());

//                    auto debugLabel = frameBufCmd.beginDebugLabel(debugMess, {1.0, 0, 0, 1.0});

                    MapSceneRenderForwardVLK::drawMesh(frameBufCmd, mesh, CmdBufRecorder::ViewportType::vp_usual);
                }
            }
        }

        l_this->glowPass->doPass(frameBufCmd, swapChainCmd,
                         l_this->m_device->getSwapChainRenderPass(),
                         frameInputParams->viewPortDimensions);
    });
}

std::shared_ptr<MapRenderPlan> MapSceneRenderForwardVLK::getLastCreatedPlan() {
    return nullptr;
}

HGMesh MapSceneRenderForwardVLK::createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) {
    return std::make_shared<GMeshVLK>(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material));
}

HGSortableMesh MapSceneRenderForwardVLK::createSortableMesh(gMeshTemplate &meshTemplate, const HMaterial &material, int priorityPlane) {
    auto mesh = std::make_shared<GSortableMeshVLK>(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), priorityPlane);
    return mesh;
}

HGM2Mesh
MapSceneRenderForwardVLK::createM2Mesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2Material> &material,
                                       int layer, int priorityPlane) {

    auto mesh = std::make_shared<GM2MeshVLK>(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), priorityPlane, layer);
    return mesh;
}
HGM2Mesh MapSceneRenderForwardVLK::createM2WaterfallMesh(gMeshTemplate &meshTemplate,
                                                         const std::shared_ptr<IM2WaterFallMaterial> &material,
                                                         int layer, int priorityPlane) {
    auto mesh = std::make_shared<GM2MeshVLK>(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), priorityPlane, layer);
    return mesh;
}
void MapSceneRenderForwardVLK::createFrameBuffers() {
    {
        auto const dataFormat = {ITextureFormat::itRGBA};

        for (auto &colorFrameBuffer: m_colorFrameBuffers) {
            colorFrameBuffer = std::make_shared<GFrameBufferVLK>(
                *m_device,
                dataFormat,
                ITextureFormat::itDepth32,
                m_device->getMaxSamplesCnt(),
                m_width, m_height
            );
        }
    }

}
