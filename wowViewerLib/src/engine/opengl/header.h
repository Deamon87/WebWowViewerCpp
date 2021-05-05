#ifndef WOWMAPVIEWERREVIVED_HEADER_H
#define WOWMAPVIEWERREVIVED_HEADER_H

#define GL_GLEXT_PROTOTYPES 1
#ifdef __ANDROID__
#define WITH_GLESv2 1
#endif

#if defined(WITH_GLESv2) || defined(__EMSCRIPTEN__)
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>


#define GL_BGRA GL_BGRA_EXT
#define glClearDepth glClearDepthf
#define glDepthRange glDepthRangef

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
