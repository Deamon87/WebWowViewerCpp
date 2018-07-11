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
class GTexture;
class GShaderPermutation;
class GMesh;

class gMeshTemplate;

typedef std::shared_ptr<GVertexBuffer> HGVertexBuffer;
typedef std::shared_ptr<GIndexBuffer> HGIndexBuffer;
typedef std::shared_ptr<GVertexBufferBindings> HGVertexBufferBindings;
typedef std::shared_ptr<GUniformBuffer> HGUniformBuffer;
typedef std::shared_ptr<GShaderPermutation> HGShaderPermutation;
typedef std::shared_ptr<GMesh> HGMesh;
typedef std::shared_ptr<GTexture> HGTexture;

#include <unordered_set>
#include <list>
#include "GVertexBufferBindings.h"
#include "GIndexBuffer.h"
#include "GVertexBuffer.h"
#include "GTexture.h"
#include "GUniformBuffer.h"
#include "GShaderPermutation.h"
#include "GMesh.h"


class GDevice {
public:
    void reset() {
        m_lastDepthWrite = false;
        m_lastDepthCulling = false;
        //m_lastBlendMode = EGxBlendEnum::GxBlend_UNDEFINED;
        m_lastBindIndexBuffer = nullptr;
        m_lastBindVertexBuffer = nullptr;
        m_vertexBufferBindings = nullptr;

        m_lastTexture[0] = nullptr;
        m_lastTexture[1] = nullptr;
        m_lastTexture[2] = nullptr;
        m_lastTexture[3] = nullptr;

        m_uniformBuffer[0] = nullptr;
        m_uniformBuffer[1] = nullptr;
        m_uniformBuffer[2] = nullptr;
    }

    void bindIndexBuffer(GIndexBuffer *buffer);
    void bindVertexBuffer(GVertexBuffer *buffer);
    void bindUniformBuffer(GUniformBuffer *buffer, int slot);
    void bindVertexBufferBindings(GVertexBufferBindings *buffer);

    void bindTexture(GTexture *texture, int slot);

    void drawMeshes(std::vector<GMesh *> &meshes);
public:
    std::shared_ptr<GShaderPermutation> getShader(std::string shaderName);

    HGUniformBuffer createUniformBuffer(size_t size);
    HGVertexBuffer createVertexBuffer();
    HGIndexBuffer createIndexBuffer();
    HGVertexBufferBindings createVertexBufferBindings();

    HGTexture createTexture(HBlpTexture &texture);
    HGMesh createMesh(gMeshTemplate &meshTemplate);
private:
    bool m_lastDepthWrite = false;
    bool m_lastDepthCulling = false;
    bool m_backFaceCulling = false;
    EGxBlendEnum m_lastBlendMode = EGxBlendEnum::GxBlend_UNDEFINED;
    GIndexBuffer *m_lastBindIndexBuffer = nullptr;
    GVertexBuffer *m_lastBindVertexBuffer = nullptr;
    GVertexBufferBindings *m_vertexBufferBindings = nullptr;

    GTexture *m_lastTexture[4] = {nullptr};

    GUniformBuffer * m_uniformBuffer[3] = {nullptr};

private:
    //Caches
    std::unordered_map<size_t, HGShaderPermutation> m_shaderPermutCache;
    std::list<std::weak_ptr<GUniformBuffer>> m_unfiormBufferCache;
};



#endif //WEBWOWVIEWERCPP_GDEVICE_H
