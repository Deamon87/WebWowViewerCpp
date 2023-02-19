//
// Created by Deamon on 12.12.22.
//

#include "FrontendUIRenderForwardVLK.h"
#include "../../../../../wowViewerLib/src/gapi/UniformBufferStructures.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/materials/ISimpleMaterialVLK.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/buffers/IBufferChunkVLK.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/meshes/GMeshVLK.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/materials/MaterialBuilderVLK.h"

FrontendUIRenderForwardVLK::FrontendUIRenderForwardVLK(const HGDeviceVLK &hDevice) : FrontendUIRenderer(
    hDevice), m_device(hDevice) {

    m_lastRenderPass = m_device->getSwapChainRenderPass();

    createBuffers();
}

void FrontendUIRenderForwardVLK::createBuffers() {
    vboBuffer = m_device->createIndexBuffer(1024*1024);
    iboBuffer = m_device->createVertexBuffer(1024*1024);
    uboBuffer = m_device->createUniformBuffer(sizeof(ImgUI::modelWideBlockVS)*IDevice::MAX_FRAMES_IN_FLIGHT);

    m_imguiUbo = std::make_shared<CBufferChunkVLK<ImgUI::modelWideBlockVS>>(uboBuffer);

    m_imguiVAO = m_device->createVertexBufferBindings();
    m_imguiVAO->addVertexBufferBinding(nullptr, std::vector(imguiBindings.begin(), imguiBindings.end()));
}

HGVertexBuffer FrontendUIRenderForwardVLK::createVertexBuffer(int sizeInBytes) {
    return vboBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer FrontendUIRenderForwardVLK::createIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}

HGVertexBufferBindings FrontendUIRenderForwardVLK::createVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) {
    //VAO doesn't exist in Vulkan, but it's used to hold proper reading rules
    return m_imguiVAO;
}

HMaterial FrontendUIRenderForwardVLK::createUIMaterial(const UIMaterialTemplate &materialTemplate) {
    auto i = m_materialCache.find(materialTemplate);
    if (i != m_materialCache.end()) {
        if (!i->second.expired()) {
            return i->second.lock();
        } else {
            m_materialCache.erase(i);
        }
    }

    auto &l_imguiUbo = m_imguiUbo;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"imguiShader", "imguiShader"})
        .createDescriptorSet(0, [&l_imguiUbo](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(1, l_imguiUbo->getSubBuffer());
        })
        .createDescriptorSet(1, [&materialTemplate](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(5, std::dynamic_pointer_cast<GTextureVLK>(materialTemplate.texture));
        })
        .toMaterial();

//
//    std::vector<std::shared_ptr<IBufferVLK>> ubos = {m_imguiUbo->getSubBuffer()};
//    std::vector<HGTextureVLK> texturesVLK = {std::dynamic_pointer_cast<GTextureVLK>(materialTemplate.texture)};
//    auto material = std::make_shared<ISimpleMaterialVLK>(m_device,
//                                  "imguiShader", "imguiShader",
//                                  ubos,
//                                  texturesVLK);

    std::weak_ptr<ISimpleMaterialVLK> weakPtr(material);
    m_materialCache[materialTemplate] = weakPtr;

    return material;
}

HGMesh FrontendUIRenderForwardVLK::createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) {
    //TODO:
    return std::make_shared<GMeshVLK>(*m_device, meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material));
}

std::unique_ptr<IRenderFunction> FrontendUIRenderForwardVLK::update(
    const std::shared_ptr<FrameInputParams<ImGuiFramePlan::ImGUIParam>> &frameInputParams,
    const std::shared_ptr<ImGuiFramePlan::EmptyPlan> &framePlan) {

    auto meshes = std::make_unique<std::vector<HGMesh>>();
    this->consumeFrameInput(frameInputParams, *meshes);

    //Record commands to update buffer and draw
    auto l_this = std::dynamic_pointer_cast<FrontendUIRenderForwardVLK>(this->shared_from_this());
    return createRenderFuncVLK(std::move([meshes = std::move(meshes), l_this](CmdBufRecorder &uploadCmd, CmdBufRecorder &frameBufCmd, CmdBufRecorder &swapChainCmd) {
        // ---------------------
        // Upload stuff
        // ---------------------
        uploadCmd.submitBufferUploads(l_this->uboBuffer);
        uploadCmd.submitBufferUploads(l_this->vboBuffer);
        uploadCmd.submitBufferUploads(l_this->iboBuffer);

        // ----------------------
        // Draw meshes
        // ----------------------

        for (auto const &mesh : *meshes) {
            const auto &meshVlk = std::dynamic_pointer_cast<GMeshVLK>(mesh);

            //1. Bind VBOs
            swapChainCmd.bindVertexBuffer(l_this->vboBuffer);

            //2. Bind IBOs
            swapChainCmd.bindIndexBuffer(l_this->iboBuffer);

            //3. Bind pipeline
            auto pipeline = meshVlk->getPipeLineForRenderPass(l_this->m_lastRenderPass, false);
            swapChainCmd.bindPipeline(pipeline);

            //4. Bind Descriptor sets
            auto const &descSets = meshVlk->material()->getDescriptorSets();
            for (int i = 0; i < descSets.size(); i++) {
                if (descSets[i] != nullptr) {
                    swapChainCmd.bindDescriptorSet(i, descSets[i]);
                }
            }

            //5. Set view port

//            swapChainCmd.setViewPort();

            //5. Set view port
//            swapChainCmd.setScissors(defaultScissor);

            //    //Set scissors
//    VkRect2D defaultScissor = {};
//    defaultScissor.offset = {0, 0};
//    defaultScissor.extent = {
//        static_cast<uint32_t>(drawStage->viewPortDimensions.maxs[0]),
//        static_cast<uint32_t>(drawStage->viewPortDimensions.maxs[1])
//    };
//
//    vkCmdSetScissor(commandBufferForFilling, 0, 1, &defaultScissor);

            //6. Draw the mesh
            swapChainCmd.drawIndexed(meshVlk->end(), 1, meshVlk->start()/2, 0);
        }
    }));
}
