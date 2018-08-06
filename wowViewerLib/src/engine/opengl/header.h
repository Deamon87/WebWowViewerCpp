#ifndef WOWMAPVIEWERREVIVED_HEADER_H
#define WOWMAPVIEWERREVIVED_HEADER_H

#define GL_GLEXT_PROTOTYPES 1
#ifdef WITH_GLESv2
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3ext.h>
#include <GLES3/gl3platform.h>

#define glGenVertexArrays glGenVertexArraysOES
#define glBindVertexArray glBindVertexArrayOES

#else
#ifdef _WIN32
#undef GLFW_INCLUDE_VULKAN
#include <GL/glew.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>
#endif
#endif

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
struct BlendModeDesc {
    bool blendModeEnable;
    GLuint SrcColor;
    GLuint DestColor;
    GLuint SrcAlpha;
    GLuint DestAlpha;
};

extern BlendModeDesc blendModes[(int)EGxBlendEnum::GxBlend_MAX];

#endif //WOWMAPVIEWERREVIVED_HEADER_H
