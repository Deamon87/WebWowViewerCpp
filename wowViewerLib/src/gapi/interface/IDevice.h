//
// Created by Deamon on 8/25/2018.
//

#ifndef AWEBWOWVIEWERCPP_IDEVICE_H
#define AWEBWOWVIEWERCPP_IDEVICE_H

class IVertexBuffer;
class IVertexBufferDynamic;
class IVertexBufferBindings;
class IIndexBuffer;
class IUniformBuffer;
class IUniformBufferChunk;
class IBlpTexture;
class ITexture;
class IShaderPermutation;
class IMesh;
class IM2Mesh;
class IDeviceUI;
class IOcclusionQuery;
class IParticleMesh;
class IGPUFence;
class IFrameBuffer;
class gMeshTemplate;
#include <memory>
#include <vector>
#include <functional>
#include <algorithm>
#include "syncronization/IGPUFence.h"
#ifdef LINK_VULKAN
#include <vulkan/vulkan_core.h>
#endif

typedef std::shared_ptr<IVertexBufferDynamic> HGVertexBufferDynamic;
typedef std::shared_ptr<IVertexBuffer> HGVertexBuffer;
typedef std::shared_ptr<IIndexBuffer> HGIndexBuffer;
typedef std::shared_ptr<IVertexBufferBindings> HGVertexBufferBindings;
typedef std::shared_ptr<IUniformBuffer> HGUniformBuffer;
typedef std::shared_ptr<IUniformBufferChunk> HGUniformBufferChunk;
typedef std::shared_ptr<IShaderPermutation> HGShaderPermutation;
typedef std::shared_ptr<IMesh> HGMesh;
typedef std::shared_ptr<IMesh> HGM2Mesh;
typedef std::shared_ptr<IMesh> HGParticleMesh;
typedef std::shared_ptr<IMesh> HGOcclusionQuery;
typedef std::shared_ptr<IBlpTexture> HGBlpTexture;
typedef std::shared_ptr<ITexture> HGTexture;
typedef std::shared_ptr<IGPUFence> HGPUFence;
typedef std::shared_ptr<IFrameBuffer> HFrameBuffer;

#include "meshes/IMesh.h"
#include "meshes/IM2Mesh.h"
#include "IOcclusionQuery.h"
#include "IDeviceUI.h"
#include "IShaderPermutation.h"
#include "buffers/IIndexBuffer.h"
#include "buffers/IVertexBuffer.h"
#include "buffers/IVertexBufferDynamic.h"
#include "IVertexBufferBindings.h"
#include "buffers/IUniformBuffer.h"
#include "buffers/IUniformBufferChunk.h"
#include "../../engine/wowCommonClasses.h"
#include "../../engine/texture/BlpTexture.h"
#include "textures/ITexture.h"
#include "IFrameBuffer.h"
#include "../../engine/DrawStage.h"

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

class IDevice {
    public:
        virtual ~IDevice() {};

        virtual void reset() = 0;
        virtual unsigned int getUpdateFrameNumber() = 0;
        virtual unsigned int getCullingFrameNumber() = 0;
        virtual unsigned int getDrawFrameNumber() = 0;
        virtual bool getIsAsynBuffUploadSupported() = 0;

        virtual void increaseFrameNumber() = 0;

        virtual void bindProgram(IShaderPermutation *program) = 0;

        virtual void bindIndexBuffer(IIndexBuffer *buffer) = 0;
        virtual void bindVertexBuffer(IVertexBuffer *buffer) = 0;
        virtual void bindUniformBuffer(IUniformBuffer *buffer, int slot, int offset, int length) = 0;
        virtual void bindVertexBufferBindings(IVertexBufferBindings *buffer) = 0;

        virtual void bindTexture(ITexture *texture, int slot) = 0;


        virtual void startUpdateForNextFrame() {};
        virtual void endUpdateForNextFrame() {};
        virtual void updateBuffers(std::vector<HGMesh> &meshes)= 0;
        virtual void prepearMemoryForBuffers(std::vector<HGMesh> &meshes) = 0;
        virtual void uploadTextureForMeshes(std::vector<HGMesh> &meshes) = 0;
        virtual void drawMeshes(std::vector<HGMesh> &meshes) = 0;
        virtual void drawStageAndDeps(HDrawStage drawStage) {};

        virtual bool getIsCompressedTexturesSupported();
        virtual bool getIsAnisFiltrationSupported();
        virtual float getAnisLevel() = 0;
        virtual bool getIsVulkanAxisSystem() {return false;}
    public:
        virtual HGShaderPermutation getShader(std::string shaderName, void *permutationDescriptor) = 0;

        virtual HGPUFence createFence() = 0;

