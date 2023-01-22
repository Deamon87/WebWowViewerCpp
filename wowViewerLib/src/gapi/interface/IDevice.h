//
// Created by Deamon on 8/25/2018.
//

#ifndef AWEBWOWVIEWERCPP_IDEVICE_H
#define AWEBWOWVIEWERCPP_IDEVICE_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class IVertexBuffer;
class IVertexBufferDynamic;
class IVertexBufferBindings;
class IBuffer;
class IUniformBuffer;
class ITexture;
class IShaderPermutation;
class IMesh;
class IM2Mesh;
class IDevice;
class IGPUFence;
class IFrameBuffer;
class gMeshTemplate;
#include <memory>
#include <vector>
#include <functional>
#include <algorithm>
#include "syncronization/IGPUFence.h"

#ifdef LINK_VULKAN
    #define VK_NO_PROTOTYPES
    #include "../vulkan/volk.h"

    //HACKS for stupid defines in X11
    #undef None           // Defined by X11/X.h to 0L
    #undef Status         // Defined by X11/Xlib.h to int
    #undef True           // Defined by X11/Xlib.h to 1
    #undef False          // Defined by X11/Xlib.h to 0
    #undef Bool           // Defined by X11/Xlib.h to int
    #undef RootWindow     // Defined by X11/Xlib.h
    #undef CurrentTime    // Defined by X11/X.h to 0L
    #undef Success        // Defined by X11/X.h to 0
    #undef DestroyAll     // Defined by X11/X.h to 0
    #undef COUNT          // Defined by X11/extensions/XI.h to 0
    #undef CREATE         // Defined by X11/extensions/XI.h to 1
    #undef DeviceAdded    // Defined by X11/extensions/XI.h to 0
    #undef DeviceRemoved  // Defined by X11/extensions/XI.h to 1

    #include "../vulkan/vk_mem_alloc.h"
#endif

typedef std::shared_ptr<IBuffer> HGVertexBufferDynamic;
typedef std::shared_ptr<IBuffer> HGVertexBuffer;
typedef std::shared_ptr<IBuffer> HGIndexBuffer;
typedef std::shared_ptr<IVertexBufferBindings> HGVertexBufferBindings;
typedef std::shared_ptr<IBuffer> HGUniformBuffer;
typedef std::shared_ptr<IShaderPermutation> HGShaderPermutation;
typedef std::shared_ptr<IMesh> HGMesh;
typedef std::shared_ptr<IM2Mesh> HGM2Mesh;
typedef std::shared_ptr<IMesh> HGParticleMesh;
typedef std::shared_ptr<IMesh> HGOcclusionQuery;
typedef std::shared_ptr<ITexture> HGTexture;
typedef std::shared_ptr<IGPUFence> HGPUFence;
typedef std::shared_ptr<IFrameBuffer> HFrameBuffer;

#include "meshes/IMesh.h"
#include "meshes/IM2Mesh.h"
#include "IOcclusionQuery.h"
#include "IShaderPermutation.h"
#include "buffers/IBuffer.h"
#include "IVertexBufferBindings.h"
#include "buffers/IBufferChunk.h"
#include "../../engine/wowCommonClasses.h"
#include "../../engine/texture/BlpTexture.h"
#include "textures/ITexture.h"
#include "IFrameBuffer.h"

struct M2ShaderCacheRecord {
    int vertexShader;
    int pixelShader;
    bool unlit;
    bool alphaTestOn;
    bool unFogged;
    bool unShadowed;
    int boneInfluences;


    bool operator==(const M2ShaderCacheRecord &other) const {
        return
            (vertexShader == other.vertexShader) &&
            (pixelShader == other.pixelShader) &&
            (alphaTestOn == other.alphaTestOn) &&
            (unlit == other.unlit) &&
            (unFogged == other.unFogged) &&
            (unShadowed == other.unShadowed) &&
            (boneInfluences == other.boneInfluences);
    };
};

struct WMOShaderCacheRecord {
    int vertexShader;
    int pixelShader;
    bool unlit;
    bool alphaTestOn;
    bool unFogged;
    bool unShadowed;

    bool operator==(const WMOShaderCacheRecord &other) const {
        return
            (vertexShader == other.vertexShader) &&
            (pixelShader == other.pixelShader) &&
            (alphaTestOn == other.alphaTestOn) &&
            (unlit == other.unlit) &&
            (unFogged == other.unFogged) &&
            (unShadowed == other.unShadowed);
    };
};

