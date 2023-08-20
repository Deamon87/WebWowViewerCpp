//
// Created by Deamon on 12/1/2022.
//

#include "MapSceneRenderVisBufferVLK.h"
#include "../../vulkan/IRenderFunctionVLK.h"
#include "../../../engine/objects/scenes/map.h"
#include "../../../gapi/vulkan/materials/MaterialBuilderVLK.h"
#include "../../../gapi/vulkan/meshes/GMeshVLK.h"
#include "../../../gapi/vulkan/buffers/CBufferChunkVLK.h"
#include "../../../gapi/vulkan/meshes/GM2MeshVLK.h"
#include "materials/IMaterialInstance.h"
#include "../../../gapi/vulkan/meshes/GSortableMeshVLK.h"
#include "../../../gapi/vulkan/buffers/GBufferChunkDynamicVLK.h"
#include "../../../gapi/vulkan/buffers/GBufferChunkDynamicVersionedVLK.h"
#include "../../frame/FrameProfile.h"
#include <future>

static const ShaderConfig forwardShaderConfig = {
    "forwardRendering",
    {
        {0, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC}}
        }}
    }
};
const int m2TexturesBindlessCount = 2048;
static const ShaderConfig m2VisShaderConfig = {
    "visBuffer",
    {
        {0, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC}}
        }},
        {1, {
            {1, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC}}
        }},
        {3, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, true, m2TexturesBindlessCount}}
        }}
    }};

MapSceneRenderVisBufferVLK::MapSceneRenderVisBufferVLK(const HGDeviceVLK &hDevice, Config *config) :
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

    //Create m2 shaders
    {
        m2Buffers.placementMatrix = m_device->createSSBOBuffer(1024*1024, sizeof(M2::PlacementMatrix));
        m2Buffers.boneMatrix = m_device->createSSBOBuffer(1024*1024, sizeof(mathfu::mat4));
        m2Buffers.m2Colors = m_device->createSSBOBuffer(1024*1024, sizeof(mathfu::vec4_packed));
        m2Buffers.textureWeights = m_device->createSSBOBuffer(1024*1024, sizeof(mathfu::vec4_packed));
        m2Buffers.textureMatrices = m_device->createSSBOBuffer(1024*1024, sizeof(mathfu::mat4));
        m2Buffers.modelVertexDatas = m_device->createSSBOBuffer(1024*1024, sizeof(M2::meshWideBlockVSPS));
        m2Buffers.modelFragmentDatas = m_device->createSSBOBuffer(1024*1024, sizeof(M2::modelWideBlockPS));

        m2Buffers.m2InstanceData = m_device->createSSBOBuffer(1024*1024, sizeof(M2::M2InstanceRecordBindless));
        m2Buffers.meshWideBlocks = m_device->createSSBOBuffer(1024*1024, sizeof(M2::meshWideBlockVSPS_Bindless));
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

    sceneWideChunk = std::make_shared<GBufferChunkDynamicVersionedVLK<sceneWideBlockVSPS>>(hDevice, 3, uboBuffer);
    MaterialBuilderVLK::fromShader(m_device, {"adtShader", "adtShader"}, forwardShaderConfig)
        .createDescriptorSet(0, [&](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo_dynamic(0, sceneWideChunk);
            sceneWideDS = ds;
        });

    //Create global m2 descriptor for bindless textures
    MaterialBuilderVLK::fromShader(m_device, {"m2Shader", "m2Shader"}, m2VisShaderConfig)
        .createDescriptorSet(1, [&](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(1, m2Buffers.placementMatrix)
                .ssbo(2, m2Buffers.modelFragmentDatas)
                .ssbo(3, m2Buffers.boneMatrix)
                .ssbo(4, m2Buffers.m2Colors)
                .ssbo(5, m2Buffers.textureWeights)
                .ssbo(6, m2Buffers.textureMatrices)
                .ssbo(7, m2Buffers.m2InstanceData)
                .ssbo(8, m2Buffers.meshWideBlocks);

            m2BufferOneDS = ds;
        })
        .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(7, m2Buffers.modelVertexDatas);

            m2BufferTwoDS = ds;
        })
        .createDescriptorSet(3, [&](std::shared_ptr<GDescriptorSet> &ds) {
            m2TextureDS = ds;
        });
}

// ------------------


