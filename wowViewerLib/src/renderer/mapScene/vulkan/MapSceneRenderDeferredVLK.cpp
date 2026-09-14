#include "MapSceneRenderDeferredVLK.h"
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
#include "meshCollectors/OpaqueMeshCollectorDeferred.h"
#include "meshCollectors/OpaqueMeshCollectorForward.h"
#include "liquidData/LiquidDataForwardVLK.h"
#include "../../../gapi/vulkan/utils/SpecializedConstantsBuilder.h"
#include <future>
#include <cmath>

class IM2ModelDataVLK : public IM2ModelData {
public:
    ~IM2ModelDataVLK() override = default;
    std::shared_ptr<GDescriptorSet> m2CommonDS;
};

static const DescTypeSetBindingConfig SceneDataSetConfig = {
    {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
    {1, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {2, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {3, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {4, {VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}}
};

static const DescTypeSetBindingConfig LiquidDataSetConfig = {
    {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
    {1, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {2, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {3, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {4, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {5, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {6, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {7, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}}
};

static const ShaderConfig SceneDataSetStubShaderConfig = {
    "stubs",
    "stubs",
    {
        {0, SceneDataSetConfig}
    }
};

static const DescTypeSetBindingConfig GBufferDataSetConfig = {
    {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {1, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {2, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
    {3, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
};

static const ShaderConfig GBufferDataSetStubShaderConfig = {
    "stubs",
    "stubs",
    {
        {0, GBufferDataSetConfig}
    }
};

static const ShaderConfig forwardShaderConfig = {
    "forwardRendering",
    "forwardRendering",
    {
        {0, SceneDataSetConfig}
    }
};



static const ShaderConfig m2DeferredShaderConfig = {
    "deferred/m2/forward",
    "deferred/m2/forward",
    {
        {0, SceneDataSetConfig},
        {1, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
            {1, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
            // textureMatrices: fragment-only in m2Shader, but the waterfall vertex shader
            // reads it too (texture coordinate animation) — the shared m2CommonDS layout
            // must cover both (VUID-VkGraphicsPipelineCreateInfo-layout-07988)
            {5, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
        }},
    }};
static const ShaderConfig m2DeferredGBufferShaderConfig = {
    "deferred/m2/forward",
    "deferred/m2/deferred",
    m2DeferredShaderConfig.typeOverrides
};

static const ShaderConfig m2ProjectedShaderConfig = {
    "deferred/projected",
    "deferred/projected",
    {
            {0, SceneDataSetConfig},
            {1, m2DeferredShaderConfig.typeOverrides.at(1)},
            {2, {
                {1, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
                {2, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
                {3, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
                {4, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
                {5, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
            }},
            {3, GBufferDataSetConfig}
    }};


static const ShaderConfig deferredShaderConfig = {
    "deferred",
    "deferred",
    {
        {0, SceneDataSetConfig},
    }};

static const ShaderConfig m2ParticlesDeferredGBufferShaderConfig = {
    "deferred/m2Particle/forward",
    "deferred/m2Particle/deferred",
    deferredShaderConfig.typeOverrides
};

static const ShaderConfig adtDeferredShaderConfig = {
    "deferred/adt/forward",
    "deferred/adt/forward",
    deferredShaderConfig.typeOverrides
};

static const ShaderConfig adtDeferredGBufferShaderConfig = {
    "deferred/adt/forward",
    "deferred/adt/deferred",
    adtDeferredShaderConfig.typeOverrides
};

static const ShaderConfig m2WaterfallDeferredShaderConfig = {
    "deferred/waterfall/forward",
    "deferred/waterfall/forward",
    deferredShaderConfig.typeOverrides
};

static const ShaderConfig wmoDeferredShaderConfig = {
    "deferred/wmo/forward",
    "deferred/wmo/forward",
    {
        {0, SceneDataSetConfig },
    }
};

static const ShaderConfig wmoDeferredGBufferShaderConfig = {
    "deferred/wmo/forward",
    "deferred/wmo/deferred",
    wmoDeferredShaderConfig.typeOverrides
};

static const ShaderConfig liquidShaderConfig = {
    "deferred/liquids/forward",
    "deferred/liquids/forward",
    {
        {0, SceneDataSetConfig},
        {2, LiquidDataSetConfig}
    }
};

// Water also reads the gbuffer set (depthTex/screenData/liquidBackBuffer) at set=3
static const ShaderConfig waterLiquidShaderConfig = {
    "deferred/liquids/forward",
    "deferred/liquids/forward",
    {
        {0, SceneDataSetConfig},
        {2, LiquidDataSetConfig},
        {3, GBufferDataSetConfig}
    }
};

// Fel also reads the gbuffer set at set=3 (scene depth for the foam pass)
static const ShaderConfig felLiquidShaderConfig = {
    "deferred/liquids/forward",
    "deferred/liquids/forward",
    {
        {0, SceneDataSetConfig},
        {2, LiquidDataSetConfig},
        {3, GBufferDataSetConfig}
    }
};

// LeyLine also reads the gbuffer set at set=3 (scene depth + liquid backbuffer for refraction)
static const ShaderConfig leyLineLiquidShaderConfig = {
    "deferred/liquids/forward",
    "deferred/liquids/forward",
    {
        {0, SceneDataSetConfig},
        {2, LiquidDataSetConfig},
        {3, GBufferDataSetConfig}
    }
};

// Swamp also reads the gbuffer set at set=3 (scene depth + liquid backbuffer for refraction)
static const ShaderConfig swampLiquidShaderConfig = {
    "deferred/liquids/forward",
    "deferred/liquids/forward",
    {
        {0, SceneDataSetConfig},
        {2, LiquidDataSetConfig},
        {3, GBufferDataSetConfig}
    }
};

// Azerithe also reads the gbuffer set at set=3 (scene depth for the foam pass)
static const ShaderConfig azeritheLiquidShaderConfig = {
    "deferred/liquids/forward",
    "deferred/liquids/forward",
    {
        {0, SceneDataSetConfig},
        {2, LiquidDataSetConfig},
        {3, GBufferDataSetConfig}
    }
};

// Mercury also reads the gbuffer set at set=3 (scene depth + blurred backbuffer for FakeReflection)
static const ShaderConfig mercuryLiquidShaderConfig = {
    "deferred/liquids/forward",
    "deferred/liquids/forward",
    {
        {0, SceneDataSetConfig},
        {2, LiquidDataSetConfig},
        {3, GBufferDataSetConfig}
    }
};

// LiquidFog also reads the gbuffer set at set=3 (scene depth for the volumetric fog)
static const ShaderConfig fogLiquidShaderConfig = {
    "deferred/liquids/forward",
    "deferred/liquids/forward",
    {
        {0, SceneDataSetConfig},
        {2, LiquidDataSetConfig},
        {3, GBufferDataSetConfig}
    }
};

// Depth-only liquid variant used by the liquid depth pass (no color attachments)
static const ShaderConfig liquidDepthShaderConfig = {
    "deferred/liquids/depth",
    "deferred/liquids/depth",
    {
        {0, SceneDataSetConfig}
    }
};

static const ShaderConfig aabbDebugShaderConfig = {
    "deferred/debug",
    "deferred/debug",
    {
        {0, SceneDataSetConfig},
        {1, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, false, 1, VK_SHADER_STAGE_VERTEX_BIT}},
            {1, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
        }},
    }
};

auto deferredFactoryVlk = std::make_shared<EntityFactory<10000, GMeshId, GMeshVLK>>();

MapSceneRenderDeferredVLK::MapSceneRenderDeferredVLK(const HGDeviceVLK &hDevice, Config *config) :
    m_device(hDevice), MapSceneRenderer(config),
    m_gpuDraws(hDevice, config) {
    std::cout << "Create deferred scene renderer " << std::endl;


    const int rendererId = std::hash<uint64_t>()((uint64_t)this) & 0xFFFF;
    const std::string rendererIdStr = " " + std::to_string(rendererId);

    auto un = [rendererIdStr](const std::string &name) -> std::string {
        return name + rendererIdStr;
    };
    this->allBuffers = {
        this->uboBuffer,
        this->uboStaticBuffer,
        this->uboM2BoneMatrixBuffer,

        this->pointLightBuffer,
        this->spotLightBuffer,

        this->vboM2Buffer,
        this->vboPortalBuffer,
        this->vboM2RibbonBuffer,
        this->vboAdtBuffer,
        this->vboWMOBuffer,
        this->vboWaterBuffer,
        this->vboSkyBuffer,

//        this->liquidBuffers.dataLiquidBuffer,

        this->liquidBuffers.waterLiquidBuffer,
        this->liquidBuffers.magmaLiquidBuffer,
        this->liquidBuffers.mercuryLiquidBuffer,
        this->liquidBuffers.fogLiquidBuffer,
        this->liquidBuffers.leyLineLiquidBuffer,
        this->liquidBuffers.felLiquidBuffer,
        this->liquidBuffers.swampLiquidBuffer,
        this->liquidBuffers.azeritheLiquidBuffer,

        this->iboBuffer,
        this->m_vboQuad,
        this->m_iboQuad,
        this->m_iboBBox,
        this->m_particleIndexBuffer,
        this->m_vboSpot,
        this->m_iboSpot,
    };


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

    for (auto &vboM2ParticleBuff : this->vboM2ParticleBuffers) {
        allBuffers.push_back(vboM2ParticleBuff);
    }

    //Quad vao
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
    //Box vao
    {
        const std::array<uint16_t, 36> indices = {
            // Front face (+Y)
            3, 7, 6,
            3, 6, 2,
           // Right face (+X)
            1, 5, 7,
            1, 7, 3,
           // Back face  (–Y)
            0, 4, 5,
            0, 5, 1,
           // Left face  (–X)
            2, 6, 4,
            2, 4, 0,
           // Top face   (+Z)
            7, 5, 4,
            7, 4, 6,
           // Bottom face(–Z)
            2, 0, 1,
            2, 1, 3
        };

        m_iboBBox = m_device->createIndexBuffer(un("Scene_IBO_BBox"), indices.size() * sizeof(uint16_t));
        m_iboBBox->uploadData(indices.data(), indices.size() * sizeof(uint16_t));

        m_drawBBoxVao = m_device->createVertexBufferBindings();
        m_drawBBoxVao->setIndexBuffer(m_iboBBox);
        m_drawBBoxVao->save();
    }
    // AABB debug wireframe vao: no vertex buffer, corner positions are derived in the
    // vertex shader from gl_VertexIndex (0-7) and the per-instance AABB from the GPU
    // culling AABB buffer (see GpuIndirectDrawsVLK).
    {
        const std::array<uint16_t, 24> lineIndices = {
            0,1, 1,3, 3,2, 2,0, // bottom face (z = min)
            4,5, 5,7, 7,6, 6,4, // top face (z = max)
            0,4, 1,5, 2,6, 3,7  // vertical edges
        };

        m_iboAabbDebugLine = m_device->createIndexBuffer(un("Scene_IBO_AABBDebugLine"), lineIndices.size() * sizeof(uint16_t));
        m_iboAabbDebugLine->uploadData(lineIndices.data(), lineIndices.size() * sizeof(uint16_t));

        m_drawAabbDebugVao = m_device->createVertexBufferBindings();
        m_drawAabbDebugVao->setIndexBuffer(m_iboAabbDebugLine);
        m_drawAabbDebugVao->save();

        allBuffers.push_back(m_iboAabbDebugLine);
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
        std::vector<uint16_t> lineIndexBuffer; // index buffer for debug lines

        auto const firstPointIdx = vertexBuffer.size();
        vertexBuffer.push_back(mathfu::vec2_packed(mathfu::vec2(0,0)));

        for (uint32_t i = 0; i < spotLightSegments; i++) {
            float angle = i * 2.0f * M_PI / (float)spotLightSegments;
            vertexBuffer.push_back(mathfu::vec2_packed(
                mathfu::vec2(cos(angle),sin(angle))
            ));
        }
        auto const secondPointIdx = vertexBuffer.size();
        vertexBuffer.push_back(mathfu::vec2_packed(mathfu::vec2(0,0)));

        {
            // Cone with triangles
            //Top segment of cone is clockwise
            for (int32_t i = spotLightSegments-1; i >= 0 ; i--) {
                indexBuffer.push_back(((i+1) % spotLightSegments) + 1);
                indexBuffer.push_back(((i) % spotLightSegments) + 1);
                indexBuffer.push_back(firstPointIdx);
            }

            //Lower segment of cone is counter clock-wise: reverse order for points, but same for segments
            for (int32_t i = spotLightSegments-1; i >= 0 ; i--) {
                indexBuffer.push_back(secondPointIdx);
                indexBuffer.push_back(((i) % spotLightSegments) + 1);
                indexBuffer.push_back(((i+1) % spotLightSegments) + 1);
            }
        }

        {
            // Cone with lines
            for (uint32_t i = 0; i < spotLightSegments; i++) {
                lineIndexBuffer.push_back(firstPointIdx);
                lineIndexBuffer.push_back(((i+1) % spotLightSegments) + 1);
                lineIndexBuffer.push_back(((i+1) % spotLightSegments) + 1);
                lineIndexBuffer.push_back(((i) % spotLightSegments) + 1);
                lineIndexBuffer.push_back(((i) % spotLightSegments) + 1);
                lineIndexBuffer.push_back(firstPointIdx);
            }

            for (uint32_t i = 0; i < spotLightSegments; i++) {
                lineIndexBuffer.push_back(secondPointIdx);
                lineIndexBuffer.push_back(((i+1) % spotLightSegments) + 1);
                lineIndexBuffer.push_back(((i+1) % spotLightSegments) + 1);
                lineIndexBuffer.push_back(((i) % spotLightSegments) + 1);
                lineIndexBuffer.push_back(((i) % spotLightSegments) + 1);
                lineIndexBuffer.push_back(secondPointIdx);
            }
        }

        m_vboSpot = m_device->createVertexBuffer(un("Scene_VBO_Spot"), vertexBuffer.size() * sizeof(mathfu::vec2_packed));
        m_iboSpot = m_device->createIndexBuffer(un("Scene_IBO_Spot"), indexBuffer.size() * sizeof(uint16_t));
        m_vboSpot->uploadData(vertexBuffer.data(), vertexBuffer.size() * sizeof(mathfu::vec2_packed));
        m_iboSpot->uploadData(indexBuffer.data(), indexBuffer.size() * sizeof(uint16_t));

        m_iboSpotLine = m_device->createIndexBuffer(un("Scene_IBO_Spot_line"), lineIndexBuffer.size() * sizeof(uint16_t));
        m_iboSpotLine->uploadData(lineIndexBuffer.data(), lineIndexBuffer.size() * sizeof(uint16_t));

        m_drawSpotVao = m_device->createVertexBufferBindings();
        m_drawSpotVao->addVertexBufferBinding(m_vboSpot, std::vector(fullScreenQuad.begin(), fullScreenQuad.end()));
        m_drawSpotVao->setIndexBuffer(m_iboSpot);
        m_drawSpotVao->save();

        m_drawSpotVaoLine = m_device->createVertexBufferBindings();
        m_drawSpotVaoLine->addVertexBufferBinding(m_vboSpot, std::vector(fullScreenQuad.begin(), fullScreenQuad.end()));
        m_drawSpotVaoLine->setIndexBuffer(m_iboSpotLine);
        m_drawSpotVaoLine->save();
    }

    //Create water buffs
    {
//        liquidBuffers.dataLiquidBuffer = m_device->createSSBOBuffer(un("Scene_DataLiquidBuffer"), 1024, 16);

        liquidBuffers.waterLiquidBuffer = m_device->createUniformBuffer(un("Scene_WaterLiquidBuffer"), 1024 * sizeof(Liquid::WaterData));
        liquidBuffers.magmaLiquidBuffer = m_device->createUniformBuffer(un("Scene_MagmaLiquidBuffer"), 1024 * sizeof(Liquid::MagmaData));
        liquidBuffers.mercuryLiquidBuffer = m_device->createUniformBuffer(un("Scene_MercuryLiquidBuffer"), 1024 * sizeof(Liquid::MercuryData));
        liquidBuffers.fogLiquidBuffer = m_device->createUniformBuffer(un("Scene_FogLiquidBuffer"), 1024 * sizeof(Liquid::FogData));
        liquidBuffers.leyLineLiquidBuffer = m_device->createUniformBuffer(un("Scene_LeyLineLiquidBuffer"), 1024 * sizeof(Liquid::LeyLineData));
        liquidBuffers.felLiquidBuffer = m_device->createUniformBuffer(un("Scene_FelLiquidBuffer"), 1024 * sizeof(Liquid::FelData));
        liquidBuffers.swampLiquidBuffer = m_device->createUniformBuffer(un("Scene_SwampLiquidBuffer"), 1024 * sizeof(Liquid::SwampData));
        liquidBuffers.azeritheLiquidBuffer = m_device->createUniformBuffer(un("Scene_AzeritheLiquidBuffer"), 1024 * sizeof(Liquid::AzeritheData));
    }

    pointLightBuffer = m_device->createSSBOBuffer(un("Point Light Buffer"),200, sizeof(LocalLight));
    spotLightBuffer = m_device->createSSBOBuffer(un("Spot Light Buffer"),200, sizeof(SpotLight));

    uboBuffer = m_device->createUniformBuffer(un("UBO Buffer"), 1024*1024);
    uboStaticBuffer = m_device->createUniformBuffer(un("UBO Static"), 1024*1024);

    uboM2BoneMatrixBuffer = m_device->createUniformBuffer("Scene_UBO_M2BoneMats", 5000*64);

    m_emptyADTVAO = createADTVAO(vboAdtBuffer, iboBuffer);
    m_emptyM2VAO = createM2VAO(vboM2Buffer, iboBuffer);
    m_emptyM2ParticleVAO = createM2ParticleVAO(nullptr, nullptr);
    m_emptyM2RibbonVAO = createM2RibbonVAO(nullptr, nullptr);
    m_emptySkyVAO = createSkyVAO(nullptr, nullptr);
    m_emptyWMOVAO = createWmoVAO(vboWMOBuffer, iboBuffer);
    m_emptyWaterVAO = createWaterVAO(vboWaterBuffer, iboBuffer);
    m_emptyPortalVAO = createPortalVAO(nullptr, nullptr);


    // Create render mode buffer early so it's available for scene-wide DS
    m_renderModeBuffer = m_device->createSSBOBuffer("LiquidRenderMode",
        sizeof(uint32_t), sizeof(uint32_t));
    {
        uint32_t defaultMode = 2; // mode=all (no liquid filtering)
        m_renderModeBuffer->uploadData(&defaultMode, sizeof(uint32_t));
    }

    {
        //Create SceneWide descriptor
        sceneWideChunk = std::make_shared<GBufferChunkDynamicVersionedVLK<sceneWideBlockVSPS>>(hDevice, 3, uboBuffer);
        MaterialBuilderVLK::fromShader(m_device, {"stub", "commonSceneData"}, SceneDataSetStubShaderConfig)
            .createDescriptorSet(0, [&](std::shared_ptr<GDescriptorSet> &ds) {
                ds->beginUpdate()
                    .ubo_dynamic(0, sceneWideChunk)
                    .texture(1, hDevice->getBlackTexturePixel())
                    .texture(2, hDevice->getWhiteTexturePixel())
                    .texture_depth(3, hDevice->getEmptyDepthTexture())
                    .ssbo(4, m_renderModeBuffer);

                sceneWideDS = ds;
            }).toMaterial();
    }

    {
        //Create GBuffer descriptor
        std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> tmpUboBuffer = std::make_shared<CBufferChunkVLK<mathfu::vec4_packed>>(uboBuffer);

        MaterialBuilderVLK::fromShader(m_device, {"stub", "commonGBufferData"}, GBufferDataSetStubShaderConfig)
            .createDescriptorSet(0, [&](std::shared_ptr<GDescriptorSet> &ds) {
                ds->beginUpdate()
                    .texture(0, hDevice->getBlackTexturePixel())
                    .texture(1, hDevice->getWhiteTexturePixel())
                    .ubo(2, BufferChunkHelperVLK::cast(tmpUboBuffer)) //This is temporary. This 3rd binding is overridden in view class right before rendering
                    .texture(3, hDevice->getBlackTexturePixel()); // liquidBackBuffer placeholder

                gBufferDataDS = ds;
            }).toMaterial();
    }

     // Create separate descriptor sets for each liquid material type
    {
        MaterialBuilderVLK::fromShader(m_device, {"liquidShader", "waterLiquidShader"}, liquidShaderConfig)
        .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
            waterLiquidTexturesDS = ds;
        }).toMaterial();

        MaterialBuilderVLK::fromShader(m_device, {"liquidShader", "magmaLiquidShader"},
                                       liquidShaderConfig)
            .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
                magmaLiquidTexturesDS = ds;
            }).toMaterial();
        MaterialBuilderVLK::fromShader(m_device, {"liquidShader", "mercuryLiquidShader"},
                                       liquidShaderConfig)
            .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
                mercuryLiquidTexturesDS = ds;
            }).toMaterial();
        MaterialBuilderVLK::fromShader(m_device, {"liquidShader", "fogLiquidShader"},
                                       liquidShaderConfig)
            .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
                fogLiquidTexturesDS = ds;
            }).toMaterial();
        MaterialBuilderVLK::fromShader(m_device, {"liquidShader", "leyLineLiquidShader"},
                                       liquidShaderConfig)
            .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
                leyLineLiquidTexturesDS = ds;
            }).toMaterial();
        MaterialBuilderVLK::fromShader(m_device, {"liquidShader", "felLiquidShader"},
                                       liquidShaderConfig)
            .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
                felLiquidTexturesDS = ds;
            }).toMaterial();
        MaterialBuilderVLK::fromShader(m_device, {"liquidShader", "swampLiquidShader"},
                                       liquidShaderConfig)
            .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
                swampLiquidTexturesDS = ds;
            }).toMaterial();
        MaterialBuilderVLK::fromShader(m_device, {"liquidShader", "azeritheLiquidShader"},
                                       liquidShaderConfig)
            .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
                azeritheLiquidTexturesDS = ds;
            }).toMaterial();
    }

    defaultView = std::make_shared<RendererViewClass>(m_device, uboBuffer,
                                                      pointLightBuffer, spotLightBuffer,
                                                      sceneWideDS,
                                                      gBufferDataDS,
                                                      m_drawQuadVao,
                                                      m_drawSpotVao,
                                                      m_drawSpotVaoLine,
                                                      false);

    m_forwardRenderPass = defaultView->getForwardPass();
    m_gBufferPass = defaultView->getGBufferPass();
    m_liquidDepthRenderPass = defaultView->getLiquidDepthPass();

    m_gpuDraws.setup(allBuffers);
    createAABBDebugMaterials();
    setupObjectPicking();
}

void MapSceneRenderDeferredVLK::createAABBDebugMaterials() {
    static const PipelineTemplate s_aabbDebugPipeline = {
        DrawElementMode::LINE,
        false,
        true,
        EGxBlendEnum::GxBlend_Opaque,
        true, // depthCulling
        false, // depthWrite
        0xFF
    };

    m_aabbDebugColor = std::make_shared<CBufferChunkVLK<mathfu::vec4_packed>>(uboBuffer);
    m_aabbDebugColor->getObject() = mathfu::vec4_packed(mathfu::vec4(0.0f, 1.0f, 0.0f, 1.0f));

    m_aabbDebugMat = MaterialBuilderVLK::fromShader(m_device, {"aabbDebugDraw", "aabbDebugDraw"}, aabbDebugShaderConfig)
        .createPipeline(m_drawAabbDebugVao, m_forwardRenderPass, s_aabbDebugPipeline)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [this](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(0, m_gpuDraws.aabbBuffer())
                .ubo(1, *m_aabbDebugColor)
                .delayUpdate();
        })
        .toMaterial();

    // Single-box highlight for the currently selected object, drawn regardless of drawM2BB/drawWmoBB.
    static constexpr size_t SELECTED_AABB_RECORD_SIZE = sizeof(float) * 8; // float4 min + float4 max
    m_selectedAABBBuffer = m_device->createSSBOBuffer("SelectedAABB", SELECTED_AABB_RECORD_SIZE, SELECTED_AABB_RECORD_SIZE);
    allBuffers.push_back(m_selectedAABBBuffer);

    m_selectedAABBColor = std::make_shared<CBufferChunkVLK<mathfu::vec4_packed>>(uboBuffer);
    m_selectedAABBColor->getObject() = mathfu::vec4_packed(mathfu::vec4(1.0f, 0.1f, 0.9f, 1.0f));

    m_selectedAABBMat = MaterialBuilderVLK::fromShader(m_device, {"aabbDebugDraw", "aabbDebugDraw"}, aabbDebugShaderConfig)
        .createPipeline(m_drawAabbDebugVao, m_forwardRenderPass, s_aabbDebugPipeline)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [this](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(0, m_selectedAABBBuffer)
                .ubo(1, *m_selectedAABBColor)
                .delayUpdate();
        })
        .toMaterial();
}

