//
// Liquid Hi-Z Min-Max Pipeline
// Generates a min-max depth pyramid from the liquid depth buffer.
// Used for classifying transparent objects as above/below/intersecting liquid planes.
//

#ifndef AWEBWOWVIEWERCPP_LIQUIDHIZVLK_H
#define AWEBWOWVIEWERCPP_LIQUIDHIZVLK_H

#include "../../../../gapi/vulkan/GDeviceVulkan.h"
#include "../../../../gapi/vulkan/materials/ComputeMaterialBuilderVLK.h"
#include "../../../../gapi/vulkan/textures/GTextureVLK.h"
#include "../../../../gapi/vulkan/textures/GTextureSamplerVLK.h"
#include "../../../../gapi/vulkan/GFrameBufferVLK.h"
#include "../../../../gapi/vulkan/commandBuffer/commandBufferRecorder/CommandBufferRecorder.h"

class LiquidHiZVLK {
public:
    explicit LiquidHiZVLK(const HGDeviceVLK &device);
    ~LiquidHiZVLK();

    // Recreate liquid Hi-Z texture and materials when dimensions change
    void setup(int width, int height);

    // Generate min-max Hi-Z mipchain from liquid depth buffer.
    // Expects liquid depth in DEPTH_STENCIL_ATTACHMENT_OPTIMAL (post-liquid depth render pass).
    // Leaves liquid depth in DEPTH_STENCIL_READ_ONLY_OPTIMAL (ready for shader sampling).
    void execute(CmdBufRecorder &cmdBuf,
                 const std::shared_ptr<GFrameBufferVLK> &liquidDepthFB);

    bool isReady() const { return m_hiZTexture != nullptr && m_hiZMipCount > 0; }

    HGSamplableTexture getHiZTexture() const { return m_hiZTextureSamplable; }
    uint32_t getMipCount() const { return m_hiZMipCount; }
    uint32_t getWidth() const { return m_hiZTexture ? m_hiZTexture->getWidth() : 0; }
    uint32_t getHeight() const { return m_hiZTexture ? m_hiZTexture->getHeight() : 0; }

private:
    void cleanupMipViews();

    HGDeviceVLK m_device;

    std::shared_ptr<GTextureVLK> m_hiZTexture;          // RG32_SFLOAT: R=min, G=max
    HGSamplableTexture m_hiZTextureSamplable;
    std::vector<VkImageView> m_hiZMipViews;
    uint32_t m_hiZMipCount = 0;

    std::shared_ptr<ComputeMaterialVLK> m_depthCopyMaterial;
    std::vector<std::shared_ptr<ComputeMaterialVLK>> m_hiZGenerateMaterials;
};

#endif //AWEBWOWVIEWERCPP_LIQUIDHIZVLK_H
