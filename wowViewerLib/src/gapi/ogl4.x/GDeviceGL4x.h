//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GDEVICE_4X_H
#define WEBWOWVIEWERCPP_GDEVICE_4X_H

#include <memory>

class GVertexBufferGL4x;
class GVertexBufferBindingsGL4x;
class GIndexBufferGL4x;
class GUniformBufferGL4x;
class GBlpTextureGL4x;
class GTextureGL4x;
class GShaderPermutationGL4x;
class GMeshGL4x;
class GM2MeshGL4x;
class GOcclusionQueryGL4x;
class GParticleMeshGL4x;


class gMeshTemplate;

#include <unordered_set>
#include <list>
#include "GVertexBufferBindingsGL4x.h"
#include "buffers/GIndexBufferGL4x.h"
#include "buffers/GVertexBufferGL4x.h"
#include "textures/GTextureGL4x.h"
#include "textures/GBlpTextureGL4x.h"
#include "buffers/GUniformBufferGL4x.h"
#include "GShaderPermutationGL4x.h"
#include "meshes/GMeshGL4x.h"
#include "../interface/IDevice.h"



class GDeviceGL4x : public IDevice {
public:
    GDeviceGL4x();
    ~GDeviceGL4x() override {};

    void reset() override;

    int getFrameNumber() override;

    void increaseFrameNumber() override;
    bool getIsAsynBuffUploadSupported() override {
        return true;
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
    std::shared_ptr<IShaderPermutation> getShader(std::string shaderName) override;

    HGPUFence createFence() override;

    HGUniformBuffer createUniformBuffer(size_t size) override;
    HGVertexBuffer createVertexBuffer() override;
    HGIndexBuffer createIndexBuffer() override;
    HGVertexBufferBindings createVertexBufferBindings() override;

    HGTexture createBlpTexture(HBlpTexture &texture, bool xWrapTex, bool yWrapTex) override;
    HGTexture createTexture() override;
    HGMesh createMesh(gMeshTemplate &meshTemplate) override;
    HGM2Mesh createM2Mesh(gMeshTemplate &meshTemplate) override;
    HGParticleMesh createParticleMesh(gMeshTemplate &meshTemplate) override;

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
    std::unordered_map<BlpCacheRecord, std::weak_ptr<GTextureGL4x>, BlpCacheRecordHasher> loadedTextureCache;

    int m_frameNumber = 0;

    uint8_t m_lastColorMask = 0xFF;
    int8_t m_lastDepthWrite = -1;
    int8_t m_lastDepthCulling = -1;
    int8_t m_backFaceCulling = -1;
    int8_t m_triCCW = -1;
    int maxUniformBufferSize = -1;
    int uniformBufferOffsetAlign = -1;
    EGxBlendEnum m_lastBlendMode = EGxBlendEnum::GxBlend_UNDEFINED;
    GIndexBufferGL4x *m_lastBindIndexBuffer = nullptr;
    GVertexBufferGL4x *m_lastBindVertexBuffer = nullptr;
    GVertexBufferBindingsGL4x *m_vertexBufferBindings = nullptr;
    GShaderPermutationGL4x * m_shaderPermutation = nullptr;

    HGVertexBufferBindings m_vertexBBBindings;
    HGVertexBufferBindings m_lineBBBindings;
    HGVertexBufferBindings m_defaultVao;

    GTextureGL4x *m_lastTexture[10] = {
        nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
        nullptr};

    GUniformBufferGL4x * m_vertexUniformBuffer[3] = {nullptr};
    GUniformBufferGL4x * m_fragmentUniformBuffer[3] = {nullptr};

    HGTexture m_blackPixelTexture;
    HGPUFence m_uniformUploadFence;

protected:
    //Caches
    std::unordered_map<size_t, HGShaderPermutation> m_shaderPermutCache;
    std::list<std::weak_ptr<GUniformBufferGL4x>> m_unfiormBufferCache;
    struct FrameUniformBuffers {
        std::vector<HGUniformBuffer> m_uniformBuffersForUpload;
    };

    struct DrawElementsIndirectCommand
    {
        GLuint  count;
        GLuint primCount;
        GLuint firstIndex;
        GLuint baseVertex;
        GLuint baseInstance;
    };

    FrameUniformBuffers m_UBOFrames[4];

    void * m_indirectBufferPointer;


    std::vector<char> aggregationBufferForUpload;

    bool m_m2ShaderCreated = false;
    int uniformBuffersCreated = 0;
};




#endif //WEBWOWVIEWERCPP_GDEVICE_4X_H