void MapSceneRenderDeferredVLK::drawAABBDebug(CmdBufRecorder &frameBufCmd, const GpuIndirectDrawsVLK::FrameDrawData &gpuFrameDraws) {
    uint32_t m2Count = gpuFrameDraws.m2AabbCount;
    uint32_t wmoCount = gpuFrameDraws.wmoGroupCount;

    bool drawM2 = m_aabbDebugMat && m_config->drawM2BB && m2Count > 0;
    bool drawWmo = m_aabbDebugMat && m_config->drawWmoBB && wmoCount > 0;
    bool drawSelected = m_selectedAABBMat && m_selectedAABBValid;
    if (!drawM2 && !drawWmo && !drawSelected) return;

    VkZone(frameBufCmd, "AABB Debug");
    auto debugHelper = frameBufCmd.beginDebugLabel("AABB Debug", {0, 0.5f, 0});

    frameBufCmd.setViewPort(CmdBufRecorder::ViewportType::vp_usual);
    frameBufCmd.setDefaultScissors();
    frameBufCmd.bindVertexBindings(m_drawAabbDebugVao);

    if (drawM2 || drawWmo) {
        frameBufCmd.bindMaterial(m_aabbDebugMat);

        if (drawM2) {
            frameBufCmd.drawIndexed(24, m2Count, 0, 0, 0);
        }
        if (drawWmo) {
            frameBufCmd.drawIndexed(24, wmoCount, 0, gpuFrameDraws.wmoGroupBase, 0);
        }
    }

    if (drawSelected) {
        frameBufCmd.bindMaterial(m_selectedAABBMat);
        frameBufCmd.drawIndexed(24, 1, 0, 0, 0);
    }
}

