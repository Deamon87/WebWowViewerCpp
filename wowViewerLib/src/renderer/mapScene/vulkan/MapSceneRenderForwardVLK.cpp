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
#include "../../../gapi/vulkan/buffers/GBufferChunkDynamicVersionedVLK.h"
#include "../../frame/FrameProfile.h"
#include "../../../gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder_inline.h"
#include <future>

static const ShaderConfig forwardShaderConfig = {
    "forwardRendering",
    "forwardRendering",
    {
        {0, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, false, 1, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT}}
        }}
    }
};
static const ShaderConfig m2ForwardShaderConfig = {
    "forwardRendering",
    "forwardRendering",
    {
        {0, {
            {0, {VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC}}
        }}
}};

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
        vboWMOGroupAmbient,
        m_vboQuad,
        m_iboQuad,
        m_particleIndexBuffer,
        uboBuffer,
        uboStaticBuffer,
        uboM2BoneMatrixBuffer
    };
    for (auto &vboM2ParticleBuff : this->vboM2ParticleBuffers) {
        allBuffers.push_back(vboM2ParticleBuff);
    }


    iboBuffer   = m_device->createIndexBuffer("Scene_IBO", 1024*1024);

    vboM2Buffer         = m_device->createVertexBuffer("Scene_VBO_M2",1024*1024);
    vboPortalBuffer     = m_device->createVertexBuffer("Scene_VBO_Portal",1024*1024);
    for (int k = 0; k < PARTICLES_BUFF_NUM; k++)
        vboM2ParticleBuffers[k] = m_device->createVertexBuffer("Scene_VBO_M2Particle_fr_"+std::to_string(k),1024*1024, 64);

    vboM2RibbonBuffer   = m_device->createVertexBuffer("Scene_VBO_M2Ribbon",1024*1024);
    vboAdtBuffer        = m_device->createVertexBuffer("Scene_VBO_ADT",3*1024*1024);
    vboWMOBuffer        = m_device->createVertexBuffer("Scene_VBO_WMO",1024*1024);
    vboWaterBuffer      = m_device->createVertexBuffer("Scene_VBO_Water",1024*1024);
    vboSkyBuffer        = m_device->createVertexBuffer("Scene_VBO_Sky",1024*1024);
    vboWMOGroupAmbient  = m_device->createVertexBuffer("Scene_VBO_WMOAmbient",16*200);

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
    m_emptyWMOVAO = createWmoVAO(nullptr, nullptr, nullptr);
    m_emptyWaterVAO = createWaterVAO(nullptr, nullptr);
    m_emptyPortalVAO = createPortalVAO(nullptr, nullptr);

    //Framebuffers for rendering
    auto const dataFormat = { ITextureFormat::itRGBA};

    m_renderPass = m_device->getRenderPass(dataFormat, ITextureFormat::itDepth32,
//                                          VK_SAMPLE_COUNT_1_BIT,
                                          sampleCountToVkSampleCountFlagBits(m_device->getMaxSamplesCnt()),
                                          true, false, true, true);

    defaultView = std::make_shared<RenderViewForwardVLK>(m_device, uboBuffer, m_drawQuadVao, false);

    sceneWideChunk = std::make_shared<GBufferChunkDynamicVersionedVLK<sceneWideBlockVSPS>>(hDevice, 3, uboBuffer);
    MaterialBuilderVLK::fromShader(m_device, {"adtShader", "adtShader"}, forwardShaderConfig, {})
        .createDescriptorSet(0, [&](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo_dynamic(0, sceneWideChunk)
                .texture(1, hDevice->getBlackTexturePixel())
                .texture(2, hDevice->getWhiteTexturePixel())
                .delayUpdate();

            sceneWideDS = ds;
        });
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

