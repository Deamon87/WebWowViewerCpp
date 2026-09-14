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
#include "liquidData/LiquidDataForwardVLK.h"
#include "materials/IMaterialInstance.h"
#include "../../../gapi/vulkan/meshes/GSortableMeshVLK.h"
#include "../../../gapi/vulkan/buffers/GBufferChunkDynamicVLK.h"
#include "../../../gapi/vulkan/buffers/GBufferChunkDynamicVersionedVLK.h"
#include "../../frame/FrameProfile.h"
#include "../../../gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder_inline.h"
#include "meshCollectors/OpaqueMeshCollectorForward.h"
#include <future>

class IM2ModelDataVLK : public IM2ModelData {
public:
    ~IM2ModelDataVLK() override = default;
    std::shared_ptr<GDescriptorSet> m2CommonDS;
};

static const DescTypeSetBindingConfig SceneDataSetConfig = {
    {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
    {1, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {2, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}}
};

static const DescTypeSetBindingConfig LiquidDataSetConfig = {
    {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}},
    {1, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {2, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {3, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {4, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {5, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
    {6, {VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT}}
};

static const ShaderConfig SceneDataSetStubShaderConfig = {
    "stubs",
    "stubs",
    {
        {0, SceneDataSetConfig}
    }
};

static const ShaderConfig forwardShaderConfig = {
    "forwardRendering",
    "forwardRendering",
    {
        {0, SceneDataSetConfig}
    }
};
static const ShaderConfig m2ForwardShaderConfig = {
    "forwardRendering",
    "forwardRendering",
    {
        {0, SceneDataSetConfig},
        {1, {
            {5, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}}
        }},
}};
static const ShaderConfig liquidShaderConfig = {
    "forwardRendering",
    "forwardRendering",
    {
        {0, SceneDataSetConfig},
        {2, LiquidDataSetConfig}
    }
};

MapSceneRenderForwardVLK::MapSceneRenderForwardVLK(const HGDeviceVLK &hDevice, Config *config) :
    m_device(hDevice), MapSceneRenderer(config) {

    std::cout << "Create Forward scene renderer " << std::endl;

    allBuffers = {
        iboBuffer,
        vboM2Buffer,
        vboPortalBuffer,
        vboM2RibbonBuffer,
        vboAdtBuffer,
        vboWMOBuffer,
        vboWaterBuffer,
        vboSkyBuffer,
        m_vboQuad,
        m_iboQuad,
        m_particleIndexBuffer,
        uboBuffer,
        uboStaticBuffer,
        uboM2BoneMatrixBuffer,
        liquidBuffers.waterLiquidBuffer,
        liquidBuffers.magmaLiquidBuffer,
        liquidBuffers.mercuryLiquidBuffer,
        liquidBuffers.fogLiquidBuffer,
        liquidBuffers.leyLineLiquidBuffer,
        liquidBuffers.felLiquidBuffer,
        liquidBuffers.swampLiquidBuffer,
        liquidBuffers.azeritheLiquidBuffer
    };

    iboBuffer   = m_device->createIndexBuffer("Scene_IBO", 1024*1024);

    vboM2Buffer         = m_device->createVertexBuffer("Scene_VBO_M2",1024*1024);
    vboPortalBuffer     = m_device->createVertexBuffer("Scene_VBO_Portal",1024*1024);
    for (int k = 0; k < PARTICLES_BUFF_NUM; k++)
        vboM2ParticleBuffers[k] = m_device->createVertexBuffer("Scene_VBO_M2Particle_fr_"+std::to_string(k),1024*1024, 64);

    for (auto &vboM2ParticleBuff : this->vboM2ParticleBuffers) {
        allBuffers.push_back(vboM2ParticleBuff);
    }

    vboM2RibbonBuffer   = m_device->createVertexBuffer("Scene_VBO_M2Ribbon",1024*1024);
    vboAdtBuffer        = m_device->createVertexBuffer("Scene_VBO_ADT",3*1024*1024);
    vboWMOBuffer        = m_device->createVertexBuffer("Scene_VBO_WMO",1024*1024);
    vboWaterBuffer      = m_device->createVertexBuffer("Scene_VBO_Water",1024*1024, sizeof(LiquidVertexFormat));
    vboSkyBuffer        = m_device->createVertexBuffer("Scene_VBO_Sky",1024*1024);

    liquidBuffers.waterLiquidBuffer = m_device->createUniformBuffer("Scene_WaterLiquidBuffer", 16384);
    liquidBuffers.magmaLiquidBuffer = m_device->createUniformBuffer("Scene_MagmaLiquidBuffer", 16384);
    liquidBuffers.mercuryLiquidBuffer = m_device->createUniformBuffer("Scene_MercuryLiquidBuffer", 16384);
    liquidBuffers.fogLiquidBuffer = m_device->createUniformBuffer("Scene_FogLiquidBuffer", 16384);
    liquidBuffers.leyLineLiquidBuffer = m_device->createUniformBuffer("Scene_LeyLineLiquidBuffer", 16384);
    liquidBuffers.felLiquidBuffer = m_device->createUniformBuffer("Scene_FelLiquidBuffer", 16384);
    liquidBuffers.swampLiquidBuffer = m_device->createUniformBuffer("Scene_SwampLiquidBuffer", 16384);
    liquidBuffers.azeritheLiquidBuffer = m_device->createUniformBuffer("Scene_AzeritheLiquidBuffer", 16384);

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
        m_vboQuad = m_device->createVertexBuffer("Scene_VBO_Quad", vertexBuffer.size() * sizeof(mathfu::vec2_packed));
        m_iboQuad = m_device->createIndexBuffer("Scene_IBO_Quad", indexBuffer.size() * sizeof(uint16_t));
        m_vboQuad->uploadData(vertexBuffer.data(), vertexBuffer.size() * sizeof(mathfu::vec2_packed));
        m_iboQuad->uploadData(indexBuffer.data(), indexBuffer.size() * sizeof(uint16_t));

        m_drawQuadVao = m_device->createVertexBufferBindings();
        m_drawQuadVao->addVertexBufferBinding(m_vboQuad, std::vector(fullScreenQuad.begin(), fullScreenQuad.end()));
        m_drawQuadVao->setIndexBuffer(m_iboQuad);
        m_drawQuadVao->save();
    }

    {
        m_particleIndexBuffer = m_device->createIndexBuffer("Scene_IBO_Particle",
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

    uboBuffer = m_device->createUniformBuffer("Scene_UBO", 1024*1024);
    uboStaticBuffer = m_device->createUniformBuffer("Scene_UBOStatic", 1024*1024);

    uboM2BoneMatrixBuffer = m_device->createUniformBuffer("Scene_UBO_M2BoneMats", 5000*64);

    m_emptyADTVAO = createADTVAO(nullptr, nullptr);
    m_emptyM2VAO = createM2VAO(nullptr, nullptr);
    m_emptyM2ParticleVAO = createM2ParticleVAO(nullptr, nullptr);
    m_emptyM2RibbonVAO = createM2RibbonVAO(nullptr, nullptr);
    m_emptySkyVAO = createSkyVAO(nullptr, nullptr);
    m_emptyWMOVAO = createWmoVAO(nullptr, nullptr);
    m_emptyWaterVAO = createWaterVAO(nullptr, nullptr);
    m_emptyPortalVAO = createPortalVAO(nullptr, nullptr);

    //Framebuffers for rendering
    auto const dataFormat = { ITextureFormat::itRGBA};

    defaultView = std::make_shared<RenderViewForwardVLK>(m_device, uboBuffer, m_drawQuadVao, false);

    m_renderPass = defaultView->getRenderPass();

    sceneWideChunk = std::make_shared<GBufferChunkDynamicVersionedVLK<sceneWideBlockVSPS>>(hDevice, 3, uboBuffer);
    MaterialBuilderVLK::fromShader(m_device, {"stub", "commonSceneData"}, SceneDataSetStubShaderConfig)
        .createDescriptorSet(0, [&](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo_dynamic(0, sceneWideChunk)
                .texture(1, hDevice->getBlackTexturePixel())
                .texture(2, hDevice->getWhiteTexturePixel())
                .delayUpdate();

            sceneWideDS = ds;
        });

    defaultM2CommonDS = std::dynamic_pointer_cast<IM2ModelDataVLK>(createM2ModelMat(0,0,0,0))->m2CommonDS;

    // Create separate descriptor sets for each liquid material type
    {
        MaterialBuilderVLK::fromShader(m_device, {"liquids/liquidShader", "liquids/forward/waterLiquidShader"},
                                       liquidShaderConfig)
            .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
                waterLiquidTexturesDS = ds;
            }).toMaterial();
        MaterialBuilderVLK::fromShader(m_device, {"liquids/liquidShader", "liquids/forward/magmaLiquidShader"},
                                       liquidShaderConfig)
            .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
                magmaLiquidTexturesDS = ds;
            }).toMaterial();
        MaterialBuilderVLK::fromShader(m_device, {"liquids/liquidShader", "liquids/forward/mercuryLiquidShader"},
                                       liquidShaderConfig)
            .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
                mercuryLiquidTexturesDS = ds;
            }).toMaterial();
        MaterialBuilderVLK::fromShader(m_device, {"liquids/liquidShader", "liquids/forward/fogLiquidShader"},
                                       liquidShaderConfig)
            .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
                fogLiquidTexturesDS = ds;
            }).toMaterial();
        MaterialBuilderVLK::fromShader(m_device, {"liquids/liquidShader", "liquids/forward/leyLineLiquidShader"},
                                       liquidShaderConfig)
            .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
                leyLineLiquidTexturesDS = ds;
            }).toMaterial();
        MaterialBuilderVLK::fromShader(m_device, {"liquids/liquidShader", "liquids/forward/felLiquidShader"},
                                       liquidShaderConfig)
            .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
                felLiquidTexturesDS = ds;
            }).toMaterial();
        MaterialBuilderVLK::fromShader(m_device, {"liquids/liquidShader", "liquids/forward/swampLiquidShader"},
                                       liquidShaderConfig)
            .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
                swampLiquidTexturesDS = ds;
            }).toMaterial();
        MaterialBuilderVLK::fromShader(m_device, {"liquids/liquidShader", "liquids/forward/azeritheLiquidShader"},
                                       liquidShaderConfig)
            .createDescriptorSet(2, [&](std::shared_ptr<GDescriptorSet> &ds) {
                azeritheLiquidTexturesDS = ds;
            }).toMaterial();
    }
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

