#ifndef WOWMAPVIEWERREVIVED_HEADER_H
#define WOWMAPVIEWERREVIVED_HEADER_H

#define GL_GLEXT_PROTOTYPES 1
#if defined(WITH_GLESv2) || defined(EMSCRIPTEN)
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>

#ifndef EMSCRIPTEN
#include <GLES3/gl3ext.h>
#include <GLES3/gl3platform.h>
#else
#define GL_BGRA GL_BGRA_EXT
#define glClearDepth glClearDepthf
#define glDepthRange glDepthRangef
#endif

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



#endif //WOWMAPVIEWERREVIVED_HEADER_H
