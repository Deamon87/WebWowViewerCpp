//
// Created by Deamon on 3/17/2023.
//

#include "FFXGlowPassVLK.h"
#include "../../../../gapi/vulkan/buffers/IBufferChunkVLK.h"
#include "../../../../gapi/vulkan/materials/MaterialBuilderVLK.h"

FFXGlowPassVLK::FFXGlowPassVLK(HGBufferVLK uboBuffer) {
    m_ffxGlowVs = std::make_shared<CBufferChunkVLK<mathfu::vec4_packed>>(uboBuffer);
    for (auto &ffxGlowPs : m_ffxGlowPSs) {
        ffxGlowPs = std::make_shared<CBufferChunkVLK<FXGauss::meshWideBlockPS>>(uboBuffer);
    }
}

void FFXGlowPassVLK::assignFFXGlowUBOConsts() {
    auto &ffxGlowVs = m_ffxGlowVs->getObject();
    ffxGlowVs = {1,1,0,0};
    m_ffxGlowVs->save();

    static const std::array<std::array<float, 4>, 6> texOffsets = {{
       //X & Y
       {{-1, 0, 0, -1}},
       {{2, 2, -1, -1}},

       //X & Y
       {{-6, -1, 1, 6}},
       {{0, 0, 0, 0}},

       //X & Y
       {{0, 0, 0, 0}},
       {{10, 2, -2, -10}},
    }};

    for (int i = 0; i < 3; i++) {
        auto ffxGlowPs1 = m_ffxGlowPSs[i]->getObject();
        std::copy(std::begin(texOffsets[i*2]),   std::end(texOffsets[i*2]),   std::begin(ffxGlowPs1.texOffsetX));
        std::copy(std::begin(texOffsets[i*2+1]), std::end(texOffsets[i*2+1]), std::begin(ffxGlowPs1.texOffsetY));
        m_ffxGlowPSs[i]->save();
    }
}

void FFXGlowPassVLK::createFrameBuffers(const HGDeviceVLK &device, int m_width, int m_height) {
    {
        auto const dataFormat = {ITextureFormat::itRGBA};
        int targetWidth = m_width >> 2;
        int targetHeight = m_height >> 2;

        for (auto &colorFrameBuffer: m_GlowFrameB1) {
            colorFrameBuffer = std::make_shared<GFrameBufferVLK>(
                *device,
                dataFormat,
                ITextureFormat::itDepth32,
                1,
                targetWidth, targetHeight
            );
        }
        for (auto &colorFrameBuffer: m_GlowFrameB2) {
            colorFrameBuffer = std::make_shared<GFrameBufferVLK>(
                *device,
                dataFormat,
                ITextureFormat::itDepth32,
                1,
                targetWidth, targetHeight
            );
        }
    }
}

std::shared_ptr<IMaterial>
FFXGlowPassVLK::createFFxGlowMat(
    const std::shared_ptr<IBufferChunk<mathfu::vec4_packed>> &ffxGlowVs,
    const std::shared_ptr<IBufferChunk<FXGauss::meshWideBlockPS>> &ffxGlowPS,
    const PipelineTemplate &pipelineTemplate,
    const ) {

    auto material = MaterialBuilderVLK::fromShader(m_device, {"drawQuad", "ffxgauss4"})
        .createPipeline(m_emptySkyVAO, m_renderPass, pipelineTemplate)
        .createDescriptorSet(0, [&ffxGlowVs, &ffxGlowPS](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(2, BufferChunkHelper::cast(ffxGlowVs)->getSubBuffer())
                .ubo(4, BufferChunkHelper::cast(ffxGlowPS)->getSubBuffer());
        })
        .createDescriptorSet(1, [&ffxGlowVs, &ffxGlowPS](std::shared_ptr<GDescriptorSet> &ds) {
            ds->beginUpdate()
                .ubo(2, BufferChunkHelper::cast(ffxGlowVs)->getSubBuffer())
                .ubo(4, BufferChunkHelper::cast(ffxGlowPS)->getSubBuffer());
        })
        .toMaterial();

    return material;
}