void MapSceneRenderDeferredVLK::setupObjectPicking() {
    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = sizeof(uint32_t);
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    for (int i = 0; i < IDevice::MAX_FRAMES_IN_FLIGHT; i++) {
        VmaAllocationInfo allocationInfo = {};
        vmaCreateBuffer(m_device->getVMAAllocator(), &bufferInfo, &allocCreateInfo,
                        &m_pickResultBuffer[i], &m_pickResultAllocation[i], &allocationInfo);
        m_pickResultMapped[i] = allocationInfo.pMappedData;
        m_pickResultPending[i] = false;
    }
}

void MapSceneRenderDeferredVLK::resolvePendingPickResult() {
    uint32_t frameNum = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

    // This frame-in-flight slot is about to be reused, which means the engine has already waited on
    // its previous submission's fence as part of normal frame pacing — so the copy queued last time
    // this slot was used is guaranteed complete by now.
    if (!m_pickResultPending[frameNum]) return;
    m_pickResultPending[frameNum] = false;
    bool isHoverPeek = m_pickResultIsHoverPeek[frameNum];

    uint32_t packed = *static_cast<uint32_t*>(m_pickResultMapped[frameNum]);
    if ((packed & 0x80000000u) == 0) {
        // Hit empty space (sky/terrain/etc.) — clear the corresponding result.
        if (isHoverPeek) {
            m_hasHoveredM2 = false;
            m_hoveredWMO = emptyWMO;
            m_hoveredWMOGroupNum = -1;
        } else {
            m_hasSelectedM2 = false;
            m_selectedWMO = emptyWMO;
            m_selectedWMOGroupNum = -1;
        }
        return;
    }

    bool isWmo = (packed & 0x40000000u) != 0;
    uint32_t payload = packed & 0x3FFFFFFFu;

    if (isHoverPeek) {
        if (!isWmo) {
            m_hasHoveredM2 = true;
            m_hoveredM2 = (M2ObjId)payload;
            m_hoveredWMO = emptyWMO;
            m_hoveredWMOGroupNum = -1;
        } else {
            m_hasHoveredM2 = false;
            m_hoveredWMO = (WMOObjId)(payload >> 10);
            m_hoveredWMOGroupNum = (int)(payload & 0x3FFu);
        }
        return;
    }

    if (!isWmo) {
        m_hasSelectedM2 = true;
        m_selectedM2 = (M2ObjId)payload;
        m_selectedWMO = emptyWMO;
        m_selectedWMOGroupNum = -1;
    } else {
        m_hasSelectedM2 = false;
        m_selectedWMO = (WMOObjId)(payload >> 10);
        m_selectedWMOGroupNum = (int)(payload & 0x3FFu);
    }
}

void MapSceneRenderDeferredVLK::updateSelectedAABB(const std::shared_ptr<MapRenderPlan> &framePlan) {
    m_selectedAABBValid = false;

    if (m_hasSelectedM2) {
        auto *m2 = m2Factory->getObjectById<0>(m_selectedM2);
        if (m2 != nullptr && m2->getHasBoundingBox()) {
            m_selectedAABBData = m2->getAABB();
            m_selectedAABBValid = true;
        }
    } else if (m_selectedWMO != emptyWMO) {
        // Reuse the GPU-indirect path's persistent WMO-group registry to find the live
        // WmoGroupObject matching the selection.
        for (auto &[ptr, cullEntry] : m_gpuDraws.wmoGroupCullIndexMap()) {
            if (ptr->getWmoApi() &&
                ptr->getWmoApi()->getPickObjectId() == static_cast<uint32_t>(m_selectedWMO) &&
                ptr->getGroupNumber() == m_selectedWMOGroupNum) {
                m_selectedAABBData = ptr->getWorldAABB();
                m_selectedAABBValid = true;
                break;
            }
        }
    }

    framePlan->hasSelectedAABB = m_selectedAABBValid;
    if (m_selectedAABBValid) {
        framePlan->selectedAABB = m_selectedAABBData;

        struct GPUAabb {
            mathfu::vec4_packed min;
            mathfu::vec4_packed max;
        };
        GPUAabb entry;
        entry.min = mathfu::vec4_packed(mathfu::vec4(m_selectedAABBData.min.x, m_selectedAABBData.min.y, m_selectedAABBData.min.z, 1.0f));
        entry.max = mathfu::vec4_packed(mathfu::vec4(m_selectedAABBData.max.x, m_selectedAABBData.max.y, m_selectedAABBData.max.z, 1.0f));
        m_selectedAABBBuffer->uploadData(&entry, sizeof(GPUAabb));
    }
}


// ------------------
// Buffer creation
// ------------------

HGVertexBufferBindings MapSceneRenderDeferredVLK::createADTVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto adtVAO = m_device->createVertexBufferBindings();
    adtVAO->addVertexBufferBinding(vertexBuffer, adtVertexBufferBinding);
    adtVAO->setIndexBuffer(indexBuffer);

    return adtVAO;
};

HGVertexBufferBindings MapSceneRenderDeferredVLK::createWmoVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto wmoVAO = m_device->createVertexBufferBindings();

    wmoVAO->addVertexBufferBinding(vertexBuffer, std::vector(staticWMOBindings.begin(), staticWMOBindings.end()));
    wmoVAO->setIndexBuffer(indexBuffer);

    return wmoVAO;
}
HGVertexBufferBindings MapSceneRenderDeferredVLK::createM2VAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto m2VAO = m_device->createVertexBufferBindings();
    m2VAO->addVertexBufferBinding(vertexBuffer, staticM2Bindings);
    m2VAO->setIndexBuffer(indexBuffer);

    return m2VAO;
}
HGVertexBufferBindings MapSceneRenderDeferredVLK::createM2ParticleVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto m2ParticleVAO = m_device->createVertexBufferBindings();
    m2ParticleVAO->addVertexBufferBinding(vertexBuffer, staticM2ParticleBindings);
    m2ParticleVAO->setIndexBuffer(indexBuffer);

    return m2ParticleVAO;
}

HGVertexBufferBindings MapSceneRenderDeferredVLK::createM2RibbonVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto m2RibbonVAO = m_device->createVertexBufferBindings();
    m2RibbonVAO->addVertexBufferBinding(vertexBuffer, staticM2RibbonBindings);
    m2RibbonVAO->setIndexBuffer(indexBuffer);

    return m2RibbonVAO;
};

HGVertexBufferBindings MapSceneRenderDeferredVLK::createWaterVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto waterVAO = m_device->createVertexBufferBindings();
    waterVAO->addVertexBufferBinding(vertexBuffer, std::vector(staticWaterBindings.begin(), staticWaterBindings.end()));
    waterVAO->setIndexBuffer(indexBuffer);

    return waterVAO;
};

HGVertexBufferBindings MapSceneRenderDeferredVLK::createSkyVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto skyVAO = m_device->createVertexBufferBindings();
    skyVAO->addVertexBufferBinding(vertexBuffer, std::vector(skyConusBinding.begin(), skyConusBinding.end()));
    skyVAO->setIndexBuffer(indexBuffer);

    return skyVAO;
}