HGVertexBuffer MapSceneRenderForwardVLK::createM2ParticleVertexBuffer(int sizeInBytes, int frameIndex) {
    if (frameIndex >= PARTICLES_BUFF_NUM) return nullptr;

    return vboM2ParticleBuffers[frameIndex]->getSubBuffer(sizeInBytes);
}
HGVertexBuffer MapSceneRenderForwardVLK::createM2RibbonVertexBuffer(int sizeInBytes) {
    return vboM2RibbonBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer MapSceneRenderForwardVLK::createM2IndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer MapSceneRenderForwardVLK::getOrCreateM2ParticleIndexBuffer() {
    return m_particleIndexBuffer;
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



std::shared_ptr<IADTMaterial>
MapSceneRenderForwardVLK::createAdtMaterial(const PipelineTemplate &pipelineTemplate, const ADTMaterialTemplate &adtMaterialTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto vertexFragmentData = std::make_shared<CBufferChunkVLK<ADT::meshWideBlockVSPS>>(uboStaticBuffer);
    auto fragmentData = std::make_shared<CBufferChunkVLK<ADT::meshWideBlockPS>>(uboBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"adtShader", "adtShader"}, forwardShaderConfig)
        .createPipeline(m_emptyADTVAO, m_renderPass, pipelineTemplate)
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
MapSceneRenderForwardVLK::createM2Material(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                           const PipelineTemplate &pipelineTemplate,
                                           const M2MaterialTemplate &m2MaterialTemplate) {
 auto &l_sceneWideChunk = sceneWideChunk;
    auto vertexFragmentData = std::make_shared<CBufferChunkVLK<M2::meshWideBlockVSPS>>(uboStaticBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"m2Shader", "m2Shader"}, m2ForwardShaderConfig)
        .createPipeline(m_emptyM2VAO, m_renderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .bindDescriptorSet(1, std::dynamic_pointer_cast<IM2ModelDataVLK>(m2ModelData)->m2CommonDS)
        .createDescriptorSet(2, [&vertexFragmentData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, *vertexFragmentData).delayUpdate();
        })
        .createDescriptorSet(3, [&m2MaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(0, m2MaterialTemplate.textures[0])
                .texture(1, m2MaterialTemplate.textures[1])
                .texture(2, m2MaterialTemplate.textures[2])
                .texture(3, m2MaterialTemplate.textures[3]);
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

std::shared_ptr<IM2ProjectiveMaterial> MapSceneRenderForwardVLK::createM2ProjectiveMaterial(
    const std::shared_ptr<IM2ModelData> &m2ModelData,
    const PipelineTemplate &pipelineTemplate,
    const M2MaterialTemplate &m2MaterialTemplate
) {
    return nullptr;
}

std::shared_ptr<IM2WaterFallMaterial> MapSceneRenderForwardVLK::createM2WaterfallMaterial(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                                const PipelineTemplate &pipelineTemplate,
                                                                const M2WaterfallMaterialTemplate &m2MaterialTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto waterfallCommonData = std::make_shared<CBufferChunkVLK<M2::WaterfallData::WaterfallCommon>>(uboStaticBuffer);


    auto material = MaterialBuilderVLK::fromShader(m_device, {"waterfallShader", "waterfallShader"}, m2ForwardShaderConfig)
        .createPipeline(m_emptyM2VAO, m_renderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .bindDescriptorSet(1, std::dynamic_pointer_cast<IM2ModelDataVLK>(m2ModelData)->m2CommonDS)
        .createDescriptorSet(2, [&waterfallCommonData, m2MaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
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

std::shared_ptr<IM2ParticleMaterial> MapSceneRenderForwardVLK::createM2ParticleMaterial(
    const PipelineTemplate &pipelineTemplate,
    const M2ParticleMaterialTemplate &m2ParticleMatTemplate) {

    auto &l_sceneWideChunk = sceneWideChunk;
    auto l_fragmentData = std::make_shared<CBufferChunkVLK<Particle::meshParticleWideBlockPS>>(uboBuffer); ;

    auto material = MaterialBuilderVLK::fromShader(m_device, {"m2ParticleShader", "m2ParticleShader"}, forwardShaderConfig)
        .createPipeline(m_emptyM2ParticleVAO, m_renderPass, pipelineTemplate)
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

std::shared_ptr<IM2RibbonMaterial> MapSceneRenderForwardVLK::createM2RibbonMaterial(const std::shared_ptr<IM2ModelData> &m2ModelData,
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
std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> MapSceneRenderForwardVLK::createWmoModelMatrixChunk() {
    return std::make_shared<CBufferChunkVLK<WMO::modelWideBlockVS>>(uboBuffer);
};

std::shared_ptr<IWmoModelData> MapSceneRenderForwardVLK::createWMOWideChunk(int groupNum)  {
    auto wmoModelData = std::make_shared<IWmoModelData>();
    wmoModelData->m_placementMatrix = createWmoModelMatrixChunk();
    BufferChunkHelperVLK::create(uboBuffer, wmoModelData->m_groupInteriorData, sizeof(WMO::InteriorBlockData) * groupNum);

    return wmoModelData;
}

std::shared_ptr<IWMOMaterial> MapSceneRenderForwardVLK::createWMOMaterial(const std::shared_ptr<IWmoModelData> &wmoModelWide,
                                                                          const PipelineTemplate &pipelineTemplate,
                                                                          const WMOMaterialTemplate &wmoMaterialTemplate) {
    auto l_vertexData = std::make_shared<CBufferChunkVLK<WMO::meshWideBlockVS>>(uboStaticBuffer); ;
    auto l_fragmentData = std::make_shared<CBufferChunkVLK<WMO::meshWideBlockPS>>(uboStaticBuffer); ;

    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"wmoShader", "wmoShader"}, forwardShaderConfig)
        .createPipeline(m_emptyWMOVAO, m_renderPass, pipelineTemplate)
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

std::shared_ptr<ILiquidMaterial> MapSceneRenderForwardVLK::createLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
                                                const PipelineTemplate &pipelineTemplate,
                                                const WaterMaterialTemplate &waterMaterialTemplate) {

    auto liquidInstanceData = std::make_shared<CBufferChunkVLK<Liquid::LiquidInstance>>(uboStaticBuffer);
    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"waterShader", "waterShader"}, forwardShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_renderPass, pipelineTemplate)
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

const PipelineTemplate s_LiquidPipelineTemplate = []{
    PipelineTemplate pipelineTemplate;
    pipelineTemplate.element = DrawElementMode::TRIANGLES;
    pipelineTemplate.depthWrite = true;
    pipelineTemplate.depthCulling = true;
    pipelineTemplate.backFaceCulling = false;
    pipelineTemplate.blendMode = EGxBlendEnum::GxBlend_Alpha;
    return pipelineTemplate;
}();


std::shared_ptr<ILiquidMaterial> MapSceneRenderForwardVLK::createWaterLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<WaterLiquidData> &liquidData) {
    auto liquidInstanceData = std::make_shared<CBufferChunkVLK<Liquid::LiquidInstance>>(uboStaticBuffer);
    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"liquids/liquidShader", "liquids/forward/waterLiquidShader"}, forwardShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_renderPass, s_LiquidPipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &modelWide, &liquidInstanceData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(modelWide))
                .ubo(1, *liquidInstanceData)
                .delayUpdate();
        })
        .bindDescriptorSet(2, std::static_pointer_cast<LiquidDataForwardVLK<Liquid::WaterData>>(liquidData)->getDescriptorSet())
        .toMaterial<ILiquidMaterial>([&liquidData](ILiquidMaterial *instance) -> void {
            instance->m_ldData = liquidData;
        });
    material->m_instance = liquidInstanceData;
    return material;
}

std::shared_ptr<ILiquidMaterial> MapSceneRenderForwardVLK::createMagmaLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<MagmaLiquidData> &liquidData) {
    auto liquidInstanceData = std::make_shared<CBufferChunkVLK<Liquid::LiquidInstance>>(uboStaticBuffer);
    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"liquids/liquidShader", "liquids/forward/magmaLiquidShader"}, forwardShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_renderPass, s_LiquidPipelineTemplate)
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

