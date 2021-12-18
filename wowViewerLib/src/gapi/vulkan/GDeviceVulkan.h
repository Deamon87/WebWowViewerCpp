//
// Created by Deamon on 8/1/2019.
//

#ifndef AWEBWOWVIEWERCPP_GDEVICEVULKAN_H
#define AWEBWOWVIEWERCPP_GDEVICEVULKAN_H

#include <memory>
#include <unordered_map>
#include <mutex>


class GVertexBufferVLK;
class GVertexBufferBindingsVLK;
class GIndexBufferVLK;
class GUniformBufferVLK;
class GBlpTextureVLK;
class GTextureVLK;
class GShaderPermutationVLK;
class GMeshVLK;
class GM2MeshVLK;
class GOcclusionQueryVLK;
class GParticleMeshVLK;
class GPipelineVLK;
class GRenderPassVLK;
class GDescriptorPoolVLK;

typedef std::shared_ptr<GPipelineVLK> HPipelineVLK;

class gMeshTemplate;

#include <unordered_set>
#include <list>
#include "vk_mem_alloc.h"

#include "../interface/IDevice.h"
#include "descriptorSets/GDescriptorSet.h"
#include "descriptorSets/GDescriptorPoolVLK.h"
#include "../../engine/algorithms/FrameCounter.h"
#include <optional>

VkSampleCountFlagBits sampleCountToVkSampleCountFlagBits(uint8_t sampleCount);

class GDeviceVLK : public IDevice {
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> transferFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

public:
    enum class ViewportType {vp_none = -1, vp_usual = 0, vp_mapArea = 1, vp_skyBox = 2, vp_MAX = 3};

    explicit GDeviceVLK(vkCallInitCallback * callBacks);
    ~GDeviceVLK() override = default;;

    void reset() override;

    unsigned int getFrameNumber() override { return m_frameNumber; };
    unsigned int getUpdateFrameNumber() override;
    unsigned int getCullingFrameNumber() override;
    unsigned int getDrawFrameNumber() override;

    bool getIsRenderbufferSupported() override {return true;}

    void increaseFrameNumber() override;
    bool getIsAsynBuffUploadSupported() override {
        return true;
    }
    int getMaxSamplesCnt() override;
    VkSampleCountFlagBits getMaxSamplesBit();

    bool canUploadInSeparateThread() {
        return uploadQueue != graphicsQueue;
    }

    HGTexture getBlackPixelTexture() {
        return m_blackPixelTexture;
    }

    float getAnisLevel() override;

    void bindProgram(IShaderPermutation *program) override;

    void bindIndexBuffer(IIndexBuffer *buffer) override;
    void bindVertexBuffer(IVertexBuffer *buffer) override;
    void bindUniformBuffer(IUniformBuffer *buffer, int slot, int offset, int length) override {};
    void bindVertexBufferBindings(IVertexBufferBindings *buffer) override;

    void bindTexture(ITexture *texture, int slot) override;

    void startUpdateForNextFrame() override;
    void endUpdateForNextFrame() override;

    void updateBuffers(std::vector<std::vector<HGUniformBufferChunk>*> &bufferChunks, std::vector<HFrameDepedantData> &frameDepedantData) override;
    void uploadTextureForMeshes(std::vector<HGMesh> &meshes) override;
    void drawMeshes(std::vector<HGMesh> &meshes) override;
    void drawStageAndDeps(HDrawStage drawStage) override;
    bool wasTexturesUploaded() override {
        return m_texturesWereUploaded;
    };

    //    void drawM2Meshes(std::vector<HGM2Mesh> &meshes);
    bool getIsVulkanAxisSystem() override {return true;}

    void initUploadThread() override;
public:
    double getWaitForUpdate() override {
        return this->waitInDrawStageAndDeps.getTimePerFrame();
    }

    std::shared_ptr<IShaderPermutation> getShader(std::string shaderName, void *permutationDescriptor) override;