HGVertexBufferBindings MapSceneRenderDeferredVLK::createPortalVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto portalVAO = m_device->createVertexBufferBindings();
    portalVAO->addVertexBufferBinding(vertexBuffer, std::vector(drawPortalBindings.begin(), drawPortalBindings.end()));
    portalVAO->setIndexBuffer(indexBuffer);

    return portalVAO;
};

HGVertexBuffer MapSceneRenderDeferredVLK::createPortalVertexBuffer(int sizeInBytes) {
    return vboPortalBuffer->getSubBuffer(sizeInBytes);
};
HGIndexBuffer  MapSceneRenderDeferredVLK::createPortalIndexBuffer(int sizeInBytes){
    return iboBuffer->getSubBuffer(sizeInBytes);
};

HGVertexBuffer MapSceneRenderDeferredVLK::createM2VertexBuffer(int sizeInBytes) {
    return vboM2Buffer->getSubBuffer(sizeInBytes);
}

HGVertexBuffer MapSceneRenderDeferredVLK::createM2ParticleVertexBuffer(int sizeInBytes, int frameIndex) {
    if (frameIndex >= PARTICLES_BUFF_NUM) return nullptr;

//    auto currentProcessingFrameNumber = m_device->getCurrentProcessingFrameNumber();
//    std::cout << "createM2ParticleVertexBuffer"
//        << " frameIndex="<<frameIndex
//        <<" currentProcessingFrameNumber ="<<currentProcessingFrameNumber
//        <<" sizeInBytes ="<<sizeInBytes
//        <<std::endl;

    return vboM2ParticleBuffers[frameIndex]->getSubBuffer(sizeInBytes);
}
HGVertexBuffer MapSceneRenderDeferredVLK::createM2RibbonVertexBuffer(int sizeInBytes) {
    return vboM2RibbonBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer MapSceneRenderDeferredVLK::createM2IndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer MapSceneRenderDeferredVLK::getOrCreateM2ParticleIndexBuffer() {
    return m_particleIndexBuffer;
}


HGVertexBuffer MapSceneRenderDeferredVLK::createADTVertexBuffer(int sizeInBytes) {
    return vboAdtBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer MapSceneRenderDeferredVLK::createADTIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}

HGVertexBuffer MapSceneRenderDeferredVLK::createWMOVertexBuffer(int sizeInBytes) {
    return vboWMOBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer MapSceneRenderDeferredVLK::createWMOIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}

HGVertexBuffer MapSceneRenderDeferredVLK::createWaterVertexBuffer(int sizeInBytes) {
    return vboWaterBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer MapSceneRenderDeferredVLK::createWaterIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}
HGVertexBuffer MapSceneRenderDeferredVLK::createSkyVertexBuffer(int sizeInBytes) {
    return vboSkyBuffer->getSubBuffer(sizeInBytes);;
};

HGIndexBuffer  MapSceneRenderDeferredVLK::createSkyIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}


const auto adtConst4Textures  =
    SpecializedConstantsBuilder(4, 1)
    .WriteInt32LE(4)
    .toBuffer();

const auto adtConst8Textures  =
    SpecializedConstantsBuilder(4, 1)
    .WriteInt32LE(8)
    .toBuffer();

std::shared_ptr<IADTMaterial>
MapSceneRenderDeferredVLK::createAdtMaterial(const PipelineTemplate &pipelineTemplate, const ADTMaterialTemplate &adtMaterialTemplate) {
   auto &l_sceneWideChunk = sceneWideChunk;
    auto vertexFragmentData = std::make_shared<CBufferChunkVLK<ADT::meshWideBlockVSPS>>(uboStaticBuffer);
    auto fragmentData = std::make_shared<CBufferChunkVLK<ADT::meshWideBlockPS>>(uboBuffer);

    auto specConst = adtMaterialTemplate.maxLayerCount <=4 ? adtConst4Textures : adtConst8Textures;

    auto material = MaterialBuilderVLK::fromShader(m_device, {"adtShader", "adtShader"}, adtDeferredShaderConfig)
        .createPipeline(m_emptyADTVAO, m_forwardRenderPass, pipelineTemplate, specConst.dataBuff, specConst.metadata)
        .createGBufferPipeline(m_emptyADTVAO, {"adtShader", "adtShader"}, adtDeferredGBufferShaderConfig, m_gBufferPass, specConst.dataBuff, specConst.metadata)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [&vertexFragmentData, &fragmentData, &l_sceneWideChunk](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, *vertexFragmentData)
                .ubo(1, *fragmentData).delayUpdate();
        })
        .createDescriptorSet(2, [&adtMaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(0, adtMaterialTemplate.layerTextures[0])
                .texture(1, adtMaterialTemplate.layerTextures[1])
                .texture(2, adtMaterialTemplate.layerTextures[2])
                .texture(3, adtMaterialTemplate.layerTextures[3])
                .texture(4, adtMaterialTemplate.blendTextures[0])
                .texture(5, adtMaterialTemplate.heightTextures[0])
                .texture(6, adtMaterialTemplate.heightTextures[1])
                .texture(7, adtMaterialTemplate.heightTextures[2])
                .texture(8, adtMaterialTemplate.heightTextures[3])
                .texture(9, adtMaterialTemplate.layerTextures[4])
                .texture(10, adtMaterialTemplate.layerTextures[5])
                .texture(11, adtMaterialTemplate.layerTextures[6])
                .texture(12, adtMaterialTemplate.layerTextures[7])
                .texture(13, adtMaterialTemplate.blendTextures[1])
                .texture(14, adtMaterialTemplate.heightTextures[4])
                .texture(15, adtMaterialTemplate.heightTextures[5])
                .texture(16, adtMaterialTemplate.heightTextures[6])
                .texture(17, adtMaterialTemplate.heightTextures[7]);
        })
        .toMaterial<IADTMaterial>([&vertexFragmentData, &fragmentData](IADTMaterial *instance) -> void {
            instance->m_materialVSPS = vertexFragmentData;
            instance->m_materialPS = fragmentData;
        });

    return material;
}

std::shared_ptr<IM2Material>
MapSceneRenderDeferredVLK::createM2Material(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                            const PipelineTemplate &pipelineTemplate,
                                            const M2MaterialTemplate &m2MaterialTemplate) {

    auto &l_sceneWideChunk = sceneWideChunk;
    auto vertexFragmentData = std::make_shared<CBufferChunkVLK<M2::meshWideBlockVSPS>>(uboStaticBuffer);

    bool isTrueOpaq = pipelineTemplate.blendMode == EGxBlendEnum::GxBlend_Opaque;

    auto material = MaterialBuilderVLK::fromShader(m_device, {"m2Shader", "m2Shader"}, m2DeferredShaderConfig)
        .createPipeline(m_emptyM2VAO, m_forwardRenderPass, pipelineTemplate)
        .createGBufferPipeline(m_emptyM2VAO, {"m2Shader", isTrueOpaq ? "m2Shader_opaq" : "m2Shader"}, m2DeferredGBufferShaderConfig, m_gBufferPass)
        .bindDescriptorSet(0, sceneWideDS)
        .bindDescriptorSet(1, std::dynamic_pointer_cast<IM2ModelDataVLK>(m2ModelData)->m2CommonDS)
        .createDescriptorSet(2, [&vertexFragmentData, &m2MaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, *vertexFragmentData)
                .texture(1, m2MaterialTemplate.textures[0])
                .texture(2, m2MaterialTemplate.textures[1])
                .texture(3, m2MaterialTemplate.textures[2])
                .texture(4, m2MaterialTemplate.textures[3])
                .delayUpdate();
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

std::shared_ptr<IM2ProjectiveMaterial> MapSceneRenderDeferredVLK::createM2ProjectiveMaterial(
    const std::shared_ptr<IM2ModelData> &m2ModelData,
    const PipelineTemplate &pipelineTemplate,
    const M2MaterialTemplate &m2MaterialTemplate
) {
    ZoneScoped;

    auto m2ModelDataVLK = std::dynamic_pointer_cast<IM2ModelDataVLK>(m2ModelData);

    auto vertexFragmentData = std::make_shared<CBufferChunkVLK<M2::meshWideBlockVSPS>>(uboStaticBuffer);
    auto projectiveTextData = std::make_shared<CBufferChunkVLK<M2::ProjectiveData>>(uboStaticBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"m2_projected", "m2_projected"}, m2ProjectedShaderConfig)
        .setMaterialId(generateUniqueM2MatId())
        .createPipeline(m_drawBBoxVao, m_forwardRenderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .bindDescriptorSet(1, m2ModelDataVLK->m2CommonDS)
        .createDescriptorSet(2, [&vertexFragmentData, &projectiveTextData, &m2MaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, *vertexFragmentData)
                .texture(1, m2MaterialTemplate.textures[0])
                .texture(2, m2MaterialTemplate.textures[1])
                .texture(3, m2MaterialTemplate.textures[2])
                .texture(4, m2MaterialTemplate.textures[3])
                .ubo(5, *projectiveTextData)
                .delayUpdate();
        })
        .bindDescriptorSet(3, gBufferDataDS)
        .toMaterial<IM2ProjectiveMaterial>([&vertexFragmentData, &projectiveTextData](IM2ProjectiveMaterial *instance) -> void {
            instance->m_vertexFragmentData = vertexFragmentData;
            instance->m_projectiveTextData = projectiveTextData;
        });

    return material;
}

std::shared_ptr<IM2WaterFallMaterial> MapSceneRenderDeferredVLK::createM2WaterfallMaterial(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                                                           const PipelineTemplate &pipelineTemplate,
                                                                                           const M2WaterfallMaterialTemplate &m2MaterialTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto waterfallCommonData = std::make_shared<CBufferChunkVLK<M2::WaterfallData::WaterfallCommon>>(uboStaticBuffer);


    auto material = MaterialBuilderVLK::fromShader(m_device, {"waterfallShader", "waterfallShader"}, m2WaterfallDeferredShaderConfig)
        .createPipeline(m_emptyM2VAO, m_forwardRenderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .bindDescriptorSet(1, std::dynamic_pointer_cast<IM2ModelDataVLK>(m2ModelData)->m2CommonDS)
        .createDescriptorSet(2, [&waterfallCommonData, &m2MaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, *waterfallCommonData)
                .texture(1, m2MaterialTemplate.textures[0])
                .texture(2, m2MaterialTemplate.textures[1])
                .texture(3, m2MaterialTemplate.textures[2])
                .texture(4, m2MaterialTemplate.textures[3])
                .texture(5, m2MaterialTemplate.textures[4])
                .delayUpdate();
        })
        .toMaterial<IM2WaterFallMaterial>([&waterfallCommonData](IM2WaterFallMaterial *instance) -> void {
            instance->m_waterfallCommon = waterfallCommonData;
        });

    return material;
}

std::shared_ptr<IM2ParticleMaterial> MapSceneRenderDeferredVLK::createM2ParticleMaterial(
    const PipelineTemplate &pipelineTemplate,
    const M2ParticleMaterialTemplate &m2ParticleMatTemplate) {

    auto &l_sceneWideChunk = sceneWideChunk;
    auto l_fragmentData = std::make_shared<CBufferChunkVLK<Particle::meshParticleWideBlockPS>>(uboBuffer); ;

    bool isOpaq = pipelineTemplate.blendMode == EGxBlendEnum::GxBlend_Opaque ||
                  pipelineTemplate.blendMode == EGxBlendEnum::GxBlend_AlphaKey;

    auto material = MaterialBuilderVLK::fromShader(m_device, {"m2Particle/forward/m2ParticleShader", "m2Particle/forward/m2ParticleShader"}, deferredShaderConfig)
        .createPipeline(m_emptyM2ParticleVAO, m_forwardRenderPass, pipelineTemplate)
        .createGBufferPipeline(m_emptyM2ParticleVAO, {"m2ParticleShader", "m2ParticleShader"}, m2ParticlesDeferredGBufferShaderConfig, m_gBufferPass)
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

std::shared_ptr<IM2RibbonMaterial> MapSceneRenderDeferredVLK::createM2RibbonMaterial(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                                                     const PipelineTemplate &pipelineTemplate,
                                                                                     const M2RibbonMaterialTemplate &m2RibbonMaterialTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto l_fragmentData = std::make_shared<CBufferChunkVLK<Ribbon::meshRibbonWideBlockPS>>(uboBuffer); ;
    l_fragmentData->getObject().objectId = m2ModelData->objectId;
    auto &l_m2ModelData = m2ModelData;

    auto material = MaterialBuilderVLK::fromShader(m_device, {"m2Ribbon/forward/ribbonShader", "m2Ribbon/forward/ribbonShader"}, deferredShaderConfig)
        .createPipeline(m_emptyM2RibbonVAO,  m_forwardRenderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [&l_sceneWideChunk, &l_fragmentData, &l_m2ModelData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(l_m2ModelData->m_textureMatrices))
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

std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> MapSceneRenderDeferredVLK::createWmoModelMatrixChunk() {
    return std::make_shared<CBufferChunkVLK<WMO::modelWideBlockVS>>(uboStaticBuffer);
};

std::shared_ptr<IWmoModelData> MapSceneRenderDeferredVLK::createWMOWideChunk(int groupNum)  {
    auto wmoModelData = std::make_shared<IWmoModelData>();
    wmoModelData->m_placementMatrix = createWmoModelMatrixChunk();
    BufferChunkHelperVLK::create(uboStaticBuffer, wmoModelData->m_groupInteriorData, sizeof(WMO::InteriorBlockData) * groupNum);

    return wmoModelData;
}

std::shared_ptr<IWMOMaterial> MapSceneRenderDeferredVLK::createWMOMaterial(const std::shared_ptr<IWmoModelData> &wmoModelWide,
                                                                           const PipelineTemplate &pipelineTemplate,
                                                                           const WMOMaterialTemplate &wmoMaterialTemplate) {

    auto l_vertexData = std::make_shared<CBufferChunkVLK<WMO::meshWideBlockVS>>(uboStaticBuffer); ;
    auto l_fragmentData = std::make_shared<CBufferChunkVLK<WMO::meshWideBlockPS>>(uboStaticBuffer); ;

    bool isTrueOpaq = pipelineTemplate.blendMode == EGxBlendEnum::GxBlend_Opaque;

    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"wmoShader", "wmoShader"}, wmoDeferredShaderConfig)
        .createPipeline(m_emptyWMOVAO, m_forwardRenderPass, pipelineTemplate)
        .createGBufferPipeline(m_emptyWMOVAO, {"wmoShader", isTrueOpaq ? "wmoShader_opaq" : "wmoShader"}, wmoDeferredGBufferShaderConfig, m_gBufferPass)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &wmoModelWide, l_vertexData, l_fragmentData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(wmoModelWide->m_placementMatrix))
                .ubo(1, *l_vertexData)
                .ubo(2, *l_fragmentData)
                .ubo(3, BufferChunkHelperVLK::cast(wmoModelWide->m_groupInteriorData))
                .delayUpdate();
        })
        .createDescriptorSet(2, [&wmoMaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(0, wmoMaterialTemplate.textures[0])
                .texture(1, wmoMaterialTemplate.textures[1])
                .texture(2, wmoMaterialTemplate.textures[2])
                .texture(3, wmoMaterialTemplate.textures[3])
                .texture(4, wmoMaterialTemplate.textures[4])
                .texture(5, wmoMaterialTemplate.textures[5])
                .texture(6, wmoMaterialTemplate.textures[6])
                .texture(7, wmoMaterialTemplate.textures[7])
                .texture(8, wmoMaterialTemplate.textures[8]);
        })
        .toMaterial<IWMOMaterial>([&l_vertexData, &l_fragmentData](IWMOMaterial *instance) -> void {
            instance->m_materialVS = l_vertexData;
            instance->m_materialPS = l_fragmentData;
        });

    return material;
}