std::shared_ptr<ISimpleMaterialVLK> MapSceneRenderVisBufferVLK::getM2StaticMaterial(const PipelineTemplate &pipelineTemplate) {
    auto i = m_m2StaticMaterials.find(pipelineTemplate);
    if (i != m_m2StaticMaterials.end()) {
        if (!i->second.expired()) {
            return i->second.lock();
        } else {
            m_m2StaticMaterials.erase(i);
        }
    }

    auto staticMaterial =
        MaterialBuilderVLK::fromShader(m_device, {"m2Shader", "m2Shader"}, m2VisShaderConfig)
            .createPipeline(m_emptyM2VAO, m_renderPass, pipelineTemplate)
            .bindDescriptorSet(0, sceneWideDS)
            .bindDescriptorSet(1, m2BufferOneDS)
            .bindDescriptorSet(2, m2BufferTwoDS)
            .bindDescriptorSet(3, m2TextureDS)
            .toMaterial();

    m_m2StaticMaterials[pipelineTemplate] = staticMaterial;

    return staticMaterial;
}
// Buffer creation
// ------------------

HGVertexBufferBindings MapSceneRenderVisBufferVLK::createADTVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto adtVAO = m_device->createVertexBufferBindings();
    adtVAO->addVertexBufferBinding(vertexBuffer, adtVertexBufferBinding);
    adtVAO->setIndexBuffer(indexBuffer);

    return adtVAO;
};

HGVertexBufferBindings MapSceneRenderVisBufferVLK::createWmoVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer, mathfu::vec4 localAmbient) {
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
HGVertexBufferBindings MapSceneRenderVisBufferVLK::createM2VAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto m2VAO = m_device->createVertexBufferBindings();
    m2VAO->addVertexBufferBinding(vertexBuffer, staticM2Bindings);
    m2VAO->setIndexBuffer(indexBuffer);

    return m2VAO;
}
HGVertexBufferBindings MapSceneRenderVisBufferVLK::createM2ParticleVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto m2ParticleVAO = m_device->createVertexBufferBindings();
    m2ParticleVAO->addVertexBufferBinding(vertexBuffer, staticM2ParticleBindings);
    m2ParticleVAO->setIndexBuffer(indexBuffer);

    return m2ParticleVAO;
}

HGVertexBufferBindings MapSceneRenderVisBufferVLK::createM2RibbonVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto m2RibbonVAO = m_device->createVertexBufferBindings();
    m2RibbonVAO->addVertexBufferBinding(vertexBuffer, staticM2RibbonBindings);
    m2RibbonVAO->setIndexBuffer(indexBuffer);

    return m2RibbonVAO;
};

HGVertexBufferBindings MapSceneRenderVisBufferVLK::createWaterVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto waterVAO = m_device->createVertexBufferBindings();
    waterVAO->addVertexBufferBinding(vertexBuffer, std::vector(staticWaterBindings.begin(), staticWaterBindings.end()));
    waterVAO->setIndexBuffer(indexBuffer);

    return waterVAO;
};

HGVertexBufferBindings MapSceneRenderVisBufferVLK::createSkyVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto skyVAO = m_device->createVertexBufferBindings();
    skyVAO->addVertexBufferBinding(vertexBuffer, std::vector(skyConusBinding.begin(), skyConusBinding.end()));
    skyVAO->setIndexBuffer(indexBuffer);

    return skyVAO;
}

HGVertexBufferBindings MapSceneRenderVisBufferVLK::createPortalVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto portalVAO = m_device->createVertexBufferBindings();
    portalVAO->addVertexBufferBinding(vertexBuffer, std::vector(drawPortalBindings.begin(), drawPortalBindings.end()));
    portalVAO->setIndexBuffer(indexBuffer);

    return portalVAO;
};

HGVertexBuffer MapSceneRenderVisBufferVLK::createPortalVertexBuffer(int sizeInBytes) {
    return vboPortalBuffer->getSubBuffer(sizeInBytes);
};
HGIndexBuffer  MapSceneRenderVisBufferVLK::createPortalIndexBuffer(int sizeInBytes){
    return iboBuffer->getSubBuffer(sizeInBytes);
};

HGVertexBuffer MapSceneRenderVisBufferVLK::createM2VertexBuffer(int sizeInBytes) {
    return vboM2Buffer->getSubBuffer(sizeInBytes);
}