std::shared_ptr<ILiquidMaterial> MapSceneRenderForwardVLK::createMercuryLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<MercuryLiquidData> &liquidData) {
    auto liquidInstanceData = std::make_shared<CBufferChunkVLK<Liquid::LiquidInstance>>(uboStaticBuffer);
    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"liquids/liquidShader", "liquids/forward/mercuryLiquidShader"}, forwardShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_renderPass, s_LiquidPipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &modelWide, &liquidInstanceData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(modelWide))
                .ubo(1, *liquidInstanceData)
                .delayUpdate();
        })
        .bindDescriptorSet(2, std::static_pointer_cast<LiquidDataForwardVLK<Liquid::MercuryData>>(liquidData)->getDescriptorSet())
        .toMaterial<ILiquidMaterial>([&liquidData](ILiquidMaterial *instance) -> void {
            instance->m_ldData = liquidData;
        });
    material->m_instance = liquidInstanceData;
    return material;
}

std::shared_ptr<ILiquidMaterial> MapSceneRenderForwardVLK::createFogLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<FogLiquidData> &liquidData) {
    auto liquidInstanceData = std::make_shared<CBufferChunkVLK<Liquid::LiquidInstance>>(uboStaticBuffer);
    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"liquids/liquidShader", "liquids/forward/fogLiquidShader"}, forwardShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_renderPass, s_LiquidPipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &modelWide, &liquidInstanceData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(modelWide))
                .ubo(1, *liquidInstanceData)
                .delayUpdate();
        })
        .bindDescriptorSet(2, std::static_pointer_cast<LiquidDataForwardVLK<Liquid::FogData>>(liquidData)->getDescriptorSet())
        .toMaterial<ILiquidMaterial>([&liquidData](ILiquidMaterial *instance) -> void {
            instance->m_ldData = liquidData;
        });
    material->m_instance = liquidInstanceData;
    return material;
}

