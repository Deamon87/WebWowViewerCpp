//
// Created by Deamon on 12.12.22.
//

#include "FrontendUIRenderForwardVLK.h"
#include "../../../../../wowViewerLib/src/gapi/UniformBufferStructures.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/materials/ISimpleMaterialVLK.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/buffers/CBufferChunkVLK.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/meshes/GMeshVLK.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder_inline.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/materials/MaterialBuilderVLK.h"


FrontendUIRenderForwardVLK::FrontendUIRenderForwardVLK(const HGDeviceVLK &hDevice) :
    FrontendUIRenderer(hDevice), m_device(hDevice) {
    std::cout << "Create Bindless scene renderer " << std::endl;
    m_lastRenderPass = m_device->getSwapChainRenderPass();
    m_emptyImguiVAO = createVAO(nullptr,nullptr);

    createBuffers();
}

void FrontendUIRenderForwardVLK::createBuffers() {
    allBuffers = {iboBuffer, vboBuffer, uboBuffer};

    iboBuffer = m_device->createIndexBuffer("UI_Ibo_Buffer", 1024*1024);
    vboBuffer = m_device->createVertexBuffer("UI_Vbo_Buffer", 1024*1024);
    uboBuffer = m_device->createUniformBuffer("UI_UBO", sizeof(ImgUI::modelWideBlockVS)*IDevice::MAX_FRAMES_IN_FLIGHT);

    m_imguiUboVLK = std::make_shared<CBufferChunkVLK<ImgUI::modelWideBlockVS>>(uboBuffer);
    m_imguiUbo = m_imguiUboVLK;
}

HGVertexBuffer FrontendUIRenderForwardVLK::createVertexBuffer(int sizeInBytes) {
    return vboBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer FrontendUIRenderForwardVLK::createIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}

HGVertexBufferBindings FrontendUIRenderForwardVLK::createVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules as well as buffers
    auto imguiVAO = m_device->createVertexBufferBindings();
    imguiVAO->addVertexBufferBinding(vertexBuffer, std::vector(imguiBindings.begin(), imguiBindings.end()));
    imguiVAO->setIndexBuffer(indexBuffer);

    return imguiVAO;
}

std::shared_ptr<IUIMaterial> FrontendUIRenderForwardVLK::createUIMaterial(const HGSamplableTexture &hgtexture, bool opaque) {
    auto weakTexture = std::weak_ptr(hgtexture);
    auto &materialCache = !opaque ? m_materialCache : m_materialCacheOpaque;
    auto i = materialCache.find(weakTexture);
    if (i != materialCache.end()) {
        if (!i->second.expired()) {
            return i->second.lock();
        } else {
            materialCache.erase(i);
        }
    }

    auto &l_imguiUbo = m_imguiUboVLK;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"imguiShader", !opaque?"imguiShader":"imguiShader_opaque"}, {"forwardRendering","forwardRendering"}, {})
        .createPipeline(m_emptyImguiVAO, m_lastRenderPass, s_imguiPipelineTemplate)
        .createDescriptorSet(0, [&l_imguiUbo](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, *l_imguiUbo);
        })
        .createDescriptorSet(1, [&hgtexture](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(0, hgtexture);
        })
        .toMaterial<IUIMaterial>([&] (IUIMaterial *uiMat){
            uiMat->uniqueId = this->generateUniqueMatId();
        });

    auto weakPtr = material;
    materialCache[weakTexture] = weakPtr;
    m_materialCacheIdMap[material->uniqueId] = std::dynamic_pointer_cast<IMaterial>(material);

    return material;
}

std::shared_ptr<IUIMaterial> FrontendUIRenderForwardVLK::createUIMaterialDepth(const HGSamplableTexture &hgtexture) {
    auto weakTexture = std::weak_ptr(hgtexture);
    auto &materialCache = m_materialDepthCache ;
    auto i = materialCache.find(weakTexture);
    if (i != materialCache.end()) {
        if (!i->second.expired()) {
            return i->second.lock();
        } else {
            materialCache.erase(i);
        }
    }

    auto &l_imguiUbo = m_imguiUboVLK;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"imguiShader", "imguiShaderDepth"}, {"forwardRendering", "forwardRendering"}, {})
        .createPipeline(m_emptyImguiVAO, m_lastRenderPass, s_imguiPipelineTemplate)
        .createDescriptorSet(0, [&l_imguiUbo](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(0, *l_imguiUbo);
        })
        .createDescriptorSet(1, [&hgtexture](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(0, hgtexture);
        })
        .toMaterial<IUIMaterial>([&] (IUIMaterial *uiMat){
            uiMat->uniqueId = this->generateUniqueMatId();
        });

    auto weakPtr = material;
    materialCache[weakTexture] = weakPtr;
    m_materialCacheIdMap[material->uniqueId] = std::dynamic_pointer_cast<IMaterial>(material);

    return material;
}

