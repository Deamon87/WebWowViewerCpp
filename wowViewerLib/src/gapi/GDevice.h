//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GDEVICE_H
#define WEBWOWVIEWERCPP_GDEVICE_H

#include "GVertexBufferBindings.h"
#include "GIndexBuffer.h"
#include "GVertexBuffer.h"

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
    struct BlendModeDesc {
        bool blendModeEnable;
        GLuint SrcColor;
        GLuint DestColor;
        GLuint SrcAlpha;
        GLuint DestAlpha;
    };

    BlendModeDesc blendModes[(int)EGxBlendEnum::GxBlend_MAX] = {
        {false,GL_ONE,GL_ZERO,GL_ONE,GL_ZERO},
        {false,GL_ONE,GL_ZERO,GL_ONE,GL_ZERO},
        {true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA},
        {true,GL_SRC_ALPHA,GL_ONE,GL_ZERO,GL_ONE},
        {true,GL_DST_COLOR,GL_ZERO,GL_DST_ALPHA,GL_ZERO},
        {true,GL_DST_COLOR,GL_SRC_COLOR,GL_DST_ALPHA,GL_SRC_ALPHA},
        {true,GL_DST_COLOR,GL_ONE,GL_DST_ALPHA,GL_ONE},
        {true,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE},
        {true,GL_ONE_MINUS_SRC_ALPHA,GL_ZERO,GL_ONE_MINUS_SRC_ALPHA,GL_ZERO},
        {true,GL_SRC_ALPHA,GL_ZERO,GL_SRC_ALPHA,GL_ZERO},
        {true,GL_CONSTANT_ALPHA,GL_ONE_MINUS_CONSTANT_ALPHA,GL_CONSTANT_ALPHA,GL_ONE_MINUS_CONSTANT_ALPHA},
        {true,GL_ONE_MINUS_DST_COLOR,GL_ONE,GL_ONE,GL_ZERO},
        {true,GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA}
    };

public:
    void bindIndexBuffer(GIndexBuffer *buffer);
    void bindVertexBuffer(GVertexBuffer *buffer);
    void bindVertexBufferBindings(GVertexBufferBindings *buffer);
    void

private:
    bool m_depthWrite;
    bool m_depthCulling;
    int blendMode;
    GIndexBuffer *m_lastBindIndexBuffer;
    GVertexBuffer *m_lastBindVertexBuffer;
    GVertexBufferBindings *m_vertexBufferBindings;
};

#endif //WEBWOWVIEWERCPP_GDEVICE_H
