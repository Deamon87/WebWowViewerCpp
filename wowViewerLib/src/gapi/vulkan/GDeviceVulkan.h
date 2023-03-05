//
// Created by Deamon on 8/1/2019.
//

#ifndef AWEBWOWVIEWERCPP_GDEVICEVULKAN_H
#define AWEBWOWVIEWERCPP_GDEVICEVULKAN_H

#include <memory>
#include <unordered_map>
#include <mutex>



class GVertexBufferBindingsVLK;
class GBlpTextureVLK;
class GTextureVLK;
class GShaderPermutationVLK;
class GMeshVLK;
class GM2MeshVLK;
class GPipelineVLK;
class GRenderPassVLK;
class GDescriptorPoolVLK;
class CmdBufRecorder;
class RenderPassHelper;
class TextureManagerVLK;

typedef std::shared_ptr<GPipelineVLK> HPipelineVLK;

class gMeshTemplate;

#include <unordered_set>
#include <list>

#include "../interface/IDevice.h"
#include "descriptorSets/GDescriptorSet.h"
#include "descriptorSets/GDescriptorPoolVLK.h"
#include "../../engine/algorithms/FrameCounter.h"
#include "../../renderer/IRenderParameters.h"
#include "buffers/GBufferVLK.h"
#include "IDeviceVulkan.h"
#include "synchronization/GFenceVLK.h"
#include "synchronization/GSemaphoreVLK.h"
#include "commandBuffer/commandBufferRecorder/RenderPassHelper.h"
#include "TextureManagerVLK.h"

#include <optional>

VkSampleCountFlagBits sampleCountToVkSampleCountFlagBits(uint8_t sampleCount);

class GDeviceVLK : public IDevice, public std::enable_shared_from_this<GDeviceVLK>, public IDeviceVulkan {
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

public:
    explicit GDeviceVLK(vkCallInitCallback * callBacks);
    ~GDeviceVLK() override = default;;

    GDeviceType getDeviceType() override {return GDeviceType::GVulkan; };

    void initialize() override;

    unsigned int getFrameNumber() override { return m_frameNumber; };
    unsigned int getUpdateFrameNumber() ;
    unsigned int getCullingFrameNumber() ;
    unsigned int getOcclusionFrameNumber() ;
    unsigned int getDrawFrameNumber() ;

    bool getIsRenderbufferSupported() override {return true;}

    void increaseFrameNumber() override;
    bool getIsAsynBuffUploadSupported() override {
        return true;
    }
    int getMaxSamplesCnt() override;
    VkSampleCountFlagBits getMaxSamplesBit();

    float getAnisLevel() override;

    void startUpdateForNextFrame() override {};
    void endUpdateForNextFrame() override {};

    void drawFrame(const std::vector<std::unique_ptr<IRenderFunction>> &renderFuncs) override;

    void updateBuffers(/*std::vector<std::vector<HGUniformBufferChunk>*> &bufferChunks*/std::vector<HFrameDependantData> &frameDepedantData);
    void uploadTextureForMeshes(std::vector<HGMesh> &meshes) override;
    bool getIsVulkanAxisSystem() override {return true;}

    void initUploadThread() override;

    const QueueFamilyIndices &getQueueFamilyIndices() override {
        return indices;
    }
public:
    double getWaitForUpdate() override {
        return this->waitInDrawStageAndDeps.getTimePerFrame();
    }

    std::shared_ptr<IShaderPermutation> getShader(std::string vertexName, std::string fragmentName, void *permutationDescriptor) override;

    HGBufferVLK createUniformBuffer(size_t size);
    HGBufferVLK createVertexBuffer(size_t size);
    HGBufferVLK createIndexBuffer(size_t size);
    HGVertexBufferBindings createVertexBufferBindings() override;

    HGTexture createBlpTexture(HBlpTexture &texture, bool xWrapTex, bool yWrapTex) override;
    HGTexture createTexture(bool xWrapTex, bool yWrapTex) override;
    HGTexture getWhiteTexturePixel() override { return m_whitePixelTexture; };
    HGTexture getBlackTexturePixel() override { return m_blackPixelTexture; };
    HGMesh createMesh(gMeshTemplate &meshTemplate) override;

    HGPUFence createFence() override;

