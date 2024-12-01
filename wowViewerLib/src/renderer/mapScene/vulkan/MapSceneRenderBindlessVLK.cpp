//
// Created by Deamon on 12/1/2022.
//

#include "MapSceneRenderBindlessVLK.h"
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
#include "view/RenderViewForwardVLK.h"
#include "../../../gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder_inline.h"
#include <future>

static const ShaderConfig forwardShaderConfig = {
    "forwardRendering",
    "forwardRendering",
    {
        {0, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
            {1, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
            {2, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}}
        }}
    }
};

const int m2TexturesBindlessCount = 4096;
const int m2WaterfallTexturesBindlessCount = 128;
const int adtTexturesBindlessCount = 4096;
const int waterTexturesBindlessCount = 1024;

static const ShaderConfig m2BindlessShaderConfig = {
    "bindless/m2/forward",
    "bindless/m2/forward",
    {
        {0, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
        }},
        {1, {
            {6, {VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}}
        }},
        {2, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, true, m2TexturesBindlessCount}}
        }}
    }};
static const ShaderConfig m2BindlessGBufferShaderConfig = {
    "bindless/m2/forward",
    "bindless/m2/deferred",
    m2BindlessShaderConfig.typeOverrides
};
static const ShaderConfig bindlessShaderConfig = {
    "bindless",
    "bindless",
    {
        {0, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
        }},
    }};

static const ShaderConfig m2ParticlesBindlessGBufferShaderConfig = {
    "bindless/m2Particle/forward",
    "bindless/m2Particle/deferred",
    bindlessShaderConfig.typeOverrides
};

static const ShaderConfig m2WaterfallBindlessShaderConfig = {
    "bindless/waterfall/forward",
    "bindless/waterfall/forward",
    {
        {0, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
        }},
        {3, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, true, m2WaterfallTexturesBindlessCount}}
        }}
    }};

static const ShaderConfig wmoBindlessShaderConfig = {
    "bindless/wmo/forward",
    "bindless/wmo/forward",
    {
        {0, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
        }},
        {2, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, true, m2TexturesBindlessCount}}
        }}
    }};

static const ShaderConfig wmoBindlessGBufferShaderConfig = {
    "bindless/wmo/forward",
    "bindless/wmo/deferred",
    wmoBindlessShaderConfig.typeOverrides
};

static const ShaderConfig adtBindlessShaderConfig = {
    "bindless/adt/forward",
    "bindless/adt/forward",
    {
        {0, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
        }},
        {2, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, true, adtTexturesBindlessCount}}
        }},
        {3, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, true, adtTexturesBindlessCount}}
        }},
        {4, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, true, adtTexturesBindlessCount}}
        }}
    }};

static const ShaderConfig adtBindlessGBufferShaderConfig = {
    "bindless/adt/forward",
    "bindless/adt/deferred",
    adtBindlessShaderConfig.typeOverrides
};

static const ShaderConfig waterBindlessShaderConfig = {
    "bindless/water",
    "bindless/water",
    {
        {0, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
        }},
        {2, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, true, waterTexturesBindlessCount}}
        }}
    }};

EntityFactory<10000, GMeshId, GMeshVLK> bindlessMeshFactoryVlk;

