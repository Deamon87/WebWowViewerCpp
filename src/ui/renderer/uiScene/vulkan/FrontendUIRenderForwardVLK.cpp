//
// Created by Deamon on 12.12.22.
//

#include "FrontendUIRenderForwardVLK.h"
#include "../../../../../wowViewerLib/src/gapi/UniformBufferStructures.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/materials/ISimpleMaterialVLK.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/buffers/CBufferChunkVLK.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/meshes/GMeshVLK.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/materials/MaterialBuilderVLK.h"

FrontendUIRenderForwardVLK::FrontendUIRenderForwardVLK(const HGDeviceVLK &hDevice) : FrontendUIRenderer(
    hDevice), m_device(hDevice) {

    m_lastRenderPass = m_device->getSwapChainRenderPass();
    m_emptyImguiVAO = createVAO(nullptr,nullptr);

    createBuffers();
}

void FrontendUIRenderForwardVLK::createBuffers() {
    iboBuffer = m_device->createIndexBuffer(1024*1024);
    vboBuffer = m_device->createVertexBuffer(1024*1024);
    uboBuffer = m_device->createUniformBuffer(sizeof(ImgUI::modelWideBlockVS)*IDevice::MAX_FRAMES_IN_FLIGHT);

    m_imguiUbo = std::make_shared<CBufferChunkVLK<ImgUI::modelWideBlockVS>>(uboBuffer);
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

HMaterial FrontendUIRenderForwardVLK::createUIMaterial(const HGSamplableTexture &hgtexture) {
    auto weakTexture = std::weak_ptr(hgtexture);
    auto i = m_materialCache.find(weakTexture);
    if (i != m_materialCache.end()) {
        if (!i->second.expired()) {
            return i->second.lock();
        } else {
            m_materialCache.erase(i);
        }
    }

    auto &l_imguiUbo = m_imguiUbo;
    auto material = MaterialBuilderVLK::fromShader(m_device, {"imguiShader", "imguiShader"}, {})
        .createPipeline(m_emptyImguiVAO, m_lastRenderPass, s_imguiPipelineTemplate)
        .createDescriptorSet(0, [&l_imguiUbo](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(1, l_imguiUbo->getSubBuffer());
        })
        .createDescriptorSet(1, [&hgtexture](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .texture(5, hgtexture);
        })
        .toMaterial();

    std::weak_ptr<ISimpleMaterialVLK> weakPtr = material;
    m_materialCache[weakTexture] = weakPtr;

    return material;
}

HGMesh FrontendUIRenderForwardVLK::createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) {
    return std::make_shared<GMeshVLK>(meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material));
}

std::unique_ptr<IRenderFunction> FrontendUIRenderForwardVLK::update(
    const std::shared_ptr<FrameInputParams<ImGuiFramePlan::ImGUIParam>> &frameInputParams,
    const std::shared_ptr<ImGuiFramePlan::EmptyPlan> &framePlan) {

    auto meshes = std::make_shared<std::vector<HGMesh>>();
    this->consumeFrameInput(frameInputParams, *meshes);

    {
        //Prevents material from being created over and over again
        m_previousMeshes = meshes;
    }

    //Record commands to update buffer and draw
    auto l_this = std::dynamic_pointer_cast<FrontendUIRenderForwardVLK>(this->shared_from_this());
    return createRenderFuncVLK(std::move([meshes, l_this](CmdBufRecorder &uploadCmd, CmdBufRecorder &frameBufCmd, CmdBufRecorder &swapChainCmd) {
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
            for (int i = 0; i < descSets.size(); i++) {
                if (descSets[i] != nullptr) {
                    swapChainCmd.bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, i, descSets[i]);
                }
            }

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
    }));
}