std::shared_ptr<ILiquidMaterial> MapSceneRenderForwardVLK::createLeyLineLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<LeyLineLiquidData> &liquidData) {
    auto liquidInstanceData = std::make_shared<CBufferChunkVLK<Liquid::LiquidInstance>>(uboStaticBuffer);
    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"liquids/liquidShader", "liquids/forward/leyLineLiquidShader"}, forwardShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_renderPass, s_LiquidPipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &modelWide, &liquidInstanceData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(modelWide))
                .ubo(1, *liquidInstanceData)
                .delayUpdate();
        })
        .bindDescriptorSet(2, std::static_pointer_cast<LiquidDataForwardVLK<Liquid::LeyLineData>>(liquidData)->getDescriptorSet())
        .toMaterial<ILiquidMaterial>([&liquidData](ILiquidMaterial *instance) -> void {
            instance->m_ldData = liquidData;
        });
    material->m_instance = liquidInstanceData;
    return material;
}

std::shared_ptr<ILiquidMaterial> MapSceneRenderForwardVLK::createFelLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<FelLiquidData> &liquidData) {
    auto liquidInstanceData = std::make_shared<CBufferChunkVLK<Liquid::LiquidInstance>>(uboStaticBuffer);
    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"liquids/liquidShader", "liquids/forward/felLiquidShader"}, forwardShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_renderPass, s_LiquidPipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &modelWide, &liquidInstanceData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(modelWide))
                .ubo(1, *liquidInstanceData)
                .delayUpdate();
        })
        .bindDescriptorSet(2, std::static_pointer_cast<LiquidDataForwardVLK<Liquid::FelData>>(liquidData)->getDescriptorSet())
        .toMaterial<ILiquidMaterial>([&liquidData](ILiquidMaterial *instance) -> void {
            instance->m_ldData = liquidData;
        });
    material->m_instance = liquidInstanceData;
    return material;
}

