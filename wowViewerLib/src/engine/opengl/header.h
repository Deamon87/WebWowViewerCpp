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
#define GL_DEBUG_OUTPUT GL_DEBUG_OUTPUT_KHR
#define GL_DEBUG_OUTPUT_SYNCHRONOUS GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR
#define glDebugMessageControl glDebugMessageControlKHR
#define glDebugMessageCallback glDebugMessageCallbackKHR

#define glPushDebugGroup glPushDebugGroupKHR
#define glPopDebugGroup glPopDebugGroupKHR
#define glDebugMessageInsert glDebugMessageInsertKHR
#define GL_DEBUG_SOURCE_APPLICATION GL_DEBUG_SOURCE_APPLICATION_KHR
#define GL_DEBUG_TYPE_MARKER GL_DEBUG_TYPE_MARKER_KHR
#define GL_DEBUG_SEVERITY_LOW GL_DEBUG_SEVERITY_LOW_KHR
#define GL_DEBUG_SEVERITY_LOW GL_DEBUG_SEVERITY_LOW_KHR

#else
#ifdef _WIN32
#undef GLFW_INCLUDE_VULKAN
#include <GL/glew.h>
#elif defined(__APPLE__)
#include <OpenGL/gl3.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>
#endif
#endif



#endif //WOWMAPVIEWERREVIVED_HEADER_H