std::shared_ptr<ILiquidMaterial> MapSceneRenderDeferredVLK::createLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
                                                                               const PipelineTemplate &pipelineTemplate,
                                                                               const WaterMaterialTemplate &waterMaterialTemplate) {

    auto liquidInstanceData = std::make_shared<CBufferChunkVLK<Liquid::LiquidInstance>>(uboStaticBuffer);
    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"waterShader", "waterShader"}, forwardShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_forwardRenderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &modelWide](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(modelWide))
                .delayUpdate();
        })
        .createDescriptorSet(2, [&waterMaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(0, waterMaterialTemplate.texture);
        })
        .toMaterial<ILiquidMaterial>([](ILiquidMaterial *instance) -> void {

        });

    material->materialId = waterMaterialTemplate.liquidMaterialId;
    material->m_instance = liquidInstanceData;

    return material;
}



std::shared_ptr<ISkyMeshMaterial> MapSceneRenderDeferredVLK::createSkyMeshMaterial(const PipelineTemplate &pipelineTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto skyColors = std::make_shared<CBufferChunkVLK<DnSky::meshWideBlockVS>>(uboBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"skyConus", "skyConus"}, forwardShaderConfig)
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

std::shared_ptr<IPlanetMaterial> MapSceneRenderDeferredVLK::createPlanetMaterial(const PipelineTemplate &pipelineTemplate, const HGSamplableTexture &texture) {
    auto planetData = std::make_shared<CBufferChunkVLK<Planet::meshWideBlockVS>>(uboBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"planetShader", "planetShader"}, forwardShaderConfig)
        .createPipeline(m_emptySkyVAO, m_forwardRenderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [&planetData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, *planetData).delayUpdate();
        })
        .createDescriptorSet(2, [&texture](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(0, texture).delayUpdate();
        })
        .toMaterial<IPlanetMaterial>([&planetData](IPlanetMaterial *instance) -> void {
            instance->m_planetData = planetData;
        });

    return material;
}

std::shared_ptr<IPortalMaterial> MapSceneRenderDeferredVLK::createPortalMaterial(const PipelineTemplate &pipelineTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto materialPS = std::make_shared<CBufferChunkVLK<DrawPortalShader::meshWideBlockPS>>(uboBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"drawPortalShader", "drawPortalShader"}, forwardShaderConfig)
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

std::shared_ptr<IM2ModelData> MapSceneRenderDeferredVLK::createM2ModelMat(int bonesCount, int m2ColorsCount, int textureWeightsCount, int textureMatricesCount, uint32_t objectId) {
    auto result = std::make_shared<IM2ModelDataVLK>();

    BufferChunkHelperVLK::create(uboBuffer, result->m_placementMatrix);
    BufferChunkHelperVLK::create(uboM2BoneMatrixBuffer, result->m_bonesData, sizeof(mathfu::mat4) * bonesCount);
    BufferChunkHelperVLK::create(uboBuffer, result->m_colors, sizeof(mathfu::vec4_packed) * m2ColorsCount);
    BufferChunkHelperVLK::create(uboBuffer, result->m_textureWeights, sizeof(float) * textureWeightsCount);
    BufferChunkHelperVLK::create(uboBuffer, result->m_textureMatrices, sizeof(mathfu::mat4) * textureMatricesCount);
    BufferChunkHelperVLK::create(uboBuffer, result->m_modelFragmentData);
    result->m_modelFragmentData->getObject().objectId = objectId;
    result->objectId = objectId;


    MaterialBuilderVLK::fromShader(m_device, {"m2Shader", "m2Shader"}, m2DeferredShaderConfig)
        .createDescriptorSet(1, [&](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(result->m_placementMatrix))
                .ubo(1, BufferChunkHelperVLK::cast(result->m_modelFragmentData))
                .ubo(2, BufferChunkHelperVLK::cast(result->m_bonesData))
                .ubo(3, BufferChunkHelperVLK::cast(result->m_colors))
                .ubo(4, BufferChunkHelperVLK::cast(result->m_textureWeights))
                .ubo(5, BufferChunkHelperVLK::cast(result->m_textureMatrices))
                .delayUpdate();
            result->m2CommonDS = ds;
        }).toMaterial();

    return result;
}

static inline std::array<float,3> vec4ToArr3(const mathfu::vec4 &vec) {
    return {vec[0], vec[1], vec[2]};
}

