//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GDEVICE20_H
#define WEBWOWVIEWERCPP_GDEVICE20_H

#include <memory>

class GVertexBufferGL20;
class GVertexBufferBindingsGL20;
class GIndexBufferGL20;
class GUniformBufferGL20;
class GBlpTextureGL20;
class GTextureGL20;
class GShaderPermutationGL20;
class GMeshGL20;
class GM2MeshGL20;
class GOcclusionQueryGL20;
class GParticleMeshGL20;


class gMeshTemplate;

typedef std::shared_ptr<GUniformBufferGL20> HGL20UniformBuffer;
typedef std::shared_ptr<GMeshGL20> HGL20Mesh;

#include <unordered_set>
#include <list>
#include <array>
#include "GVertexBufferBindingsGL20.h"
#include "buffers/GIndexBufferGL20.h"
#include "buffers/GVertexBufferGL20.h"
#include "textures/GTextureGL20.h"
#include "textures/GBlpTextureGL20.h"
#include "buffers/GUniformBufferGL20.h"
#include "GShaderPermutationGL20.h"
#include "meshes/GMeshGL20.h"
#include "../interface/IDevice.h"



class GDeviceGL20 : public IDevice {
public:
    GDeviceGL20();
    ~GDeviceGL20() override = default;;

    GDeviceType getDeviceType() override {return GDeviceType::GOpenGL2; };

    void initialize() override;
    void reset() override;

    unsigned int getFrameNumber() { return m_frameNumber; };
    unsigned int getUpdateFrameNumber() ;
    unsigned int getCullingFrameNumber() ;
    unsigned int getOcclusionFrameNumber() ;
    unsigned int getDrawFrameNumber() ;


    void increaseFrameNumber() override;
    bool getIsAsynBuffUploadSupported() override {
        return false;
    }
    int getMaxSamplesCnt() override {
        return 1;
    }

    float getAnisLevel() override;

    void bindProgram(IShaderPermutation *program) override;

    void bindIndexBuffer(IBuffer *buffer) override;
    void bindVertexBuffer(IVertexBuffer *buffer) override;
    void bindUniformBuffer(IUniformBuffer *buffer, int slot, int offset, int length) override;
    void bindVertexBufferBindings(IVertexBufferBindings *buffer) override;

    void bindTexture(ITexture *texture, int slot) override;

    void updateBuffers(std::vector<std::vector<HGUniformBufferChunk>*> &bufferChunks, std::vector<HFrameDependantData> &frameDepedantDataVec);
    void uploadTextureForMeshes(std::vector<HGMesh> &meshes) override;
    void drawMeshes(std::vector<HGMesh> &meshes) override;
    void drawStageAndDeps(HDrawStage drawStage) override;
    //    void drawM2Meshes(std::vector<HGM2Mesh> &meshes);
public:
    std::shared_ptr<IShaderPermutation> getShader(std::string shaderName, void *permutationDescriptor) override;

    HGUniformBuffer createUniformBuffer(size_t size) override;
    HGVertexBuffer createVertexBuffer() override;
    HGVertexBufferDynamic createVertexBufferDynamic(size_t size) override;
    HGIndexBuffer createIndexBuffer() override;
    HGVertexBufferBindings createVertexBufferBindings() override;
    HFrameBuffer createFrameBuffer(int width, int height, std::vector<ITextureFormat> attachments, ITextureFormat depthAttachment, int multiSampleCnt, int frameNumber) override {return nullptr;};

    HGTexture createBlpTexture(HBlpTexture &texture, bool xWrapTex, bool yWrapTex) override;
    HGTexture createTexture(bool xWrapTex, bool yWrapTex) override;
    HGTexture getWhiteTexturePixel() override { return m_whitePixelTexture; };
    HGTexture getBlackTexturePixel() override { return m_blackPixelTexture; };
    HGMesh createMesh(gMeshTemplate &meshTemplate) override;
    HGM2Mesh createM2Mesh(gMeshTemplate &meshTemplate) override;
    HGParticleMesh createParticleMesh(gMeshTemplate &meshTemplate) override;
    HGPUFence createFence() override;

    HGOcclusionQuery createQuery(HGMesh boundingBoxMesh) override;

    HGVertexBufferBindings getBBVertexBinding() override;
    HGVertexBufferBindings getBBLinearBinding() override;

    std::string loadShader(std::string fileName, IShaderType shaderType) override;

    virtual void clearScreen() override;
    void setClearScreenColor(float r, float g, float b) override ;
    void beginFrame() override ;
    void commitFrame() override ;
    void setViewPortDimensions(float x, float y, float width, float height) override;
    void setInvertZ(bool value) override {m_isInvertZ = value;};
    void shrinkData() override;
    bool wasTexturesUploaded() override {
        return false;
    };
private:
    void drawMesh(HGMesh hIMesh, HGUniformBufferChunk matrixChunk);
    bool isDepthPreFill = false;
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
    std::unordered_map<BlpCacheRecord, std::weak_ptr<GTextureGL20>, BlpCacheRecordHasher> loadedTextureCache;

    unsigned int m_frameNumber = 0;

    uint8_t m_lastColorMask = 0xFF;
    int8_t m_lastDepthWrite = -1;
    int8_t m_lastDepthCulling = -1;
    int8_t m_backFaceCulling = -1;
    int8_t m_triCCW = -1;
    int maxUniformBufferSize = -1;
    int uniformBufferOffsetAlign = -1;
    float m_anisotropicLevel = 0.0;
    bool m_isInSkyBoxDepthMode = false;
    bool m_isInvertZ = false;
    EGxBlendEnum m_lastBlendMode = EGxBlendEnum::GxBlend_UNDEFINED;
    GIndexBufferGL20 *m_lastBindIndexBuffer = nullptr;
	IVertexBuffer* m_lastBindVertexBuffer = nullptr;
    GVertexBufferBindingsGL20 *m_vertexBufferBindings = nullptr;
    GShaderPermutationGL20 * m_shaderPermutation = nullptr;

    HGVertexBufferBindings m_vertexBBBindings;
    HGVertexBufferBindings m_lineBBBindings;
    HGVertexBufferBindings m_defaultVao;

    GTextureGL20 *m_lastTexture[10] = {
        nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
        nullptr};

    HGTexture m_blackPixelTexture;
    HGTexture m_whitePixelTexture;

    float clearColor[3] = {0,0,0};

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
    std::list<std::weak_ptr<GUniformBufferGL20>> m_unfiormBufferCache;
    struct FrameUniformBuffers {
        HGUniformBuffer m_uniformBufferForUpload;
    };

    std::array<FrameUniformBuffers, MAX_FRAMES_IN_FLIGHT> m_UBOFrames = {};

    std::vector<char> aggregationBufferForUpload = std::vector<char>(1024*1024);

    std::unordered_map<ShaderContentCacheRecord, std::string, ShaderContentCacheRecordHasher> shaderCache;

    int uniformBuffersCreated = 0;
};




#endif //WEBWOWVIEWERCPP_GDEVICE_H
