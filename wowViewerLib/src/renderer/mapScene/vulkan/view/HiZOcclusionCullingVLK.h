//
// Hi-Z Occlusion Culling Pipeline
// Reusable across deferred and bindless renderers.
//

#ifndef AWEBWOWVIEWERCPP_HIZOCCLUSIONCULLINGVLK_H
#define AWEBWOWVIEWERCPP_HIZOCCLUSIONCULLINGVLK_H

#include "../../../../gapi/vulkan/GDeviceVulkan.h"
#include "../../../../gapi/vulkan/materials/ComputeMaterialBuilderVLK.h"
#include "../../../../gapi/vulkan/textures/GTextureVLK.h"
#include "../../../../gapi/vulkan/textures/GTextureSamplerVLK.h"
#include "../../../../gapi/vulkan/GFrameBufferVLK.h"
#include "../../../../gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder.h"

class HiZOcclusionCullingVLK {
public:
    explicit HiZOcclusionCullingVLK(const HGDeviceVLK &device);
    ~HiZOcclusionCullingVLK();

    // Recreate Hi-Z texture and materials when dimensions change
    void setup(int width, int height);

    // Generate Hi-Z mipchain from GBuffer depth, then dispatch occlusion culling.
    // Writes occlusion results to visibilityBuffer.
    // Expects depth in DEPTH_STENCIL_ATTACHMENT_OPTIMAL (post-GBuffer render pass).
    // Leaves depth in DEPTH_STENCIL_ATTACHMENT_OPTIMAL (ready for forward pass).
    void execute(CmdBufRecorder &cmdBuf,
                 const std::shared_ptr<GFrameBufferVLK> &gBufferFB,
                 const mathfu::mat4 &viewProj,
                 const HGBufferVLK &aabbBuffer,
                 const HGBufferVLK &visibilityBuffer,
                 uint32_t objectCount,
                 bool invertZ);

    bool isReady() const { return m_hiZTexture != nullptr && m_hiZMipCount > 0; }

private:
    void cleanupMipViews();

    HGDeviceVLK m_device;

    std::shared_ptr<GTextureVLK> m_hiZTexture;
    HGSamplableTexture m_hiZTextureSamplable;
    std::vector<VkImageView> m_hiZMipViews;
    uint32_t m_hiZMipCount = 0;

    std::shared_ptr<ComputeMaterialVLK> m_depthCopyMaterial;
    std::vector<std::shared_ptr<ComputeMaterialVLK>> m_hiZGenerateMaterials;
    std::shared_ptr<ComputeMaterialVLK> m_occlusionCullMaterial;
};

#endif //AWEBWOWVIEWERCPP_HIZOCCLUSIONCULLINGVLK_H