inline void MapSceneRenderDeferredVLK::drawMesh(CmdBufRecorder &cmdBuf, const HGMesh &mesh, CmdBufRecorder::ViewportType viewportType ) {
    if (mesh == nullptr) return;

    const auto &meshVlk = (GMeshVLK*) mesh.get();

#ifdef DEBUG_MESH_NAMES
    auto debugLabel = cmdBuf.beginDebugLabel(mesh->debugName(), {1.0, 0, 0, 1.0});
#endif


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

std::unique_ptr<IRenderFunction> MapSceneRenderDeferredVLK::update(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams,
                                                                   const std::shared_ptr<MapRenderPlan> &framePlan) {
    TracyMessageStr(("Update stage frame = " + std::to_string(FrameContext::getCurrentProcessingFrameNumber())));

    ZoneScoped;
    auto l_this = std::dynamic_pointer_cast<MapSceneRenderDeferredVLK>(this->shared_from_this());
    auto mapScene = std::dynamic_pointer_cast<Map>(frameInputParams->frameParameters->scene);

    resolvePendingPickResult();
    framePlan->hasSelectedM2 = m_hasSelectedM2;
    framePlan->selectedM2 = m_selectedM2;
    framePlan->selectedWMO = m_selectedWMO;
    framePlan->selectedWMOGroupNum = m_selectedWMOGroupNum;

    framePlan->hasHoveredM2 = m_hasHoveredM2;
    framePlan->hoveredM2 = m_hoveredM2;
    framePlan->hoveredWMO = m_hoveredWMO;
    framePlan->hoveredWMOGroupNum = m_hoveredWMOGroupNum;

    //Create meshes
    std::unique_ptr<COpaqueMeshCollectorDeferredVLK> u_collector = std::make_unique<COpaqueMeshCollectorDeferredVLK>();
    auto transparentMeshes = std::make_shared<framebased::vector<HGSortableMesh>>();



    auto skyMeshes = std::make_shared<framebased::vector<HGMesh>>();
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


    // CPU-side collection: view meshes (ADT, WMO opaque — the latter is a no-op in this
    // collector), M2 decals and sky. M2 meshes and WMO transparent meshes are collected
    // by the GPU-indirect path instead, so they get frustum/occlusion culling on the GPU.
    collectViewMeshes(framePlan, *u_collector, *transparentMeshes, false);
    collectM2ProjectiveMeshes(framePlan, *u_collector);
    collectSkyMeshes(framePlan, *skyMeshes);

    // GPU-indirect path: sync dirty AABB/template data to the GPU, collect the frame's
    // active draw list and queue its upload (the actual VkDrawIndexedIndirectCommands
    // are assembled on the GPU from static per-mesh templates).
    // The returned snapshot is captured by the draw lambda below, since the draw stage
    // may run while the next frame's update is already rebuilding m_gpuDraws state.
    auto gpuFrameDraws = m_gpuDraws.buildFrameDraws(framePlan);
    updateSelectedAABB(framePlan);

//    u_collector->fillMeshCount(lastMeshCount);

//    {
//        ZoneScopedN("collect meshes wait");
//        collectMeshAsync.wait();
//    }

    // Compute viewProj for GPU culling
    mathfu::mat4 cullViewProj = mathfu::mat4::Identity();
    if (!renderingMatricessAndSizes.empty()) {
        auto &rm = renderingMatricessAndSizes[0].renderingMat;
        cullViewProj = MathHelper::getVulkanMat4Fix() * rm->perspectiveMat * rm->lookAtMat;
    }

    bool renderSky = framePlan->renderSky;
    return createRenderFuncVLK(
        [l_this, mapScene, framePlan, transparentMeshes, frameInputParams, stepBufferUpdates]() -> void {
            {
                ZoneScopedN("Post Load");
                //Do postLoad here. So creation of stuff is done from main thread
                if (stepBufferUpdates)
                    mapScene->doPostLoad(l_this, framePlan);

                for (auto &renderTarget : frameInputParams->frameParameters->renderTargets) {
                    auto updatingTarget = std::dynamic_pointer_cast<RendererViewClass>(renderTarget.target);
                    if (!updatingTarget) updatingTarget = l_this->defaultView;

                    updatingTarget->setFxaaEnabled(l_this->m_config->enableFXAA);
                    updatingTarget->update(
                        renderTarget.viewPortDimensions.maxs[0],
                        renderTarget.viewPortDimensions.maxs[1],
                        framePlan->frameDependentData->currentGlow,
                        framePlan->pointLights,
                        framePlan->spotLights,
                        framePlan->insideSpotLights
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
        },
        [l_this, stepBufferUpdates](CmdBufRecorder &uploadCmd) -> void {
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
    }, [transparentMeshes,
            l_opaqueMeshes = std::move(u_collector),
            skyMeshes,
            renderSky,
            mapScene,
            framePlan,
            l_this,
            frameInputParams,
            cullViewProj,
            gpuFrameDraws
        ](CmdBufRecorder &frameBufCmd, CmdBufRecorder &swapChainCmd) -> void {

        TracyMessageStr(("Draw stage frame = " + std::to_string(FrameContext::getCurrentProcessingFrameNumber())));

        // ----------------------
        // Phase 1: Frustum Culling → GBuffer pass (depth fill)
        // ----------------------
        l_this->m_gpuDraws.recordFrustumCull(frameBufCmd, cullViewProj, *gpuFrameDraws);

        // ----------------------
        // Draw meshes
        // ----------------------
        {
            uint8_t wideChunkVersion = 0;
            uint32_t renderTargetIndex = 0;
            for (auto &renderTarget : frameInputParams->frameParameters->renderTargets) {
                const uint32_t currentTargetIndex = renderTargetIndex++;
                l_this->sceneWideChunk->setCurrentVersion(wideChunkVersion++);

                auto currentView = renderTarget.target == nullptr ?
                                   l_this->defaultView :
                                   std::dynamic_pointer_cast<RendererViewClass>(renderTarget.target);

                currentView->updateExternalDSes();

                // Record the liquid transparent draws once per target; they are executed
                // twice below (below-liquid and above-liquid passes), differing only in
                // the render-mode SSBO updated on the primary buffer between executions.
                auto liquidSecondaryCmd = l_this->recordLiquidTransparentSecondary(
                    currentView, currentTargetIndex, transparentMeshes, *gpuFrameDraws);
                {
                    {
                        auto debugHelper = frameBufCmd.beginDebugLabel("GBuffer pass", {0, 0, 0.5f});
                        VkZone(frameBufCmd, "GBuffer pass");
                        auto passHelper = currentView->beginGBufferPass(frameBufCmd, false,
                                                                        frameInputParams->frameParameters->clearColor);
                        frameBufCmd.setGBufferMode(true);
                        {
                            auto debugHelper = frameBufCmd.beginDebugLabel("GBuffer ADT", {0, 0, 0.5f});
                            VkZone(frameBufCmd, "GBuffer ADT");
                            l_opaqueMeshes->renderADT(frameBufCmd, CmdBufRecorder::ViewportType::vp_usual);
                        }
                        {
                            auto debugHelper = frameBufCmd.beginDebugLabel("GBuffer Opaque", {0, 0, 0.5f});
                            VkZone(frameBufCmd, "GBuffer Opaque Indirect");
                            l_this->m_gpuDraws.drawOpaque(frameBufCmd,
                                                          CmdBufRecorder::ViewportType::vp_usual,
                                                          *gpuFrameDraws);
                        }
                        frameBufCmd.setGBufferMode(false);
                    }

                    // ----------------------
                    // Phase 2: Hi-Z generation → Occlusion Culling → re-cull indirect draws
                    // ----------------------
                    {
                        auto debugHelper = frameBufCmd.beginDebugLabel("Hi-Z + Occlusion Cull", {0, 0, 0.5f});
                        VkZone(frameBufCmd, "Hi-Z + Occlusion Cull");
                        currentView->doHiZOcclusionCulling(frameBufCmd, cullViewProj,
                                                            l_this->m_gpuDraws.aabbBuffer(),
                                                            l_this->m_gpuDraws.visibilityBuffer(),
                                                            gpuFrameDraws->totalCullCount);
                        l_this->m_gpuDraws.recordOcclusionRecull(frameBufCmd, *gpuFrameDraws);
                    }
                    currentView->doGBufferBarrier(frameBufCmd);

                    currentView->doLightPass(frameBufCmd);

                    // =============================================
                    // Liquid Depth Pass (depth-only, separate FBO — no forward pass dependency)
                    // =============================================
                    {
                        VkZone(frameBufCmd, "Liquid Depth Pass");
                        auto liquidPassHelper = currentView->beginLiquidDepthPass(frameBufCmd);
                        frameBufCmd.setZPrefillMode(true);
                        l_opaqueMeshes->renderWater(frameBufCmd, CmdBufRecorder::ViewportType::vp_usual);
                        frameBufCmd.setZPrefillMode(false);
                    }

                    // =============================================
                    // Compute Phase: Liquid HiZ + Classification + Cull
                    // =============================================
                    {
                        VkZone(frameBufCmd, "Liquid HiZ + Classify");
                        currentView->doLiquidHiZ(frameBufCmd);
                        l_this->m_gpuDraws.recordLiquidClassifyAndRecullBelow(frameBufCmd, cullViewProj,
                                                                              currentView->getLiquidDepthPass()->getInvertZ(),
                                                                              currentView->getLiquidHiZ(),
                                                                              *gpuFrameDraws);
                    }

                    // =============================================
                    // Forward Pass #1: Opaque + Sky
                    // =============================================
                    {
                        auto debugHelper = frameBufCmd.beginDebugLabel("Forward Pass", {0, 0, 0.5f});
                        auto passHelper = currentView->beginForwardPass(frameBufCmd, false, false,
                                                                        frameInputParams->frameParameters->clearColor);

                        {
                            auto debugHelper = frameBufCmd.beginDebugLabel("Forward Pass: ADT", {0, 0, 0.5f});
                            VkZone(frameBufCmd, "ADT Opaque");
                            l_opaqueMeshes->renderADT(frameBufCmd, CmdBufRecorder::ViewportType::vp_usual);
                        }
                        {
                            auto debugHelper = frameBufCmd.beginDebugLabel("Forward Pass: Opaque indirect", {0, 0, 0.5f});
                            VkZone(frameBufCmd, "Opaque Indirect");
                            l_this->m_gpuDraws.drawOpaque(frameBufCmd,
                                                          CmdBufRecorder::ViewportType::vp_usual,
                                                          *gpuFrameDraws);
                        }
                        {
                            VkZone(frameBufCmd, "Decals pass");
                            auto debugHelper = frameBufCmd.beginDebugLabel("Decals Pass", {0, 0, 0.5f});
                            l_opaqueMeshes->renderProjective(frameBufCmd, CmdBufRecorder::ViewportType::vp_usual);
                        }

                        if (l_this->m_config->drawDebugLights) {
                            currentView->doDebugLightPass(frameBufCmd);
                        }

                        {
                            VkZone(frameBufCmd, "Sky pass");
                            for (int i = 0; i < skyMeshes->size(); i++) {
                                auto const &mesh = skyMeshes->at(i);

                                MapSceneRenderDeferredVLK::drawMesh(frameBufCmd, mesh,
                                                                    CmdBufRecorder::ViewportType::vp_skyBox);
                            }
                        }
                    } // END Forward Pass #1

                    // =============================================
                    // Forward Pass #2: Below-liquid transparent
                    // =============================================
                    {
                        auto debugHelper = frameBufCmd.beginDebugLabel("Below Liquid Pass", {0, 0, 0.5f});
                        // Set render mode to "below liquid" (0)
                        uint32_t modeBelow = 0;
                        vkCmdUpdateBuffer(frameBufCmd.getNativeCmdBuffer(),
                                          l_this->m_renderModeBuffer->getGPUBuffer(),
                                          0, sizeof(uint32_t), &modeBelow);
                        {
                            VkMemoryBarrier memBarrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER};
                            memBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                            memBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                            vkCmdPipelineBarrier(frameBufCmd.getNativeCmdBuffer(),
                                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                                 0, 1, &memBarrier, 0, nullptr, 0, nullptr);
                        }

                        auto passHelper = currentView->beginForwardPass(frameBufCmd, true, false,
                                                                        frameInputParams->frameParameters->clearColor, false);
                        frameBufCmd.executeSecondaryCmdBuffer(liquidSecondaryCmd);
                    } // END Forward Pass #2

                    // =============================================
                    // Forward Pass #2b: Liquid color
                    // =============================================
                    {
                        // Copy current forward color to backbuffer texture for liquid refraction
                        VkZone(frameBufCmd, "Copy Liquid BackBuffer");
                        currentView->copyForwardColorToLiquidBackBuffer(frameBufCmd);
                    }
                    {
                        auto passHelper = currentView->beginForwardPass(frameBufCmd, false, false,
                                                                        frameInputParams->frameParameters->clearColor, false);
                        {
                            auto debugHelper = frameBufCmd.beginDebugLabel("Render Liquid", {0, 0, 0.5f});
                            VkZone(frameBufCmd, "Render Liquid");
                            l_opaqueMeshes->renderWater(frameBufCmd, CmdBufRecorder::ViewportType::vp_usual);
                        }
                    } // END Forward Pass #2b

                    // =============================================
                    // Restore indirect buffer + re-cull for above-liquid
                    // =============================================
                    l_this->m_gpuDraws.recordRestoreAndRecullAbove(frameBufCmd, *gpuFrameDraws);

                    // =============================================
                    // Forward Pass #3: Above-liquid transparent
                    // =============================================
                    {
                        auto debugHelper = frameBufCmd.beginDebugLabel("Above liquid pass", {0, 0, 0.5f});
                        // Set render mode to "above liquid" (1)
                        uint32_t modeAbove = 1;
                        vkCmdUpdateBuffer(frameBufCmd.getNativeCmdBuffer(),
                                          l_this->m_renderModeBuffer->getGPUBuffer(),
                                          0, sizeof(uint32_t), &modeAbove);
                        {
                            VkMemoryBarrier memBarrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER};
                            memBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                            memBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                            vkCmdPipelineBarrier(frameBufCmd.getNativeCmdBuffer(),
                                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                                 0, 1, &memBarrier, 0, nullptr, 0, nullptr);
                        }

                        auto passHelper = currentView->beginForwardPass(frameBufCmd, true, false,
                                                                        frameInputParams->frameParameters->clearColor, false);
                        frameBufCmd.executeSecondaryCmdBuffer(liquidSecondaryCmd);
                    } // END Forward Pass #3

                    // =============================================
                    // Object-id pick readback (async, only when requested this frame)
                    // =============================================
                    if (l_this->m_config->enableObjectPicking &&
                        l_this->m_device->supportsSelection() &&
                        frameInputParams->frameParameters->pickRequestPixel &&
                        &renderTarget == &frameInputParams->frameParameters->renderTargets.front()) {
                        auto &pixel = *frameInputParams->frameParameters->pickRequestPixel;
                        uint32_t pickFrameNum = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

                        VkZone(frameBufCmd, "Object Pick Copy");
                        auto debugHelper = frameBufCmd.beginDebugLabel("Object Pick Copy", {0.5f, 0, 0.5f});
                        currentView->copyObjectIdPixelToBuffer(frameBufCmd, pixel[0], pixel[1],
                                                               l_this->m_pickResultBuffer[pickFrameNum]);
                        l_this->m_pickResultPending[pickFrameNum] = true;
                        l_this->m_pickResultIsHoverPeek[pickFrameNum] = frameInputParams->frameParameters->pickIsHoverPeek;
                    }

                    if (l_this->m_config->drawM2BB || l_this->m_config->drawWmoBB || l_this->m_selectedAABBValid) {
                        auto debugHelper = frameBufCmd.beginDebugLabel("AABB Debug Pass", {0, 0.5f, 0});
                        auto passHelper = currentView->beginForwardPass(frameBufCmd, false, false,
                                                                        frameInputParams->frameParameters->clearColor, false);
                        l_this->drawAABBDebug(frameBufCmd, *gpuFrameDraws);
                    }

                    // Reset render mode to "all" (2) for subsequent passes
                    {
                        uint32_t modeAll = 2;
                        vkCmdUpdateBuffer(frameBufCmd.getNativeCmdBuffer(),
                                          l_this->m_renderModeBuffer->getGPUBuffer(),
                                          0, sizeof(uint32_t), &modeAll);
                    }
                }
                {
                    currentView->doPostGlow(frameBufCmd);
                    if (currentView == l_this->defaultView) {
                        if (l_this->m_config->enableFXAA) {
                            currentView->doComposite(frameBufCmd);
                            currentView->doPostFinalFxaa(swapChainCmd);
                        } else {
                            currentView->doPostFinal(swapChainCmd);
                        }
                    } else {
                        currentView->doOutputPass(frameBufCmd);
                    }
                }
            }
        }
    });
}

std::shared_ptr<MapRenderPlan> MapSceneRenderDeferredVLK::getLastCreatedPlan() {
    return m_lastCreatedPlan;
}

std::shared_ptr<GCommandBuffer> MapSceneRenderDeferredVLK::recordLiquidTransparentSecondary(
    const std::shared_ptr<RendererViewClass> &currentView,
    uint32_t renderTargetIndex,
    const std::shared_ptr<framebased::vector<HGSortableMesh>> &transparentMeshes,
    const GpuIndirectDrawsVLK::FrameDrawData &gpuFrameDraws) {

    ZoneScopedN("Record liquid transparent secondary");

    const uint32_t frameNum = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;

    if (m_liquidSecondaryCmdBufs.size() <= renderTargetIndex)
        m_liquidSecondaryCmdBufs.resize(renderTargetIndex + 1);

    auto &cmdBuf = m_liquidSecondaryCmdBufs[renderTargetIndex][frameNum];
    if (cmdBuf == nullptr)
        cmdBuf = m_device->createSecondaryCommandBuffer();

    // The secondary buffer continues the forward no-clear render pass instance that the
    // below/above liquid passes open on the primary command buffer. It is executed twice
    // into the same primary buffer per frame (below-liquid and above-liquid passes), so it
    // must be recorded with VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
    // (VUID-vkCmdExecuteCommands-pCommandBuffers-00092). No Tracy GPU zones here:
    // secondary buffers have no tracy context (debug labels are still valid).
    auto recorder = cmdBuf->beginRecord(currentView->getForwardPassNoClear(), true);
    recorder.setSecondaryCmdRenderArea({0, 0}, currentView->getForwardRenderAreaSize());

    {
        auto debugHelper = recorder.beginDebugLabel("Transparent Liquid Indirect", {0, 0, 0.5f});
        m_gpuDraws.drawTransparent(recorder, CmdBufRecorder::ViewportType::vp_usual, gpuFrameDraws);
    }
    {
        auto debugHelper = recorder.beginDebugLabel("Transparent Liquid Direct", {0, 0, 0.5f});
        for (int i = 0; i < transparentMeshes->size(); i++) {
            auto const &mesh = transparentMeshes->at(i);
#ifdef DEBUG_MESH_NAMES
            std::string debugMess =
                "Drawing mesh "
                " meshType = " + std::to_string((int)mesh->getMeshType()) +
                " priorityPlane = " + std::to_string(mesh->priorityPlane()) +
                " sortDistance = " + std::to_string(mesh->getSortDistance()) +
                " blendMode = " + std::to_string((int)mesh->getGxBlendMode());
            auto debugLabel = recorder.beginDebugLabel(debugMess, {1.0, 0, 0, 1.0});
#endif
            MapSceneRenderDeferredVLK::drawMesh(recorder, mesh, CmdBufRecorder::ViewportType::vp_usual);
        }
    }

    return cmdBuf;
}

const PipelineTemplate s_LiquidPipelineTemplate = []{
    PipelineTemplate pipelineTemplate;
    pipelineTemplate.element = DrawElementMode::TRIANGLES;
    pipelineTemplate.depthWrite = true;
    pipelineTemplate.depthCulling = true;
    pipelineTemplate.backFaceCulling = false;
    pipelineTemplate.blendMode = EGxBlendEnum::GxBlend_Alpha;
    return pipelineTemplate;
}();

std::shared_ptr<ILiquidMaterial> MapSceneRenderDeferredVLK::createWaterLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<WaterLiquidData> &liquidData) {
    auto liquidInstanceData = std::make_shared<CBufferChunkVLK<Liquid::LiquidInstance>>(uboStaticBuffer);
    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"liquidShader", "waterLiquidShader"}, waterLiquidShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_forwardRenderPass, s_LiquidPipelineTemplate)
        .createZPrefillPipeline(m_emptyWaterVAO, {"liquidDepthShader", "liquidDepthShader"}, liquidDepthShaderConfig, m_liquidDepthRenderPass)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &modelWide, &liquidInstanceData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(modelWide))
                .ubo(1, *liquidInstanceData)
                .delayUpdate();
        })
        .bindDescriptorSet(2, std::static_pointer_cast<LiquidDataForwardVLK<Liquid::WaterData>>(liquidData)->getDescriptorSet())
        .bindDescriptorSet(3, gBufferDataDS)
        .toMaterial<ILiquidMaterial>([&liquidData](ILiquidMaterial *instance) -> void {
            instance->m_ldData = liquidData;
        });
    material->m_instance = liquidInstanceData;
    return material;
}
std::shared_ptr<ILiquidMaterial> MapSceneRenderDeferredVLK::createMagmaLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<MagmaLiquidData> &liquidData) {
    auto liquidInstanceData = std::make_shared<CBufferChunkVLK<Liquid::LiquidInstance>>(uboStaticBuffer);
    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"liquidShader", "magmaLiquidShader"}, liquidShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_forwardRenderPass, s_LiquidPipelineTemplate)
        .createZPrefillPipeline(m_emptyWaterVAO, {"liquidDepthShader", "liquidDepthShader"}, liquidDepthShaderConfig, m_liquidDepthRenderPass)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &modelWide, &liquidInstanceData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(modelWide))
                .ubo(1, *liquidInstanceData)
                .delayUpdate();
        })
        .bindDescriptorSet(2, std::static_pointer_cast<LiquidDataForwardVLK<Liquid::MagmaData>>(liquidData)->getDescriptorSet())
        .toMaterial<ILiquidMaterial>([&liquidData](ILiquidMaterial *instance) -> void {
            instance->m_ldData = liquidData;
        });

    material->m_instance = liquidInstanceData;
    return material;
}
std::shared_ptr<ILiquidMaterial> MapSceneRenderDeferredVLK::createMercuryLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<MercuryLiquidData> &liquidData) {
    auto liquidInstanceData = std::make_shared<CBufferChunkVLK<Liquid::LiquidInstance>>(uboStaticBuffer);
    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"liquidShader", "mercuryLiquidShader"}, mercuryLiquidShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_forwardRenderPass, s_LiquidPipelineTemplate)
        .createZPrefillPipeline(m_emptyWaterVAO, {"liquidDepthShader", "liquidDepthShader"}, liquidDepthShaderConfig, m_liquidDepthRenderPass)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &modelWide, &liquidInstanceData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(modelWide))
                .ubo(1, *liquidInstanceData)
                .delayUpdate();
        })
        .bindDescriptorSet(2, std::static_pointer_cast<LiquidDataForwardVLK<Liquid::MercuryData>>(liquidData)->getDescriptorSet())
        .bindDescriptorSet(3, gBufferDataDS)
        .toMaterial<ILiquidMaterial>([&liquidData](ILiquidMaterial *instance) -> void {
            instance->m_ldData = liquidData;
        });
    material->m_instance = liquidInstanceData;
    return material;
}
std::shared_ptr<ILiquidMaterial> MapSceneRenderDeferredVLK::createFogLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<FogLiquidData> &liquidData) {
    auto liquidInstanceData = std::make_shared<CBufferChunkVLK<Liquid::LiquidInstance>>(uboStaticBuffer);
    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"liquidShader", "fogLiquidShader"}, fogLiquidShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_forwardRenderPass, s_LiquidPipelineTemplate)
        .createZPrefillPipeline(m_emptyWaterVAO, {"liquidDepthShader", "liquidDepthShader"}, liquidDepthShaderConfig, m_liquidDepthRenderPass)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &modelWide, &liquidInstanceData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(modelWide))
                .ubo(1, *liquidInstanceData)
                .delayUpdate();
        })
        .bindDescriptorSet(2, std::static_pointer_cast<LiquidDataForwardVLK<Liquid::FogData>>(liquidData)->getDescriptorSet())
        .bindDescriptorSet(3, gBufferDataDS)
        .toMaterial<ILiquidMaterial>([&liquidData](ILiquidMaterial *instance) -> void {
            instance->m_ldData = liquidData;
        });
    material->m_instance = liquidInstanceData;
    return material;
}
std::shared_ptr<ILiquidMaterial> MapSceneRenderDeferredVLK::createLeyLineLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<LeyLineLiquidData> &liquidData) {
    auto liquidInstanceData = std::make_shared<CBufferChunkVLK<Liquid::LiquidInstance>>(uboStaticBuffer);
    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"liquidShader", "leyLineLiquidShader"}, leyLineLiquidShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_forwardRenderPass, s_LiquidPipelineTemplate)
        .createZPrefillPipeline(m_emptyWaterVAO, {"liquidDepthShader", "liquidDepthShader"}, liquidDepthShaderConfig, m_liquidDepthRenderPass)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &modelWide, &liquidInstanceData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(modelWide))
                .ubo(1, *liquidInstanceData)
                .delayUpdate();
        })
        .bindDescriptorSet(2, std::static_pointer_cast<LiquidDataForwardVLK<Liquid::LeyLineData>>(liquidData)->getDescriptorSet())
        .bindDescriptorSet(3, gBufferDataDS)
        .toMaterial<ILiquidMaterial>([&liquidData](ILiquidMaterial *instance) -> void {
            instance->m_ldData = liquidData;
        });
    material->m_instance = liquidInstanceData;
    return material;
}
std::shared_ptr<ILiquidMaterial> MapSceneRenderDeferredVLK::createFelLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<FelLiquidData> &liquidData) {
    auto liquidInstanceData = std::make_shared<CBufferChunkVLK<Liquid::LiquidInstance>>(uboStaticBuffer);
    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"liquidShader", "felLiquidShader"}, felLiquidShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_forwardRenderPass, s_LiquidPipelineTemplate)
        .createZPrefillPipeline(m_emptyWaterVAO, {"liquidDepthShader", "liquidDepthShader"}, liquidDepthShaderConfig, m_liquidDepthRenderPass)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &modelWide, &liquidInstanceData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(modelWide))
                .ubo(1, *liquidInstanceData)
                .delayUpdate();
        })
        .bindDescriptorSet(2, std::static_pointer_cast<LiquidDataForwardVLK<Liquid::FelData>>(liquidData)->getDescriptorSet())
        .bindDescriptorSet(3, gBufferDataDS)
        .toMaterial<ILiquidMaterial>([&liquidData](ILiquidMaterial *instance) -> void {
            instance->m_ldData = liquidData;
        });
    material->m_instance = liquidInstanceData;
    return material;
}
std::shared_ptr<ILiquidMaterial> MapSceneRenderDeferredVLK::createSwampLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<SwampLiquidData> &liquidData) {
    auto liquidInstanceData = std::make_shared<CBufferChunkVLK<Liquid::LiquidInstance>>(uboStaticBuffer);
    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"liquidShader", "swampLiquidShader"}, swampLiquidShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_forwardRenderPass, s_LiquidPipelineTemplate)
        .createZPrefillPipeline(m_emptyWaterVAO, {"liquidDepthShader", "liquidDepthShader"}, liquidDepthShaderConfig, m_liquidDepthRenderPass)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &modelWide, &liquidInstanceData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(modelWide))
                .ubo(1, *liquidInstanceData)
                .delayUpdate();
        })
        .bindDescriptorSet(2, std::static_pointer_cast<LiquidDataForwardVLK<Liquid::SwampData>>(liquidData)->getDescriptorSet())
        .bindDescriptorSet(3, gBufferDataDS)
        .toMaterial<ILiquidMaterial>([&liquidData](ILiquidMaterial *instance) -> void {
            instance->m_ldData = liquidData;
        });
    material->m_instance = liquidInstanceData;
    return material;
}
std::shared_ptr<ILiquidMaterial> MapSceneRenderDeferredVLK::createAzeritheLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<AzeritheLiquidData> &liquidData) {
    auto liquidInstanceData = std::make_shared<CBufferChunkVLK<Liquid::LiquidInstance>>(uboStaticBuffer);
    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"liquidShader", "azeritheLiquidShader"}, azeritheLiquidShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_forwardRenderPass, s_LiquidPipelineTemplate)
        .createZPrefillPipeline(m_emptyWaterVAO, {"liquidDepthShader", "liquidDepthShader"}, liquidDepthShaderConfig, m_liquidDepthRenderPass)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &modelWide, &liquidInstanceData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(modelWide))
                .ubo(1, *liquidInstanceData)
                .delayUpdate();
        })
        .bindDescriptorSet(2, std::static_pointer_cast<LiquidDataForwardVLK<Liquid::AzeritheData>>(liquidData)->getDescriptorSet())
        .bindDescriptorSet(3, gBufferDataDS)
        .toMaterial<ILiquidMaterial>([&liquidData](ILiquidMaterial *instance) -> void {
            instance->m_ldData = liquidData;
        });
    material->m_instance = liquidInstanceData;
    return material;
}