MapSceneRenderBindlessVLK::MapSceneRenderBindlessVLK(const HGDeviceVLK &hDevice, Config *config) :
    m_device(hDevice), MapSceneRenderer(config) {
    std::cout << "Create Bindless scene renderer " << std::endl;


    const int rendererId = std::hash<uint64_t>()((uint64_t)this) & 0xFFFF;
    const std::string rendererIdStr = " " + std::to_string(rendererId);

    auto un = [rendererIdStr](const std::string &name) -> std::string {
        return name + rendererIdStr;
    };

    
    this->allBuffers = {
        this->uboBuffer,
        this->uboStaticBuffer,

        this->pointLightBuffer,
        this->spotLightBuffer,

        this->vboM2Buffer,
        this->vboPortalBuffer,
        this->vboM2RibbonBuffer,
        this->vboAdtBuffer,
        this->vboWMOBuffer,
        this->vboWaterBuffer,
        this->vboSkyBuffer,

        this->m2Buffers.placementMatrix,
        this->m2Buffers.boneMatrix,
        this->m2Buffers.m2Colors,
        this->m2Buffers.textureWeights,
        this->m2Buffers.textureMatrices,
        this->m2Buffers.modelFragmentDatas,
        this->m2Buffers.m2InstanceData,
        this->m2Buffers.meshWideBlocks,
        this->m2Buffers.meshWideBlocksBindless,

        this->m2WaterfallBuffer.waterfallCommon,
        this->m2WaterfallBuffer.waterfallBindless,

        this->adtBuffers.adtMeshWidePSes,
        this->adtBuffers.adtMeshWideVSPSes,
        this->adtBuffers.adtInstanceDatas,

        this->wmoBuffers.wmoPlacementMats,
        this->wmoBuffers.wmoMeshWideVSes,
        this->wmoBuffers.wmoMeshWidePSes,
        this->wmoBuffers.wmoMeshWideBindless,
        this->wmoBuffers.wmoGroupAmbient,
        this->wmoBuffers.wmoPerMeshData,

        this->waterBuffer.waterDataBuffer,
        this->waterBuffer.waterBindlessBuffer,

        this->iboBuffer,
        this->m_vboQuad,
        this->m_iboQuad,
        this->m_particleIndexBuffer,
        this->m_vboSpot,
        this->m_iboSpot,
    };
    for (auto &vboM2ParticleBuff : this->vboM2ParticleBuffers) {
        allBuffers.push_back(vboM2ParticleBuff);
    }
    
    iboBuffer   = m_device->createIndexBuffer(un("Scene_IBO"), 1024*1024);


    vboM2Buffer         = m_device->createVertexBuffer(un("Scene_VBO_M2"),1024*1024, sizeof(M2Vertex));
    vboPortalBuffer     = m_device->createVertexBuffer(un("Scene_VBO_Portal"),1024*1024);
    for (int k = 0; k < PARTICLES_BUFF_NUM; k++)
        vboM2ParticleBuffers[k] = m_device->createVertexBuffer(un("Scene_VBO_M2Particle_fr_"+std::to_string(k)),1024*1024, 64);
    vboM2RibbonBuffer   = m_device->createVertexBuffer(un("Scene_VBO_M2Ribbon"),1024*1024, 64);
    vboAdtBuffer        = m_device->createVertexBuffer(un("Scene_VBO_ADT"),3*1024*1024, sizeof(AdtVertex));
    vboWMOBuffer        = m_device->createVertexBuffer(un("Scene_VBO_WMO"),1024*1024, sizeof(WMOVertex));
    vboWaterBuffer      = m_device->createVertexBuffer(un("Scene_VBO_Water"),1024*1024, sizeof(LiquidVertexFormat));
    vboSkyBuffer        = m_device->createVertexBuffer(un("Scene_VBO_Sky"),1024*1024);


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
        m_vboQuad = m_device->createVertexBuffer(un("Scene_VBO_Quad"), vertexBuffer.size() * sizeof(mathfu::vec2_packed));
        m_iboQuad = m_device->createIndexBuffer(un("Scene_IBO_Quad"), indexBuffer.size() * sizeof(uint16_t));
        m_vboQuad->uploadData(vertexBuffer.data(), vertexBuffer.size() * sizeof(mathfu::vec2_packed));
        m_iboQuad->uploadData(indexBuffer.data(), indexBuffer.size() * sizeof(uint16_t));

        m_drawQuadVao = m_device->createVertexBufferBindings();
        m_drawQuadVao->addVertexBufferBinding(m_vboQuad, std::vector(fullScreenQuad.begin(), fullScreenQuad.end()));
        m_drawQuadVao->setIndexBuffer(m_iboQuad);
        m_drawQuadVao->save();
    }
    {
        m_particleIndexBuffer = m_device->createIndexBuffer(un("Scene_IBO_Particle"),
                                                            MAX_PARTICLES_PER_EMITTER * 6 * sizeof(uint16_t));

        std::vector<uint16_t> szIndexBuff;

        int vo = 0;
        for (int i = 0; i < MAX_PARTICLES_PER_EMITTER; i++) {
            szIndexBuff.push_back(vo + 0);
            szIndexBuff.push_back(vo + 1);
            szIndexBuff.push_back(vo + 2);
            szIndexBuff.push_back(vo + 3);
            szIndexBuff.push_back(vo + 2);
            szIndexBuff.push_back(vo + 1);
            vo += 4;
        }
        m_particleIndexBuffer->uploadData((void *) szIndexBuff.data(), (int) (szIndexBuff.size() * sizeof(uint16_t)));
    }
    //Create vertex data for SpotLight in z-up
    {
        std::vector<mathfu::vec2_packed> vertexBuffer;
        std::vector<uint16_t> indexBuffer;

        vertexBuffer.push_back(mathfu::vec2_packed(mathfu::vec2(0,0)));

        for (uint32_t i = 0; i < spotLightSegments; i++) {
            float angle = i * 2.0f * M_PI / (float)spotLightSegments;
            vertexBuffer.push_back(mathfu::vec2_packed(
                mathfu::vec2(cos(angle),sin(angle))
            ));
        }
        vertexBuffer.push_back(mathfu::vec2_packed(mathfu::vec2(0,0)));

        for (uint32_t i = 0; i < spotLightSegments; i++) {
            indexBuffer.push_back(0);
            indexBuffer.push_back(((i+1) % spotLightSegments) + 1);
            indexBuffer.push_back(((i) % spotLightSegments) + 1);
        }

        for (uint32_t i = 2; i < spotLightSegments; i++) {
            indexBuffer.push_back(1);
            indexBuffer.push_back(((i+1) % spotLightSegments) + 1);
            indexBuffer.push_back(((i) % spotLightSegments) + 1);
        }

        m_vboSpot = m_device->createVertexBuffer(un("Scene_VBO_Spot"), vertexBuffer.size() * sizeof(mathfu::vec2_packed));
        m_iboSpot = m_device->createIndexBuffer(un("Scene_IBO_Spot"), indexBuffer.size() * sizeof(uint16_t));
        m_vboSpot->uploadData(vertexBuffer.data(), vertexBuffer.size() * sizeof(mathfu::vec2_packed));
        m_iboSpot->uploadData(indexBuffer.data(), indexBuffer.size() * sizeof(uint16_t));

        m_drawSpotVao = m_device->createVertexBufferBindings();
        m_drawSpotVao->addVertexBufferBinding(m_vboSpot, std::vector(fullScreenQuad.begin(), fullScreenQuad.end()));
        m_drawSpotVao->setIndexBuffer(m_iboSpot);
        m_drawSpotVao->save();
    }

    //Create m2 shaders
    {
        m2Buffers.placementMatrix = m_device->createSSBOBuffer(un("M2 Placement"), 1024*1024, sizeof(M2::PlacementMatrix));
        m2Buffers.boneMatrix = m_device->createSSBOBuffer(un("M2 BoneMatrices"),1024*1024, sizeof(mathfu::mat4));
        m2Buffers.m2Colors = m_device->createSSBOBuffer(un("M2 BoneMatrices"), 1024*1024, sizeof(mathfu::vec4_packed));
        m2Buffers.textureWeights = m_device->createSSBOBuffer(un("M2 TextureWeight"), 1024*1024, sizeof(mathfu::vec4_packed));
        m2Buffers.textureMatrices = m_device->createSSBOBuffer(un("M2 TextureMatrices"), 1024*1024, sizeof(mathfu::mat4));
        m2Buffers.modelFragmentDatas = m_device->createSSBOBuffer(un("M2 FragmentData"), 1024*1024, sizeof(M2::modelWideBlockPS));
        m2Buffers.m2InstanceData = m_device->createSSBOBuffer(un("M2 InstanceData"), 1024*1024, sizeof(M2::M2InstanceRecordBindless));
        m2Buffers.meshWideBlocks = m_device->createSSBOBuffer(un("M2 MeshWide"), 1024*1024, sizeof(M2::meshWideBlockVSPS));
        m2Buffers.meshWideBlocksBindless = m_device->createSSBOBuffer(un("M2 MeshWide Bindless"), 1024*1024, sizeof(M2::meshWideBlockVSPS_Bindless));
    }
    //Create adt Shader buffs
    {
        adtBuffers.adtMeshWideVSPSes = m_device->createSSBOBuffer(un("ADT MeshVSPS"), 1024*1024, sizeof(ADT::meshWideBlockVSPS));
        adtBuffers.adtMeshWidePSes = m_device->createSSBOBuffer(un("ADT MeshPS"), 1024*1024, sizeof(ADT::meshWideBlockPS));
        adtBuffers.adtInstanceDatas = m_device->createSSBOBuffer(un("ADT InstanceData"), 1024*1024, sizeof(ADT::AdtInstanceData));
    }
    //Create wmo Shader buffs
    {
        wmoBuffers.wmoPlacementMats = m_device->createSSBOBuffer(un("WMO PlaceMat"), 1024*1024, sizeof(mathfu::mat4));
        wmoBuffers.wmoMeshWideVSes = m_device->createSSBOBuffer(un("WMO MeshWideVS"), 1024*1024, sizeof(WMO::meshWideBlockVS));
        wmoBuffers.wmoMeshWidePSes = m_device->createSSBOBuffer(un("WMO MeshWidePS"), 1024*1024, sizeof(WMO::meshWideBlockPS));
        wmoBuffers.wmoMeshWideBindless = m_device->createSSBOBuffer(un("WMO MeshWideBindless"), 1024*1024, sizeof(WMO::meshWideBlockBindless));
        wmoBuffers.wmoPerMeshData = m_device->createSSBOBuffer(un("WMO PerMeshData"), 1024*1024, sizeof(WMO::perMeshData));
        wmoBuffers.wmoGroupAmbient = m_device->createSSBOBuffer(un("Scene_VBO_WMOAmbient"),16*200, sizeof(mathfu::vec4_packed));
    }
    //Create water buffs
    {
        waterBuffer.waterDataBuffer = m_device->createSSBOBuffer(un("Water data"), 1024, sizeof(Water::meshWideBlockPS));
        waterBuffer.waterBindlessBuffer = m_device->createSSBOBuffer(un("Water Bindless"), 1024, sizeof(Water::WaterBindless));
    }

    m2WaterfallBuffer.waterfallCommon = m_device->createSSBOBuffer(un("M2 Waterfall Common"),200, sizeof(M2::WaterfallData::WaterfallCommon));
    m2WaterfallBuffer.waterfallBindless = m_device->createSSBOBuffer(un("M2 Waterfall Bindless"),200, sizeof(M2::WaterfallData::WaterfallBindless));

    pointLightBuffer = m_device->createSSBOBuffer(un("Point Light Buffer"),200, sizeof(LocalLight));
    spotLightBuffer = m_device->createSSBOBuffer(un("Spot Light Buffer"),200, sizeof(SpotLight));

    uboBuffer = m_device->createUniformBuffer(un("UBO Buffer"), 1024*1024);
    uboStaticBuffer = m_device->createUniformBuffer(un("UBO Static"), 1024*1024);

    m_emptyADTVAO = createADTVAO(vboAdtBuffer, iboBuffer);
    m_emptyM2VAO = createM2VAO(vboM2Buffer, iboBuffer);
    m_emptyM2ParticleVAO = createM2ParticleVAO(nullptr, nullptr);
    m_emptyM2RibbonVAO = createM2RibbonVAO(nullptr, nullptr);
    m_emptySkyVAO = createSkyVAO(nullptr, nullptr);
    m_emptyWMOVAO = createWmoVAO(vboWMOBuffer, iboBuffer, nullptr);
    m_emptyWaterVAO = createWaterVAO(vboWaterBuffer, iboBuffer);
    m_emptyPortalVAO = createPortalVAO(nullptr, nullptr);

    {
        //Create SceneWide descriptor
        sceneWideChunk = std::make_shared<GBufferChunkDynamicVersionedVLK<sceneWideBlockVSPS>>(hDevice, 3, uboBuffer);
        MaterialBuilderVLK::fromShader(m_device, {"adtShader", "adtShader"}, adtBindlessShaderConfig, {})
            .createDescriptorSet(0, [&](std::shared_ptr<GDescriptorSet> &ds) {
                ds->beginUpdate()
                    .ubo_dynamic(0, sceneWideChunk)
                    .texture(1, hDevice->getBlackTexturePixel())
//Untill AO is ready.
//#ifndef NDEBUG
                    .texture(2, hDevice->getWhiteTexturePixel())
//#endif
                    ;

                sceneWideDS = ds;
            });
    }

    defaultView = std::make_shared<RendererViewClass>(m_device, uboBuffer,
                                                      pointLightBuffer, spotLightBuffer,
                                                      sceneWideDS,
                                                      m_drawQuadVao,
                                                      m_drawSpotVao,
                                                      false);

    m_forwardRenderPass = defaultView->getForwardPass();
    m_gBufferPass = defaultView->getGBufferPass();

    createM2GlobalMaterialData();
    createWMOGlobalMaterialData();
    createADTGlobalMaterialData();
    createM2WaterfallGlobalMaterialData();
    createWaterGlobalMaterialData();
}

void MapSceneRenderBindlessVLK::createADTGlobalMaterialData() {
    adtLayerTextureHolder = std::make_shared<BindlessTextureHolder>(adtTexturesBindlessCount);
    adtHeightLayerTextureHolder = std::make_shared<BindlessTextureHolder>(adtTexturesBindlessCount);
    adtAlphaTextureHolder = std::make_shared<BindlessTextureHolder>(adtTexturesBindlessCount);

    //Create global ADT descriptor for bindless textures
    {
        PipelineTemplate pipelineTemplate;
        pipelineTemplate.element = DrawElementMode::TRIANGLES;
        pipelineTemplate.depthWrite = true;
        pipelineTemplate.depthCulling = true;
        pipelineTemplate.backFaceCulling = true;
        pipelineTemplate.blendMode = EGxBlendEnum::GxBlend_Opaque;

        g_adtMaterial = MaterialBuilderVLK::fromShader(m_device, {"adtShader", "adtShader"}, adtBindlessShaderConfig, {{0, sceneWideDS}})
            .createPipeline(m_emptyADTVAO, m_forwardRenderPass, pipelineTemplate)
            .createGBufferPipeline(m_emptyADTVAO, {"adtShader", "adtShader"}, adtBindlessGBufferShaderConfig, m_gBufferPass)

            .bindDescriptorSet(0, sceneWideDS)
            .createDescriptorSet(1, [&](std::shared_ptr<GDescriptorSet> &ds) {
                ds->beginUpdate()
                    .ssbo(1, adtBuffers.adtMeshWideVSPSes)
                    .ssbo(2, adtBuffers.adtMeshWidePSes)
                    .ssbo(3, adtBuffers.adtInstanceDatas);
            })
            .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
                adtLayerTextureDS = ds;
            })
            .createDescriptorSet(3, [&](std::shared_ptr<GDescriptorSet> &ds) {
                adtAlphaTextureDS = ds;
            })
            .createDescriptorSet(4, [&](std::shared_ptr<GDescriptorSet> &ds) {
                adtHeightLayerTextureDS = ds;
            }).toMaterial();
    }
}

