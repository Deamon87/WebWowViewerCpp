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

static const DescTypeSetBindingConfig SceneDataSetConfig = {
    {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
    {1, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {2, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {3, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {4, {VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}}
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

const int m2TexturesBindlessCount = 4096;
const int m2WaterfallTexturesBindlessCount = 128;
const int adtTexturesBindlessCount = 4096;
const int waterTexturesBindlessCount = 1024;

static const ShaderConfig m2BindlessShaderConfig = {
    "bindless/m2/forward",
    "bindless/m2/forward",
    {
        {0, SceneDataSetConfig},
        {1, {
            {1, {VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
            {6, {VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
            {10, {VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}}
        }},
        {2, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, true, m2TexturesBindlessCount, VK_SHADER_STAGE_FRAGMENT_BIT}}
        }}
    }};
static const ShaderConfig m2ProjectedBindlessShaderConfig = {
    "bindless/projected",
    "bindless/projected",
    {
            {0, SceneDataSetConfig},
            {1, m2BindlessShaderConfig.typeOverrides.at(1)},
            {2, {
                {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, true, m2TexturesBindlessCount}}
            }},
            {3, GBufferDataSetConfig}
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
        {0, SceneDataSetConfig},
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
        {0, SceneDataSetConfig},
        {3, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, true, m2WaterfallTexturesBindlessCount}}
        }}
    }};

static const ShaderConfig wmoBindlessShaderConfig = {
    "bindless/wmo/forward",
    "bindless/wmo/forward",
    {
        {0, SceneDataSetConfig },
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
        {0, SceneDataSetConfig },
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
        {0, SceneDataSetConfig },
        {2, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, true, waterTexturesBindlessCount}}
        }}
    }};