HGVertexBuffer MapSceneRenderVisBufferVLK::createM2ParticleVertexBuffer(int sizeInBytes) {
    return vboM2ParticleBuffer->getSubBuffer(sizeInBytes);
}
HGVertexBuffer MapSceneRenderVisBufferVLK::createM2RibbonVertexBuffer(int sizeInBytes) {
    return vboM2RibbonBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer MapSceneRenderVisBufferVLK::createM2IndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}

HGVertexBuffer MapSceneRenderVisBufferVLK::createADTVertexBuffer(int sizeInBytes) {
    return vboAdtBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer MapSceneRenderVisBufferVLK::createADTIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}

HGVertexBuffer MapSceneRenderVisBufferVLK::createWMOVertexBuffer(int sizeInBytes) {
    return vboWMOBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer MapSceneRenderVisBufferVLK::createWMOIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}

HGVertexBuffer MapSceneRenderVisBufferVLK::createWaterVertexBuffer(int sizeInBytes) {
    return vboWaterBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer MapSceneRenderVisBufferVLK::createWaterIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}
HGVertexBuffer MapSceneRenderVisBufferVLK::createSkyVertexBuffer(int sizeInBytes) {
    return vboSkyBuffer->getSubBuffer(sizeInBytes);;
};

HGIndexBuffer  MapSceneRenderVisBufferVLK::createSkyIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}