std::shared_ptr<ILiquidMaterial> MapSceneRenderForwardVLK::createSwampLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<SwampLiquidData> &liquidData) {
    auto liquidInstanceData = std::make_shared<CBufferChunkVLK<Liquid::LiquidInstance>>(uboStaticBuffer);
    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"liquids/liquidShader", "liquids/forward/swampLiquidShader"}, forwardShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_renderPass, s_LiquidPipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &modelWide, &liquidInstanceData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(modelWide))
                .ubo(1, *liquidInstanceData)
                .delayUpdate();
        })
        .bindDescriptorSet(2, std::static_pointer_cast<LiquidDataForwardVLK<Liquid::SwampData>>(liquidData)->getDescriptorSet())
        .toMaterial<ILiquidMaterial>([&liquidData](ILiquidMaterial *instance) -> void {
            instance->m_ldData = liquidData;
        });
    material->m_instance = liquidInstanceData;
    return material;
}

std::shared_ptr<ILiquidMaterial> MapSceneRenderForwardVLK::createAzeritheLiquidMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, const std::shared_ptr<AzeritheLiquidData> &liquidData) {
    auto liquidInstanceData = std::make_shared<CBufferChunkVLK<Liquid::LiquidInstance>>(uboStaticBuffer);
    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"liquids/liquidShader", "liquids/forward/azeritheLiquidShader"}, forwardShaderConfig)
        .createPipeline(m_emptyWaterVAO, m_renderPass, s_LiquidPipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &modelWide, &liquidInstanceData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(modelWide))
                .ubo(1, *liquidInstanceData)
                .delayUpdate();
        })
        .bindDescriptorSet(2, std::static_pointer_cast<LiquidDataForwardVLK<Liquid::AzeritheData>>(liquidData)->getDescriptorSet())
        .toMaterial<ILiquidMaterial>([&liquidData](ILiquidMaterial *instance) -> void {
            instance->m_ldData = liquidData;
        });
    material->m_instance = liquidInstanceData;
    return material;
}