void MapSceneRenderBindlessVLK::createWaterGlobalMaterialData() {
    PipelineTemplate pipelineTemplate;
    pipelineTemplate.element = DrawElementMode::TRIANGLES;
    pipelineTemplate.depthWrite = true;
    pipelineTemplate.depthCulling = true;
    pipelineTemplate.backFaceCulling = false;
    pipelineTemplate.blendMode = EGxBlendEnum::GxBlend_Alpha;

    //Create global water descriptor for bindless textures
    g_waterMaterial = MaterialBuilderVLK::fromShader(m_device, {"waterShader", "waterShader"}, waterBindlessShaderConfig, {{0, sceneWideDS}})
        .createPipeline(m_emptyWaterVAO, m_forwardRenderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [this](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(0, waterBuffer.waterDataBuffer)
                .ssbo(1, wmoBuffers.wmoPlacementMats)
                .ssbo(2, waterBuffer.waterBindlessBuffer);

            waterDataDS = ds;
        })
        .createDescriptorSet(2, [this](std::shared_ptr<GDescriptorSet> &ds) {
            waterTexturesDS = ds;
        }).toMaterial();;

    waterTextureHolder = std::make_shared<BindlessTextureHolder>(waterTexturesBindlessCount);
}
void MapSceneRenderBindlessVLK::createWMOGlobalMaterialData() {
    PipelineTemplate pipelineTemplate;
    pipelineTemplate.element = DrawElementMode::TRIANGLES;
    pipelineTemplate.depthWrite = true;
    pipelineTemplate.depthCulling = true;
    pipelineTemplate.backFaceCulling = true;
    pipelineTemplate.blendMode = EGxBlendEnum::GxBlend_Opaque;

    //Create global wmo descriptor for bindless textures
    g_wmoMaterial = MaterialBuilderVLK::fromShader(m_device, {"wmoShader", "wmoShader"}, wmoBindlessShaderConfig, {{0, sceneWideDS}})
        .createPipeline(m_emptyWMOVAO, m_forwardRenderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [&](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(1, wmoBuffers.wmoPlacementMats)
                .ssbo(2, wmoBuffers.wmoMeshWideVSes)
                .ssbo(3, wmoBuffers.wmoMeshWidePSes)
                .ssbo(4, wmoBuffers.wmoMeshWideBindless)
                .ssbo(5, wmoBuffers.wmoGroupAmbient)
                .ssbo(6, wmoBuffers.wmoPerMeshData);

            wmoBufferOneDS = ds;
        })
        .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
            wmoTexturesDS = ds;
        }).toMaterial();
    wmoTextureHolder = std::make_shared<BindlessTextureHolder>(m2TexturesBindlessCount);
}

void MapSceneRenderBindlessVLK::createM2GlobalMaterialData() {
    PipelineTemplate pipelineTemplate;
    pipelineTemplate.element = DrawElementMode::TRIANGLES;
    pipelineTemplate.depthWrite = true;
    pipelineTemplate.depthCulling = true;
    pipelineTemplate.backFaceCulling = true;
    pipelineTemplate.blendMode = EGxBlendEnum::GxBlend_Opaque;

    //Create global m2 descriptor for bindless textures
    g_m2Material = MaterialBuilderVLK::fromShader(m_device, {"m2Shader", "m2Shader"}, m2BindlessShaderConfig, {{0, sceneWideDS}})
        .createPipeline(m_emptyM2VAO, m_forwardRenderPass, pipelineTemplate)
        .createGBufferPipeline(m_emptyM2VAO, {"m2Shader", "m2Shader"}, m2BindlessGBufferShaderConfig, m_gBufferPass)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [&](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(1, m2Buffers.placementMatrix)
                .ssbo(2, m2Buffers.modelFragmentDatas)
                .ssbo(3, m2Buffers.boneMatrix)
                .ssbo(4, m2Buffers.m2Colors)
                .ssbo(5, m2Buffers.textureWeights)
                .ssbo(6, m2Buffers.textureMatrices)
                .ssbo(7, m2Buffers.m2InstanceData)
                .ssbo(8, m2Buffers.meshWideBlocks)
                .ssbo(9, m2Buffers.meshWideBlocksBindless);

            m2BufferOneDS = ds;
        })
        .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
            m2TextureDS = ds;
        }).toMaterial();
    m2TextureHolder = std::make_shared<BindlessTextureHolder>(m2TexturesBindlessCount);
}
void MapSceneRenderBindlessVLK::createM2WaterfallGlobalMaterialData() {
    MaterialBuilderVLK::fromShader(m_device, {"waterfallShader", "waterfallShader"}, m2WaterfallBindlessShaderConfig, {{0, sceneWideDS}, {1, m2BufferOneDS}})
        .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(0, m2WaterfallBuffer.waterfallCommon)
                .ssbo(1, m2WaterfallBuffer.waterfallBindless);

            m2WaterfallBufferDS = ds;
        })
        .createDescriptorSet(3, [&](std::shared_ptr<GDescriptorSet> &ds) {
            m2WaterfallTextureDS = ds;
        });

    m2WaterfallTextureHolder = std::make_shared<BindlessTextureHolder>(m2WaterfallTexturesBindlessCount);
}

std::shared_ptr<ISimpleMaterialVLK> MapSceneRenderBindlessVLK::getM2StaticMaterial(const PipelineTemplate &pipelineTemplate) {
    auto i = m_m2StaticMaterials.find(pipelineTemplate);
    if (i != m_m2StaticMaterials.end()) {
        return i->second;
    }

    bool isOpaq = pipelineTemplate.blendMode == EGxBlendEnum::GxBlend_Opaque ||
                  pipelineTemplate.blendMode == EGxBlendEnum::GxBlend_AlphaKey;

    bool isTrueOpaq = pipelineTemplate.blendMode == EGxBlendEnum::GxBlend_Opaque;

    auto staticMaterial =
        MaterialBuilderVLK::fromShader(m_device, {"m2Shader", isTrueOpaq ? "m2Shader_opaq" : "m2Shader"}, m2BindlessShaderConfig, {{0, sceneWideDS}})
            .setMaterialId(generateUniqueM2MatId())
            .createPipeline(m_emptyM2VAO, m_forwardRenderPass, pipelineTemplate)
            .createGBufferPipeline(m_emptyM2VAO, {"m2Shader", isTrueOpaq ? "m2Shader_opaq" : "m2Shader"}, m2BindlessGBufferShaderConfig, m_gBufferPass)
            .bindDescriptorSet(0, sceneWideDS)
            .bindDescriptorSet(1, m2BufferOneDS)
            .bindDescriptorSet(2, m2TextureDS)
            .toMaterial();

    m_m2StaticMaterials[pipelineTemplate] = staticMaterial;
    m_m2MatCacheId[staticMaterial->getMaterialId()] = staticMaterial;

    return staticMaterial;
}

std::shared_ptr<ISimpleMaterialVLK> MapSceneRenderBindlessVLK::getWMOStaticMaterial(const PipelineTemplate &pipelineTemplate) {
    auto i = m_wmoStaticMaterials.find(pipelineTemplate);
    if (i != m_wmoStaticMaterials.end()) {
        return i->second;
    }

    bool isTrueOpaq = pipelineTemplate.blendMode == EGxBlendEnum::GxBlend_Opaque;

    auto staticMaterial =
        MaterialBuilderVLK::fromShader(m_device, {"wmoShader", isTrueOpaq ? "wmoShader_opaq" : "wmoShader"}, wmoBindlessShaderConfig, {{0, sceneWideDS}})
            .setMaterialId(generateUniqueWMOMatId())
            .createPipeline(m_emptyWMOVAO, m_forwardRenderPass, pipelineTemplate)
            .createGBufferPipeline(m_emptyWMOVAO, {"wmoShader", isTrueOpaq ? "wmoShader_opaq" : "wmoShader"}, wmoBindlessGBufferShaderConfig, m_gBufferPass)
            .bindDescriptorSet(0, sceneWideDS)
            .bindDescriptorSet(1, wmoBufferOneDS)
            .bindDescriptorSet(2, wmoTexturesDS)
            .toMaterial();

    m_wmoStaticMaterials[pipelineTemplate] = staticMaterial;
    m_wmoMatCacheId[staticMaterial->getMaterialId()] = staticMaterial;

    return staticMaterial;
}

// ------------------
// Buffer creation
// ------------------

HGVertexBufferBindings MapSceneRenderBindlessVLK::createADTVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto adtVAO = m_device->createVertexBufferBindings();
    adtVAO->addVertexBufferBinding(vertexBuffer, adtVertexBufferBinding);
    adtVAO->setIndexBuffer(indexBuffer);

    return adtVAO;
};

HGVertexBufferBindings MapSceneRenderBindlessVLK::createWmoVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer, const std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> &ambientBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto wmoVAO = m_device->createVertexBufferBindings();

    wmoVAO->addVertexBufferBinding(vertexBuffer, std::vector(staticWMOBindings.begin(), staticWMOBindings.end()));
    wmoVAO->setIndexBuffer(indexBuffer);

    return wmoVAO;
}
HGVertexBufferBindings MapSceneRenderBindlessVLK::createM2VAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto m2VAO = m_device->createVertexBufferBindings();
    m2VAO->addVertexBufferBinding(vertexBuffer, staticM2Bindings);
    m2VAO->setIndexBuffer(indexBuffer);

    return m2VAO;
}
HGVertexBufferBindings MapSceneRenderBindlessVLK::createM2ParticleVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto m2ParticleVAO = m_device->createVertexBufferBindings();
    m2ParticleVAO->addVertexBufferBinding(vertexBuffer, staticM2ParticleBindings);
    m2ParticleVAO->setIndexBuffer(indexBuffer);

    return m2ParticleVAO;
}