HGMesh FrontendUIRenderForwardVLK::createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) {
    return meshFactory->createObject(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material), 0,0);
}

std::unique_ptr<IRenderFunction> FrontendUIRenderForwardVLK::update(
    const std::shared_ptr<FrameInputParams<ImGuiFramePlan::ImGUIParam>> &frameInputParams,
    const std::shared_ptr<ImGuiFramePlan::EmptyPlan> &framePlan) {

    TracyMessageStr(("Update UI buffers = " + std::to_string(m_device->getCurrentProcessingFrameNumber())));
    ZoneScopedN("Update UI buffers");

    auto meshes = std::make_shared<std::vector<HGMesh>>();
    this->consumeFrameInput(frameInputParams, *meshes);

    {
        //Prevents material from being created over and over again
        m_previousMeshes = meshes;
    }

    //Record commands to update buffer and draw
    auto l_this = std::dynamic_pointer_cast<FrontendUIRenderForwardVLK>(this->shared_from_this());
    return createRenderFuncVLK(
        std::move([meshes, l_this](CmdBufRecorder &uploadCmd) {
            TracyMessageStr(("Upload stuff stage frame = " + std::to_string(l_this->m_device->getCurrentProcessingFrameNumber())));
            ZoneScopedN("Upload UI buffs");
            // ---------------------
            // Upload stuff
            // ---------------------
            for (const auto& buffer : l_this->allBuffers) {
                uploadCmd.submitFullMemoryBarrierPreWrite(buffer);
            }

            for (const auto& buffer : l_this->allBuffers) {
                uploadCmd.submitBufferUploads(buffer);
            }

            for (const auto& buffer : l_this->allBuffers) {
                uploadCmd.submitFullMemoryBarrierPostWrite(buffer);
            }
        }),
        std::move([meshes, l_this](CmdBufRecorder &frameBufCmd, CmdBufRecorder &swapChainCmd) {
                TracyMessageStr(("Render stage frame = " + std::to_string(l_this->m_device->getCurrentProcessingFrameNumber())));
                ZoneScopedN("Render UI");
                // ----------------------
                // Draw meshes
                // ----------------------

                for (auto const &mesh : *meshes) {
                    const auto &meshVlk = std::dynamic_pointer_cast<GMeshVLK>(mesh);
                    auto vulkanBindings = std::dynamic_pointer_cast<GVertexBufferBindingsVLK>(mesh->bindings());

                    //1. Bind VBOs
                    swapChainCmd.bindVertexBuffers(vulkanBindings->getVertexBuffers());

                    //2. Bind IBOs
                    swapChainCmd.bindIndexBuffer(vulkanBindings->getIndexBuffer());

                    //3. Bind pipeline
                    auto material = meshVlk->material();
                    swapChainCmd.bindPipeline(material->getPipeline());

                    //4. Bind Descriptor sets
                    auto const &descSets = material->getDescriptorSets();
                    swapChainCmd.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, descSets);


                    //5. Set view port
                    swapChainCmd.setViewPort(CmdBufRecorder::ViewportType::vp_usual);

                    //6. Set scissors
                    if (meshVlk->scissorEnabled()) {
                        swapChainCmd.setScissors(meshVlk->scissorOffset(), meshVlk->scissorSize());
                    } else {
                        swapChainCmd.setDefaultScissors();
                    }

                    //7. Draw the mesh
                    swapChainCmd.drawIndexed(meshVlk->end(), 1, meshVlk->start()/2, 0);
                }
            })
    );
}

uint32_t FrontendUIRenderForwardVLK::generateUniqueMatId() {
    uint32_t random;
    do {
        random = idDistr(eng);
    } while (!m_materialCacheIdMap[random].expired());

    return random;
}