std::shared_ptr<ISkyMeshMaterial> MapSceneRenderForwardVLK::createSkyMeshMaterial(const PipelineTemplate &pipelineTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto skyColors = std::make_shared<CBufferChunkVLK<DnSky::meshWideBlockVS>>(uboBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"skyConus", "skyConus"}, forwardShaderConfig)
        .createPipeline(m_emptySkyVAO, m_renderPass, pipelineTemplate)
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

std::shared_ptr<IPlanetMaterial> MapSceneRenderForwardVLK::createPlanetMaterial(const PipelineTemplate &pipelineTemplate, const HGSamplableTexture &texture) {
    auto planetData = std::make_shared<CBufferChunkVLK<Planet::meshWideBlockVS>>(uboBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"planetShader", "planetShader"}, forwardShaderConfig)
        .createPipeline(m_emptySkyVAO, m_renderPass, pipelineTemplate)
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

std::shared_ptr<IPortalMaterial> MapSceneRenderForwardVLK::createPortalMaterial(const PipelineTemplate &pipelineTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto materialPS = std::make_shared<CBufferChunkVLK<DrawPortalShader::meshWideBlockPS>>(uboBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"drawPortalShader", "drawPortalShader"}, forwardShaderConfig)
        .createPipeline(m_emptyPortalVAO, m_renderPass, pipelineTemplate)
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

std::shared_ptr<IM2ModelData> MapSceneRenderForwardVLK::createM2ModelMat(int bonesCount, int m2ColorsCount, int textureWeightsCount, int textureMatricesCount, uint32_t objectId) {
    auto result = std::make_shared<IM2ModelDataVLK>();

    BufferChunkHelperVLK::create(uboBuffer, result->m_placementMatrix);
    BufferChunkHelperVLK::create(uboM2BoneMatrixBuffer, result->m_bonesData, sizeof(mathfu::mat4) * bonesCount);
    BufferChunkHelperVLK::create(uboBuffer, result->m_colors, sizeof(mathfu::vec4_packed) * m2ColorsCount);
    BufferChunkHelperVLK::create(uboBuffer, result->m_textureWeights, sizeof(float) * textureWeightsCount);
    BufferChunkHelperVLK::create(uboBuffer, result->m_textureMatrices, sizeof(mathfu::mat4) * textureMatricesCount);
    BufferChunkHelperVLK::create(uboBuffer, result->m_modelFragmentData);
    result->m_modelFragmentData->getObject().objectId = objectId;
    result->objectId = objectId;

    MaterialBuilderVLK::fromShader(m_device, {"m2Shader", "m2Shader"}, m2ForwardShaderConfig)
        .createDescriptorSet(1, [&](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(result->m_placementMatrix))
                .ubo(1, BufferChunkHelperVLK::cast(result->m_modelFragmentData))
                .ubo(2, BufferChunkHelperVLK::cast(result->m_bonesData))
                .ubo(3, BufferChunkHelperVLK::cast(result->m_colors))
                .ubo(4, BufferChunkHelperVLK::cast(result->m_textureWeights))
                .ubo(5, BufferChunkHelperVLK::cast(result->m_textureMatrices)).delayUpdate();
            result->m2CommonDS = ds;
        });

    return result;
}