HGVertexBufferBindings MapSceneRenderBindlessVLK::createM2RibbonVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto m2RibbonVAO = m_device->createVertexBufferBindings();
    m2RibbonVAO->addVertexBufferBinding(vertexBuffer, staticM2RibbonBindings);
    m2RibbonVAO->setIndexBuffer(indexBuffer);

    return m2RibbonVAO;
};

HGVertexBufferBindings MapSceneRenderBindlessVLK::createWaterVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto waterVAO = m_device->createVertexBufferBindings();
    waterVAO->addVertexBufferBinding(vertexBuffer, std::vector(staticWaterBindings.begin(), staticWaterBindings.end()));
    waterVAO->setIndexBuffer(indexBuffer);

    return waterVAO;
};

HGVertexBufferBindings MapSceneRenderBindlessVLK::createSkyVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto skyVAO = m_device->createVertexBufferBindings();
    skyVAO->addVertexBufferBinding(vertexBuffer, std::vector(skyConusBinding.begin(), skyConusBinding.end()));
    skyVAO->setIndexBuffer(indexBuffer);

    return skyVAO;
}

HGVertexBufferBindings MapSceneRenderBindlessVLK::createPortalVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto portalVAO = m_device->createVertexBufferBindings();
    portalVAO->addVertexBufferBinding(vertexBuffer, std::vector(drawPortalBindings.begin(), drawPortalBindings.end()));
    portalVAO->setIndexBuffer(indexBuffer);

    return portalVAO;
};

HGVertexBuffer MapSceneRenderBindlessVLK::createPortalVertexBuffer(int sizeInBytes) {
    return vboPortalBuffer->getSubBuffer(sizeInBytes);
};
HGIndexBuffer  MapSceneRenderBindlessVLK::createPortalIndexBuffer(int sizeInBytes){
    return iboBuffer->getSubBuffer(sizeInBytes);
};

HGVertexBuffer MapSceneRenderBindlessVLK::createM2VertexBuffer(int sizeInBytes) {
    return vboM2Buffer->getSubBuffer(sizeInBytes);
}

HGVertexBuffer MapSceneRenderBindlessVLK::createM2ParticleVertexBuffer(int sizeInBytes, int frameIndex) {
    if (frameIndex >= PARTICLES_BUFF_NUM) return nullptr;

//    auto currentProcessingFrameNumber = m_device->getCurrentProcessingFrameNumber();
//    std::cout << "createM2ParticleVertexBuffer"
//        << " frameIndex="<<frameIndex
//        <<" currentProcessingFrameNumber ="<<currentProcessingFrameNumber
//        <<" sizeInBytes ="<<sizeInBytes
//        <<std::endl;

    return vboM2ParticleBuffers[frameIndex]->getSubBuffer(sizeInBytes);
}
HGVertexBuffer MapSceneRenderBindlessVLK::createM2RibbonVertexBuffer(int sizeInBytes) {
    return vboM2RibbonBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer MapSceneRenderBindlessVLK::createM2IndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer MapSceneRenderBindlessVLK::getOrCreateM2ParticleIndexBuffer() {
    return m_particleIndexBuffer;
}


HGVertexBuffer MapSceneRenderBindlessVLK::createADTVertexBuffer(int sizeInBytes) {
    return vboAdtBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer MapSceneRenderBindlessVLK::createADTIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}

HGVertexBuffer MapSceneRenderBindlessVLK::createWMOVertexBuffer(int sizeInBytes) {
    return vboWMOBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer MapSceneRenderBindlessVLK::createWMOIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}

HGVertexBuffer MapSceneRenderBindlessVLK::createWaterVertexBuffer(int sizeInBytes) {
    return vboWaterBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer MapSceneRenderBindlessVLK::createWaterIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}
HGVertexBuffer MapSceneRenderBindlessVLK::createSkyVertexBuffer(int sizeInBytes) {
    return vboSkyBuffer->getSubBuffer(sizeInBytes);;
};

HGIndexBuffer  MapSceneRenderBindlessVLK::createSkyIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}



std::shared_ptr<IADTMaterial>
MapSceneRenderBindlessVLK::createAdtMaterial(const PipelineTemplate &pipelineTemplate, const ADTMaterialTemplate &adtMaterialTemplate) {
    ZoneScoped;
    auto &l_sceneWideChunk = sceneWideChunk;
    auto vertexFragmentData = std::make_shared<CBufferChunkVLK<ADT::meshWideBlockVSPS>>(adtBuffers.adtMeshWideVSPSes);
    auto fragmentData = std::make_shared<CBufferChunkVLK<ADT::meshWideBlockPS>>(adtBuffers.adtMeshWidePSes);
    auto instanceData = std::make_shared<CBufferChunkVLK<ADT::AdtInstanceData>>(adtBuffers.adtInstanceDatas);

    auto material = MaterialBuilderVLK::fromMaterial(m_device, g_adtMaterial)
        .toMaterial<IADTMaterialVis>([&fragmentData, &vertexFragmentData, &instanceData](IADTMaterialVis *instance) -> void {
            instance->m_materialVSPS = vertexFragmentData;
            instance->m_materialPS = fragmentData;
            instance->m_instanceData = instanceData;
        });

    {
        auto &adtInstanceData = instanceData->getObject();
        adtInstanceData.meshIndexVSPS = vertexFragmentData->getSubBuffer()->getIndex();
        adtInstanceData.meshIndexPS = fragmentData->getSubBuffer()->getIndex();
        {
            auto dsUpdate = adtLayerTextureDS->beginUpdate();

            for (int i = 0; i < 4; i++) {
                auto bindlessText = adtLayerTextureHolder->allocate(adtMaterialTemplate.textures[i]);
                adtInstanceData.LayerIndexes[i] = bindlessText->getIndex();
                material->m_bindlessText.push_back(bindlessText);
                dsUpdate.texture(0, adtMaterialTemplate.textures[i], bindlessText->getIndex());
            }
        }

        {
            auto dsUpdate = adtAlphaTextureDS->beginUpdate();

            for (int i = 4; i <= 4; i++) {
                auto bindlessText = adtAlphaTextureHolder->allocate(adtMaterialTemplate.textures[i]);
                adtInstanceData.AlphaTextureInd = bindlessText->getIndex();
                material->m_bindlessText.push_back(bindlessText);
                dsUpdate.texture(0, adtMaterialTemplate.textures[i], bindlessText->getIndex());
            }
        }

        {
            auto dsUpdate = adtHeightLayerTextureDS->beginUpdate();

            for (int i = 5; i < 9; i++) {
                auto bindlessText = adtHeightLayerTextureHolder->allocate(adtMaterialTemplate.textures[i]);
                adtInstanceData.LayerHeight[i-5] = bindlessText->getIndex();
                material->m_bindlessText.push_back(bindlessText);
                dsUpdate.texture(0, adtMaterialTemplate.textures[i], bindlessText->getIndex());
            }
        }
        instanceData->save();
    }

    material->instanceIndex = instanceData->getSubBuffer()->getIndex();

    return material;
}

std::shared_ptr<IM2Material>
MapSceneRenderBindlessVLK::createM2Material(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                            const PipelineTemplate &pipelineTemplate,
                                            const M2MaterialTemplate &m2MaterialTemplate) {
    ZoneScoped;

    auto m2ModelDataVisVLK = std::dynamic_pointer_cast<IM2ModelData>(m2ModelData);

    auto vertexFragmentDataBindless = std::make_shared<CBufferChunkVLK<M2::meshWideBlockVSPS_Bindless>>(m2Buffers.meshWideBlocksBindless);
    auto vertexFragmentData = std::make_shared<CBufferChunkVLK<M2::meshWideBlockVSPS>>(m2Buffers.meshWideBlocks);

    auto staticMaterial = getM2StaticMaterial(pipelineTemplate);

    auto material = MaterialBuilderVLK::fromMaterial(m_device, staticMaterial)
        .toMaterial<IM2MaterialVis>([&vertexFragmentDataBindless, &vertexFragmentData](IM2MaterialVis *instance) -> void {
            instance->m_vertexFragmentDataBindless = vertexFragmentDataBindless;
            instance->m_vertexFragmentData = vertexFragmentData;
        });
    {
        auto dsUpdate = m2TextureDS->beginUpdate();

        for (int i = 0; i < 4; i++) {
            auto bindlessText = m2TextureHolder->allocate(m2MaterialTemplate.textures[i]);
            material->m_bindlessText.push_back(bindlessText);
            dsUpdate.texture(0, m2MaterialTemplate.textures[i], bindlessText->getIndex());
        }
    }

    {
        auto &modelFragmentDataVis = vertexFragmentDataBindless->getObject();

        modelFragmentDataVis.instanceIndex =
            BufferChunkHelperVLK::castToChunk(m2ModelDataVisVLK->m_instanceBindless)->getSubBuffer()->getIndex();
        modelFragmentDataVis.meshIndex =
            vertexFragmentData->getSubBuffer()->getIndex();
        for (int i = 0; i < 4; i++) {
            modelFragmentDataVis.textureIndicies[i] = material->m_bindlessText[i]->getIndex();
        }

        vertexFragmentDataBindless->save();
    }

    material->instanceIndex = vertexFragmentDataBindless->getSubBuffer()->getIndex();

    material->blendMode = pipelineTemplate.blendMode;
    material->depthWrite = pipelineTemplate.depthWrite;
    material->depthCulling = pipelineTemplate.depthCulling;
    material->backFaceCulling = pipelineTemplate.backFaceCulling;

    material->batchIndex = m2MaterialTemplate.batchIndex;
    material->vertexShader = m2MaterialTemplate.vertexShader;
    material->pixelShader = m2MaterialTemplate.pixelShader;


    return material;
}