    HGUniformBuffer createUniformBuffer(size_t size) override;
    HGVertexBuffer createVertexBuffer() override;
    HGVertexBufferDynamic createVertexBufferDynamic(size_t size) override;
    HGIndexBuffer createIndexBuffer() override;
    HGVertexBufferBindings createVertexBufferBindings() override;

    HGTexture createBlpTexture(HBlpTexture &texture, bool xWrapTex, bool yWrapTex) override;
    HGTexture createTexture(bool xWrapTex, bool yWrapTex) override;
    HGTexture getWhiteTexturePixel() override { return m_whitePixelTexture; };
    HGMesh createMesh(gMeshTemplate &meshTemplate) override;
    HGM2Mesh createM2Mesh(gMeshTemplate &meshTemplate) override;
    HGParticleMesh createParticleMesh(gMeshTemplate &meshTemplate) override;
    HGPUFence createFence() override;

    HFrameBuffer createFrameBuffer(int width, int height, std::vector<ITextureFormat> attachments, ITextureFormat depthAttachment, int multiSampleCnt, int frameNumber) override ;

    HPipelineVLK createPipeline(HGVertexBufferBindings m_bindings,
                                HGShaderPermutation shader,
                                std::shared_ptr<GRenderPassVLK> renderPass,
                                DrawElementMode element,
                                int8_t backFaceCulling,
                                int8_t triCCW,
                                EGxBlendEnum blendMode,
                                int8_t depthCulling,
                                int8_t depthWrite,
                                bool invertZ);

    std::shared_ptr<GRenderPassVLK> getRenderPass(std::vector<ITextureFormat> textureAttachments,
                                                  ITextureFormat depthAttachment,
                                                  VkSampleCountFlagBits sampleCountFlagBits,
                                                  bool isSwapChainPass);

    HGOcclusionQuery createQuery(HGMesh boundingBoxMesh) override;

    HGVertexBufferBindings getBBVertexBinding() override;
    HGVertexBufferBindings getBBLinearBinding() override;

    std::string loadShader(std::string fileName, IShaderType shaderType) override;

    virtual void clearScreen() override;
    virtual void beginFrame() override;
    virtual void commitFrame() override;

    virtual void setClearScreenColor(float r, float g, float b) override;
    virtual void setViewPortDimensions(float x, float y, float width, float height) override;
    void setInvertZ(bool value) override {m_isInvertZ = true;};
    bool getInvertZ()  { return m_isInvertZ;};


    std::shared_ptr<GDescriptorSets> createDescriptorSet(VkDescriptorSetLayout layout, int uniforms, int images);

    virtual VkDevice getVkDevice() {
        return device;
    };
    virtual VkPhysicalDevice getVkPhysicalDevice() {
        return physicalDevice;
    };
    virtual VkExtent2D getCurrentExtent() {
        return swapChainExtent;
    };

    //    int currentFrameSemaphore = 0;
    bool framebufferResized = false;

    VmaAllocator getVMAAllocator() {
        return vmaAllocator;
    }

    VmaPool getUBOPool() {
        return uboVmaPool;
    }

    VkCommandBuffer getUploadCommandBuffer() {
        int uploadFrame = getUpdateFrameNumber();
        return uploadCommandBuffers[uploadFrame];
    }

    HGUniformBuffer getUploadBuffer(int index) {
        return m_UBOFrames[index].m_uniformBufferForUpload;
    }

    VkCommandBuffer getTextureTransferCommandBuffer() {
        int uploadFrame = getUpdateFrameNumber();
        return textureTransferCommandBuffers[uploadFrame];
    }

    void signalTextureTransferCommandRecorded() {
        int uploadFrame = getUpdateFrameNumber();
        textureTransferCommandBufferNull[uploadFrame] = false;
    }

    QueueFamilyIndices getQueueFamilyIndices() {
        return indices;
    }

    struct DeallocationRecord {
        unsigned int frameNumberToDoAt;
        std::function<void()> callback;
    };

    std::mutex m_listOfDeallocatorsAccessMtx;

