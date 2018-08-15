//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GDEVICE_H
#define WEBWOWVIEWERCPP_GDEVICE_H
#include <memory>

class GVertexBuffer;
class GVertexBufferBindings;
class GIndexBuffer;
class GUniformBuffer;
class GBlpTexture;
class GTexture;
class GShaderPermutation;
class GMesh;
class GM2Mesh;
class GParticleMesh;

class gMeshTemplate;

typedef std::shared_ptr<GVertexBuffer> HGVertexBuffer;
typedef std::shared_ptr<GIndexBuffer> HGIndexBuffer;
typedef std::shared_ptr<GVertexBufferBindings> HGVertexBufferBindings;
typedef std::shared_ptr<GUniformBuffer> HGUniformBuffer;
typedef std::shared_ptr<GShaderPermutation> HGShaderPermutation;
typedef std::shared_ptr<GMesh> HGMesh;
typedef std::shared_ptr<GM2Mesh> HGM2Mesh;
typedef std::shared_ptr<GParticleMesh> HGParticleMesh;
typedef std::shared_ptr<GBlpTexture> HGBlpTexture;
typedef std::shared_ptr<GTexture> HGTexture;

#include <unordered_set>
#include <list>
#include "GVertexBufferBindings.h"
#include "GIndexBuffer.h"
#include "GVertexBuffer.h"
#include "GBlpTexture.h"
#include "GTexture.h"
#include "GUniformBuffer.h"
#include "GShaderPermutation.h"
#include "meshes/GMesh.h"


class GDevice {
public:
    GDevice();

    void reset() {
        m_lastDepthWrite = -1;
        m_lastDepthCulling = -1;
        m_backFaceCulling = -1;
        m_lastBlendMode = EGxBlendEnum::GxBlend_UNDEFINED;
        m_lastBindIndexBuffer = nullptr;
        m_lastBindVertexBuffer = nullptr;
        m_vertexBufferBindings = nullptr;

        m_lastTexture[0] = nullptr;
        m_lastTexture[1] = nullptr;
        m_lastTexture[2] = nullptr;
        m_lastTexture[3] = nullptr;

        m_vertexUniformBuffer[0] = nullptr;
        m_vertexUniformBuffer[1] = nullptr;
        m_vertexUniformBuffer[2] = nullptr;

        m_fragmentUniformBuffer[0] = nullptr;
        m_fragmentUniformBuffer[1] = nullptr;
        m_fragmentUniformBuffer[2] = nullptr;

        m_shaderPermutation = nullptr;
    }

    void bindProgram(GShaderPermutation *program);
    void bindIndexBuffer(GIndexBuffer *buffer);
    void bindVertexBuffer(GVertexBuffer *buffer);
    void bindVertexUniformBuffer(GUniformBuffer *buffer, int slot);
    void bindFragmentUniformBuffer(GUniformBuffer *buffer, int slot);
    void bindVertexBufferBindings(GVertexBufferBindings *buffer);

    void bindTexture(GTexture *texture, int slot);

    void updateBuffers(std::vector<HGMesh> &meshes);
    void drawMeshes(std::vector<HGMesh> &meshes);
//    void drawM2Meshes(std::vector<HGM2Mesh> &meshes);
public:
    std::shared_ptr<GShaderPermutation> getShader(std::string shaderName);

    HGUniformBuffer createUniformBuffer(size_t size);
    HGVertexBuffer createVertexBuffer();
    HGIndexBuffer createIndexBuffer();
    HGVertexBufferBindings createVertexBufferBindings();

    HGTexture createBlpTexture(HBlpTexture &texture, bool xWrapTex, bool yWrapTex);
    HGTexture createTexture();
    HGMesh createMesh(gMeshTemplate &meshTemplate);
    HGM2Mesh createM2Mesh(gMeshTemplate &meshTemplate);
    HGParticleMesh createParticleMesh(gMeshTemplate &meshTemplate);

    static bool sortMeshes(const HGMesh& a, const HGMesh& b);

private:
    void drawMesh(HGMesh &hmesh);

private:
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
    std::unordered_map<BlpCacheRecord, std::weak_ptr<GTexture>, BlpCacheRecordHasher> loadedTextureCache;


    int8_t m_lastDepthWrite = -1;
    int8_t m_lastDepthCulling = -1;
    int8_t m_backFaceCulling = -1;
    int maxUniformBufferSize = -1;
    int uniformBufferOffsetAlign = -1;
    EGxBlendEnum m_lastBlendMode = EGxBlendEnum::GxBlend_UNDEFINED;
    GIndexBuffer *m_lastBindIndexBuffer = nullptr;
    GVertexBuffer *m_lastBindVertexBuffer = nullptr;
    GVertexBufferBindings *m_vertexBufferBindings = nullptr;
    GShaderPermutation * m_shaderPermutation = nullptr;

    GTexture *m_lastTexture[10] = {
        nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
        nullptr};

    GUniformBuffer * m_vertexUniformBuffer[3] = {nullptr};
    GUniformBuffer * m_fragmentUniformBuffer[3] = {nullptr};

    HGTexture m_blackPixelTexture;

private:
    //Caches
    std::unordered_map<size_t, HGShaderPermutation> m_shaderPermutCache;
    std::list<std::weak_ptr<GUniformBuffer>> m_unfiormBufferCache;
    std::vector<HGUniformBuffer> m_unfiormBuffersForUpload;
    std::vector<char> aggregationBufferForUpload;

    bool m_m2ShaderCreated = false;
    int uniformBuffersCreated = 0;
};

#endif //WEBWOWVIEWERCPP_GDEVICE_H