// Per-instance liquid data (set 1) shared by all liquid types: placement matrices,
// LiquidBindless records, 8 per-type data arrays and per-instance LiquidInstanceData
static const DescTypeSetBindingConfig LiquidBindlessDataSetConfig = {
    {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
    {1, {VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
    {2, {VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {3, {VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {4, {VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {5, {VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {6, {VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {7, {VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {8, {VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {9, {VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {10, {VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}}
};

static const ShaderConfig liquidBindlessShaderConfig = {
    "bindless/liquids",
    "bindless/liquids",
    {
        {0, SceneDataSetConfig },
        {1, LiquidBindlessDataSetConfig },
        {2, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, true, waterTexturesBindlessCount, VK_SHADER_STAGE_FRAGMENT_BIT}}
        }},
        {3, GBufferDataSetConfig}
    }};

// Depth-only liquid variant used by the liquid depth pass (no color attachments)
static const ShaderConfig liquidBindlessDepthShaderConfig = {
    "bindless/liquids/depth",
    "bindless/liquids/depth",
    {
            {0, SceneDataSetConfig },
            {1, LiquidBindlessDataSetConfig },
            {2, {
                {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, true, waterTexturesBindlessCount, VK_SHADER_STAGE_FRAGMENT_BIT}}
            }},
            {3, GBufferDataSetConfig}
    }};

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

auto bindlessMeshFactoryVlk = std::make_shared<EntityFactory<10000, GMeshId, GMeshVLK>>();

MapSceneRenderBindlessVLK::MapSceneRenderBindlessVLK(const HGDeviceVLK &hDevice, Config *config) :
    m_device(hDevice), MapSceneRenderer(config),
    m_gpuDraws(hDevice, config,
        [this](uint32_t matId) -> std::shared_ptr<ISimpleMaterialVLK> {
            // Static materials are registered in exactly one of the caches
            if (auto m = m_m2MatCacheId[matId].lock()) return m;
            return m_wmoMatCacheId[matId].lock();
        }),
    m_gpuM2Animation(hDevice, config) {
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
        this->m2Buffers.projectiveData,

        this->m2WaterfallBuffer.waterfallCommon,
        this->m2WaterfallBuffer.waterfallBindless,

        this->adtBuffers.adtMeshWidePSes,
        this->adtBuffers.adtMeshWideVSPSes,
        this->adtBuffers.adtInstanceDatas,

        this->wmoBuffers.wmoPlacementMats,
        this->wmoBuffers.wmoMeshWideVSes,
        this->wmoBuffers.wmoMeshWidePSes,
        this->wmoBuffers.wmoMeshWideBindless,
        this->wmoBuffers.wmoPerMeshData,
        this->wmoBuffers.wmoGroupInteriorData,

//        this->liquidBuffers.dataLiquidBuffer,
        this->liquidBuffers.liquidBindlessBuffer,
        this->liquidBuffers.liquidInstanceBuffer,

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

    //Create m2 shaders
    {
        m2Buffers.placementMatrix = m_device->createSSBOBuffer(un("M2 Placement"), 1024*1024, sizeof(M2::PlacementMatrix));
        m2Buffers.boneMatrix = m_device->createSSBOBuffer(un("M2 BoneMatrices"),1024*1024, sizeof(mathfu::mat4));
        m2Buffers.m2Colors = m_device->createSSBOBuffer(un("M2 Colors"), 1024*1024, sizeof(mathfu::vec4_packed));
        m2Buffers.textureWeights = m_device->createSSBOBuffer(un("M2 TextureWeight"), 1024*1024, sizeof(mathfu::vec4_packed));
        m2Buffers.textureMatrices = m_device->createSSBOBuffer(un("M2 TextureMatrices"), 1024*1024, sizeof(mathfu::mat4));
        m2Buffers.modelFragmentDatas = m_device->createSSBOBuffer(un("M2 FragmentData"), 1024*1024, sizeof(M2::modelWideBlockPS));
        m2Buffers.m2InstanceData = m_device->createSSBOBuffer(un("M2 InstanceData"), 1024*1024, sizeof(M2::M2InstanceRecordBindless));
        m2Buffers.meshWideBlocks = m_device->createSSBOBuffer(un("M2 MeshWide"), 1024*1024, sizeof(M2::meshWideBlockVSPS));
        m2Buffers.meshWideBlocksBindless = m_device->createSSBOBuffer(un("M2 MeshWide Bindless"), 1024*1024, sizeof(M2::meshWideBlockVSPS_Bindless));
        m2Buffers.projectiveData = m_device->createSSBOBuffer(un("M2 Projective Data"), 64, sizeof(M2::ProjectiveData));
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
        wmoBuffers.wmoGroupInteriorData = m_device->createSSBOBuffer(un("WMO Group Interior Data"),1024*1024, sizeof(WMO::InteriorBlockData));
    }
    //Create water buffs
    {
//        liquidBuffers.dataLiquidBuffer = m_device->createSSBOBuffer(un("Scene_DataLiquidBuffer"), 1024, 16);

        liquidBuffers.waterLiquidBuffer = m_device->createSSBOBuffer(un("Scene_WaterLiquidBuffer"), 1024, sizeof(Liquid::WaterData));
        liquidBuffers.magmaLiquidBuffer = m_device->createSSBOBuffer(un("Scene_MagmaLiquidBuffer"), 1024, sizeof(Liquid::MagmaData));
        liquidBuffers.mercuryLiquidBuffer = m_device->createSSBOBuffer(un("Scene_MercuryLiquidBuffer"), 1024, sizeof(Liquid::MercuryData));
        liquidBuffers.fogLiquidBuffer = m_device->createSSBOBuffer(un("Scene_FogLiquidBuffer"), 1024, sizeof(Liquid::FogData));
        liquidBuffers.leyLineLiquidBuffer = m_device->createSSBOBuffer(un("Scene_LeyLineLiquidBuffer"), 1024, sizeof(Liquid::LeyLineData));
        liquidBuffers.felLiquidBuffer = m_device->createSSBOBuffer(un("Scene_FelLiquidBuffer"), 1024, sizeof(Liquid::FelData));
        liquidBuffers.swampLiquidBuffer = m_device->createSSBOBuffer(un("Scene_SwampLiquidBuffer"), 1024, sizeof(Liquid::SwampData));
        liquidBuffers.azeritheLiquidBuffer = m_device->createSSBOBuffer(un("Scene_AzeritheLiquidBuffer"), 1024, sizeof(Liquid::AzeritheData));
        
        liquidBuffers.liquidBindlessBuffer = m_device->createSSBOBuffer(un("Water Bindless"), 1024, sizeof(Liquid::LiquidBindless));
        liquidBuffers.liquidInstanceBuffer = m_device->createSSBOBuffer(un("Liquid Instance Data"), 1024, sizeof(Liquid::LiquidInstance));
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
    m_emptyM2ParticleGpuVAO = createM2ParticleGpuVAO(nullptr);
    m_emptyM2RibbonVAO = createM2RibbonVAO(nullptr, nullptr);
    m_emptyM2RibbonGpuVAO = createM2RibbonGpuVAO(nullptr);
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

    m_gpuDraws.setup(allBuffers);

    m_gpuM2Animation.setup(allBuffers, m2Buffers.boneMatrix, m2Buffers.placementMatrix);

    createM2GlobalMaterialData();
    createWMOGlobalMaterialData();
    createADTGlobalMaterialData();
    createM2WaterfallGlobalMaterialData();
    createWaterGlobalMaterialData();

    createAABBDebugMaterials();
    setupObjectPicking();
}

void MapSceneRenderBindlessVLK::createAABBDebugMaterials() {
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

void MapSceneRenderBindlessVLK::drawAABBDebug(CmdBufRecorder &frameBufCmd, const GpuIndirectDrawsVLK::FrameDrawData &gpuFrameDraws) {
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

void MapSceneRenderBindlessVLK::setupObjectPicking() {
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

void MapSceneRenderBindlessVLK::resolvePendingPickResult() {
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

void MapSceneRenderBindlessVLK::updateSelectedAABB(const std::shared_ptr<MapRenderPlan> &framePlan) {
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

        g_adtMaterial = MaterialBuilderVLK::fromShader(m_device, {"adtShader", "adtShader"}, adtBindlessShaderConfig)
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
    g_waterMaterial = MaterialBuilderVLK::fromShader(m_device, {"liquidShader", "liquidShader"},
                                                     liquidBindlessShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_forwardRenderPass, pipelineTemplate)
        .createZPrefillPipeline(m_emptyWaterVAO, {"liquidDepthShader", "liquidDepthShader"}, liquidBindlessDepthShaderConfig, defaultView->getLiquidDepthPass())
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [this](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(0, wmoBuffers.wmoPlacementMats)
                .ssbo(1, liquidBuffers.liquidBindlessBuffer)
                .ssbo(2, liquidBuffers.waterLiquidBuffer)
                .ssbo(3, liquidBuffers.magmaLiquidBuffer)
                .ssbo(4, liquidBuffers.mercuryLiquidBuffer)
                .ssbo(5, liquidBuffers.fogLiquidBuffer)
                .ssbo(6, liquidBuffers.leyLineLiquidBuffer)
                .ssbo(7, liquidBuffers.felLiquidBuffer)
                .ssbo(8, liquidBuffers.swampLiquidBuffer)
                .ssbo(9, liquidBuffers.azeritheLiquidBuffer)
                .ssbo(10, liquidBuffers.liquidInstanceBuffer);

            waterDataDS = ds;
        })
        .createDescriptorSet(2, [this](std::shared_ptr<GDescriptorSet> &ds) {
            waterTexturesDS = ds;
        })
        .bindDescriptorSet(3, gBufferDataDS)
        .toMaterial();;

    waterTextureHolder = std::make_shared<BindlessTextureHolder>(waterTexturesBindlessCount);

    // Slot 0-style fallback: black placeholder for sampler slots with no texture this frame
    waterFallbackTexture = waterTextureHolder->allocate(m_device->getBlackTexturePixel());
    waterTexturesDS->beginUpdate().texture(0, m_device->getBlackTexturePixel(), waterFallbackTexture->getIndex());
}
void MapSceneRenderBindlessVLK::createWMOGlobalMaterialData() {
    PipelineTemplate pipelineTemplate;
    pipelineTemplate.element = DrawElementMode::TRIANGLES;
    pipelineTemplate.depthWrite = true;
    pipelineTemplate.depthCulling = true;
    pipelineTemplate.backFaceCulling = true;
    pipelineTemplate.blendMode = EGxBlendEnum::GxBlend_Opaque;

    //Create global wmo descriptor for bindless textures
    g_wmoMaterial = MaterialBuilderVLK::fromShader(m_device, {"wmoShader", "wmoShader"}, wmoBindlessShaderConfig)
        .createPipeline(m_emptyWMOVAO, m_forwardRenderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [&](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(1, wmoBuffers.wmoPlacementMats)
                .ssbo(2, wmoBuffers.wmoMeshWideVSes)
                .ssbo(3, wmoBuffers.wmoMeshWidePSes)
                .ssbo(4, wmoBuffers.wmoMeshWideBindless)
                .ssbo(5, wmoBuffers.wmoGroupInteriorData)
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
    g_m2Material = MaterialBuilderVLK::fromShader(m_device, {"m2Shader", "m2Shader"}, m2BindlessShaderConfig)
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
                .ssbo(9, m2Buffers.meshWideBlocksBindless)
                .ssbo(10, m2Buffers.projectiveData);

            m2BufferOneDS = ds;
        })
        .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
            m2TextureDS = ds;
        }).toMaterial();
    m2TextureHolder = std::make_shared<BindlessTextureHolder>(m2TexturesBindlessCount);
}
void MapSceneRenderBindlessVLK::createM2WaterfallGlobalMaterialData() {
    MaterialBuilderVLK::fromShader(m_device, {"waterfallShader", "waterfallShader"}, m2WaterfallBindlessShaderConfig)
        .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ssbo(0, m2WaterfallBuffer.waterfallCommon)
                .ssbo(1, m2WaterfallBuffer.waterfallBindless);

            m2WaterfallBufferDS = ds;
        })
        .createDescriptorSet(3, [&](std::shared_ptr<GDescriptorSet> &ds) {
            m2WaterfallTextureDS = ds;
        }).toMaterial();

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
        MaterialBuilderVLK::fromShader(m_device, {"m2Shader", isTrueOpaq ? "m2Shader_opaq" : "m2Shader"}, m2BindlessShaderConfig)
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

std::shared_ptr<ISimpleMaterialVLK> MapSceneRenderBindlessVLK::getM2ProjectiveStaticMaterial(const PipelineTemplate &pipelineTemplate) {
    auto i = m_m2ProjectiveStaticMaterials.find(pipelineTemplate);
    if (i != m_m2ProjectiveStaticMaterials.end() && i->second) {
        return i->second;
    }

    bool isOpaq = pipelineTemplate.blendMode == EGxBlendEnum::GxBlend_Opaque ||
                  pipelineTemplate.blendMode == EGxBlendEnum::GxBlend_AlphaKey;

    bool isTrueOpaq = pipelineTemplate.blendMode == EGxBlendEnum::GxBlend_Opaque;

    auto staticMaterial =
        MaterialBuilderVLK::fromShader(m_device, {"m2_projected", "m2_projected"}, m2ProjectedBindlessShaderConfig)
            .setMaterialId(generateUniqueM2MatId())
            .createPipeline(m_drawBBoxVao, m_forwardRenderPass, pipelineTemplate)
            .bindDescriptorSet(0, sceneWideDS)
            .bindDescriptorSet(1, m2BufferOneDS)
            .bindDescriptorSet(2, m2TextureDS)
            .bindDescriptorSet(3, gBufferDataDS)
            .toMaterial();

    m_m2ProjectiveStaticMaterials[pipelineTemplate] = staticMaterial;

    return staticMaterial;
}

std::shared_ptr<ISimpleMaterialVLK> MapSceneRenderBindlessVLK::getWMOStaticMaterial(const PipelineTemplate &pipelineTemplate) {
    auto i = m_wmoStaticMaterials.find(pipelineTemplate);
    if (i != m_wmoStaticMaterials.end()) {
        return i->second;
    }

    bool isTrueOpaq = pipelineTemplate.blendMode == EGxBlendEnum::GxBlend_Opaque;

    auto staticMaterial =
        MaterialBuilderVLK::fromShader(m_device, {"wmoShader", isTrueOpaq ? "wmoShader_opaq" : "wmoShader"}, wmoBindlessShaderConfig)
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

HGVertexBufferBindings MapSceneRenderBindlessVLK::createWmoVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
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

// Pull-model particle VAO for the GPU particle path: no vertex buffers, the vertex
// shader expands quads from the particle SSBO pools.
HGVertexBufferBindings MapSceneRenderBindlessVLK::createM2ParticleGpuVAO(HGIndexBuffer indexBuffer) {
    auto vao = m_device->createVertexBufferBindings();
    vao->setIndexBuffer(indexBuffer);
    return vao;
}

// Pull-model ribbon VAO for the GPU ribbon path: no vertex buffers, the vertex
// shader reads the ribbon edge SSBO; the index buffer is sim-written per frame.
HGVertexBufferBindings MapSceneRenderBindlessVLK::createM2RibbonGpuVAO(HGIndexBuffer indexBuffer) {
    auto vao = m_device->createVertexBufferBindings();
    vao->setIndexBuffer(indexBuffer);
    return vao;
}

std::shared_ptr<IBuffer> MapSceneRenderBindlessVLK::createM2RibbonGpuIndexBuffer(int32_t edgeCount) {
    if (!m_gpuM2Animation.isEnabled()) return nullptr;
    return m_gpuM2Animation.createRibbonGpuIndexBuffer(edgeCount);
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
        .toMaterial<IADTMaterialBindless>([&fragmentData, &vertexFragmentData, &instanceData](IADTMaterialBindless *instance) -> void {
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

            for (int i = 0; i < 8; i++) {
                auto bindlessText = adtLayerTextureHolder->allocate(adtMaterialTemplate.layerTextures[i]);
                adtInstanceData.LayerIndexes[i] = bindlessText->getIndex();
                material->m_bindlessText.push_back(bindlessText);
                dsUpdate.texture(0, adtMaterialTemplate.layerTextures[i], bindlessText->getIndex());
            }
        }

        {
            auto dsUpdate = adtAlphaTextureDS->beginUpdate();

            auto bindlessText = adtAlphaTextureHolder->allocate(adtMaterialTemplate.blendTextures[0]);
            adtInstanceData.AlphaTextureInd = bindlessText->getIndex();
            material->m_bindlessText.push_back(bindlessText);
            dsUpdate.texture(0, adtMaterialTemplate.blendTextures[0], bindlessText->getIndex());
        }
        {
            auto dsUpdate = adtAlphaTextureDS->beginUpdate();

            auto bindlessText = adtAlphaTextureHolder->allocate(adtMaterialTemplate.blendTextures[1]);
            adtInstanceData.AlphaTextureInd2 = bindlessText->getIndex();
            material->m_bindlessText.push_back(bindlessText);
            dsUpdate.texture(0, adtMaterialTemplate.blendTextures[1], bindlessText->getIndex());
        }

        {
            auto dsUpdate = adtHeightLayerTextureDS->beginUpdate();

            for (int i = 0; i < 8; i++) {
                auto bindlessText = adtHeightLayerTextureHolder->allocate(adtMaterialTemplate.heightTextures[i]);
                adtInstanceData.LayerHeight[i] = bindlessText->getIndex();
                material->m_bindlessText.push_back(bindlessText);
                dsUpdate.texture(0, adtMaterialTemplate.heightTextures[i], bindlessText->getIndex());
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
        .toMaterial<IM2MaterialBindless>([&vertexFragmentDataBindless, &vertexFragmentData](IM2MaterialBindless *instance) -> void {
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

std::shared_ptr<IM2ProjectiveMaterial> MapSceneRenderBindlessVLK::createM2ProjectiveMaterial(
    const std::shared_ptr<IM2ModelData> &m2ModelData,
    const PipelineTemplate &pipelineTemplate,
    const M2MaterialTemplate &m2MaterialTemplate
) {
    ZoneScoped;

    auto m2ModelDataVisVLK = std::dynamic_pointer_cast<IM2ModelData>(m2ModelData);

    auto vertexFragmentDataBindless = std::make_shared<CBufferChunkVLK<M2::meshWideBlockVSPS_Bindless>>(m2Buffers.meshWideBlocksBindless);
    auto vertexFragmentData = std::make_shared<CBufferChunkVLK<M2::meshWideBlockVSPS>>(m2Buffers.meshWideBlocks);
    auto projectiveTextData = std::make_shared<CBufferChunkVLK<M2::ProjectiveData>>(m2Buffers.projectiveData);

    auto staticMaterial = getM2ProjectiveStaticMaterial(pipelineTemplate);

    auto material = MaterialBuilderVLK::fromMaterial(m_device, staticMaterial)
        .toMaterial<IM2ProjectiveMaterialBindless>([&vertexFragmentDataBindless, &vertexFragmentData, &projectiveTextData](IM2ProjectiveMaterialBindless *instance) -> void {
            instance->m_vertexFragmentDataBindless = vertexFragmentDataBindless;
            instance->m_vertexFragmentData = vertexFragmentData;
            instance->m_projectiveTextData = projectiveTextData;
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
        modelFragmentDataVis.meshIndex = vertexFragmentData->getSubBuffer()->getIndex();
        modelFragmentDataVis.projectiveDataIndex = projectiveTextData->getSubBuffer()->getIndex();
        for (int i = 0; i < 4; i++) {
            modelFragmentDataVis.textureIndicies[i] = material->m_bindlessText[i]->getIndex();
        }

        vertexFragmentDataBindless->save();
    }

    material->instanceIndex = vertexFragmentDataBindless->getSubBuffer()->getIndex();

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
                                                   m2WaterfallBindlessShaderConfig)
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

    if (m2ParticleMatTemplate.forGpuPath) {
        // GPU particle path: pull-model vertex shader reading the particle SSBO pools
        auto material = MaterialBuilderVLK::fromShader(m_device, {"m2Particle/forward/m2ParticleGpuShader", "m2Particle/forward/m2ParticleShader"}, bindlessShaderConfig)
            .createPipeline(m_emptyM2ParticleGpuVAO, m_forwardRenderPass, pipelineTemplate)
            .createGBufferPipeline(m_emptyM2ParticleGpuVAO, {"m2ParticleGpuShader", "m2ParticleShader"}, m2ParticlesBindlessGBufferShaderConfig, m_gBufferPass)
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
            .createDescriptorSet(3, [this](std::shared_ptr<GDescriptorSet> &ds) {
                ds->beginUpdate()
                    .ssbo(0, m_gpuM2Animation.getParticleStatesBuffer())
                    .ssbo(1, m_gpuM2Animation.getParticleStaticsBuffer())
                    .ssbo(2, m_gpuM2Animation.getParticlePropsBuffer())
                    .ssbo(3, m_gpuM2Animation.getParticlesABuffer())
                    .ssbo(4, m_gpuM2Animation.getParticlesBBuffer())
                    .ssbo(5, m_gpuM2Animation.getPartTimesBuffer())
                    .ssbo(6, m_gpuM2Animation.getValuesVec4Buffer())
                    .ssbo(7, m_gpuM2Animation.getValuesFloatBuffer())
                    .ssbo(8, m_gpuM2Animation.getRandTableBuffer())
                    .ssbo(9, m_gpuM2Animation.getColorReplBuffer())
                    .delayUpdate();
            })
            .toMaterial<IM2ParticleMaterial>([l_fragmentData](IM2ParticleMaterial *instance) -> void {
                instance->m_fragmentData = l_fragmentData;
            });

        return material;
    }

    auto material = MaterialBuilderVLK::fromShader(m_device, {"m2Particle/forward/m2ParticleShader", "m2Particle/forward/m2ParticleShader"}, bindlessShaderConfig)
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

    if (m2RibbonMaterialTemplate.forGpuPath) {
        // GPU ribbon path: pull-model vertex shader reading the ribbon SSBO pools
        auto material = MaterialBuilderVLK::fromShader(m_device, {"m2Ribbon/forward/ribbonGpuShader", "m2Ribbon/forward/ribbonShader"}, bindlessShaderConfig)
            .createPipeline(m_emptyM2RibbonGpuVAO, m_forwardRenderPass, pipelineTemplate)
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
            .createDescriptorSet(3, [this](std::shared_ptr<GDescriptorSet> &ds) {
                ds->beginUpdate()
                    .ssbo(0, m_gpuM2Animation.getRibbonStatesBuffer())
                    .ssbo(1, m_gpuM2Animation.getRibbonStaticsBuffer())
                    .ssbo(2, m_gpuM2Animation.getRibbonPropsBuffer())
                    .ssbo(3, m_gpuM2Animation.getRibbonEdgesBuffer())
                    .delayUpdate();
            })
            .toMaterial<IM2RibbonMaterial>([l_fragmentData](IM2RibbonMaterial *instance) -> void {
                instance->m_fragmentData = l_fragmentData;
            });

        return material;
    }

    auto material = MaterialBuilderVLK::fromShader(m_device, {"m2Ribbon/forward/ribbonShader", "m2Ribbon/forward/ribbonShader"}, bindlessShaderConfig)
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

std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> MapSceneRenderBindlessVLK::createWmoModelMatrixChunk() {
    return std::make_shared<CBufferChunkVLK<WMO::modelWideBlockVS>>(wmoBuffers.wmoPlacementMats);
};

std::shared_ptr<IWmoModelData> MapSceneRenderBindlessVLK::createWMOWideChunk(int groupNum)  {
    auto wmoModelData = std::make_shared<IWmoModelData>();
    wmoModelData->m_placementMatrix = createWmoModelMatrixChunk();
    BufferChunkHelperVLK::create(wmoBuffers.wmoGroupInteriorData, wmoModelData->m_groupInteriorData, sizeof(WMO::InteriorBlockData) * groupNum);

    return wmoModelData;
}

std::shared_ptr<IWMOMaterial> MapSceneRenderBindlessVLK::createWMOMaterial(const std::shared_ptr<IWmoModelData> &wmoModelWide,
                                                                           const PipelineTemplate &pipelineTemplate,
                                                                           const WMOMaterialTemplate &wmoMaterialTemplate) {
    auto l_vertexData = std::make_shared<CBufferChunkVLK<WMO::meshWideBlockVS>>(wmoBuffers.wmoMeshWideVSes); ;
    auto l_fragmentData = std::make_shared<CBufferChunkVLK<WMO::meshWideBlockPS>>(wmoBuffers.wmoMeshWidePSes); ;
    auto l_bindless = std::make_shared<CBufferChunkVLK<WMO::meshWideBlockBindless>>(wmoBuffers.wmoMeshWideBindless);

    auto staticMaterial = getWMOStaticMaterial(pipelineTemplate);

    auto material = MaterialBuilderVLK::fromMaterial(m_device, staticMaterial)
        .toMaterial<IWMOMaterialBindless>([&l_vertexData, &l_fragmentData, &l_bindless](IWMOMaterialBindless *instance) -> void {
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
        bindless.placementMat = BufferChunkHelperVLK::cast(wmoModelWide->m_placementMatrix)->getIndex();

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
    material->interiorDataIndex = BufferChunkHelperVLK::cast(wmoModelWide->m_groupInteriorData)->getIndex();

    return material;
}

std::shared_ptr<ILiquidMaterial> MapSceneRenderBindlessVLK::createLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
                                                                               const PipelineTemplate &pipelineTemplate,
                                                                               const WaterMaterialTemplate &waterMaterialTemplate) {

    auto l_liquidBindless = std::make_shared<CBufferChunkVLK<Liquid::LiquidBindless>>(liquidBuffers.liquidBindlessBuffer);

    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"waterShader", "waterShader"}, waterBindlessShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_forwardRenderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .bindDescriptorSet(1, waterDataDS)
        .bindDescriptorSet(2, waterTexturesDS)
        .toMaterial<IWaterMaterialBindless>([l_liquidBindless](IWaterMaterialBindless *instance) -> void {
            instance->m_instanceBindless = l_liquidBindless;
        });

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
        auto &bindless = l_liquidBindless->getObject();
        bindless.placementMatInd = BufferChunkHelperVLK::cast(modelWide)->getIndex();

        l_liquidBindless->save();
    }

    material->instanceIndex = l_liquidBindless->getIndex();

    return material;
}



std::shared_ptr<ISkyMeshMaterial> MapSceneRenderBindlessVLK::createSkyMeshMaterial(const PipelineTemplate &pipelineTemplate) {
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

std::shared_ptr<IPlanetMaterial> MapSceneRenderBindlessVLK::createPlanetMaterial(const PipelineTemplate &pipelineTemplate, const HGSamplableTexture &texture) {
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

std::shared_ptr<IPortalMaterial> MapSceneRenderBindlessVLK::createPortalMaterial(const PipelineTemplate &pipelineTemplate) {
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

std::shared_ptr<IM2ModelData> MapSceneRenderBindlessVLK::createM2ModelMat(int bonesCount, int m2ColorsCount, int textureWeightsCount, int textureMatricesCount, uint32_t objectId) {
    auto result = std::make_shared<IM2ModelData>();

    BufferChunkHelperVLK::create(m2Buffers.placementMatrix, result->m_placementMatrix);
    BufferChunkHelperVLK::create(m2Buffers.boneMatrix, result->m_bonesData, sizeof(mathfu::mat4) * bonesCount);
    BufferChunkHelperVLK::create(m2Buffers.m2Colors, result->m_colors, sizeof(mathfu::vec4_packed) * m2ColorsCount);
    BufferChunkHelperVLK::create(m2Buffers.textureWeights, result->m_textureWeights, sizeof(float) * textureWeightsCount);
    BufferChunkHelperVLK::create(m2Buffers.textureMatrices, result->m_textureMatrices, sizeof(mathfu::mat4) * textureMatricesCount);
    BufferChunkHelperVLK::create(m2Buffers.modelFragmentDatas, result->m_modelFragmentData);
    result->m_modelFragmentData->getObject().objectId = objectId;
    result->objectId = objectId;

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

std::shared_ptr<IM2GpuAnimData> MapSceneRenderBindlessVLK::createM2GpuAnimData(
    const void *modelKey,
    const std::function<M2GpuTrackPack()> &packBuilder,
    const std::shared_ptr<IM2ModelData> &m2ModelData,
    const std::vector<M2GpuEmitterSeeds> &emitterSeeds,
    const std::vector<int32_t> &emitterRandomizedTextureIndexMasks,
    const std::vector<std::shared_ptr<IBuffer>> &ribbonGpuIndexBuffers) {

    if (!m_gpuM2Animation.isEnabled() || m2ModelData == nullptr) return nullptr;

    auto boneChunk = BufferChunkHelperVLK::castToChunk(m2ModelData->m_bonesData);
    if (boneChunk == nullptr) return nullptr;

    auto placementChunk = BufferChunkHelperVLK::castToChunk(m2ModelData->m_placementMatrix);
    if (placementChunk == nullptr) return nullptr;

    return m_gpuM2Animation.createAnimData(modelKey,
                                           packBuilder,
                                           (int32_t)boneChunk->getIndex(),
                                           (int32_t)placementChunk->getIndex(),
                                           emitterSeeds,
                                           emitterRandomizedTextureIndexMasks,
                                           ribbonGpuIndexBuffers);
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
protected:
    MapSceneRenderBindlessVLK &m_renderer;
    struct DrawCommand {
        uint32_t matId;
        uint32_t priorityPlane;
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
    framebased::vector<HGMesh> projectiveMeshes;
    framebased::vector<HGMesh> commonMeshes;

     static void fillDrawCommand(DrawCommand &drawCommand, GMeshVLK *meshVlk) {
        auto const matId = meshVlk->material()->getMaterialId();

        drawCommand.matId = matId;
        drawCommand.priorityPlane = meshVlk->priorityPlane();
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
        const auto meshVlk = bindlessMeshFactoryVlk->getObjectById<0>(meshId);
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
        auto meshVlk = bindlessMeshFactoryVlk->getObjectById<0>(meshId);

        auto &drawCommand = drawVec.emplace_back();
        fillDrawCommand(drawCommand, meshVlk);
    }
    inline void addDrawCommand(tbb::concurrent_vector<DrawCommand> &drawVec, const HGMesh &mesh) {
        auto meshId = mesh->getObjectId();
        auto meshVlk = bindlessMeshFactoryVlk->getObjectById<0>(meshId);

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
    void addProjectiveMesh(const HGMesh &mesh) override {
        projectiveMeshes.push_back(mesh);
    }
    virtual COpaqueMeshCollector * clone() override {
        return new COpaqueMeshCollectorBindlessVLK(m_renderer);
    }

    void merge(COpaqueMeshCollector & collector) override {
        auto l_collector = (COpaqueMeshCollectorBindlessVLK &) collector;

        m2DrawVec.insert(m2DrawVec.end(), l_collector.m2DrawVec.begin(), l_collector.m2DrawVec.end());
        wmoDrawVec.insert(wmoDrawVec.end(), l_collector.wmoDrawVec.begin(), l_collector.wmoDrawVec.end());
        adtDrawVec.insert(adtDrawVec.end(), l_collector.adtDrawVec.begin(), l_collector.adtDrawVec.end());
        commonMeshes.insert(commonMeshes.end(), l_collector.commonMeshes.begin(), l_collector.commonMeshes.end());
        waterMeshVec.insert(waterMeshVec.end(), l_collector.waterMeshVec.begin(), l_collector.waterMeshVec.end());
        projectiveMeshes.insert(projectiveMeshes.end(), l_collector.projectiveMeshes.begin(), l_collector.projectiveMeshes.end());

    }

    void render(CmdBufRecorder &cmdBuf, CmdBufRecorder::ViewportType viewPortType) {
        std::sort(m2DrawVec.begin(), m2DrawVec.end(), [](DrawCommand const &a, DrawCommand const &b) {
            if (a.matId == b.matId) {
                return a.priorityPlane < b.priorityPlane;
            }
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
            cmdBuf.drawMesh(mesh, viewPortType);
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

            // Note: no explicit bindPipeline here — bindMaterial picks the z-prefill
            // pipeline automatically when the recorder is in z-prefill mode (liquid depth pass)
            cmdBuf.bindMaterial(gWaterMat);

            for (auto const &drawCmd : waterMeshVec) {
                cmdBuf.drawIndexed(drawCmd.indexCount, drawCmd.instanceCount, drawCmd.firstIndex, drawCmd.firstInstance, drawCmd.vertexOffset);
            }
        }
    }
    void renderProjective(CmdBufRecorder &cmdBuf, CmdBufRecorder::ViewportType viewPortType) {
        //Render projectiveMeshes
        for (auto const &mesh : projectiveMeshes ) {
            cmdBuf.drawMesh(mesh, viewPortType);
        }
    }
    void fillMeshCount(MeshCount &meshCount) {
        meshCount.adtMesh = adtDrawVec.size();
        meshCount.m2Mesh = m2DrawVec.size();
        meshCount.wmoMesh = wmoDrawVec.size();
        meshCount.commonMesh = commonMeshes.size();
    }

};

// Main-scene collector for the GPU-indirect path: M2 and WMO opaque meshes are collected
// via draw groups (BindlessIndirectDrawsVLK), so they are no-ops here — only ADT, water,
// common (particles/waterfall) and projective meshes are kept. Sky collection still uses
// the base collector above, since skybox objects aren't part of the GPU cull path.
class COpaqueMeshCollectorBindlessIndirectVLK : public COpaqueMeshCollectorBindlessVLK {
public:
    using COpaqueMeshCollectorBindlessVLK::COpaqueMeshCollectorBindlessVLK;

    void addM2Mesh(const HGM2Mesh &mesh) override {};
    void addWMOMesh(const HGMesh &mesh) override {};

    COpaqueMeshCollector * clone() override {
        return new COpaqueMeshCollectorBindlessIndirectVLK(m_renderer);
    }
};

std::unique_ptr<IRenderFunction> MapSceneRenderBindlessVLK::update(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams,
                                                                   const std::shared_ptr<MapRenderPlan> &framePlan) {
    TracyMessageStr(("Update stage frame = " + std::to_string(FrameContext::getCurrentProcessingFrameNumber())));

    ZoneScoped;
    auto l_this = std::dynamic_pointer_cast<MapSceneRenderBindlessVLK>(this->shared_from_this());
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
    std::unique_ptr<COpaqueMeshCollectorBindlessIndirectVLK> u_collector = std::make_unique<COpaqueMeshCollectorBindlessIndirectVLK>(*this, lastMeshCount);
    std::unique_ptr<COpaqueMeshCollectorBindlessVLK> u_skyCollector = std::make_unique<COpaqueMeshCollectorBindlessVLK>(*this);
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


    // CPU-side collection: view meshes (ADT, water, WMO opaque — the latter is a no-op in
    // this collector), M2 decals and sky. M2 meshes and WMO transparent meshes are
    // collected by the GPU-indirect path instead, so they get frustum/occlusion culling
    // on the GPU.
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

    // GPU M2 animation: collect the per-frame dispatch list from the objects whose
    // GPU anim state was written during this frame's update (skipped objects keep
    // last frame's GPU state, same as the CPU path keeps last frame's arrays).
    uint32_t gpuAnimDispatchCount = 0;
    uint32_t gpuParticleEmitterCount = 0;
    uint32_t gpuRibbonCount = 0;
    {
        uint32_t processingFrame = FrameContext::getCurrentProcessingFrameNumber();
        std::vector<uint32_t> animStateIndices;
        std::vector<uint32_t> emitterStateIndices;
        std::vector<uint32_t> ribbonStateIndices;
        auto collectGpuAnimObjects = [&](const framebased::vector<M2ObjId> &m2List) {
            for (auto m2Id : m2List) {
                auto m2Object = m2Factory->getObjectById<0>(m2Id);
                if (m2Object == nullptr) continue;
                int32_t stateIndex = m2Object->getGpuAnimStateIndexForFrame(processingFrame);
                if (stateIndex < 0) continue;
                animStateIndices.push_back((uint32_t)stateIndex);
                // Particle/ribbon sims run for the same objects (their bones were just computed)
                m2Object->appendGpuParticleStateIndices(emitterStateIndices);
                m2Object->appendGpuRibbonStateIndices(ribbonStateIndices);
            }
        };
        collectGpuAnimObjects(framePlan->m2Array.getDrawn());
        if (auto skyboxView = framePlan->viewsHolder.getSkybox()) {
            collectGpuAnimObjects(skyboxView->m2List.getDrawn());
            collectGpuAnimObjects(skyboxView->stars.getDrawn());
        }
        m_gpuM2Animation.updateDispatchList(animStateIndices);
        gpuAnimDispatchCount = (uint32_t)animStateIndices.size();
        m_gpuM2Animation.updateEmitterDispatchList(emitterStateIndices);
        gpuParticleEmitterCount = (uint32_t)emitterStateIndices.size();
        m_gpuM2Animation.updateRibbonDispatchList(ribbonStateIndices);
        gpuRibbonCount = (uint32_t)ribbonStateIndices.size();
    }
    const float gpuParticleDeltaSeconds = (float)(frameInputParams->delta / 1000.0);

    u_collector->fillMeshCount(lastMeshCount);

//    {
//        ZoneScopedN("collect meshes wait");
//        collectMeshAsync.wait();
//    }

    // Compute viewProj for GPU culling
    mathfu::mat4 cullViewProj = mathfu::mat4::Identity();
    mathfu::mat4 gpuParticleViewMat = mathfu::mat4::Identity();
    if (!renderingMatricessAndSizes.empty()) {
        auto &rm = renderingMatricessAndSizes[0].renderingMat;
        cullViewProj = MathHelper::getVulkanMat4Fix() * rm->perspectiveMat * rm->lookAtMat;
        gpuParticleViewMat = rm->lookAtMat;
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
        l_skyOpaqueMeshes = std::move(u_skyCollector),
        skyMeshes,
        renderSky,
        mapScene, framePlan,
        l_this,
        frameInputParams,
        cullViewProj,
        gpuAnimDispatchCount,
        gpuParticleEmitterCount,
        gpuParticleDeltaSeconds,
        gpuParticleViewMat,
        gpuRibbonCount,
        gpuFrameDraws](CmdBufRecorder &frameBufCmd, CmdBufRecorder &swapChainCmd) -> void {

        TracyMessageStr(("Draw stage frame = " + std::to_string(FrameContext::getCurrentProcessingFrameNumber())));

        // ----------------------
        // Phase 0: GPU M2 animation (bones evaluated into the bone-matrix SSBO)
        // ----------------------
        l_this->m_gpuM2Animation.recordDispatch(frameBufCmd, gpuAnimDispatchCount);
        // Particle/ribbon sims read the bones + emitter/ribbon props written above
        l_this->m_gpuM2Animation.recordParticleSim(frameBufCmd, gpuParticleEmitterCount, gpuParticleViewMat, gpuParticleDeltaSeconds);
        l_this->m_gpuM2Animation.recordRibbonSim(frameBufCmd, gpuRibbonCount, gpuParticleDeltaSeconds);

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
                        auto passHelper = currentView->beginGBufferPass(frameBufCmd, false,
                                                                        frameInputParams->frameParameters->clearColor);
                        frameBufCmd.setGBufferMode(true);
                        {
                            // CPU part: ADT + common meshes (particles/waterfall)
                            l_this->drawOpaque(frameBufCmd, l_opaqueMeshes.get());
                        }
                        {
                            auto debugHelper = frameBufCmd.beginDebugLabel("GBuffer Opaque Indirect", {0, 0, 0.5f});
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
                            // CPU part: ADT + common meshes (particles/waterfall)
                            l_this->drawOpaque(frameBufCmd, l_opaqueMeshes.get());
                        }
                        {
                            auto debugHelper = frameBufCmd.beginDebugLabel("Forward Pass: Opaque indirect", {0, 0, 0.5f});
                            VkZone(frameBufCmd, "Opaque Indirect");
                            l_this->m_gpuDraws.drawOpaque(frameBufCmd,
                                                          CmdBufRecorder::ViewportType::vp_usual,
                                                          *gpuFrameDraws);
                        }
                        {
                            auto debugHelper = frameBufCmd.beginDebugLabel("Decals Pass", {0, 0, 0.5f});

                            l_opaqueMeshes->renderProjective(frameBufCmd, CmdBufRecorder::ViewportType::vp_usual);
                        }


                        if (l_this->m_config->drawDebugLights) {
                            currentView->doDebugLightPass(frameBufCmd);
                        }

                        {
                            //Sky meshes
                            for (int i = 0; i < skyMeshes->size(); i++) {
                                auto const &mesh = skyMeshes->at(i);


                                frameBufCmd.drawMesh(mesh, CmdBufRecorder::ViewportType::vp_skyBox);
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

void MapSceneRenderBindlessVLK::drawOpaque(CmdBufRecorder &frameBufCmd,
                                           COpaqueMeshCollectorBindlessVLK *l_opaqueMeshes) {
    {
        ZoneScopedN("submit opaque");
        VkZone(frameBufCmd, "render opaque")
        l_opaqueMeshes->render(frameBufCmd, CmdBufRecorder::ViewportType::vp_usual);
    }
}

std::shared_ptr<GCommandBuffer> MapSceneRenderBindlessVLK::recordLiquidTransparentSecondary(
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
            recorder.drawMesh(mesh, CmdBufRecorder::ViewportType::vp_usual);
        }
    }

    return cmdBuf;
}

std::shared_ptr<MapRenderPlan> MapSceneRenderBindlessVLK::getLastCreatedPlan() {
    return m_lastCreatedPlan;
}


template<typename T>
std::shared_ptr<ILiquidMaterial> MapSceneRenderBindlessVLK::createLiquidMaterialBindless(
    const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
    const std::shared_ptr<LiquidData<T>> &liquidData,
    Liquid::LiquidMaterialType materialType) {

    // Per-instance record read by the bindless liquid shaders (set 1, binding 1)
    const int liquidDataInd = (int)BufferChunkHelperVLK::cast(liquidData->m_liquidData)->getIndex();
    const int placementMatInd = (int)BufferChunkHelperVLK::cast(modelWide)->getIndex();

    auto l_liquidBindless = std::make_shared<CBufferChunkVLK<Liquid::LiquidBindless>>(liquidBuffers.liquidBindlessBuffer);
    {
        auto &bindless = l_liquidBindless->getObject();
        bindless = Liquid::LiquidBindless{};
        bindless.liquidDataInd = liquidDataInd;
        bindless.placementMatInd = placementMatInd;
        bindless.liquidMaterialType = (int)materialType;
        for (int &texInd : bindless.textureInd) texInd = waterFallbackTexture->getIndex();
        l_liquidBindless->save();
    }

    // Per-instance LiquidInstanceData (flow params, cell dims) — the engine writes into
    // m_instance renderer-agnostically; here it lives in the shared SSBO (set 1, binding 10)
    auto liquidInstanceData = std::make_shared<CBufferChunkVLK<Liquid::LiquidInstance>>(liquidBuffers.liquidInstanceBuffer);

    std::static_pointer_cast<LiquidDataBindlessVLK<T>>(liquidData)->setBindlessUpdateTarget(
        l_liquidBindless, waterTexturesDS, waterFallbackTexture->getIndex(),
        liquidDataInd, placementMatInd, (int)materialType);

    // Everything rides the global liquid material: the per-liquid material shares its
    // pipelines and descriptor sets, differing only in the instance record it points at.
    auto material = MaterialBuilderVLK::fromMaterial(m_device, g_waterMaterial)
        .toMaterial<IWaterMaterialBindless>([&liquidData, &l_liquidBindless](IWaterMaterialBindless *instance) -> void {
            instance->m_instanceBindless = l_liquidBindless;
            instance->m_ldData = liquidData;
        });
    material->m_instance = liquidInstanceData;
    material->instanceIndex = l_liquidBindless->getIndex();

    return material;
}

std::shared_ptr<ILiquidMaterial> MapSceneRenderBindlessVLK::createWaterLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<WaterLiquidData> &liquidData) {
    return createLiquidMaterialBindless(modelWide, liquidData, Liquid::LiquidMaterialType::Water);
}
std::shared_ptr<ILiquidMaterial> MapSceneRenderBindlessVLK::createMagmaLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<MagmaLiquidData> &liquidData) {
    return createLiquidMaterialBindless(modelWide, liquidData, Liquid::LiquidMaterialType::Magma);
}
std::shared_ptr<ILiquidMaterial> MapSceneRenderBindlessVLK::createMercuryLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<MercuryLiquidData> &liquidData) {
    return createLiquidMaterialBindless(modelWide, liquidData, Liquid::LiquidMaterialType::Mercury);
}
std::shared_ptr<ILiquidMaterial> MapSceneRenderBindlessVLK::createFogLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<FogLiquidData> &liquidData) {
    return createLiquidMaterialBindless(modelWide, liquidData, Liquid::LiquidMaterialType::Fog);
}
std::shared_ptr<ILiquidMaterial> MapSceneRenderBindlessVLK::createLeyLineLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<LeyLineLiquidData> &liquidData) {
    return createLiquidMaterialBindless(modelWide, liquidData, Liquid::LiquidMaterialType::LeyLine);
}
std::shared_ptr<ILiquidMaterial> MapSceneRenderBindlessVLK::createFelLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<FelLiquidData> &liquidData) {
    return createLiquidMaterialBindless(modelWide, liquidData, Liquid::LiquidMaterialType::Fel);
}
std::shared_ptr<ILiquidMaterial> MapSceneRenderBindlessVLK::createSwampLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<SwampLiquidData> &liquidData) {
    return createLiquidMaterialBindless(modelWide, liquidData, Liquid::LiquidMaterialType::Swamp);
}
std::shared_ptr<ILiquidMaterial> MapSceneRenderBindlessVLK::createAzeritheLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<AzeritheLiquidData> &liquidData) {
    return createLiquidMaterialBindless(modelWide, liquidData, Liquid::LiquidMaterialType::Azerithe);
}


HGMesh MapSceneRenderBindlessVLK::createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) {
    auto mesh = bindlessMeshFactoryVlk->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0,0);
    m_gpuDraws.registerMesh(mesh);
    return mesh;
}

HGSortableMesh MapSceneRenderBindlessVLK::createSortableMesh(gMeshTemplate &meshTemplate, const HMaterial &material, int priorityPlane) {
    auto mesh = bindlessMeshFactoryVlk->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0, priorityPlane);
    m_gpuDraws.registerMesh(mesh);
    return mesh;
}

HGSortableMesh MapSceneRenderBindlessVLK::createWaterMesh(gMeshTemplate &meshTemplate, const HMaterial &material, int priorityPlane) {
    auto realVAO = (GVertexBufferBindingsVLK *)meshTemplate.bindings.get();
    meshTemplate.bindings = m_emptyWaterVAO;

    auto _material = std::dynamic_pointer_cast<IWaterMaterialBindless>(material);
    auto mesh = bindlessMeshFactoryVlk->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0, priorityPlane);

    mesh->instanceIndex = _material->instanceIndex;
    mesh->vertexStart = ((IBufferVLK * )realVAO->getVertexBuffers()[0].get())->getIndex();
    mesh->start() += ((IBufferVLK * )realVAO->getIndexBuffer().get())->getOffset();

    m_gpuDraws.registerMesh(mesh);
    return mesh;
}

HGMesh
MapSceneRenderBindlessVLK::createAdtMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IADTMaterial> &material) {
    ZoneScoped;

    auto realVAO = (GVertexBufferBindingsVLK *)meshTemplate.bindings.get();
    meshTemplate.bindings = m_emptyADTVAO;

    auto mesh = bindlessMeshFactoryVlk->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0, 0);
    mesh->instanceIndex = std::dynamic_pointer_cast<IADTMaterialBindless>(material)->instanceIndex;
    mesh->vertexStart = ((IBufferVLK * )realVAO->getVertexBuffers()[0].get())->getIndex();
    mesh->start() += ((IBufferVLK * )realVAO->getIndexBuffer().get())->getOffset();

    m_gpuDraws.registerMesh(mesh);
    return mesh;
}
HGM2Mesh
MapSceneRenderBindlessVLK::createM2Mesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2Material> &material,
                                        int layer, int priorityPlane) {
    ZoneScoped;
    auto realVAO = (GVertexBufferBindingsVLK *)meshTemplate.bindings.get();
    meshTemplate.bindings = m_emptyM2VAO;
    auto mesh = bindlessMeshFactoryVlk->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), layer, priorityPlane);
    mesh->instanceIndex = std::dynamic_pointer_cast<IM2MaterialBindless>(material)->instanceIndex;
    mesh->vertexStart = ((IBufferVLK * )realVAO->getVertexBuffers()[0].get())->getIndex();
    mesh->start() += ((IBufferVLK * )realVAO->getIndexBuffer().get())->getOffset();

    m_gpuDraws.registerMesh(mesh);
    return mesh;
}
HGM2Mesh
MapSceneRenderBindlessVLK::createM2ProjectiveMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2ProjectiveMaterial> &material, int layer, int priorityPlane) {
    ZoneScoped;
    meshTemplate.bindings = m_drawBBoxVao;
    auto mesh = bindlessMeshFactoryVlk->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), layer, priorityPlane);
    mesh->instanceIndex = std::dynamic_pointer_cast<IM2ProjectiveMaterialBindless>(material)->instanceIndex;
    mesh->vertexStart = 0;
    mesh->start() = 0;
    mesh->end() = 36;

    m_gpuDraws.registerMesh(mesh);
    return mesh;
}

HGM2Mesh
MapSceneRenderBindlessVLK::createM2ParticleMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2Material> &material, int layer, int priorityPlane) {
//    auto realVAO = (GVertexBufferBindingsVLK *)meshTemplate.bindings.get();
//    meshTemplate.bindings = m_emptyM2ParticleVAO;
//    auto mesh = bindlessMeshFactoryVlk.createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), layer, priorityPlane);
//    mesh->instanceIndex = 1;
//    mesh->vertexStart = ((IBufferVLK * )realVAO->getVertexBuffers()[0].get())->getIndex();

    auto mesh = bindlessMeshFactoryVlk->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0, priorityPlane);
    m_gpuDraws.registerMesh(mesh);
    return mesh;
}

HGSortableMesh MapSceneRenderBindlessVLK::createWMOMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IWMOMaterial> &material, int groupNum, int canHaveExteriorLit, uint32_t wmoObjId) {
    auto realVAO = (GVertexBufferBindingsVLK *)meshTemplate.bindings.get();
    meshTemplate.bindings = m_emptyWMOVAO;

    auto originalMat = std::dynamic_pointer_cast<IWMOMaterialBindless>(material);
    auto c_perMeshData = std::make_shared<CBufferChunkVLK<WMO::perMeshData>>(wmoBuffers.wmoPerMeshData);

    auto newMat = MaterialBuilderVLK::fromMaterial(m_device, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material))
    .toMaterial<IWMOMaterialBindless>([&c_perMeshData](IWMOMaterialBindless *instance) -> void {
        instance->m_perMeshData = c_perMeshData;
    });

    {
        auto &perMeshData = c_perMeshData->getObject();
        perMeshData.meshWideBindlessIndex = originalMat->meshWideBindlessIndex;
        perMeshData.interiorDataIndex = originalMat->interiorDataIndex + groupNum;
        perMeshData.canHaveExteriorLit = canHaveExteriorLit;
        // Bits [30:10] = wmoObjId, bits [9:0] = groupNum (for GPU object-id picking)
        perMeshData.unused1 = (int)((wmoObjId & 0xFFFFF) << 10) | (groupNum & 0x3FF);
        c_perMeshData->save();
    }

    auto mesh = bindlessMeshFactoryVlk->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(newMat), 0, 0);
    mesh->instanceIndex = c_perMeshData->getIndex();
    mesh->vertexStart = ((IBufferVLK * )realVAO->getVertexBuffers()[0].get())->getIndex();
    mesh->start() += ((IBufferVLK * )realVAO->getIndexBuffer().get())->getOffset();
    m_gpuDraws.registerMesh(mesh);
    return mesh;
}
HGM2Mesh MapSceneRenderBindlessVLK::createM2WaterfallMesh(gMeshTemplate &meshTemplate,
                                                          const std::shared_ptr<IM2WaterFallMaterial> &material,
                                                          int layer, int priorityPlane) {
    auto mesh = bindlessMeshFactoryVlk->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), layer, priorityPlane);
    mesh->instanceIndex = std::dynamic_pointer_cast<IM2WaterFallMaterialBindless>(material)->instanceIndex;
    m_gpuDraws.registerMesh(mesh);
    return mesh;
}

std::shared_ptr<IRenderView> MapSceneRenderBindlessVLK::createRenderView(bool createOutput) {
    return std::make_shared<RendererViewClass>(m_device, uboBuffer,
                                               pointLightBuffer, spotLightBuffer,
                                               sceneWideDS,
                                               gBufferDataDS,
                                               m_drawQuadVao,
                                               m_drawSpotVao,
                                               m_drawSpotVaoLine,
                                               createOutput);
}

template<typename T>
MapSceneRenderBindlessVLK::LiquidDataBindlessVLK<T>::LiquidDataBindlessVLK(const std::shared_ptr<BindlessTextureHolder> &textureHolder, const std::shared_ptr<IBufferChunk<T>> &liquidData) : m_textureHolder(textureHolder) {
    m_bindlessTextures.fill(nullptr);
    this->m_liquidData = liquidData;
}

template<typename T>
void MapSceneRenderBindlessVLK::LiquidDataBindlessVLK<T>::setBindlessUpdateTarget(const std::shared_ptr<IBufferChunk<Liquid::LiquidBindless>> &bindlessChunk,
                                                                                  const std::shared_ptr<GDescriptorSet> &texturesDS,
                                                                                  int fallbackTextureIndex,
                                                                                  int liquidDataInd, int placementMatInd, int liquidMaterialType) {
    m_texturesDS = texturesDS;
    m_fallbackTextureIndex = fallbackTextureIndex;

    BindlessTarget target;
    target.chunk = bindlessChunk;
    target.liquidDataInd = liquidDataInd;
    target.placementMatInd = placementMatInd;
    target.liquidMaterialType = liquidMaterialType;
    m_bindlessTargets.push_back(std::move(target));
}

template<typename T>
void MapSceneRenderBindlessVLK::LiquidDataBindlessVLK<T>::updateAnimatedTextures(const std::array<HGSamplableTexture, MAX_LIQUID_TYPE_TEXTURES + 1> &textures, float currentTime) {
    // Let the (GAPI-agnostic) LiquidData specialization resolve material-specific
    // animation (e.g. fel normal frame pair) and the slot-6 extra texture
    auto resolved = textures;
    this->resolveAnimatedTextures(resolved, currentTime);

    if (m_bindlessTargets.empty() || m_texturesDS == nullptr) return;

    // Resolve the 9 texture slots once — shared by every material record of this liquid data.
    // Slot layout matches the deferred per-type descriptor sets (LiquidDataForwardVLK):
    // slots 0-6 = resolved animated textures, 7 = extraTexture2, 8 = extraTexture3
    int texIndices[9];
    {
        auto dsUpdate = m_texturesDS->beginUpdate();
        auto updateSlot = [&](const HGSamplableTexture &tex, int slot) {
            int texIndex = m_fallbackTextureIndex;
            if (tex != nullptr) {
                m_bindlessTextures[slot] = m_textureHolder->allocate(tex);
                texIndex = m_bindlessTextures[slot]->getIndex();
                dsUpdate.texture(0, tex, texIndex);
            } else {
                m_bindlessTextures[slot] = nullptr;
            }
            texIndices[slot] = texIndex;
        };

        for (int i = 0; i <= MAX_LIQUID_TYPE_TEXTURES; i++) {
            updateSlot(resolved[i], i);
        }
        updateSlot(this->extraTexture2, 7);
        updateSlot(this->extraTexture3, 8);
    }

    // Rewrite every registered record in full — getObject() hands out a fresh staging span
    // on every call, so fields left untouched would upload as garbage
    for (auto it = m_bindlessTargets.begin(); it != m_bindlessTargets.end();) {
        auto chunk = it->chunk.lock();
        if (chunk == nullptr) {
            it = m_bindlessTargets.erase(it);
            continue;
        }

        auto &bindless = chunk->getObject();
        bindless = Liquid::LiquidBindless{};
        bindless.liquidDataInd = it->liquidDataInd;
        bindless.placementMatInd = it->placementMatInd;
        bindless.liquidMaterialType = it->liquidMaterialType;
        for (int i = 0; i < 9; i++) {
            bindless.textureInd[i] = texIndices[i];
        }
        chunk->save();

        ++it;
    }
}

// Explicit template instantiations
template class MapSceneRenderBindlessVLK::LiquidDataBindlessVLK<Liquid::WaterData>;
template class MapSceneRenderBindlessVLK::LiquidDataBindlessVLK<Liquid::MagmaData>;
template class MapSceneRenderBindlessVLK::LiquidDataBindlessVLK<Liquid::MercuryData>;
template class MapSceneRenderBindlessVLK::LiquidDataBindlessVLK<Liquid::FogData>;
template class MapSceneRenderBindlessVLK::LiquidDataBindlessVLK<Liquid::LeyLineData>;
template class MapSceneRenderBindlessVLK::LiquidDataBindlessVLK<Liquid::FelData>;
template class MapSceneRenderBindlessVLK::LiquidDataBindlessVLK<Liquid::SwampData>;
template class MapSceneRenderBindlessVLK::LiquidDataBindlessVLK<Liquid::AzeritheData>;

std::shared_ptr<WaterLiquidData> MapSceneRenderBindlessVLK::createWaterLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::WaterData>>(liquidBuffers.waterLiquidBuffer);
    return std::make_shared<LiquidDataBindlessVLK<Liquid::WaterData>>(waterTextureHolder, liquidData);
}

std::shared_ptr<MagmaLiquidData> MapSceneRenderBindlessVLK::createMagmaLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::MagmaData>>(liquidBuffers.magmaLiquidBuffer);
    return std::make_shared<LiquidDataBindlessVLK<Liquid::MagmaData>>(waterTextureHolder, liquidData);
}

std::shared_ptr<MercuryLiquidData> MapSceneRenderBindlessVLK::createMercuryLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::MercuryData>>(liquidBuffers.mercuryLiquidBuffer);
    return std::make_shared<LiquidDataBindlessVLK<Liquid::MercuryData>>(waterTextureHolder, liquidData);
}

std::shared_ptr<FogLiquidData> MapSceneRenderBindlessVLK::createFogLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::FogData>>(liquidBuffers.fogLiquidBuffer);
    return std::make_shared<LiquidDataBindlessVLK<Liquid::FogData>>(waterTextureHolder, liquidData);
}

std::shared_ptr<LeyLineLiquidData> MapSceneRenderBindlessVLK::createLeyLineLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::LeyLineData>>(liquidBuffers.leyLineLiquidBuffer);
    return std::make_shared<LiquidDataBindlessVLK<Liquid::LeyLineData>>(waterTextureHolder, liquidData);
}

std::shared_ptr<FelLiquidData> MapSceneRenderBindlessVLK::createFelLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::FelData>>(liquidBuffers.felLiquidBuffer);
    return std::make_shared<LiquidDataBindlessVLK<Liquid::FelData>>(waterTextureHolder, liquidData);
}

std::shared_ptr<SwampLiquidData> MapSceneRenderBindlessVLK::createSwampLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::SwampData>>(liquidBuffers.swampLiquidBuffer);
    return std::make_shared<LiquidDataBindlessVLK<Liquid::SwampData>>(waterTextureHolder, liquidData);
}

std::shared_ptr<AzeritheLiquidData> MapSceneRenderBindlessVLK::createAzeritheLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::AzeritheData>>(liquidBuffers.azeritheLiquidBuffer);
    return std::make_shared<LiquidDataBindlessVLK<Liquid::AzeritheData>>(waterTextureHolder, liquidData);
}