    void addDeallocationRecord(std::function<void()> callback) override {
        std::lock_guard<std::mutex> lock(m_listOfDeallocatorsAccessMtx);
        DeallocationRecord dr;
        dr.frameNumberToDoAt = m_frameNumber+4;
        dr.callback = callback;
        listOfDeallocators.push_back(dr);
    };

    VkFormat findDepthFormat();

    void singleExecuteAndWait(std::function<void(VkCommandBuffer commandBuffer)> callback);
private:
    void drawMesh(HGMesh &hmesh);
    void internalDrawStageAndDeps(HDrawStage drawStage);

    void setupDebugMessenger();
    void pickPhysicalDevice();
    void createLogicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    void createSwapChain();
    void createImageViews();
    void createFramebuffers();
    void createRenderPass();

    void recreateSwapChain();

    void createCommandPool();
    void createCommandPoolForUpload();
    void createCommandBuffers();
    void createCommandBuffersForUpload();
    void createSyncObjects();

    void createColorResources();
    void createDepthResources();



    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkImageLayout vkLaylout);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

    bool drawMeshesInternal(
        const HDrawStage &drawStage,
        VkCommandBuffer commandBufferForFilling,
        std::shared_ptr<GRenderPassVLK> renderPass,
        const HMeshesToRender &iMeshes,
        const std::array<VkViewport, (int) ViewportType::vp_MAX> &viewportsForThisStage,
        VkRect2D &defaultScissor);