std::unique_ptr<IRenderFunction> MapSceneRenderForwardVLK::update(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams,
                                             const std::shared_ptr<MapRenderPlan> &framePlan) {
    TracyMessageStr(("Update stage frame = " + std::to_string(FrameContext::getCurrentProcessingFrameNumber())));

    ZoneScoped;
    auto l_this = std::dynamic_pointer_cast<MapSceneRenderForwardVLK>(this->shared_from_this());
    auto mapScene = std::dynamic_pointer_cast<Map>(frameInputParams->frameParameters->scene);


    //Create meshes
    std::unique_ptr<COpaqueMeshCollectorForwardVLK> u_collector = std::make_unique<COpaqueMeshCollectorForwardVLK>();
    auto transparentMeshes = std::make_shared<framebased::vector<HGSortableMesh>>();



    auto skyMeshes = std::make_shared<framebased::vector<HGMesh>>();
    framePlan->m2Array.lock();
    framePlan->wmoArray.lock();
    framePlan->wmoGroupArray.lock();



    mapScene->update(framePlan);
    mapScene->updateBuffers(l_this, framePlan);

    //    TracyMessageL("collect meshes created");
//    std::future<void> collectMeshAsync = std::async(std::launch::async,
//                                                    [&]() {
    collectMeshes(framePlan, *u_collector, transparentMeshes, skyMeshes);
//                                                    }
//    );

    std::vector<RenderingMatAndSceneSize> renderingMatricessAndSizes;
    for (auto &rt : frameInputParams->frameParameters->renderTargets) {
        auto &matAndSceneSize = renderingMatricessAndSizes.emplace_back();
        matAndSceneSize.renderingMat = rt.cameraMatricesForRendering;
        matAndSceneSize.width = rt.viewPortDimensions.maxs[0];
        matAndSceneSize.height = rt.viewPortDimensions.maxs[1];
    }


    updateSceneWideChunk(sceneWideChunk,
                         renderingMatricessAndSizes,
                         framePlan->frameDependentData,
                         true,
                         mapScene->getCurrentSceneTime());


//    {
//        ZoneScopedN("collect meshes wait");
//        collectMeshAsync.wait();
//    }

    bool renderSky = framePlan->renderSky;
    return createRenderFuncVLK(
        [l_this, mapScene, framePlan, transparentMeshes, frameInputParams]() -> void {
            {
                ZoneScopedN("Post Load");
                //Do postLoad here. So creation of stuff is done from main thread

                mapScene->doPostLoad(l_this, framePlan);
                for (auto &renderTarget : frameInputParams->frameParameters->renderTargets) {
                    auto updatingTarget = std::dynamic_pointer_cast<RenderViewForwardVLK>(renderTarget.target);
                    if (!updatingTarget) updatingTarget = l_this->defaultView;

                    updatingTarget->update(
                        renderTarget.viewPortDimensions.maxs[0],
                        renderTarget.viewPortDimensions.maxs[1],
                        framePlan->frameDependentData->currentGlow
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
        [l_this](CmdBufRecorder &uploadCmd) -> void {
        // ---------------------
        // Upload stuff
        // ---------------------
        {
           ZoneScopedN("submit buffers");
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
        skyMeshes,
    renderSky,
    mapScene, framePlan,
    l_this, frameInputParams](CmdBufRecorder &frameBufCmd, CmdBufRecorder &swapChainCmd) -> void {

        TracyMessageStr(("Draw stage frame = " + std::to_string(FrameContext::getCurrentProcessingFrameNumber())));

        // ----------------------
        // Draw meshes
        // ----------------------
        {
            uint8_t wideChunkVersion = 0;
            for (auto &renderTarget : frameInputParams->frameParameters->renderTargets) {
                l_this->sceneWideChunk->setCurrentVersion(wideChunkVersion++);

                auto currentView = renderTarget.target == nullptr ?
                    l_this->defaultView :
                    std::dynamic_pointer_cast<RenderViewForwardVLK>(renderTarget.target);
                {
                    auto passHelper = currentView->beginPass(frameBufCmd, l_this->m_renderPass,
                                                             false,
                                                             frameInputParams->frameParameters->clearColor);

                    {
                        ZoneScopedN("submit opaque");
                        VkZone(frameBufCmd, "render opaque")
                        l_opaqueMeshes->render(frameBufCmd, CmdBufRecorder::ViewportType::vp_usual);
                    }
                    {
                        //Sky transparent
                        for (int i = 0; i < skyMeshes->size(); i++) {
                            auto const &mesh = skyMeshes->at(i);

                            frameBufCmd.drawMesh( mesh, CmdBufRecorder::ViewportType::vp_skyBox);
                        }
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

                            frameBufCmd.drawMesh( mesh, CmdBufRecorder::ViewportType::vp_usual);
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

std::shared_ptr<MapRenderPlan> MapSceneRenderForwardVLK::getLastCreatedPlan() {
    return m_lastCreatedPlan;
}

HGMesh MapSceneRenderForwardVLK::createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) {
    return meshFactory->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0,0);
}

HGSortableMesh MapSceneRenderForwardVLK::createSortableMesh(gMeshTemplate &meshTemplate, const HMaterial &material, int priorityPlane) {
    auto mesh = meshFactory->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0, priorityPlane);
    return mesh;
}

HGMesh MapSceneRenderForwardVLK::createAdtMesh(gMeshTemplate &meshTemplate,  const std::shared_ptr<IADTMaterial> &material) {
    auto mesh = meshFactory->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0, 0);
    return mesh;
};

HGM2Mesh
MapSceneRenderForwardVLK::createM2Mesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2Material> &material,
                                       int layer, int priorityPlane) {

    auto mesh = meshFactory->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), layer, priorityPlane);
    return mesh;
}
HGM2Mesh MapSceneRenderForwardVLK::createM2ProjectiveMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2ProjectiveMaterial> &material, int layer, int priorityPlane) {
    // auto mesh = meshFactory->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), layer, priorityPlane);
    // return mesh;

    return nullptr;
}
HGM2Mesh
MapSceneRenderForwardVLK::createM2ParticleMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2Material> &material, int layer, int priorityPlane) {
    auto mesh = meshFactory->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0, priorityPlane);
    return mesh;
}
HGM2Mesh MapSceneRenderForwardVLK::createM2WaterfallMesh(gMeshTemplate &meshTemplate,
                                                         const std::shared_ptr<IM2WaterFallMaterial> &material,
                                                         int layer, int priorityPlane) {
    auto mesh = meshFactory->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), layer, priorityPlane);
    return mesh;
}