        virtual HGUniformBuffer createUniformBuffer(size_t size) = 0;
        virtual HGUniformBufferChunk createUniformBufferChunk(size_t size) {
            HGUniformBufferChunk h_uniformBuffer;
            h_uniformBuffer.reset(new IUniformBufferChunk(size));

            return h_uniformBuffer;
        };
        virtual HGVertexBufferDynamic createVertexBufferDynamic(size_t size) = 0;
        virtual HGVertexBuffer createVertexBuffer() = 0;
        virtual HGIndexBuffer createIndexBuffer() = 0;
        virtual HGVertexBufferBindings createVertexBufferBindings() = 0;

        virtual HGTexture createBlpTexture(HBlpTexture &texture, bool xWrapTex, bool yWrapTex) = 0;
        virtual HGTexture createTexture() = 0;
        virtual HGTexture getWhiteTexturePixel() = 0;
        virtual HGTexture getBlackTexturePixel() {return nullptr;};
        virtual HGMesh createMesh(gMeshTemplate &meshTemplate) = 0;
        virtual HGM2Mesh createM2Mesh(gMeshTemplate &meshTemplate) = 0;
        virtual HGParticleMesh createParticleMesh(gMeshTemplate &meshTemplate) = 0;

        virtual HGOcclusionQuery createQuery(HGMesh boundingBoxMesh) = 0;

        static inline bool sortMeshes(const HGMesh a, const HGMesh b) {
            auto* pA = a.get();
            auto* pB = b.get();

            if (pA->getIsTransparent() > pB->getIsTransparent()) {
                return false;
            }
            if (pA->getIsTransparent() < pB->getIsTransparent()) {
                return true;
            }

            if (pA->getMeshType() > pB->getMeshType()) {
                return false;
            }
            if (pA->getMeshType() < pB->getMeshType()) {
                return true;
            }

            if (pA->m_renderOrder != pB->m_renderOrder ) {
                if (!pA->getIsTransparent()) {
                    return pA->m_renderOrder < pB->m_renderOrder;
                } else {
                    return pA->m_renderOrder > pB->m_renderOrder;
                }
            }

            if (pA->m_isSkyBox > pB->m_isSkyBox) {
                return true;
            }
            if (pA->m_isSkyBox < pB->m_isSkyBox) {
                return false;
            }

            if (pA->getMeshType() == MeshType::eM2Mesh && pA->getIsTransparent() && pB->getIsTransparent()) {
                if (pA->m_priorityPlane != pB->m_priorityPlane) {
                    return pB->m_priorityPlane > pA->m_priorityPlane;
                }

                if (pA->m_sortDistance > pB->m_sortDistance) {
                    return true;
                }
                if (pA->m_sortDistance < pB->m_sortDistance) {
                    return false;
                }

                if (pA->m_m2Object > pB->m_m2Object) {
                    return true;
                }
                if (pA->m_m2Object < pB->m_m2Object) {
                    return false;
                }

                if (pB->m_layer != pA->m_layer) {
                    return pB->m_layer < pA->m_layer;
                }
            }

            if (pA->getMeshType() == MeshType::eParticleMesh && pB->getMeshType() == MeshType::eParticleMesh) {
                if (pA->m_priorityPlane != pB->m_priorityPlane) {
                    return pB->m_priorityPlane > pA->m_priorityPlane;
                }

                if (pA->m_sortDistance > pB->m_sortDistance) {
                    return true;
                }
                if (pA->m_sortDistance < pB->m_sortDistance) {
                    return false;
                }
            }

            if (pA->m_bindings != pB->m_bindings) {
                return pA->m_bindings > pB->m_bindings;
            }

            if (pA->getGxBlendMode() != pB->getGxBlendMode()) {
                return pA->getGxBlendMode() < pB->getGxBlendMode();
            }

            int minTextureCount = pA->m_textureCount < pB->m_textureCount ? pA->m_textureCount : pB->m_textureCount;
            for (int i = 0; i < minTextureCount; i++) {
                if (pA->m_texture[i] != pB->m_texture[i]) {
                    return pA->m_texture[i] < pB->m_texture[i];
                }
            }

            if (pA->m_textureCount != pB->m_textureCount) {
                return pA->m_textureCount < pB->m_textureCount;
            }

            if (pA->m_start != pB->m_start) {
                return pA->m_start < pB->m_start;
            }
            if (pA->m_end != pB->m_end) {
                return pA->m_end < pB->m_end;
            }


            return a > b;
        }
        virtual HGVertexBufferBindings getBBVertexBinding() = 0;
        virtual HGVertexBufferBindings getBBLinearBinding() = 0;
        virtual std::string loadShader(std::string fileName, IShaderType shaderType) = 0;
        virtual void clearScreen() = 0;
        virtual void setClearScreenColor(float r, float g, float b) = 0;
        virtual void setViewPortDimensions(float x, float y, float width, float height) = 0;

        virtual void beginFrame() = 0;
        virtual void commitFrame() = 0;

        virtual void shrinkData() {};
};

#include <cassert>

#define _DEBUG
#ifdef _DEBUG
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