std::shared_ptr<IM2WaterFallMaterial> MapSceneRenderBindlessVLK::createM2WaterfallMaterial(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                                                           const PipelineTemplate &pipelineTemplate,
                                                                                           const M2WaterfallMaterialTemplate &m2MaterialTemplate) {
    auto m2ModelDataVisVLK = std::dynamic_pointer_cast<IM2ModelData>(m2ModelData);

    auto &l_sceneWideChunk = sceneWideChunk;
    auto commonData = std::make_shared<CBufferChunkVLK<M2::WaterfallData::WaterfallCommon>>(
        m2WaterfallBuffer.waterfallCommon);
    auto bindlessData = std::make_shared<CBufferChunkVLK<M2::WaterfallData::WaterfallBindless>>(
        m2WaterfallBuffer.waterfallBindless);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"waterfallShader", "waterfallShader"},
                                                   m2WaterfallBindlessShaderConfig, {{0, sceneWideDS}, {1, m2BufferOneDS}})
        .createPipeline(m_emptyM2VAO, m_forwardRenderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .bindDescriptorSet(1, m2BufferOneDS)
        .bindDescriptorSet(2, m2WaterfallBufferDS)
        .bindDescriptorSet(3, m2WaterfallTextureDS)
        .toMaterial<IM2WaterFallMaterialBindless>(
            [&commonData, &bindlessData](IM2WaterFallMaterialBindless *instance) -> void {
                instance->m_waterfallCommon = commonData;
                instance->m_waterfallBindless = bindlessData;
            });

    {
        auto dsUpdate = m2WaterfallTextureDS->beginUpdate();

        for (int i = 0; i < 5; i++) {
            auto bindlessText = m2WaterfallTextureHolder->allocate(m2MaterialTemplate.textures[i]);
            material->m_bindlessText.push_back(bindlessText);
            dsUpdate.texture(0, m2MaterialTemplate.textures[i], bindlessText->getIndex());
        }
    }

    {
        auto &bindless = bindlessData->getObject();
        bindless.instanceIndex = BufferChunkHelperVLK::castToChunk(m2ModelDataVisVLK->m_instanceBindless)->getSubBuffer()->getIndex();
        bindless.waterfallInd = commonData->getIndex();

        bindless.maskInd =        material->m_bindlessText[0]->getIndex();
        bindless.whiteWaterInd =  material->m_bindlessText[1]->getIndex();
        bindless.noiseInd =       material->m_bindlessText[2]->getIndex();
        bindless.bumpTextureInd = material->m_bindlessText[3]->getIndex();
        bindless.normalTexInd =   material->m_bindlessText[4]->getIndex();


        bindlessData->save();
    }

    material->instanceIndex = bindlessData->getIndex();

    return material;
}

std::shared_ptr<IM2ParticleMaterial> MapSceneRenderBindlessVLK::createM2ParticleMaterial(
    const PipelineTemplate &pipelineTemplate,
    const M2ParticleMaterialTemplate &m2ParticleMatTemplate) {

    auto &l_sceneWideChunk = sceneWideChunk;
    auto l_fragmentData = std::make_shared<CBufferChunkVLK<Particle::meshParticleWideBlockPS>>(uboBuffer); ;

    bool isOpaq = pipelineTemplate.blendMode == EGxBlendEnum::GxBlend_Opaque ||
                  pipelineTemplate.blendMode == EGxBlendEnum::GxBlend_AlphaKey;

    auto material = MaterialBuilderVLK::fromShader(m_device, {"m2Particle/forward/m2ParticleShader", "m2Particle/forward/m2ParticleShader"}, bindlessShaderConfig, {{0, sceneWideDS}})
        .createPipeline(m_emptyM2ParticleVAO, m_forwardRenderPass, pipelineTemplate)
        .createGBufferPipeline(m_emptyM2ParticleVAO, {"m2ParticleShader", "m2ParticleShader"}, m2ParticlesBindlessGBufferShaderConfig, m_gBufferPass)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [&l_fragmentData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, *l_fragmentData).delayUpdate();
        })
        .createDescriptorSet(2, [&m2ParticleMatTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(0, m2ParticleMatTemplate.textures[0])
                .texture(1, m2ParticleMatTemplate.textures[1])
                .texture(2, m2ParticleMatTemplate.textures[2]);
        })
        .toMaterial<IM2ParticleMaterial>([l_fragmentData](IM2ParticleMaterial *instance) -> void {
            instance->m_fragmentData = l_fragmentData;
        });

    return material;
}

std::shared_ptr<IM2RibbonMaterial> MapSceneRenderBindlessVLK::createM2RibbonMaterial(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                                                     const PipelineTemplate &pipelineTemplate,
                                                                                     const M2RibbonMaterialTemplate &m2RibbonMaterialTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto l_fragmentData = std::make_shared<CBufferChunkVLK<Ribbon::meshRibbonWideBlockPS>>(uboBuffer); ;
    auto &l_m2ModelData = m2ModelData;

    auto material = MaterialBuilderVLK::fromShader(m_device, {"m2Ribbon/forward/ribbonShader", "m2Ribbon/forward/ribbonShader"}, bindlessShaderConfig, {{0, sceneWideDS}})
        .createPipeline(m_emptyM2RibbonVAO,  m_forwardRenderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [&l_sceneWideChunk, &l_fragmentData, &l_m2ModelData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(0, BufferChunkHelperVLK::cast(l_m2ModelData->m_textureMatrices))
                .ubo(1, *l_fragmentData).delayUpdate();
        })
        .createDescriptorSet(2, [&m2RibbonMaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(0, m2RibbonMaterialTemplate.textures[0]);
        })
        .toMaterial<IM2RibbonMaterial>([l_fragmentData](IM2RibbonMaterial *instance) -> void {
            instance->m_fragmentData = l_fragmentData;
        });

    return material;
};

std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> MapSceneRenderBindlessVLK::createWMOWideChunk() {
    return std::make_shared<CBufferChunkVLK<WMO::modelWideBlockVS>>(wmoBuffers.wmoPlacementMats);
}
std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> MapSceneRenderBindlessVLK::createWMOGroupAmbientChunk() {
    return std::make_shared<CBufferChunkVLK<mathfu::vec4_packed>>(wmoBuffers.wmoGroupAmbient);
}

std::shared_ptr<IWMOMaterial> MapSceneRenderBindlessVLK::createWMOMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
                                                                           const PipelineTemplate &pipelineTemplate,
                                                                           const WMOMaterialTemplate &wmoMaterialTemplate) {
    auto l_vertexData = std::make_shared<CBufferChunkVLK<WMO::meshWideBlockVS>>(wmoBuffers.wmoMeshWideVSes); ;
    auto l_fragmentData = std::make_shared<CBufferChunkVLK<WMO::meshWideBlockPS>>(wmoBuffers.wmoMeshWidePSes); ;
    auto l_bindless = std::make_shared<CBufferChunkVLK<WMO::meshWideBlockBindless>>(wmoBuffers.wmoMeshWideBindless);

    auto staticMaterial = getWMOStaticMaterial(pipelineTemplate);

    auto material = MaterialBuilderVLK::fromMaterial(m_device, staticMaterial)
        .toMaterial<IWMOMaterialVis>([&l_vertexData, &l_fragmentData, &l_bindless](IWMOMaterialVis *instance) -> void {
            instance->m_materialPS = l_fragmentData;
            instance->m_materialVS = l_vertexData;
            instance->m_meshBindless = l_bindless;
        });
    {
        auto dsUpdate = wmoTexturesDS->beginUpdate();

        for (int i = 0; i < 9; i++) {
            auto bindlessText = wmoTextureHolder->allocate(wmoMaterialTemplate.textures[i]);
            material->m_bindlessText.push_back(bindlessText);
            dsUpdate.texture(0, wmoMaterialTemplate.textures[i], bindlessText->getIndex());
        }
    }

    {
        auto &bindless = l_bindless->getObject();
        bindless.blockVSIndex = l_vertexData->getIndex();
        bindless.meshWideIndex = l_fragmentData->getIndex();
        bindless.placementMat = BufferChunkHelperVLK::cast(modelWide)->getIndex();

        bindless.texture1 = material->m_bindlessText[0]->getIndex();
        bindless.texture2 = material->m_bindlessText[1]->getIndex();
        bindless.texture3 = material->m_bindlessText[2]->getIndex();
        bindless.texture4 = material->m_bindlessText[3]->getIndex();
        bindless.texture5 = material->m_bindlessText[4]->getIndex();
        bindless.texture6 = material->m_bindlessText[5]->getIndex();
        bindless.texture7 = material->m_bindlessText[6]->getIndex();
        bindless.texture8 = material->m_bindlessText[7]->getIndex();
        bindless.texture9 = material->m_bindlessText[8]->getIndex();

        l_bindless->save();
    }

    material->meshWideBindlessIndex = l_bindless->getIndex();

    return material;
}