HGSortableMesh MapSceneRenderForwardVLK::createWaterMesh(gMeshTemplate &meshTemplate, const HMaterial &material, int priorityPlane) {
    auto mesh = meshFactory->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0, priorityPlane);
    return mesh;
}
HGSortableMesh MapSceneRenderForwardVLK::createWMOMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IWMOMaterial> &material, int groupNum, int canHaveExteriorLit, uint32_t wmoObjId) {
    auto mesh = meshFactory->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0, 0);
    mesh->instanceIndex = ((canHaveExteriorLit & 1) << 10) | (groupNum & ((1 << 10) -1));
    return mesh;
}

std::shared_ptr<IRenderView> MapSceneRenderForwardVLK::createRenderView(bool createOutput) {
    return std::make_shared<RenderViewForwardVLK>(m_device, uboBuffer, m_drawQuadVao, createOutput);
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

std::shared_ptr<WaterLiquidData> MapSceneRenderForwardVLK::createWaterLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::WaterData>>(liquidBuffers.waterLiquidBuffer);
    return std::make_shared<LiquidDataForwardVLK<Liquid::WaterData>>(waterLiquidTexturesDS, liquidData);
}

std::shared_ptr<MagmaLiquidData> MapSceneRenderForwardVLK::createMagmaLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::MagmaData>>(liquidBuffers.magmaLiquidBuffer);
    return std::make_shared<LiquidDataForwardVLK<Liquid::MagmaData>>(magmaLiquidTexturesDS, liquidData);
}

std::shared_ptr<MercuryLiquidData> MapSceneRenderForwardVLK::createMercuryLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::MercuryData>>(liquidBuffers.mercuryLiquidBuffer);
    return std::make_shared<LiquidDataForwardVLK<Liquid::MercuryData>>(mercuryLiquidTexturesDS, liquidData);
}

std::shared_ptr<FogLiquidData> MapSceneRenderForwardVLK::createFogLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::FogData>>(liquidBuffers.fogLiquidBuffer);
    return std::make_shared<LiquidDataForwardVLK<Liquid::FogData>>(fogLiquidTexturesDS, liquidData);
}

std::shared_ptr<LeyLineLiquidData> MapSceneRenderForwardVLK::createLeyLineLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::LeyLineData>>(liquidBuffers.leyLineLiquidBuffer);
    return std::make_shared<LiquidDataForwardVLK<Liquid::LeyLineData>>(leyLineLiquidTexturesDS, liquidData);
}

std::shared_ptr<FelLiquidData> MapSceneRenderForwardVLK::createFelLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::FelData>>(liquidBuffers.felLiquidBuffer);
    return std::make_shared<LiquidDataForwardVLK<Liquid::FelData>>(felLiquidTexturesDS, liquidData);
}

std::shared_ptr<SwampLiquidData> MapSceneRenderForwardVLK::createSwampLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::SwampData>>(liquidBuffers.swampLiquidBuffer);
    return std::make_shared<LiquidDataForwardVLK<Liquid::SwampData>>(swampLiquidTexturesDS, liquidData);
}

std::shared_ptr<AzeritheLiquidData> MapSceneRenderForwardVLK::createAzeritheLiquidData() {
    auto liquidData = std::make_shared<CBufferChunkVLK<Liquid::AzeritheData>>(liquidBuffers.azeritheLiquidBuffer);
    return std::make_shared<LiquidDataForwardVLK<Liquid::AzeritheData>>(azeritheLiquidTexturesDS, liquidData);
}
