//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GDEVICE_H
#define WEBWOWVIEWERCPP_GDEVICE_H
class GVertexBuffer;
class GVertexBufferBindings;
class GIndexBuffer;

#include "GVertexBufferBindings.h"
#include "GIndexBuffer.h"
#include "GVertexBuffer.h"
#include "GTexture.h"
#include "GUniformBuffer.h"

enum class EGxBlendEnum {
    GxBlend_UNDEFINED = -1,
    GxBlend_Opaque = 0,
    GxBlend_AlphaKey = 1,
    GxBlend_Alpha = 2,
    GxBlend_Add = 3,
    GxBlend_Mod = 4,
    GxBlend_Mod2x = 5,
    GxBlend_ModAdd = 6,
    GxBlend_InvSrcAlphaAdd = 7,
    GxBlend_InvSrcAlphaOpaque = 8,
    GxBlend_SrcAlphaOpaque = 9,
    GxBlend_NoAlphaAdd = 10,
    GxBlend_ConstantAlpha = 11,
    GxBlend_Screen = 12,
    GxBlend_BlendAdd = 13,
    GxBlend_MAX
};

class GDevice {
public:
    void reset() {
        m_lastDepthWrite = false;
        m_lastDepthCulling = false;
        m_lastBlendMode = EGxBlendEnum::GxBlend_UNDEFINED;
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

private:
    bool m_lastDepthWrite = false;
    bool m_lastDepthCulling = false;
    EGxBlendEnum m_lastBlendMode = EGxBlendEnum::GxBlend_UNDEFINED;
    GIndexBuffer *m_lastBindIndexBuffer = nullptr;
    GVertexBuffer *m_lastBindVertexBuffer = nullptr;
    GVertexBufferBindings *m_vertexBufferBindings = nullptr;

    GTexture *m_lastTexture1 = nullptr;
    GTexture *m_lastTexture2 = nullptr;
    GTexture *m_lastTexture3 = nullptr;

    GUniformBuffer * m_uniformBuffer[3] = {nullptr};

};



#endif //WEBWOWVIEWERCPP_GDEVICE_H