std::shared_ptr<IWaterMaterial> MapSceneRenderBindlessVLK::createWaterMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
                                                                               const PipelineTemplate &pipelineTemplate,
                                                                               const WaterMaterialTemplate &waterMaterialTemplate) {
    auto l_fragmentData = std::make_shared<CBufferChunkVLK<Water::meshWideBlockPS>>(waterBuffer.waterDataBuffer); ;
    auto l_waterBindless = std::make_shared<CBufferChunkVLK<Water::WaterBindless>>(waterBuffer.waterBindlessBuffer); ;

    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"waterShader", "waterShader"}, waterBindlessShaderConfig, {{0, sceneWideDS}})
        .createPipeline(m_emptyWaterVAO, m_forwardRenderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .bindDescriptorSet(1, waterDataDS)
        .bindDescriptorSet(2, waterTexturesDS)
        .toMaterial<IWaterMaterialBindless>([&l_fragmentData, l_waterBindless](IWaterMaterialBindless *instance) -> void {
            instance->m_materialPS = l_fragmentData;
            instance->m_bindless = l_waterBindless;
        });

    material->color = waterMaterialTemplate.color;
    material->liquidFlags = waterMaterialTemplate.liquidFlags;
    material->materialId = waterMaterialTemplate.liquidMaterialId;

    {
        auto dsUpdate = waterTexturesDS->beginUpdate();

//        for (int i = 0; i < 9; i++) {
            auto bindlessText = waterTextureHolder->allocate(waterMaterialTemplate.texture);
            material->m_bindlessText.push_back(bindlessText);
            dsUpdate.texture(0, waterMaterialTemplate.texture, bindlessText->getIndex());
//        }
    }

    {
        auto &bindless = l_waterBindless->getObject();
        bindless.placementMatInd = BufferChunkHelperVLK::cast(modelWide)->getIndex();
        bindless.textureInd = material->m_bindlessText[0]->getIndex();
        bindless.waterDataInd = l_fragmentData->getIndex();

        l_waterBindless->save();
    }

    material->instanceIndex = l_waterBindless->getIndex();

    return material;
}



std::shared_ptr<ISkyMeshMaterial> MapSceneRenderBindlessVLK::createSkyMeshMaterial(const PipelineTemplate &pipelineTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto skyColors = std::make_shared<CBufferChunkVLK<DnSky::meshWideBlockVS>>(uboBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"skyConus", "skyConus"}, forwardShaderConfig, {{0, sceneWideDS}})
        .createPipeline(m_emptySkyVAO, m_forwardRenderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [&skyColors, &l_sceneWideChunk](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, *skyColors).delayUpdate();
        })
        .toMaterial<ISkyMeshMaterial>([&skyColors](ISkyMeshMaterial *instance) -> void {
            instance->m_skyColors = skyColors;
        });

    return material;
}

std::shared_ptr<IPortalMaterial> MapSceneRenderBindlessVLK::createPortalMaterial(const PipelineTemplate &pipelineTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto materialPS = std::make_shared<CBufferChunkVLK<DrawPortalShader::meshWideBlockPS>>(uboBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"drawPortalShader", "drawPortalShader"}, forwardShaderConfig, {{0, sceneWideDS}})
        .createPipeline(m_emptyPortalVAO, m_forwardRenderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [&materialPS, &l_sceneWideChunk](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, *materialPS).delayUpdate();
        })
        .toMaterial<IPortalMaterial>([&materialPS](IPortalMaterial *instance) -> void {
            instance->m_materialPS = materialPS;
        });

    return material;
}