std::shared_ptr<IADTMaterial>
MapSceneRenderVisBufferVLK::createAdtMaterial(const PipelineTemplate &pipelineTemplate, const ADTMaterialTemplate &adtMaterialTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto vertexFragmentData = std::make_shared<CBufferChunkVLK<ADT::meshWideBlockVSPS>>(uboStaticBuffer);
    auto fragmentData = std::make_shared<CBufferChunkVLK<ADT::meshWideBlockPS>>(uboBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"adtShader", "adtShader"}, forwardShaderConfig)
        .createPipeline(m_emptyADTVAO, m_renderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [&vertexFragmentData, &fragmentData, &l_sceneWideChunk](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(1, *vertexFragmentData)
                .ubo(2, *fragmentData);
        })
        .createDescriptorSet(2, [&adtMaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
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
MapSceneRenderVisBufferVLK::createM2Material(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                           const PipelineTemplate &pipelineTemplate,
                                           const M2MaterialTemplate &m2MaterialTemplate) {

    auto &l_sceneWideChunk = sceneWideChunk;
    auto vertexFragmentData = std::make_shared<CBufferChunkVLK<M2::meshWideBlockVSPS>>(m2Buffers.modelVertexDatas);

    auto staticMaterial = getM2StaticMaterial(pipelineTemplate);

    auto material = MaterialBuilderVLK::fromMaterial(m_device, staticMaterial)
        .toMaterial<IM2MaterialVis>([&vertexFragmentData](IM2MaterialVis *instance) -> void {
            instance->m_vertexFragmentData = vertexFragmentData;
        });

    /*
         .createDescriptorSet(3, [&m2MaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(6, m2MaterialTemplate.textures[0])
                .texture(7, m2MaterialTemplate.textures[1])
                .texture(8, m2MaterialTemplate.textures[2])
                .texture(9, m2MaterialTemplate.textures[3]);
        })
     */

    material->blendMode = pipelineTemplate.blendMode;
    material->depthWrite = pipelineTemplate.depthWrite;
    material->depthCulling = pipelineTemplate.depthCulling;
    material->backFaceCulling = pipelineTemplate.backFaceCulling;

    material->batchIndex = m2MaterialTemplate.batchIndex;
    material->vertexShader = m2MaterialTemplate.vertexShader;
    material->pixelShader = m2MaterialTemplate.pixelShader;
    material->instanceIndex = vertexFragmentData->getIndex();


    return material;
}

std::shared_ptr<IM2WaterFallMaterial> MapSceneRenderVisBufferVLK::createM2WaterfallMaterial(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                                                          const PipelineTemplate &pipelineTemplate,
                                                                                          const M2WaterfallMaterialTemplate &m2MaterialTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto vertexData = std::make_shared<CBufferChunkVLK<M2::WaterfallData::meshWideBlockVS>>(uboStaticBuffer);
    auto fragmentData = std::make_shared<CBufferChunkVLK<M2::WaterfallData::meshWideBlockPS>>(uboStaticBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"waterfallShader", "waterfallShader"}, m2VisShaderConfig)
        .createPipeline(m_emptyM2VAO, m_renderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .bindDescriptorSet(1, std::dynamic_pointer_cast<IM2ModelDataVisVLK>(m2ModelData)->placementMatrixDS)
        .createDescriptorSet(2, [&m2ModelData, &vertexData, &fragmentData, &l_sceneWideChunk](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(4, *vertexData)
                .ubo(5, *fragmentData);
        })
        .createDescriptorSet(3, [&m2MaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
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

std::shared_ptr<IM2ParticleMaterial> MapSceneRenderVisBufferVLK::createM2ParticleMaterial(
    const PipelineTemplate &pipelineTemplate,
    const M2ParticleMaterialTemplate &m2ParticleMatTemplate) {

    auto &l_sceneWideChunk = sceneWideChunk;
    auto l_fragmentData = std::make_shared<CBufferChunkVLK<Particle::meshParticleWideBlockPS>>(uboBuffer); ;

    auto material = MaterialBuilderVLK::fromShader(m_device, {"m2ParticleShader", "m2ParticleShader"}, forwardShaderConfig)
        .createPipeline(m_emptyM2ParticleVAO, m_renderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [&l_sceneWideChunk, l_fragmentData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(4, *l_fragmentData);
        })
        .createDescriptorSet(2, [&m2ParticleMatTemplate](std::shared_ptr<GDescriptorSet> &ds) {
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

std::shared_ptr<IM2RibbonMaterial> MapSceneRenderVisBufferVLK::createM2RibbonMaterial(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                                                    const PipelineTemplate &pipelineTemplate,
                                                                                    const M2RibbonMaterialTemplate &m2RibbonMaterialTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto l_fragmentData = std::make_shared<CBufferChunkVLK<Ribbon::meshRibbonWideBlockPS>>(uboBuffer); ;
    auto &l_m2ModelData = m2ModelData;

    auto material = MaterialBuilderVLK::fromShader(m_device, {"ribbonShader", "ribbonShader"}, forwardShaderConfig)
        .createPipeline(m_emptyM2RibbonVAO, m_renderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [&l_sceneWideChunk, &l_fragmentData, &l_m2ModelData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(3, BufferChunkHelperVLK::cast(l_m2ModelData->m_textureMatrices))
                .ubo(4, *l_fragmentData);
        })
        .createDescriptorSet(2, [&m2RibbonMaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(5, m2RibbonMaterialTemplate.textures[0]);
        })
        .toMaterial<IM2RibbonMaterial>([l_fragmentData](IM2RibbonMaterial *instance) -> void {
            instance->m_fragmentData = l_fragmentData;
        });

    return material;
};

std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> MapSceneRenderVisBufferVLK::createWMOWideChunk() {
    return std::make_shared<CBufferChunkVLK<WMO::modelWideBlockVS>>(uboBuffer);
}

std::shared_ptr<IWMOMaterial> MapSceneRenderVisBufferVLK::createWMOMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
                                                                          const PipelineTemplate &pipelineTemplate,
                                                                          const WMOMaterialTemplate &wmoMaterialTemplate) {
    auto l_vertexData = std::make_shared<CBufferChunkVLK<WMO::meshWideBlockVS>>(uboStaticBuffer); ;
    auto l_fragmentData = std::make_shared<CBufferChunkVLK<WMO::meshWideBlockPS>>(uboStaticBuffer); ;

    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"wmoShader", "wmoShader"}, forwardShaderConfig)
        .createPipeline(m_emptyWMOVAO, m_renderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &modelWide, l_vertexData, l_fragmentData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(1, BufferChunkHelperVLK::cast(modelWide))
                .ubo(2, *l_vertexData)
                .ubo(4, *l_fragmentData);
        })
        .createDescriptorSet(2, [&wmoMaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
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

std::shared_ptr<IWaterMaterial> MapSceneRenderVisBufferVLK::createWaterMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
                                                                              const PipelineTemplate &pipelineTemplate,
                                                                              const WaterMaterialTemplate &waterMaterialTemplate) {
    auto l_fragmentData = std::make_shared<CBufferChunkVLK<Water::meshWideBlockPS>>(uboStaticBuffer); ;

    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"waterShader", "waterShader"}, forwardShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_renderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &modelWide, l_fragmentData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(1, BufferChunkHelperVLK::cast(modelWide))
                .ubo(4, *l_fragmentData);
        })
        .createDescriptorSet(2, [&waterMaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
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



std::shared_ptr<ISkyMeshMaterial> MapSceneRenderVisBufferVLK::createSkyMeshMaterial(const PipelineTemplate &pipelineTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto skyColors = std::make_shared<CBufferChunkVLK<DnSky::meshWideBlockVS>>(uboBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"skyConus", "skyConus"}, forwardShaderConfig)
        .createPipeline(m_emptySkyVAO, m_renderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [&skyColors, &l_sceneWideChunk](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(1, *skyColors);
        })
        .toMaterial<ISkyMeshMaterial>([&skyColors](ISkyMeshMaterial *instance) -> void {
            instance->m_skyColors = skyColors;
        });

    return material;
}

std::shared_ptr<IPortalMaterial> MapSceneRenderVisBufferVLK::createPortalMaterial(const PipelineTemplate &pipelineTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto materialPS = std::make_shared<CBufferChunkVLK<DrawPortalShader::meshWideBlockPS>>(uboBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"drawPortalShader", "drawPortalShader"}, forwardShaderConfig)
        .createPipeline(m_emptyPortalVAO, m_renderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [&materialPS, &l_sceneWideChunk](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(1, *materialPS);
        })
        .toMaterial<IPortalMaterial>([&materialPS](IPortalMaterial *instance) -> void {
            instance->m_materialPS = materialPS;
        });

    return material;
}

std::shared_ptr<IM2ModelData> MapSceneRenderVisBufferVLK::createM2ModelMat(int bonesCount, int m2ColorsCount, int textureWeightsCount, int textureMatricesCount) {
    auto result = std::make_shared<IM2ModelDataVisVLK>();

    BufferChunkHelperVLK::create(m2Buffers.placementMatrix, result->m_placementMatrix);
    BufferChunkHelperVLK::create(m2Buffers.boneMatrix, result->m_bonesData, sizeof(mathfu::mat4) * bonesCount);
    BufferChunkHelperVLK::create(m2Buffers.m2Colors, result->m_colors, sizeof(mathfu::vec4_packed) * m2ColorsCount);
    BufferChunkHelperVLK::create(m2Buffers.textureWeights, result->m_textureWeights, sizeof(float) * textureWeightsCount);
    BufferChunkHelperVLK::create(m2Buffers.textureMatrices, result->m_textureMatrices, sizeof(mathfu::mat4) * textureMatricesCount);
    BufferChunkHelperVLK::create(m2Buffers.modelFragmentDatas, result->m_modelFragmentData);

    BufferChunkHelperVLK::create(m2Buffers.m2InstanceData, result->m_instanceBindless);

    auto &instanceData = result->m_instanceBindless->getObject();
    instanceData.placementMatrixInd = BufferChunkHelperVLK::castToChunk(result->m_placementMatrix)->getSubBuffer()->getIndex();
    instanceData.boneMatrixInd      = BufferChunkHelperVLK::castToChunk(result->m_bonesData)->getSubBuffer()->getIndex();
    instanceData.m2ColorsInd        = BufferChunkHelperVLK::castToChunk(result->m_colors)->getSubBuffer()->getIndex();
    instanceData.textureWeightsInd  = BufferChunkHelperVLK::castToChunk(result->m_textureWeights)->getSubBuffer()->getIndex();
    instanceData.textureMatricesInd = BufferChunkHelperVLK::castToChunk(result->m_textureMatrices)->getSubBuffer()->getIndex();
    instanceData.modelFragmentDatasInd = BufferChunkHelperVLK::castToChunk(result->m_modelFragmentData)->getSubBuffer()->getIndex();
    result->m_instanceBindless->save();

    return result;
}

inline void MapSceneRenderVisBufferVLK::drawMesh(CmdBufRecorder &cmdBuf, const HGMesh &mesh, CmdBufRecorder::ViewportType viewportType ) {
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

std::unique_ptr<IRenderFunction> MapSceneRenderVisBufferVLK::update(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams,
                                                                  const std::shared_ptr<MapRenderPlan> &framePlan) {

    ZoneScoped;

    auto l_this = std::dynamic_pointer_cast<MapSceneRenderVisBufferVLK>(this->shared_from_this());
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

    m_lastCreatedPlan = framePlan;

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
                                   mapScene, framePlan,
                                   l_this, frameInputParams](CmdBufRecorder &uploadCmd, CmdBufRecorder &frameBufCmd, CmdBufRecorder &swapChainCmd) -> void {

        // ---------------------
        // Upload stuff
        // ---------------------
        {
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
        l_this->sceneWideChunk->setCurrentVersion(0);
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
                    MapSceneRenderVisBufferVLK::drawMesh(frameBufCmd, mesh, CmdBufRecorder::ViewportType::vp_usual);
                }
            }
            if (true) {
                if (renderSky && skyMesh)
                    MapSceneRenderVisBufferVLK::drawMesh(frameBufCmd, skyMesh, CmdBufRecorder::ViewportType::vp_skyBox);

                for (auto const &mesh: *skyOpaqueMeshes) {
                    MapSceneRenderVisBufferVLK::drawMesh(frameBufCmd, mesh, CmdBufRecorder::ViewportType::vp_skyBox);
                }
                for (int i = 0; i < skyTransparentMeshes->size(); i++) {
                    auto const &mesh = skyTransparentMeshes->at(i);

//                    std::string debugMess =
//                        "Drawing mesh "
//                        " meshType = " + std::to_string((int)mesh->getMeshType()) +
//                        " priorityPlane = " + std::to_string(mesh->priorityPlane()) +
//                        " sortDistance = " + std::to_string(mesh->getSortDistance()) +
//                        " blendMode = " + std::to_string((int)mesh->getGxBlendMode());
//
//                    auto debugLabel = frameBufCmd.beginDebugLabel(debugMess, {1.0, 0, 0, 1.0});

                    MapSceneRenderVisBufferVLK::drawMesh(frameBufCmd, mesh, CmdBufRecorder::ViewportType::vp_skyBox);
                }
                if (renderSky && skyMesh0x4)
                    MapSceneRenderVisBufferVLK::drawMesh(frameBufCmd, skyMesh0x4, CmdBufRecorder::ViewportType::vp_skyBox);
            }
            {
                ZoneScopedN("submit transparent");
                for (int i = 0; i < transparentMeshes->size(); i++) {
                    auto const &mesh = transparentMeshes->at(i);
//
//                    std::string debugMess =
//                        "Drawing mesh "
//                        " meshType = " + std::to_string((int)mesh->getMeshType()) +
//                        " priorityPlane = " + std::to_string(mesh->priorityPlane()) +
//                        " sortDistance = " + std::to_string(mesh->getSortDistance()) +
//                        " blendMode = " + std::to_string((int)mesh->getGxBlendMode());
//
//                    auto debugLabel = frameBufCmd.beginDebugLabel(debugMess, {1.0, 0, 0, 1.0});

                    MapSceneRenderVisBufferVLK::drawMesh(frameBufCmd, mesh, CmdBufRecorder::ViewportType::vp_usual);
                }
            }
        }

        l_this->glowPass->doPass(frameBufCmd, swapChainCmd,
                                 l_this->m_device->getSwapChainRenderPass(),
                                 frameInputParams->viewPortDimensions);
    });
}

std::shared_ptr<MapRenderPlan> MapSceneRenderVisBufferVLK::getLastCreatedPlan() {
    return m_lastCreatedPlan;
}

HGMesh MapSceneRenderVisBufferVLK::createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) {
    return std::make_shared<GMeshVLK>(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material));
}

HGSortableMesh MapSceneRenderVisBufferVLK::createSortableMesh(gMeshTemplate &meshTemplate, const HMaterial &material, int priorityPlane) {
    auto mesh = std::make_shared<GSortableMeshVLK>(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), priorityPlane);
    return mesh;
}

HGM2Mesh
MapSceneRenderVisBufferVLK::createM2Mesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2Material> &material,
                                       int layer, int priorityPlane) {

    auto mesh = std::make_shared<GM2MeshVLK>(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), priorityPlane, layer);
    return mesh;
}
HGM2Mesh MapSceneRenderVisBufferVLK::createM2WaterfallMesh(gMeshTemplate &meshTemplate,
                                                         const std::shared_ptr<IM2WaterFallMaterial> &material,
                                                         int layer, int priorityPlane) {
    auto mesh = std::make_shared<GM2MeshVLK>(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), priorityPlane, layer);
    return mesh;
}
void MapSceneRenderVisBufferVLK::createFrameBuffers() {
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