    HFrameBuffer createFrameBuffer(int width, int height, std::vector<ITextureFormat> attachments, ITextureFormat depthAttachment, int multiSampleCnt, int frameNumber) override ;

    HPipelineVLK createPipeline(const HGVertexBufferBindings &m_bindings,
                                const HGShaderPermutation &shader,
                                const std::shared_ptr<GRenderPassVLK> &renderPass,
                                DrawElementMode element,
                                bool backFaceCulling,
                                bool triCCW,
                                EGxBlendEnum blendMode,
                                bool depthCulling,
                                bool depthWrite);

    std::shared_ptr<GRenderPassVLK> getRenderPass(std::vector<ITextureFormat> textureAttachments,
                                                  ITextureFormat depthAttachment,
                                                  VkSampleCountFlagBits sampleCountFlagBits,
                                                  bool invertZ,
                                                  bool isSwapChainPass);

    std::shared_ptr<GRenderPassVLK> getSwapChainRenderPass();

    void submitDrawCommands() override;
    void submitQueue(
        VkQueue queue,
        const std::vector<VkSemaphore> &waitSemaphores,
        const std::vector<VkPipelineStageFlags> &waitStages,
        const std::vector<VkCommandBuffer> &commandBuffers,
        const std::vector<VkSemaphore> &signalSemaphoresOnCompletion,
        const VkFence signalFenceOnCompletion) ;

    void presentQueue(const std::vector<VkSemaphore> &waitSemaphores,
                      const std::vector<VkSwapchainKHR> &swapchains,
                      const std::vector<uint32_t> &imageIndexes);

    VkDescriptorSet allocateDescriptorSetPrimitive(
        const std::shared_ptr<GDescriptorSetLayout> &hDescriptorSetLayout,
        std::shared_ptr<GDescriptorPoolVLK> &desciptorPool) override;

    std::shared_ptr<GDescriptorSet> createDescriptorSet(std::shared_ptr<GDescriptorSetLayout> &hDescriptorSetLayout);

    virtual VkDevice getVkDevice() override {
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

    VmaAllocator getVMAAllocator() override {
        return vmaAllocator;
    }

    struct DeallocationRecord {
        unsigned int frameNumberToDoAt;
        std::function<void()> callback;
    };

    std::mutex m_listOfDeallocatorsAccessMtx;

    void addDeallocationRecord(std::function<void()> callback) override {
        std::lock_guard<std::mutex> lock(m_listOfDeallocatorsAccessMtx);
        DeallocationRecord dr;
        dr.frameNumberToDoAt = m_frameNumber+MAX_FRAMES_IN_FLIGHT;
        dr.callback = callback;
        listOfDeallocators.push_back(dr);
    };

    VkFormat findDepthFormat();

    void singleExecuteAndWait(std::function<void(VkCommandBuffer commandBuffer)> callback);
private:
//    void internalDrawStageAndDeps(HDrawStage drawStage);

    void setupDebugMessenger();
    void pickPhysicalDevice();
    void createLogicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    void findQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);


    void createSwapChainAndFramebuffer();
    void createSwapChain(SwapChainSupportDetails &swapChainSupport, VkSurfaceFormatKHR &surfaceFormat, VkExtent2D &extent);
    void createSwapChainImageViews(std::vector<VkImage> &swapChainImages, std::vector<VkImageView> &swapChainImageViews, VkFormat swapChainImageFormat);

    void createSwapChainRenderPass(VkFormat swapChainImageFormat);

    void createFramebuffers(std::vector<HGTextureVLK> &swapChainTextures, VkExtent2D &extent);

    void createCommandPool();
    void createCommandPoolForUpload();
    void createCommandBuffers();
    void createSyncObjects();

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) override;