std::shared_ptr<IM2ModelData> MapSceneRenderBindlessVLK::createM2ModelMat(int bonesCount, int m2ColorsCount, int textureWeightsCount, int textureMatricesCount) {
    auto result = std::make_shared<IM2ModelData>();

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

inline void MapSceneRenderBindlessVLK::drawMesh(CmdBufRecorder &cmdBuf, const HGMesh &mesh, CmdBufRecorder::ViewportType viewportType ) {
    if (mesh == nullptr) return;

    const auto &meshVlk = (GMeshVLK*) mesh.get();

    //1. Bind Vertex bindings
    cmdBuf.bindVertexBindings(mesh->bindings());

    //2. Bind Material
    cmdBuf.bindMaterial(meshVlk->material());

    //3. Set view port
    cmdBuf.setViewPort(viewportType);

    //4. Set scissors
    if (meshVlk->scissorEnabled()) {
        cmdBuf.setScissors(meshVlk->scissorOffset(), meshVlk->scissorSize());
    } else {
        cmdBuf.setDefaultScissors();
    }

    //5. Draw the mesh
    if (meshVlk->instanceIndex != -1) {
        cmdBuf.drawIndexed(meshVlk->end(), 1, meshVlk->start() / 2, meshVlk->instanceIndex, meshVlk->vertexStart);
    } else {
        cmdBuf.drawIndexed(meshVlk->end(), 1, meshVlk->start() / 2, 0);
    }
}

static inline std::array<float,3> vec4ToArr3(const mathfu::vec4 &vec) {
    return {vec[0], vec[1], vec[2]};
}

class COpaqueMeshCollectorBindlessVLK : public COpaqueMeshCollector{
public:
    COpaqueMeshCollectorBindlessVLK(MapSceneRenderBindlessVLK &rendererVlk) : m_renderer(rendererVlk) {
        commonMeshes.reserve(1000);
        m2DrawVec.reserve(5000);
        wmoDrawVec.reserve(5000);
        adtDrawVec.reserve(1000);
    }
    COpaqueMeshCollectorBindlessVLK(MapSceneRenderBindlessVLK &rendererVlk, MeshCount &lastMeshCount) : m_renderer(rendererVlk) {
        commonMeshes.reserve(lastMeshCount.commonMesh);
        m2DrawVec.reserve(lastMeshCount.m2Mesh);
        wmoDrawVec.reserve(lastMeshCount.wmoMesh);
        adtDrawVec.reserve(lastMeshCount.adtMesh);
    }
private:
    MapSceneRenderBindlessVLK &m_renderer;
    struct DrawCommand {
        uint32_t matId;
        uint32_t indexCount;
        uint32_t instanceCount;
        uint32_t firstIndex;
        uint32_t firstInstance;
        uint32_t vertexOffset;
    };

    typedef robin_hood::unordered_flat_map<std::shared_ptr<GPipelineVLK>, std::vector<DrawCommand>> MeshMap;

    framebased::vector<DrawCommand> m2DrawVec;
    framebased::vector<DrawCommand> wmoDrawVec;
    framebased::vector<DrawCommand> waterMeshVec;
    framebased::vector<DrawCommand> adtDrawVec;
    framebased::vector<HGMesh> commonMeshes;

     static void fillDrawCommand(DrawCommand &drawCommand, GMeshVLK *meshVlk) {
        auto const matId = meshVlk->material()->getMaterialId();

        drawCommand.matId = matId;
        drawCommand.indexCount = meshVlk->end();
        drawCommand.instanceCount = 1;
        drawCommand.firstIndex = meshVlk->start() / 2;

        if (meshVlk->instanceIndex != -1) {
            drawCommand.firstInstance = meshVlk->instanceIndex;
            drawCommand.vertexOffset = meshVlk->vertexStart;
        } else {
            drawCommand.firstInstance = 0;
            drawCommand.vertexOffset = 0;
        }
    }

    static void fillMapWithMesh(MeshMap &meshMap, const HGMesh &mesh) {
        auto meshId = mesh->getObjectId();
        const auto meshVlk = bindlessMeshFactoryVlk.getObjectById<0>(meshId);
//        const auto &meshVlk = (GMeshVLK*) mesh.get();
        auto const &pipeline = meshVlk->material()->getPipeline();

        auto &vec = meshMap[pipeline];
        if (vec.empty())
            vec.reserve(3000);

        auto &drawCommand = meshMap[pipeline].emplace_back();
        fillDrawCommand(drawCommand, meshVlk);
    }

    inline void addDrawCommand(framebased::vector<DrawCommand> &drawVec, const HGMesh &mesh) {
        auto meshId = mesh->getObjectId();
        auto meshVlk = bindlessMeshFactoryVlk.getObjectById<0>(meshId);

        auto &drawCommand = drawVec.emplace_back();
        fillDrawCommand(drawCommand, meshVlk);
    }
    inline void addDrawCommand(tbb::concurrent_vector<DrawCommand> &drawVec, const HGMesh &mesh) {
        auto meshId = mesh->getObjectId();
        auto meshVlk = bindlessMeshFactoryVlk.getObjectById<0>(meshId);

        DrawCommand drawCommand;
        fillDrawCommand(drawCommand, meshVlk);
        drawVec.push_back(drawCommand);
    }
public:
    void addM2Mesh(const HGM2Mesh &mesh) override {
        addDrawCommand(m2DrawVec, mesh);
    };
    void addWMOMesh(const HGMesh &mesh) override {
        addDrawCommand(wmoDrawVec, mesh);
    } ;
    void addWaterMesh(const HGMesh &mesh) override {
        addDrawCommand(waterMeshVec, mesh);
    } ;
    void addADTMesh(const HGMesh &mesh) override {
        addDrawCommand(adtDrawVec, mesh);
    } ;

    void addMesh(const HGMesh &mesh) override {
        commonMeshes.push_back(mesh);
    };

    virtual COpaqueMeshCollector * clone() {
        return new COpaqueMeshCollectorBindlessVLK(m_renderer);
    }

    void merge(COpaqueMeshCollector & collector) override {
        auto l_collector = (COpaqueMeshCollectorBindlessVLK &) collector;

        m2DrawVec.insert(m2DrawVec.end(), l_collector.m2DrawVec.begin(), l_collector.m2DrawVec.end());
        wmoDrawVec.insert(wmoDrawVec.end(), l_collector.wmoDrawVec.begin(), l_collector.wmoDrawVec.end());
        adtDrawVec.insert(adtDrawVec.end(), l_collector.adtDrawVec.begin(), l_collector.adtDrawVec.end());
        commonMeshes.insert(commonMeshes.end(), l_collector.commonMeshes.begin(), l_collector.commonMeshes.end());
        waterMeshVec.insert(waterMeshVec.end(), l_collector.waterMeshVec.begin(), l_collector.waterMeshVec.end());
    }

    void render(CmdBufRecorder &cmdBuf, CmdBufRecorder::ViewportType viewPortType) {
        std::sort(m2DrawVec.begin(), m2DrawVec.end(), [](DrawCommand const &a, DrawCommand const &b) {
            return a.matId < b.matId;
        });
        std::sort(wmoDrawVec.begin(), wmoDrawVec.end(), [](DrawCommand const &a, DrawCommand const &b) {
            return a.matId < b.matId;
        });

        cmdBuf.setDefaultScissors();
        cmdBuf.setViewPort(viewPortType);

        if (!adtDrawVec.empty())
        {
            //1. Render ADT
            cmdBuf.bindVertexBindings(m_renderer.getDefaultADTVao());
            cmdBuf.bindMaterial(m_renderer.getGlobalADTMaterial());

            for (auto const & drawCmd : adtDrawVec) {
                cmdBuf.drawIndexed(drawCmd.indexCount, drawCmd.instanceCount, drawCmd.firstIndex, drawCmd.firstInstance, drawCmd.vertexOffset);
            }
        }

        if (!wmoDrawVec.empty())
        {
            //2. Render WMO
            cmdBuf.bindVertexBindings(m_renderer.getDefaultWMOVao());

            uint32_t currentMatId = 0xFFFFFFFF;
            for (auto const &drawCmd : wmoDrawVec) {
                if (currentMatId != drawCmd.matId) {
                    auto material = m_renderer.m_wmoMatCacheId[drawCmd.matId].lock();
                    if (!material)
                        continue;

                    cmdBuf.bindMaterial(material);

                    currentMatId = drawCmd.matId;
                }

                cmdBuf.drawIndexed(drawCmd.indexCount, drawCmd.instanceCount, drawCmd.firstIndex, drawCmd.firstInstance, drawCmd.vertexOffset);
            }
        }

        if (!m2DrawVec.empty())
        {
            //3. Render m2
            cmdBuf.bindVertexBindings(m_renderer.getDefaultM2Vao());

            uint32_t currentMatId = 0xFFFFFFFF;
            for (auto const &drawCmd : m2DrawVec) {
                if (currentMatId != drawCmd.matId) {
                    auto material = m_renderer.m_m2MatCacheId[drawCmd.matId].lock();
                    if (!material)
                        continue;

                    cmdBuf.bindMaterial(material);

                    currentMatId = drawCmd.matId;
                }

                cmdBuf.drawIndexed(drawCmd.indexCount, drawCmd.instanceCount, drawCmd.firstIndex, drawCmd.firstInstance, drawCmd.vertexOffset);
            }
        }

        //Render commonMeshes
        for (auto const &mesh : commonMeshes ) {
            MapSceneRenderBindlessVLK::drawMesh(cmdBuf, mesh, viewPortType);
        }
    }
    void renderWater(CmdBufRecorder &cmdBuf, CmdBufRecorder::ViewportType viewPortType) {
        cmdBuf.setDefaultScissors();
        cmdBuf.setViewPort(viewPortType);

        if (!waterMeshVec.empty())
        {
            //4. Render water
            cmdBuf.bindVertexBindings(m_renderer.getDefaultWaterVao());
            auto const &gWaterMat = m_renderer.getGlobalWaterMaterial();

            cmdBuf.bindMaterial(gWaterMat);
            cmdBuf.bindPipeline(gWaterMat->getPipeline());

            for (auto const &drawCmd : waterMeshVec) {
                cmdBuf.drawIndexed(drawCmd.indexCount, drawCmd.instanceCount, drawCmd.firstIndex, drawCmd.firstInstance, drawCmd.vertexOffset);
            }
        }
    }
    void fillMeshCount(MeshCount &meshCount) {
        meshCount.adtMesh = adtDrawVec.size();
        meshCount.m2Mesh = m2DrawVec.size();
        meshCount.wmoMesh = wmoDrawVec.size();
        meshCount.commonMesh = commonMeshes.size();
    }

};

std::unique_ptr<IRenderFunction> MapSceneRenderBindlessVLK::update(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams,
                                                                   const std::shared_ptr<MapRenderPlan> &framePlan) {
    TracyMessageStr(("Update stage frame = " + std::to_string(m_device->getCurrentProcessingFrameNumber())));

    ZoneScoped;
    auto l_this = std::dynamic_pointer_cast<MapSceneRenderBindlessVLK>(this->shared_from_this());
    auto mapScene = std::dynamic_pointer_cast<Map>(frameInputParams->frameParameters->scene);


    //Create meshes
    std::unique_ptr<COpaqueMeshCollectorBindlessVLK> u_collector = std::make_unique<COpaqueMeshCollectorBindlessVLK>(*this, lastMeshCount);
    std::unique_ptr<COpaqueMeshCollectorBindlessVLK> u_skyCollector = std::make_unique<COpaqueMeshCollectorBindlessVLK>(*this);
    auto transparentMeshes = std::make_shared<framebased::vector<HGSortableMesh>>();



    auto skyTransparentMeshes = std::make_shared<framebased::vector<HGSortableMesh>>();
    framePlan->m2Array.lock();
    framePlan->wmoArray.lock();
    framePlan->wmoGroupArray.lock();

    const bool stepBufferUpdates = !m_config->stopBufferUpdates || m_config->stepBufferUpdate;
    m_config->stepBufferUpdate = false;

    if (stepBufferUpdates) {
        mapScene->update(framePlan);
        mapScene->updateBuffers(l_this, framePlan);
    }

    std::vector<RenderingMatAndSceneSize> renderingMatricessAndSizes;
    for (auto &rt : frameInputParams->frameParameters->renderTargets) {
        auto &matAndSceneSize = renderingMatricessAndSizes.emplace_back();
        matAndSceneSize.renderingMat = rt.cameraMatricesForRendering;
        matAndSceneSize.width = rt.viewPortDimensions.maxs[0];
        matAndSceneSize.height = rt.viewPortDimensions.maxs[1];
    }

    if (stepBufferUpdates) {
        updateSceneWideChunk(sceneWideChunk,
                             renderingMatricessAndSizes,
                             framePlan->frameDependentData,
                             true,
                             mapScene->getCurrentSceneTime());
    }


    collectMeshes(framePlan, *u_collector, *u_skyCollector, transparentMeshes, skyTransparentMeshes);
    u_collector->fillMeshCount(lastMeshCount);

//    {
//        ZoneScopedN("collect meshes wait");
//        collectMeshAsync.wait();
//    }

    bool renderSky = framePlan->renderSky;
    auto skyMesh = framePlan->skyMesh;
    auto skyMesh0x4 = framePlan->skyMesh0x4;
    return createRenderFuncVLK([l_this, mapScene, framePlan, transparentMeshes, frameInputParams, stepBufferUpdates](CmdBufRecorder &uploadCmd) -> void {
        {
            ZoneScopedN("Post Load");
            //Do postLoad here. So creation of stuff is done from main thread
            if (stepBufferUpdates)
                mapScene->doPostLoad(l_this, framePlan);

            for (auto &renderTarget : frameInputParams->frameParameters->renderTargets) {
                auto updatingTarget = std::dynamic_pointer_cast<RendererViewClass>(renderTarget.target);
                if (!updatingTarget) updatingTarget = l_this->defaultView;

                updatingTarget->update(
                    renderTarget.viewPortDimensions.maxs[0],
                    renderTarget.viewPortDimensions.maxs[1],
                    framePlan->frameDependentData->currentGlow,
                    framePlan->pointLights,
                    framePlan->spotLights
                );
            }
        }
        {
            ZoneScopedN("Collect Portal Meshes");
            //And add portal meshes
            for (auto const &view: framePlan->viewsHolder.getInteriorViews()) {
                view->collectPortalMeshes(*transparentMeshes);
            }
            {
                auto const &exteriorView = framePlan->viewsHolder.getExterior();
                if (exteriorView != nullptr) {
                    exteriorView->collectPortalMeshes(*transparentMeshes);
                }
            }
        }
        {
            ZoneScopedN("Set Last Created Plan");
            //Needs to be executed only after lock
            l_this->m_lastCreatedPlan = framePlan;
        }
        // ---------------------
        // Upload stuff
        // ---------------------
        if (stepBufferUpdates)
        {
//            ZoneScopedN("submit buffers");
            VkZone(uploadCmd, "submit buffers")
            for (const auto& buffer : l_this->allBuffers) {
                uploadCmd.submitFullMemoryBarrierPreWrite(buffer);
            }

            for (const auto& buffer : l_this->allBuffers) {
                uploadCmd.submitBufferUploads(buffer);
            }

            for (const auto& buffer : l_this->allBuffers) {
                uploadCmd.submitFullMemoryBarrierPostWrite(buffer);
            }

        }
    }, [transparentMeshes, l_opaqueMeshes = std::move(u_collector),
                           l_skyOpaqueMeshes = std::move(u_skyCollector),
                           skyTransparentMeshes,
                                   renderSky,
                                   skyMesh,
                                   skyMesh0x4,
                                   mapScene, framePlan,
                                   l_this, frameInputParams](CmdBufRecorder &frameBufCmd, CmdBufRecorder &swapChainCmd) -> void {

        TracyMessageStr(("Draw stage frame = " + std::to_string(l_this->m_device->getCurrentProcessingFrameNumber())));

        // ----------------------
        // Draw meshes
        // ----------------------
        {
            uint8_t wideChunkVersion = 0;
            for (auto &renderTarget : frameInputParams->frameParameters->renderTargets) {
                l_this->sceneWideChunk->setCurrentVersion(wideChunkVersion++);

                auto currentView = renderTarget.target == nullptr ?
                                   l_this->defaultView :
                                   std::dynamic_pointer_cast<RendererViewClass>(renderTarget.target);

                currentView->setLightBuffers(l_this->sceneWideDS);
                {

                    {
                        auto passHelper = currentView->beginGBufferPass(frameBufCmd, false,
                                                                        frameInputParams->frameParameters->clearColor);
                        frameBufCmd.setGBufferMode(true);
                        {
                            //Opaque part
                            l_this->drawOpaque(frameBufCmd, l_opaqueMeshes);
                        }
                        frameBufCmd.setGBufferMode(false);
                    }
                    currentView->doGBufferBarrier(frameBufCmd);
                    currentView->doLightPass(frameBufCmd);
                    {
                        auto passHelper = currentView->beginForwardPass(frameBufCmd, false, false,
                                                                        frameInputParams->frameParameters->clearColor);

                        l_this->drawOpaque(frameBufCmd, l_opaqueMeshes);

                        {
                            //Sky opaque
                            if (renderSky && skyMesh)
                                MapSceneRenderBindlessVLK::drawMesh(frameBufCmd, skyMesh,
                                                                    CmdBufRecorder::ViewportType::vp_skyBox);

                            l_skyOpaqueMeshes->render(frameBufCmd, CmdBufRecorder::ViewportType::vp_skyBox);
                        }
                        {
                            //Sky transparent
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

                                MapSceneRenderBindlessVLK::drawMesh(frameBufCmd, mesh,
                                                                    CmdBufRecorder::ViewportType::vp_skyBox);
                            }
                            if (renderSky && skyMesh0x4)
                                MapSceneRenderBindlessVLK::drawMesh(frameBufCmd, skyMesh0x4,
                                                                    CmdBufRecorder::ViewportType::vp_skyBox);
                        }
                        {
                            //Render liquids
                            l_opaqueMeshes->renderWater(frameBufCmd, CmdBufRecorder::ViewportType::vp_usual);
                        }
                        {
                            VkZone(frameBufCmd, "render transparent")
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

                                MapSceneRenderBindlessVLK::drawMesh(frameBufCmd, mesh,
                                                                    CmdBufRecorder::ViewportType::vp_usual);
                            }
                        }
                    }
                }
                {
                    currentView->doPostGlow(frameBufCmd);
                    if (currentView == l_this->defaultView) {
                        currentView->doPostFinal(swapChainCmd);
                    } else {
                        currentView->doOutputPass(frameBufCmd);
                    }
                }
            }
        }
    });
}

