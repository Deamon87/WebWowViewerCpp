//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GDEVICE_H
#define WEBWOWVIEWERCPP_GDEVICE_H

#include <memory>

class GVertexBufferGL33;
class GVertexBufferBindingsGL33;
class GIndexBufferGL33;
class GUniformBufferGL33;
class GBlpTextureGL33;
class GTextureGL33;
class GShaderPermutationGL33;
class GMeshGL33;
class GM2MeshGL33;
class GOcclusionQueryGL33;
class GParticleMeshGL33;


class gMeshTemplate;

typedef std::shared_ptr<GUniformBufferGL33> HGLUniformBuffer;
typedef std::shared_ptr<GMeshGL33> HGLMesh;

#include <unordered_set>
#include <list>
#include "GVertexBufferBindingsGL33.h"
#include "buffers/GIndexBufferGL33.h"
#include "buffers/GVertexBufferGL33.h"
#include "textures/GTextureGL33.h"
#include "textures/GBlpTextureGL33.h"
#include "buffers/GUniformBufferGL33.h"
#include "GShaderPermutationGL33.h"
#include "meshes/GMeshGL33.h"
#include "../interface/IDevice.h"



class GDeviceGL33 : public IDevice {
public:
    GDeviceGL33();
    ~GDeviceGL33() override {};

    void reset() override;

    int getFrameNumber() override;

    void increaseFrameNumber() override;
    bool getIsAsynBuffUploadSupported() override {
        return false;
    }

    void bindProgram(IShaderPermutation *program) override;

    void bindIndexBuffer(IIndexBuffer *buffer) override;
    void bindVertexBuffer(IVertexBuffer *buffer) override;
    void bindVertexUniformBuffer(IUniformBuffer *buffer, int slot) override;
    void bindFragmentUniformBuffer(IUniformBuffer *buffer, int slot) override;
    void bindVertexBufferBindings(IVertexBufferBindings *buffer) override;

    void bindTexture(ITexture *texture, int slot) override;

    void updateBuffers(std::vector<HGMesh> &meshes) override;
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


private:
    void drawMesh(HGMesh &hmesh);

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
    std::unordered_map<BlpCacheRecord, std::weak_ptr<GTextureGL33>, BlpCacheRecordHasher> loadedTextureCache;

    int m_frameNumber = 0;

    uint8_t m_lastColorMask = 0xFF;
    int8_t m_lastDepthWrite = -1;
    int8_t m_lastDepthCulling = -1;
    int8_t m_backFaceCulling = -1;
    int8_t m_triCCW = -1;
    int maxUniformBufferSize = -1;
    int uniformBufferOffsetAlign = -1;
    EGxBlendEnum m_lastBlendMode = EGxBlendEnum::GxBlend_UNDEFINED;
    GIndexBufferGL33 *m_lastBindIndexBuffer = nullptr;
    GVertexBufferGL33 *m_lastBindVertexBuffer = nullptr;
    GVertexBufferBindingsGL33 *m_vertexBufferBindings = nullptr;
    GShaderPermutationGL33 * m_shaderPermutation = nullptr;

    HGVertexBufferBindings m_vertexBBBindings;
    HGVertexBufferBindings m_lineBBBindings;
    HGVertexBufferBindings m_defaultVao;

    GTextureGL33 *m_lastTexture[10] = {
        nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
        nullptr};

    GUniformBufferGL33 * m_vertexUniformBuffer[3] = {nullptr};
    GUniformBufferGL33 * m_fragmentUniformBuffer[3] = {nullptr};

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
    std::list<std::weak_ptr<GUniformBufferGL33>> m_unfiormBufferCache;
    struct FrameUniformBuffers {
        std::vector<HGUniformBuffer> m_uniformBuffersForUpload;
    };

    FrameUniformBuffers m_UBOFrames[4];

    std::vector<char> aggregationBufferForUpload;

    int uniformBuffersCreated = 0;
};




#endif //WEBWOWVIEWERCPP_GDEVICE_H