HGVertexBufferBindings MapSceneRenderForwardVLK::createWmoVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer, const std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> &ambientBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto wmoVAO = m_device->createVertexBufferBindings();

    wmoVAO->addVertexBufferBinding(vertexBuffer, std::vector(staticWMOBindings.begin(), staticWMOBindings.end()));
    wmoVAO->addVertexBufferBinding(ambientBuffer ? BufferChunkHelperVLK::cast(ambientBuffer) : nullptr,
                                   std::vector(staticWmoGroupAmbient.begin(), staticWmoGroupAmbient.end()), true);
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

    auto material = MaterialBuilderVLK::fromShader(m_device, {"adtShader", "adtShader"}, forwardShaderConfig, {})
        .createPipeline(m_emptyADTVAO, m_renderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [&vertexFragmentData, &fragmentData, &l_sceneWideChunk](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, *vertexFragmentData)
                .ubo(1, *fragmentData).delayUpdate();
        })
        .createDescriptorSet(2, [&adtMaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(0, adtMaterialTemplate.textures[0])
                .texture(1, adtMaterialTemplate.textures[1])
                .texture(2, adtMaterialTemplate.textures[2])
                .texture(3, adtMaterialTemplate.textures[3])
                .texture(4, adtMaterialTemplate.textures[4])
                .texture(5, adtMaterialTemplate.textures[5])
                .texture(6, adtMaterialTemplate.textures[6])
                .texture(7, adtMaterialTemplate.textures[7])
                .texture(8, adtMaterialTemplate.textures[8]);
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

    auto material = MaterialBuilderVLK::fromShader(m_device, {"m2Shader", "m2Shader"}, m2ForwardShaderConfig, {})
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

std::shared_ptr<IM2WaterFallMaterial> MapSceneRenderForwardVLK::createM2WaterfallMaterial(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                                const PipelineTemplate &pipelineTemplate,
                                                                const M2WaterfallMaterialTemplate &m2MaterialTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto waterfallCommonData = std::make_shared<CBufferChunkVLK<M2::WaterfallData::WaterfallCommon>>(uboStaticBuffer);


    auto material = MaterialBuilderVLK::fromShader(m_device, {"waterfallShader", "waterfallShader"}, m2ForwardShaderConfig, {})
        .createPipeline(m_emptyM2VAO, m_renderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .bindDescriptorSet(1, std::dynamic_pointer_cast<IM2ModelDataVLK>(m2ModelData)->m2CommonDS)
        .createDescriptorSet(2, [&waterfallCommonData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, *waterfallCommonData).delayUpdate();
        })
        .createDescriptorSet(3, [&m2MaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(0, m2MaterialTemplate.textures[0])
                .texture(1, m2MaterialTemplate.textures[1])
                .texture(2, m2MaterialTemplate.textures[2])
                .texture(3, m2MaterialTemplate.textures[3])
                .texture(4, m2MaterialTemplate.textures[4]);
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

    auto material = MaterialBuilderVLK::fromShader(m_device, {"m2ParticleShader", "m2ParticleShader"}, forwardShaderConfig, {})
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

    auto material = MaterialBuilderVLK::fromShader(m_device, {"ribbonShader", "ribbonShader"}, forwardShaderConfig, {})
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

std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> MapSceneRenderForwardVLK::createWMOWideChunk() {
    return std::make_shared<CBufferChunkVLK<WMO::modelWideBlockVS>>(uboBuffer);
}
std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> MapSceneRenderForwardVLK::createWMOGroupAmbientChunk() {
    return std::make_shared<CBufferChunkVLK<mathfu::vec4_packed>>(vboWMOGroupAmbient);
}

std::shared_ptr<IWMOMaterial> MapSceneRenderForwardVLK::createWMOMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
                                                                          const PipelineTemplate &pipelineTemplate,
                                                                          const WMOMaterialTemplate &wmoMaterialTemplate) {
    auto l_vertexData = std::make_shared<CBufferChunkVLK<WMO::meshWideBlockVS>>(uboStaticBuffer); ;
    auto l_fragmentData = std::make_shared<CBufferChunkVLK<WMO::meshWideBlockPS>>(uboStaticBuffer); ;

    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"wmoShader", "wmoShader"}, forwardShaderConfig, {})
        .createPipeline(m_emptyWMOVAO, m_renderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &modelWide, l_vertexData, l_fragmentData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(modelWide))
                .ubo(1, *l_vertexData)
                .ubo(2, *l_fragmentData).delayUpdate();
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

std::shared_ptr<IWaterMaterial> MapSceneRenderForwardVLK::createWaterMaterial(const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
                                                const PipelineTemplate &pipelineTemplate,
                                                const WaterMaterialTemplate &waterMaterialTemplate) {
    auto l_fragmentData = std::make_shared<CBufferChunkVLK<Water::meshWideBlockPS>>(uboStaticBuffer); ;

    auto &l_sceneWideChunk = sceneWideChunk;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"waterShader", "waterShader"}, forwardShaderConfig, {})
        .createPipeline(m_emptyWaterVAO, m_renderPass, pipelineTemplate)
        .bindDescriptorSet(0, sceneWideDS)
        .createDescriptorSet(1, [l_sceneWideChunk, &modelWide, l_fragmentData](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, BufferChunkHelperVLK::cast(modelWide))
                .ubo(1, *l_fragmentData).delayUpdate();
        })
        .createDescriptorSet(2, [&waterMaterialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(0, waterMaterialTemplate.texture);
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

    auto material = MaterialBuilderVLK::fromShader(m_device, {"skyConus", "skyConus"}, forwardShaderConfig, {})
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

std::shared_ptr<IPortalMaterial> MapSceneRenderForwardVLK::createPortalMaterial(const PipelineTemplate &pipelineTemplate) {
    auto &l_sceneWideChunk = sceneWideChunk;
    auto materialPS = std::make_shared<CBufferChunkVLK<DrawPortalShader::meshWideBlockPS>>(uboBuffer);

    auto material = MaterialBuilderVLK::fromShader(m_device, {"drawPortalShader", "drawPortalShader"}, forwardShaderConfig, {})
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

std::shared_ptr<IM2ModelData> MapSceneRenderForwardVLK::createM2ModelMat(int bonesCount, int m2ColorsCount, int textureWeightsCount, int textureMatricesCount) {
    auto result = std::make_shared<IM2ModelDataVLK>();

    BufferChunkHelperVLK::create(uboBuffer, result->m_placementMatrix);
    BufferChunkHelperVLK::create(uboM2BoneMatrixBuffer, result->m_bonesData, sizeof(mathfu::mat4) * bonesCount);
    BufferChunkHelperVLK::create(uboBuffer, result->m_colors, sizeof(mathfu::vec4_packed) * m2ColorsCount);
    BufferChunkHelperVLK::create(uboBuffer, result->m_textureWeights, sizeof(float) * textureWeightsCount);
    BufferChunkHelperVLK::create(uboBuffer, result->m_textureMatrices, sizeof(mathfu::mat4) * textureMatricesCount);
    BufferChunkHelperVLK::create(uboBuffer, result->m_modelFragmentData);

    MaterialBuilderVLK::fromShader(m_device, {"m2Shader", "m2Shader"}, m2ForwardShaderConfig, {})
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

inline void MapSceneRenderForwardVLK::drawMesh(CmdBufRecorder &cmdBuf, const HGMesh &mesh, CmdBufRecorder::ViewportType viewportType ) {
    if (mesh == nullptr) return;

    const auto &meshVlk = (GMeshVLK*) mesh.get();
    auto vulkanBindings = std::dynamic_pointer_cast<GVertexBufferBindingsVLK>(mesh->bindings());

    //1. Bind VBOs
    cmdBuf.bindVertexBuffers(vulkanBindings->getVertexBuffers());

    //2. Bind IBOs
    cmdBuf.bindIndexBuffer(vulkanBindings->getIndexBuffer());

    //3. Bind pipeline
    const auto &material = meshVlk->material();
    cmdBuf.bindPipeline(material->getPipeline());

    //4. Bind Descriptor sets
    auto const &descSets = material->getDescriptorSets();
    cmdBuf.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, descSets);

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

class COpaqueMeshCollectorForwardVLK : public COpaqueMeshCollector{
public:
    COpaqueMeshCollectorForwardVLK() {
        commonMeshes.reserve(10000);
        waterMeshes.reserve(1000);
    }
private:
    std::vector<HGMesh> commonMeshes;
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

    void render(CmdBufRecorder &cmdBuf, CmdBufRecorder::ViewportType viewPortType) {
         //Render commonMeshes
        for (auto const &mesh : commonMeshes ) {
            MapSceneRenderForwardVLK::drawMesh(cmdBuf, mesh, viewPortType);
        }
    }
    void renderWater(CmdBufRecorder &cmdBuf, CmdBufRecorder::ViewportType viewPortType) {
        for (auto const &mesh : waterMeshes ) {
            MapSceneRenderForwardVLK::drawMesh(cmdBuf, mesh, viewPortType);
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

std::unique_ptr<IRenderFunction> MapSceneRenderForwardVLK::update(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams,
                                             const std::shared_ptr<MapRenderPlan> &framePlan) {
    TracyMessageStr(("Update stage frame = " + std::to_string(m_device->getCurrentProcessingFrameNumber())));

    ZoneScoped;
    auto l_this = std::dynamic_pointer_cast<MapSceneRenderForwardVLK>(this->shared_from_this());
    auto mapScene = std::dynamic_pointer_cast<Map>(frameInputParams->frameParameters->scene);


    //Create meshes
    std::unique_ptr<COpaqueMeshCollectorForwardVLK> u_collector = std::make_unique<COpaqueMeshCollectorForwardVLK>();
    std::unique_ptr<COpaqueMeshCollectorForwardVLK> u_skyCollector = std::make_unique<COpaqueMeshCollectorForwardVLK>();
    auto transparentMeshes = std::make_shared<framebased::vector<HGSortableMesh>>();



    auto skyTransparentMeshes = std::make_shared<framebased::vector<HGSortableMesh>>();
    framePlan->m2Array.lock();
    framePlan->wmoArray.lock();
    framePlan->wmoGroupArray.lock();



    mapScene->update(framePlan);
    mapScene->updateBuffers(l_this, framePlan);

    //    TracyMessageL("collect meshes created");
//    std::future<void> collectMeshAsync = std::async(std::launch::async,
//                                                    [&]() {
    collectMeshes(framePlan, *u_collector, *u_skyCollector, transparentMeshes, skyTransparentMeshes);
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
    auto skyMesh = framePlan->skyMesh;
    auto skyMesh0x4 = framePlan->skyMesh0x4;
    return createRenderFuncVLK([l_this, mapScene, framePlan, transparentMeshes, frameInputParams](CmdBufRecorder &uploadCmd) -> void {
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
                        //Sky opaque
                        if (renderSky && skyMesh)
                            MapSceneRenderForwardVLK::drawMesh(frameBufCmd, skyMesh,
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

                            MapSceneRenderForwardVLK::drawMesh(frameBufCmd, mesh,
                                                               CmdBufRecorder::ViewportType::vp_skyBox);
                        }
                        if (renderSky && skyMesh0x4)
                            MapSceneRenderForwardVLK::drawMesh(frameBufCmd, skyMesh0x4,
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

                            MapSceneRenderForwardVLK::drawMesh(frameBufCmd, mesh,
                                                               CmdBufRecorder::ViewportType::vp_usual);
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
HGSortableMesh MapSceneRenderForwardVLK::createWMOMesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IWMOMaterial> &material, const std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> &ambientBuffer) {
    auto mesh = meshFactory->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0, 0);
    return mesh;
}

std::shared_ptr<IRenderView> MapSceneRenderForwardVLK::createRenderView(bool createOutput) {
    return std::make_shared<RenderViewForwardVLK>(m_device, uboBuffer, m_drawQuadVao, createOutput);
}


