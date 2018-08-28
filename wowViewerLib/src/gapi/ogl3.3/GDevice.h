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
class GOcclusionQuery;
class GParticleMesh;


class gMeshTemplate;

typedef std::shared_ptr<GVertexBuffer> HGVertexBuffer;
typedef std::shared_ptr<GIndexBuffer> HGIndexBuffer;
typedef std::shared_ptr<GVertexBufferBindings> HGVertexBufferBindings;
typedef std::shared_ptr<GUniformBuffer> HGUniformBuffer;
typedef std::shared_ptr<GShaderPermutation> HGShaderPermutation;
typedef std::shared_ptr<GMesh> HGMesh;
typedef std::shared_ptr<GM2Mesh> HGM2Mesh;
typedef std::shared_ptr<GOcclusionQuery> HGOcclusionQuery;
typedef std::shared_ptr<GParticleMesh> HGParticleMesh;
typedef std::shared_ptr<GBlpTexture> HGBlpTexture;
typedef std::shared_ptr<GTexture> HGTexture;


#include <unordered_set>
#include <list>
#include "GVertexBufferBindings.h"
#include "buffers/GIndexBuffer.h"
#include "buffers/GVertexBuffer.h"
#include "textures/GBlpTexture.h"
#include "textures/GTexture.h"
#include "buffers/GUniformBuffer.h"
#include "GShaderPermutation.h"
#include "meshes/GMesh.h"
#include "../interface/IDevice.h"


class GDeviceGL33 : public IDevice {
public:
    GDeviceGL33();

    void reset() override;

    bool getIsEvenFrame() override;

    void toogleEvenFrame() override;

    void bindProgram(GShaderPermutation *program) override;

    void bindVertexBuffer(GVertexBuffer *buffer) override;
    void bindVertexUniformBuffer(GUniformBuffer *buffer, int slot) override;
    void bindFragmentUniformBuffer(GUniformBuffer *buffer, int slot) override;
    void bindVertexBufferBindings(GVertexBufferBindings *buffer) override;

    void bindTexture(GTexture *texture, int slot) override;

    void updateBuffers(std::vector<HGMesh> &meshes) override;
    void drawMeshes(std::vector<HGMesh> &meshes) override;
    //    void drawM2Meshes(std::vector<HGM2Mesh> &meshes);
public:
    std::shared_ptr<GShaderPermutation> getShader(std::string shaderName) override;

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

    static bool sortMeshes(const HGMesh& a, const HGMesh& b) override;
    HGVertexBufferBindings getBBVertexBinding() override;
    HGVertexBufferBindings getBBLinearBinding() override;

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

    bool m_isEvenFrame = false;

    uint8_t m_lastColorMask = 0xFF;
    int8_t m_lastDepthWrite = -1;
    int8_t m_lastDepthCulling = -1;
    int8_t m_backFaceCulling = -1;
    int8_t m_triCCW = -1;
    int maxUniformBufferSize = -1;
    int uniformBufferOffsetAlign = -1;
    EGxBlendEnum m_lastBlendMode = EGxBlendEnum::GxBlend_UNDEFINED;
    GIndexBuffer *m_lastBindIndexBuffer = nullptr;
    GVertexBuffer *m_lastBindVertexBuffer = nullptr;
    GVertexBufferBindings *m_vertexBufferBindings = nullptr;
    GShaderPermutation * m_shaderPermutation = nullptr;

    HGVertexBufferBindings m_vertexBBBindings;
    HGVertexBufferBindings m_lineBBBindings;
    HGVertexBufferBindings m_defaultVao;

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
    struct FrameUniformBuffers {
        std::vector<HGUniformBuffer> m_unfiormBuffersForUpload;
    };
    FrameUniformBuffers m_firstUBOFrame;
    FrameUniformBuffers m_secondUBOFrame;

    std::vector<char> aggregationBufferForUpload;

    bool m_m2ShaderCreated = false;
    int uniformBuffersCreated = 0;
};




#endif //WEBWOWVIEWERCPP_GDEVICE_H
