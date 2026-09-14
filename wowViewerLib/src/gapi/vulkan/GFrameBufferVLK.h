//
// Created by Deamon on 12/11/2020.
//

#ifndef AWEBWOWVIEWERCPP_GFRAMEBUFFERVLK_H
#define AWEBWOWVIEWERCPP_GFRAMEBUFFERVLK_H

#include <vector>
#include "../interface/IFrameBuffer.h"
#include "../interface/textures/ITexture.h"
#include "GDeviceVulkan.h"

class GFrameBufferVLK : public IFrameBuffer {
public:
    GFrameBufferVLK(IDevice &device,
                    const HGTexture &colorImage,
                    const HGTexture &depthBuffer,
                    int width, int height,
                    const std::shared_ptr<GRenderPassVLK> &renderPass);

    // Constructor that takes render pass and creates textures based on its format info
    GFrameBufferVLK(IDevice &device,
                    const std::shared_ptr<GRenderPassVLK> &renderPass,
                    const HGTexture &depthBuffer,
                    int width, int height);

    ~GFrameBufferVLK() override;

    void readRGBAPixels(int x, int y, int width, int height, void *data) override;
    HGSamplableTexture getAttachment(int index) override;
    HGTexture getDepthTexture() override;
    void bindFrameBuffer() override;
    void copyRenderBufferToTexture() override;

    int getWidth() {return m_width;}
    int getHeight() {return m_height;}

    static void iterateOverAttachments(const std::vector<ITextureFormat> &textureAttachments, std::function<void(int i, VkFormat textureFormat)> callback);

    VkFramebuffer getFrameBuffer() {return m_frameBuffer;};
    int getColorOrDataAttachmentCount() { return m_attachmentFormats.size(); };


private:
    GDeviceVLK &mdevice;

    std::shared_ptr<ITextureSampler> m_sampler = nullptr;

    std::shared_ptr<GRenderPassVLK> m_renderPass;
    VkFramebuffer m_frameBuffer;

    std::vector<HGTexture> m_attachmentTextures;

    // Separate storage for cleaner getAttachment logic
    std::vector<HGSamplableTexture> m_colorAttachmentsSampled;
    std::vector<HGSamplableTexture> m_resolveAttachmentsSampled;

    HGTexture m_depthTexture = nullptr;

    //Used only in readRGBAPixels function
    std::vector<VkFormat> m_attachmentFormats;

    //Stored for copying
    std::vector<ITextureFormat> m_textureAttachments;
    ITextureFormat m_depthAttachment = ITextureFormat::itNone;

    int m_multiSampleCnt = 0;

    int m_width = 0;
    int m_height = 0;

    inline void initSampler(GDeviceVLK &device);

    void copyToImage(int width, int height, VkImage srcImage, VkFormat inputColorFormat, VkImage dstImage,
                     const VkCommandBuffer &copyCmd) const;

};



#endif //AWEBWOWVIEWERCPP_GFRAMEBUFFERVLK_H