protected:
    struct BlpCacheRecord {
        BlpTexture* texture;
        bool wrapX;
        bool wrapY;

        bool operator==(const BlpCacheRecord &other) const {
            return
                (texture == other.texture) &&
                (wrapX == other.wrapX) &&
                (wrapY == other.wrapY);

        };
    };
    struct BlpCacheRecordHasher {
        std::size_t operator()(const BlpCacheRecord& k) const {
            using std::hash;
            return hash<void*>{}(k.texture) ^ (hash<bool>{}(k.wrapX) << 8) ^ (hash<bool>{}(k.wrapY) << 16);
        };
    };
    std::unordered_map<BlpCacheRecord, std::weak_ptr<GTextureVLK>, BlpCacheRecordHasher> loadedTextureCache;

    struct PipelineCacheRecord {
        HGShaderPermutation shader;
        std::shared_ptr<GRenderPassVLK> renderPass;
        DrawElementMode element;
        int8_t backFaceCulling;
        int8_t triCCW;
        EGxBlendEnum blendMode;
        int8_t depthCulling;
        int8_t depthWrite;
        bool invertZ;


        bool operator==(const PipelineCacheRecord &other) const {
            return
                (shader == other.shader) &&
                (renderPass == other.renderPass) &&
                (element == other.element) &&
                (backFaceCulling == other.backFaceCulling) &&
                (triCCW == other.triCCW) &&
                (blendMode == other.blendMode) &&
                (depthCulling == other.depthCulling) &&
                (depthWrite == other.depthWrite) &&
                (invertZ == other.invertZ);

        };
    };
    struct PipelineCacheRecordHasher {
        std::size_t operator()(const PipelineCacheRecord& k) const {
            using std::hash;
            return hash<void*>{}(k.shader.get()) ^
            hash<void*>{}(k.renderPass.get()) ^
            (hash<int8_t >{}(k.backFaceCulling) << 2) ^
            (hash<int8_t >{}(k.triCCW) << 4) ^
            (hash<int8_t >{}(k.depthCulling) << 8) ^
            (hash<int8_t >{}(k.depthWrite) << 10) ^
            (hash<EGxBlendEnum>{}(k.blendMode) << 16) ^
            (hash<DrawElementMode>{}(k.element) << 24);
        };
    };
    std::unordered_map<PipelineCacheRecord, std::weak_ptr<GPipelineVLK>, PipelineCacheRecordHasher> loadedPipeLines;

    VkDebugUtilsMessengerEXT debugMessenger;

    int threadCount = 1;
    FrameCounter waitInDrawStageAndDeps;

    QueueFamilyIndices indices;
    VkInstance vkInstance;
    VkSurfaceKHR vkSurface;
    bool enableValidationLayers = true ;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue uploadQueue;

    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    std::shared_ptr<GRenderPassVLK> swapchainRenderPass;


    VkCommandPool commandPool;
    VkCommandPool commandPoolForImageTransfer;
    VkCommandPool renderCommandPool;
    VkCommandPool uploadCommandPool;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkCommandBuffer> renderCommandBuffers;
    std::vector<bool> renderCommandBuffersNotNull;
    std::vector<VkCommandBuffer> renderCommandBuffersForFrameBuffers;
    std::vector<bool> renderCommandBuffersForFrameBuffersNotNull;
    std::vector<VkCommandBuffer> uploadCommandBuffers;
    std::vector<VkCommandBuffer> textureTransferCommandBuffers;
    std::vector<bool> textureTransferCommandBufferNull;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkSemaphore> textureTransferFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> inFlightTextureTransferFences;
    std::vector<bool> uploadSemaphoresSubmited;
    std::vector<VkSemaphore> uploadSemaphores;
    std::vector<VkFence> uploadFences;

    std::vector<GDescriptorPoolVLK*> m_descriptorPools;

    VmaAllocator vmaAllocator;
    VmaPool uboVmaPool;


    VkPhysicalDeviceProperties deviceProperties;

    float clearColor[3] = {0,0,0};


    unsigned int m_frameNumber = 0;
    bool m_firstFrame = true;
    bool m_shaderDescriptorUpdateNeeded = false;


    uint8_t m_lastColorMask = 0xFF;
    int8_t m_lastDepthWrite = -1;
    int8_t m_lastDepthCulling = -1;
    int8_t m_backFaceCulling = -1;
    int8_t m_triCCW = -1;
    int maxUniformBufferSize = -1;
    int uniformBufferOffsetAlign = -1;
    int maxMultiSample = -1;
    float m_anisotropicLevel = 0.0;
    bool m_isInvertZ = false;
    EGxBlendEnum m_lastBlendMode = EGxBlendEnum::GxBlend_UNDEFINED;
    GIndexBufferVLK *m_lastBindIndexBuffer = nullptr;
    GVertexBufferVLK *m_lastBindVertexBuffer = nullptr;
    GVertexBufferBindingsVLK *m_vertexBufferBindings = nullptr;
    GShaderPermutationVLK * m_shaderPermutation = nullptr;

    HGVertexBufferBindings m_vertexBBBindings;
    HGVertexBufferBindings m_lineBBBindings;
    HGVertexBufferBindings m_defaultVao;

    GTextureVLK *m_lastTexture[10] = {
        nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
        nullptr};

    GUniformBufferVLK * m_vertexUniformBuffer[3] = {nullptr};
    GUniformBufferVLK * m_fragmentUniformBuffer[3] = {nullptr};

    HGTexture m_blackPixelTexture = nullptr;
    HGTexture m_whitePixelTexture = nullptr;

    bool m_texturesWereUploaded = false;
protected:
    //Caches
    std::unordered_map<size_t, HGShaderPermutation> m_shaderPermutCache;
    std::list<std::weak_ptr<GUniformBufferVLK>> m_unfiormBufferCache;
    struct FrameUniformBuffers {
        HGUniformBuffer m_uniformBufferForUpload;
    };

    std::array<FrameUniformBuffers, 4> m_UBOFrames;

    std::vector<char> aggregationBufferForUpload = std::vector<char>(1024*1024);

    std::list<DeallocationRecord> listOfDeallocators;

    int uniformBuffersCreated = 0;
    bool attachmentsReady = false;

    std::vector<FramebufAvalabilityStruct> m_createdFrameBuffers;

    struct RenderPassAvalabilityStruct {
        std::vector<ITextureFormat> attachments;
        ITextureFormat depthAttachment;
        std::shared_ptr<GRenderPassVLK> renderPass;
        VkSampleCountFlagBits sampleCountFlagBits;
        bool isSwapChainPass;
    };

    std::vector<RenderPassAvalabilityStruct> m_createdRenderPasses;
};




#endif //AWEBWOWVIEWERCPP_GDEVICEVULKAN_H
