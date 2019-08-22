//
// Created by Deamon on 8/1/2019.
//

#ifndef AWEBWOWVIEWERCPP_GDEVICEVULKAN_H
#define AWEBWOWVIEWERCPP_GDEVICEVULKAN_H

#include <memory>
#include <unordered_map>


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


class gMeshTemplate;

#include <unordered_set>
#include <list>

#include "../interface/IDevice.h"

class GDeviceVLK : public IDevice {
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

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
    explicit GDeviceVLK(vkCallInitCallback * callBacks);
    ~GDeviceVLK() override = default;;

    void reset() override;

    unsigned int getUpdateFrameNumber() override;
    unsigned int getCullingFrameNumber() override;
    unsigned int getDrawFrameNumber() override;


    void increaseFrameNumber() override;
    bool getIsAsynBuffUploadSupported() override {
        return false;
    }

    float getAnisLevel() override;

    void bindProgram(IShaderPermutation *program) override;

    void bindIndexBuffer(IIndexBuffer *buffer) override;
    void bindVertexBuffer(IVertexBuffer *buffer) override;
    void bindVertexUniformBuffer(IUniformBuffer *buffer, int slot) override;
    void bindFragmentUniformBuffer(IUniformBuffer *buffer, int slot) override;
    void bindVertexBufferBindings(IVertexBufferBindings *buffer) override;

    void bindTexture(ITexture *texture, int slot) override;

    void updateBuffers(std::vector<HGMesh> &meshes) override;
    void uploadTextureForMeshes(std::vector<HGMesh> &meshes) override;
    void drawMeshes(std::vector<HGMesh> &meshes) override;
    //    void drawM2Meshes(std::vector<HGM2Mesh> &meshes);
public:
    std::shared_ptr<IShaderPermutation> getShader(std::string shaderName, void *permutationDescriptor) override;

    HGUniformBuffer createUniformBuffer(size_t size) override;
    HGVertexBuffer createVertexBuffer() override;
    HGIndexBuffer createIndexBuffer() override;
    HGVertexBufferBindings createVertexBufferBindings() override;

    HGTexture createBlpTexture(HBlpTexture &texture, bool xWrapTex, bool yWrapTex) override;
    HGTexture createTexture() override;
    HGMesh createMesh(gMeshTemplate &meshTemplate) override;
    HGM2Mesh createM2Mesh(gMeshTemplate &meshTemplate) override;
    HGParticleMesh createParticleMesh(gMeshTemplate &meshTemplate) override;
    HGPUFence createFence() override;

    HGOcclusionQuery createQuery(HGMesh boundingBoxMesh) override;

    HGVertexBufferBindings getBBVertexBinding() override;
    HGVertexBufferBindings getBBLinearBinding() override;

    std::string loadShader(std::string fileName, bool common) override;

    virtual void clearScreen();

    virtual VkInstance getVkInstance() override;

private:
    void drawMesh(HGMesh &hmesh);

    void pickPhysicalDevice();
    void createLogicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    void createSwapChain();
    void createImageViews();
    void createFramebuffers();
    void createRenderPass();


protected:
    struct BlpCacheRecord {
        HBlpTexture texture;
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
            return hash<void*>{}(k.texture.get()) ^ (hash<bool>{}(k.wrapX) << 8) ^ (hash<bool>{}(k.wrapY) << 16);
        };
    };
    std::unordered_map<BlpCacheRecord, std::weak_ptr<GDeviceVLK>, BlpCacheRecordHasher> loadedTextureCache;

    VkInstance vkInstance;
    VkSurfaceKHR vkSurface;
    bool enableValidationLayers = false;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;


    unsigned int m_frameNumber = 0;

    uint8_t m_lastColorMask = 0xFF;
    int8_t m_lastDepthWrite = -1;
    int8_t m_lastDepthCulling = -1;
    int8_t m_backFaceCulling = -1;
    int8_t m_triCCW = -1;
    int maxUniformBufferSize = -1;
    int uniformBufferOffsetAlign = -1;
    float m_anisotropicLevel = 0.0;
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

    HGTexture m_blackPixelTexture;

public:
    struct M2ShaderCacheRecordHasher {
        std::size_t operator()(const M2ShaderCacheRecord& k) const {
            using std::hash;
            return hash<int>{}(k.vertexShader) ^ (hash<int>{}(k.pixelShader)) ^
                   (hash<bool>{}(k.unlit)) ^
                   (hash<bool>{}(k.alphaTestOn) << 8) ^
                   (hash<bool>{}(k.unFogged) << 16) ^
                   (hash<bool>{}(k.unShadowed) << 24);
        };
    };
    std::unordered_map<M2ShaderCacheRecord, std::weak_ptr<IShaderPermutation>, M2ShaderCacheRecordHasher> m2ShaderCache;

    struct WMOShaderCacheRecordHasher {
        std::size_t operator()(const WMOShaderCacheRecord& k) const {
            using std::hash;
            return hash<int>{}(k.vertexShader) ^ (hash<int>{}(k.pixelShader)) ^
                   (hash<bool>{}(k.unlit)) ^
                   (hash<bool>{}(k.alphaTestOn) << 8) ^
                   (hash<bool>{}(k.unFogged) << 16) ^
                   (hash<bool>{}(k.unShadowed) << 24);
        };
    };
    std::unordered_map<WMOShaderCacheRecord, std::weak_ptr<IShaderPermutation>, WMOShaderCacheRecordHasher> wmoShaderCache;
protected:
    //Caches
    std::unordered_map<size_t, HGShaderPermutation> m_shaderPermutCache;
    std::list<std::weak_ptr<GUniformBufferVLK>> m_unfiormBufferCache;
    struct FrameUniformBuffers {
        std::vector<HGUniformBuffer> m_uniformBuffersForUpload;
    };

    FrameUniformBuffers m_UBOFrames[4];

    std::vector<char> aggregationBufferForUpload;

    std::unordered_map<std::string, std::string> shaderCache;

    int uniformBuffersCreated = 0;
};

#endif //AWEBWOWVIEWERCPP_GDEVICEVULKAN_H