void MapSceneRenderBindlessVLK::drawOpaque(CmdBufRecorder &frameBufCmd,
                                           const std::unique_ptr<COpaqueMeshCollectorBindlessVLK> &l_opaqueMeshes) {
    {
        ZoneScopedN("submit opaque");
        VkZone(frameBufCmd, "render opaque")
        l_opaqueMeshes->render(frameBufCmd, CmdBufRecorder::ViewportType::vp_usual);
    }
}

std::shared_ptr<MapRenderPlan> MapSceneRenderBindlessVLK::getLastCreatedPlan() {
    return m_lastCreatedPlan;
}

HGMesh MapSceneRenderBindlessVLK::createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) {
    return bindlessMeshFactoryVlk.createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0,0);
}

HGSortableMesh MapSceneRenderBindlessVLK::createSortableMesh(gMeshTemplate &meshTemplate, const HMaterial &material, int priorityPlane) {
    auto mesh = bindlessMeshFactoryVlk.createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0, priorityPlane);
    return mesh;
}

HGSortableMesh MapSceneRenderBindlessVLK::createWaterMesh(gMeshTemplate &meshTemplate, const HMaterial &material, int priorityPlane) {
    auto realVAO = (GVertexBufferBindingsVLK *)meshTemplate.bindings.get();
    meshTemplate.bindings = m_emptyWaterVAO;

    auto _material = std::dynamic_pointer_cast<IWaterMaterialBindless>(material);
    auto mesh = bindlessMeshFactoryVlk.createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0, priorityPlane);

    mesh->instanceIndex = _material->instanceIndex;
    mesh->vertexStart = ((IBufferVLK * )realVAO->getVertexBuffers()[0].get())->getIndex();
    mesh->start() += ((IBufferVLK * )realVAO->getIndexBuffer().get())->getOffset();

    return mesh;
}

HGMesh
MapSceneRenderBindlessVLK::createAdtMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IADTMaterial> &material) {
    ZoneScoped;

    auto realVAO = (GVertexBufferBindingsVLK *)meshTemplate.bindings.get();
    meshTemplate.bindings = m_emptyADTVAO;

    auto mesh = bindlessMeshFactoryVlk.createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0, 0);
    mesh->instanceIndex = std::dynamic_pointer_cast<IADTMaterialVis>(material)->instanceIndex;
    mesh->vertexStart = ((IBufferVLK * )realVAO->getVertexBuffers()[0].get())->getIndex();
    mesh->start() += ((IBufferVLK * )realVAO->getIndexBuffer().get())->getOffset();

    return mesh;
}
HGM2Mesh
MapSceneRenderBindlessVLK::createM2Mesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2Material> &material,
                                        int layer, int priorityPlane) {
    ZoneScoped;
    auto realVAO = (GVertexBufferBindingsVLK *)meshTemplate.bindings.get();
    meshTemplate.bindings = m_emptyM2VAO;
    auto mesh = bindlessMeshFactoryVlk.createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), layer, priorityPlane);
    mesh->instanceIndex = std::dynamic_pointer_cast<IM2MaterialVis>(material)->instanceIndex;
    mesh->vertexStart = ((IBufferVLK * )realVAO->getVertexBuffers()[0].get())->getIndex();
    mesh->start() += ((IBufferVLK * )realVAO->getIndexBuffer().get())->getOffset();

    return mesh;
}
HGM2Mesh
MapSceneRenderBindlessVLK::createM2ParticleMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2Material> &material, int layer, int priorityPlane) {
//    auto realVAO = (GVertexBufferBindingsVLK *)meshTemplate.bindings.get();
//    meshTemplate.bindings = m_emptyM2ParticleVAO;
//    auto mesh = bindlessMeshFactoryVlk.createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), layer, priorityPlane);
//    mesh->instanceIndex = 1;
//    mesh->vertexStart = ((IBufferVLK * )realVAO->getVertexBuffers()[0].get())->getIndex();

    auto mesh = bindlessMeshFactoryVlk.createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0, priorityPlane);
    return mesh;
}

HGSortableMesh MapSceneRenderBindlessVLK::createWMOMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IWMOMaterial> &material,
                                                const std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> &ambientBuffer) {
    auto realVAO = (GVertexBufferBindingsVLK *)meshTemplate.bindings.get();
    meshTemplate.bindings = m_emptyWMOVAO;

    auto originalMat = std::dynamic_pointer_cast<IWMOMaterialVis>(material);
    auto c_perMeshData = std::make_shared<CBufferChunkVLK<WMO::perMeshData>>(wmoBuffers.wmoPerMeshData);

    auto newMat = MaterialBuilderVLK::fromMaterial(m_device, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material))
    .toMaterial<IWMOMaterialVis>([&c_perMeshData](IWMOMaterialVis *instance) -> void {
        instance->m_perMeshData = c_perMeshData;
    });

    {
        auto &perMeshData = c_perMeshData->getObject();
        perMeshData.meshWideBindlessIndex = originalMat->meshWideBindlessIndex;
        perMeshData.wmoAmbientIndex = BufferChunkHelperVLK::castToChunk(ambientBuffer)->getIndex();
        c_perMeshData->save();
    }

    auto mesh = bindlessMeshFactoryVlk.createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(newMat), 0, 0);
    mesh->instanceIndex = c_perMeshData->getIndex();
    mesh->vertexStart = ((IBufferVLK * )realVAO->getVertexBuffers()[0].get())->getIndex();
    mesh->start() += ((IBufferVLK * )realVAO->getIndexBuffer().get())->getOffset();
    return mesh;
}
HGM2Mesh MapSceneRenderBindlessVLK::createM2WaterfallMesh(gMeshTemplate &meshTemplate,
                                                          const std::shared_ptr<IM2WaterFallMaterial> &material,
                                                          int layer, int priorityPlane) {
    auto mesh = bindlessMeshFactoryVlk.createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), layer, priorityPlane);
    mesh->instanceIndex = std::dynamic_pointer_cast<IM2WaterFallMaterialBindless>(material)->instanceIndex;
    return mesh;
}

std::shared_ptr<IRenderView> MapSceneRenderBindlessVLK::createRenderView(bool createOutput) {
    return std::make_shared<RendererViewClass>(m_device, uboBuffer,
                                               pointLightBuffer, spotLightBuffer,
                                               sceneWideDS,
                                               m_drawQuadVao,
                                               m_drawSpotVao,
                                               createOutput);
}