enum class IShaderType {
    gVertexShader,
    gFragmentShader
};

struct ShaderContentCacheRecord {
    std::string fileName;
    IShaderType shaderType;
    bool operator==(const ShaderContentCacheRecord &other) const {
        return
            (fileName == other.fileName) &&
            (shaderType == other.shaderType);
    }
};

struct ShaderContentCacheRecordHasher {
    std::size_t operator()(const ShaderContentCacheRecord& k) const {
        using std::hash;
        return hash<std::string>{}(k.fileName) ^
                (hash<int>{}((int)k.shaderType));
    };
};

#ifdef LINK_VULKAN
struct vkCallInitCallback {
    std::function<void(char** &extensionNames, int &extensionCnt)> getRequiredExtensions;
    std::function<VkSurfaceKHR(VkInstance vkInstance )> createSurface;
    int extensionCnt;
};
#endif

struct FramebufAvalabilityStruct {
    int width; int height;
    std::vector<ITextureFormat> attachments;
    ITextureFormat depthAttachment;
    HFrameBuffer frameBuffer;
    int frame;
};

enum class GDeviceType {
    GOpenGL2, GOpenGL3, GVulkan,
};

class IDevice {
    public:
        static constexpr uint8_t MAX_FRAMES_IN_FLIGHT = 3;

        virtual ~IDevice() {};

        virtual GDeviceType getDeviceType() = 0;

        virtual void initialize() = 0;

        virtual bool getIsAsynBuffUploadSupported() = 0;
        virtual int getMaxSamplesCnt() = 0;
        virtual int getUploadSize() {return 0;};

        virtual void increaseFrameNumber() = 0;

        virtual void drawScenario() {};

        virtual void startUpdateForNextFrame() {};
        virtual void endUpdateForNextFrame() {};

//        virtual void updateBuffers(std::vector<std::vector<HGUniformBufferChunk>*> &bufferChunks, std::vector<HFrameDependantData> &frameDepedantData)= 0;
        virtual void uploadTextureForMeshes(std::vector<HGMesh> &meshes) = 0;
        virtual void drawMeshes(std::vector<HGMesh> &meshes) = 0;
//        virtual void drawStageAndDeps(HDrawStage drawStage) = 0;

        virtual bool getIsCompressedTexturesSupported();
        virtual bool getIsAnisFiltrationSupported();
        virtual float getAnisLevel() = 0;
        virtual bool getIsVulkanAxisSystem() {return false;}
        virtual bool getIsRenderbufferSupported() {return false;}

        virtual void initUploadThread(){}
        virtual double getWaitForUpdate() {return 0;}

    public:
        virtual HGShaderPermutation getShader(std::string shaderName, std::string fragmentName, void *permutationDescriptor) = 0;

        virtual HGPUFence createFence() = 0;
        virtual HGVertexBufferBindings createVertexBufferBindings() = 0;
        //Creates or receives framebuffer and tells it would be occupied for frameNumber frames
        virtual HFrameBuffer createFrameBuffer(int width, int height, std::vector<ITextureFormat> attachments, ITextureFormat depthAttachment, int multiSampleCnt, int frameNumber) = 0;

        virtual HGTexture createBlpTexture(HBlpTexture &texture, bool xWrapTex, bool yWrapTex) = 0;
        virtual HGTexture createTexture(bool xWrapTex, bool yWrapTex) = 0;
        virtual HGTexture getWhiteTexturePixel() = 0;
        virtual HGTexture getBlackTexturePixel() = 0;
        virtual HGMesh createMesh(gMeshTemplate &meshTemplate) = 0;

        virtual void shrinkData() {};

        static std::string insertAfterVersion(std::string &glslShaderString, std::string stringToPaste);
        virtual void addDeallocationRecord(std::function<void()> callback) {};

        virtual int getCurrentTextureAllocated() {return 0;}
};

typedef std::shared_ptr<IDevice> HGDevice;

#include <cassert>

#define IDEVICE_DEBUG
#ifdef IDEVICE_DEBUG
#define TEST(expr) do { \
            if(!(expr)) { \
                assert(0 && #expr); \
            } \
        } while(0)
#else
#define TEST(expr) do { \
            if(!(expr)) { \
                throw std::runtime_error("TEST FAILED: " #expr); \
            } \
        } while(0)
#endif

#define ERR_GUARD_VULKAN(expr) TEST((expr) >= 0)


#endif //AWEBWOWVIEWERCPP_IDEVICE_H
