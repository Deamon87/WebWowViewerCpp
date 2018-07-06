//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GDEVICE_H
#define WEBWOWVIEWERCPP_GDEVICE_H
class GVertexBuffer;
class GVertexBufferBindings;
class GIndexBuffer;
class GUniformBuffer;
class GShaderPermutation;

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

        m_lastTexture1 = nullptr;
        m_lastTexture2 = nullptr;
        m_lastTexture3 = nullptr;

        m_uniformBuffer[0] = nullptr;
        m_uniformBuffer[1] = nullptr;
        m_uniformBuffer[2] = nullptr;
    }

    void bindIndexBuffer(GIndexBuffer *buffer);
    void bindVertexBuffer(GVertexBuffer *buffer);
    void bindUniformBuffer(GUniformBuffer *buffer, int slot);
    void bindVertexBufferBindings(GVertexBufferBindings *buffer);


    void drawMeshes(std::vector<GMesh *> &meshes);
public:
    GShaderPermutation *getShader(std::string shaderName);
    GUniformBuffer &createUniformBuffer();

private:
    bool m_lastDepthWrite = false;
    bool m_lastDepthCulling = false;
   // EGxBlendEnum m_lastBlendMode = EGxBlendEnum::GxBlend_UNDEFINED;
    GIndexBuffer *m_lastBindIndexBuffer = nullptr;
    GVertexBuffer *m_lastBindVertexBuffer = nullptr;
    GVertexBufferBindings *m_vertexBufferBindings = nullptr;

    GTexture *m_lastTexture1 = nullptr;
    GTexture *m_lastTexture2 = nullptr;
    GTexture *m_lastTexture3 = nullptr;

    GUniformBuffer * m_uniformBuffer[3] = {nullptr};

private:
    //Caches
    std::unordered_map<size_t, GShaderPermutation> m_shaderPermutCache;
    std::list<std::weak_ptr<GUniformBuffer>> m_unfiormBufferCache;

};



#endif //WEBWOWVIEWERCPP_GDEVICE_H