HGMesh MapSceneRenderDeferredVLK::createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) {
    auto mesh = deferredFactoryVlk->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0,0);
    m_gpuDraws.registerMesh(mesh);
    return mesh;
}

HGSortableMesh MapSceneRenderDeferredVLK::createSortableMesh(gMeshTemplate &meshTemplate, const HMaterial &material, int priorityPlane) {
    auto mesh = deferredFactoryVlk->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0, priorityPlane);
    m_gpuDraws.registerMesh(mesh);
    return mesh;
}

HGSortableMesh MapSceneRenderDeferredVLK::createWaterMesh(gMeshTemplate &meshTemplate, const HMaterial &material, int priorityPlane) {
    auto mesh = deferredFactoryVlk->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0, priorityPlane);
    m_gpuDraws.registerMesh(mesh);
    return mesh;
}

HGMesh
MapSceneRenderDeferredVLK::createAdtMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IADTMaterial> &material) {
    ZoneScoped;

    auto mesh = deferredFactoryVlk->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0, 0);
    m_gpuDraws.registerMesh(mesh);
    return mesh;
}
HGM2Mesh
MapSceneRenderDeferredVLK::createM2Mesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2Material> &material,
                                        int layer, int priorityPlane) {
    auto mesh = deferredFactoryVlk->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), layer, priorityPlane);
    m_gpuDraws.registerMesh(mesh);
    return mesh;
}
HGM2Mesh
MapSceneRenderDeferredVLK::createM2ProjectiveMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2ProjectiveMaterial> &material, int layer, int priorityPlane) {
    ZoneScoped;
    // Projective/decal meshes don't draw the M2's own submesh geometry — they draw a unit
    // bbox cube that the shader unprojects against the G-buffer, same as the bindless variant.
    meshTemplate.bindings = m_drawBBoxVao;
    auto mesh = deferredFactoryVlk->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), layer, priorityPlane);
    mesh->vertexStart = 0;
    mesh->start() = 0;
    mesh->end() = 36;
    m_gpuDraws.registerMesh(mesh);
    return mesh;
}

