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

enum class EGxBlendEnum {
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
    void bindIndexBuffer(GIndexBuffer *buffer);
    void bindVertexBuffer(GVertexBuffer *buffer);
    void bindVertexBufferBindings(GVertexBufferBindings *buffer);

private:
    bool m_lastDepthWrite;
    bool m_lastDepthCulling;
    int m_lastBlendMode;
    GIndexBuffer *m_lastBindIndexBuffer;
    GVertexBuffer *m_lastBindVertexBuffer;
    GVertexBufferBindings *m_vertexBufferBindings;

    GTexture m_lastTexture1;
    GTexture m_lastTexture2;
    GTexture m_lastTexture3;
};

#endif //WEBWOWVIEWERCPP_GDEVICE_H
