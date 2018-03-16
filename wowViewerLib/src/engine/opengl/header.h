#ifndef WOWMAPVIEWERREVIVED_HEADER_H
#define WOWMAPVIEWERREVIVED_HEADER_H

#define GL_GLEXT_PROTOTYPES 1
#ifdef WITH_GLESv2
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>

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