protected:
    struct PipelineCacheRecord {
        HGShaderPermutation shader;
        std::shared_ptr<GRenderPassVLK> renderPass;
        DrawElementMode element;
        bool backFaceCulling;
        bool triCCW;
        EGxBlendEnum blendMode;
        bool depthCulling;
        bool depthWrite;


        bool operator==(const PipelineCacheRecord &other) const {
            return
                (shader == other.shader) &&
                (renderPass == other.renderPass) &&
                (element == other.element) &&
                (backFaceCulling == other.backFaceCulling) &&
                (triCCW == other.triCCW) &&
                (blendMode == other.blendMode) &&
                (depthCulling == other.depthCulling) &&
                (depthWrite == other.depthWrite);
        };
    };
    struct PipelineCacheRecordHasher {
        std::size_t operator()(const PipelineCacheRecord& k) const {
            using std::hash;
            return hash<void*>{}(k.shader.get()) ^
            hash<void*>{}(k.renderPass.get()) ^
            (hash<bool >{}(k.backFaceCulling) << 2) ^
            (hash<bool >{}(k.triCCW) << 4) ^
            (hash<bool >{}(k.depthCulling) << 8) ^
            (hash<bool >{}(k.depthWrite) << 10) ^
            (hash<EGxBlendEnum>{}(k.blendMode) << 14) ^
            (hash<DrawElementMode>{}(k.element) << 16);
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

    VkExtent2D swapChainExtent;

    std::vector<std::shared_ptr<GFrameBufferVLK>> swapChainFramebuffers;

    std::shared_ptr<GRenderPassVLK> swapchainRenderPass;

    VkCommandPool commandPool;
    VkCommandPool commandPoolForImageTransfer;
    VkCommandPool renderCommandPool;
    VkCommandPool uploadCommandPool;

    std::array<std::shared_ptr<GCommandBuffer>, MAX_FRAMES_IN_FLIGHT> fbCommandBuffers = {nullptr};
    std::array<std::shared_ptr<GCommandBuffer>, MAX_FRAMES_IN_FLIGHT> swapChainCommandBuffers = {nullptr};
    std::array<std::shared_ptr<GCommandBuffer>, MAX_FRAMES_IN_FLIGHT> uploadCommandBuffers = {nullptr};


    std::array<std::shared_ptr<GFenceVLK>, MAX_FRAMES_IN_FLIGHT> inFlightFences;
    std::array<std::shared_ptr<GFenceVLK>, MAX_FRAMES_IN_FLIGHT> uploadFences;

    std::array<std::shared_ptr<GSemaphoreVLK>, MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphores;
    // Is used to signal for present queue that frame has finished rendering and it can be presented on screen
    std::array<std::shared_ptr<GSemaphoreVLK>, MAX_FRAMES_IN_FLIGHT> renderFinishedSemaphores;

    std::array<std::shared_ptr<GSemaphoreVLK>, MAX_FRAMES_IN_FLIGHT> uploadSemaphores;

    std::vector<std::shared_ptr<GDescriptorPoolVLK>> m_descriptorPools;

    VmaAllocator vmaAllocator;

    VkPhysicalDeviceProperties deviceProperties;

    unsigned int m_frameNumber = 0;
    bool m_firstFrame = true;
    bool m_shaderDescriptorUpdateNeeded = false;

    int maxUniformBufferSize = -1;
    int uniformBufferOffsetAlign = -1;
    int maxMultiSample = -1;
    float m_anisotropicLevel = 0.0;

    HGVertexBufferBindings m_vertexBBBindings;
    HGVertexBufferBindings m_lineBBBindings;
    HGVertexBufferBindings m_defaultVao;

    HGTexture m_blackPixelTexture = nullptr;
    HGTexture m_whitePixelTexture = nullptr;

    std::shared_ptr<TextureManagerVLK> m_textureManager;
protected:
    //Caches
    std::unordered_map<size_t, HGShaderPermutation> m_shaderPermuteCache;
    struct FrameUniformBuffers {
        HGUniformBuffer m_uniformBufferForUpload;
    };

    std::vector<char> aggregationBufferForUpload = std::vector<char>(1024*1024);

    std::list<DeallocationRecord> listOfDeallocators;

    std::vector<FramebufAvalabilityStruct> m_createdFrameBuffers;

    struct RenderPassAvalabilityStruct {
        std::vector<ITextureFormat> attachments;
        ITextureFormat depthAttachment;
        std::shared_ptr<GRenderPassVLK> renderPass;
        VkSampleCountFlagBits sampleCountFlagBits;
        bool isSwapChainPass;
        bool invertZ;
    };

    std::vector<RenderPassAvalabilityStruct> m_createdRenderPasses;

    void executeDeallocators();

    RenderPassHelper beginSwapChainRenderPass(uint32_t imageIndex, CmdBufRecorder &swapChainCmd);

    void getNextSwapImageIndex(uint32_t &imageIndex);
};




#endif //AWEBWOWVIEWERCPP_GDEVICEVULKAN_H