HGM2Mesh
MapSceneRenderDeferredVLK::createM2ParticleMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2Material> &material, int layer, int priorityPlane) {
    auto mesh = deferredFactoryVlk->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0, priorityPlane);
    m_gpuDraws.registerMesh(mesh);
    return mesh;
}

HGSortableMesh MapSceneRenderDeferredVLK::createWMOMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IWMOMaterial> &material, int groupNum, int canHaveExteriorLit, uint32_t wmoObjId) {
    auto mesh = deferredFactoryVlk->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0, 0);
    // Bits [30:11] = wmoObjId (for GPU object-id picking), bit 10 = canHaveExteriorLit, bits [9:0] = groupNum
    mesh->instanceIndex = (int)((wmoObjId & 0xFFFFF) << 11) | ((canHaveExteriorLit & 1) << 10) | (groupNum & ((1 << 10) -1));
    m_gpuDraws.registerMesh(mesh);
    return mesh;
}
HGM2Mesh MapSceneRenderDeferredVLK::createM2WaterfallMesh(gMeshTemplate &meshTemplate,
                                                          const std::shared_ptr<IM2WaterFallMaterial> &material,
                                                          int layer, int priorityPlane) {
    auto mesh = deferredFactoryVlk->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), layer, priorityPlane);
    m_gpuDraws.registerMesh(mesh);
    return mesh;
}

std::shared_ptr<IRenderView> MapSceneRenderDeferredVLK::createRenderView(bool createOutput) {
    return std::make_shared<RendererViewClass>(m_device, uboBuffer,
                                               pointLightBuffer, spotLightBuffer,
                                               sceneWideDS,
                                               gBufferDataDS,
                                               m_drawQuadVao,
                                               m_drawSpotVao,
                                               m_drawSpotVaoLine,
                                               createOutput);
}

// Explicit template instantiations
template class LiquidDataForwardVLK<Liquid::WaterData>;
template class LiquidDataForwardVLK<Liquid::MagmaData>;
template class LiquidDataForwardVLK<Liquid::MercuryData>;
template class LiquidDataForwardVLK<Liquid::FogData>;
template class LiquidDataForwardVLK<Liquid::LeyLineData>;
template class LiquidDataForwardVLK<Liquid::FelData>;
template class LiquidDataForwardVLK<Liquid::SwampData>;
template class LiquidDataForwardVLK<Liquid::AzeritheData>;

std::shared_ptr<WaterLiquidData> MapSceneRenderDeferredVLK::createWaterLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::WaterData>>(liquidBuffers.waterLiquidBuffer);
    return std::make_shared<LiquidDataForwardVLK<Liquid::WaterData>>(waterLiquidTexturesDS, liquidData);
}

std::shared_ptr<MagmaLiquidData> MapSceneRenderDeferredVLK::createMagmaLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::MagmaData>>(liquidBuffers.magmaLiquidBuffer);
    return std::make_shared<LiquidDataForwardVLK<Liquid::MagmaData>>(magmaLiquidTexturesDS, liquidData);
}

std::shared_ptr<MercuryLiquidData> MapSceneRenderDeferredVLK::createMercuryLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::MercuryData>>(liquidBuffers.mercuryLiquidBuffer);
    return std::make_shared<LiquidDataForwardVLK<Liquid::MercuryData>>(mercuryLiquidTexturesDS, liquidData);
}

std::shared_ptr<FogLiquidData> MapSceneRenderDeferredVLK::createFogLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::FogData>>(liquidBuffers.fogLiquidBuffer);
    return std::make_shared<LiquidDataForwardVLK<Liquid::FogData>>(fogLiquidTexturesDS, liquidData);
}

std::shared_ptr<LeyLineLiquidData> MapSceneRenderDeferredVLK::createLeyLineLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::LeyLineData>>(liquidBuffers.leyLineLiquidBuffer);
    return std::make_shared<LiquidDataForwardVLK<Liquid::LeyLineData>>(leyLineLiquidTexturesDS, liquidData);
}

std::shared_ptr<FelLiquidData> MapSceneRenderDeferredVLK::createFelLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::FelData>>(liquidBuffers.felLiquidBuffer);
    return std::make_shared<LiquidDataForwardVLK<Liquid::FelData>>(felLiquidTexturesDS, liquidData);
}

std::shared_ptr<SwampLiquidData> MapSceneRenderDeferredVLK::createSwampLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::SwampData>>(liquidBuffers.swampLiquidBuffer);
    return std::make_shared<LiquidDataForwardVLK<Liquid::SwampData>>(swampLiquidTexturesDS, liquidData);
}

std::shared_ptr<AzeritheLiquidData> MapSceneRenderDeferredVLK::createAzeritheLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::AzeritheData>>(liquidBuffers.azeritheLiquidBuffer);
    return std::make_shared<LiquidDataForwardVLK<Liquid::AzeritheData>>(azeritheLiquidTexturesDS, liquidData);
}
