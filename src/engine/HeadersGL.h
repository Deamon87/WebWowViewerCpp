//
// Created by Deamon on 3/12/2018.
//

#ifndef WEBWOWVIEWERCPP_HEADERSGL_H
#define WEBWOWVIEWERCPP_HEADERSGL_H

#ifdef GLES
// Core GLES
    #define glClearDepth glClearDepthf

    // OES_vertex_array_object
    #define glGenVertexArrays glGenVertexArraysOES
    #define glBindVertexArray glBindVertexArrayOES
    #define glDeleteVertexArrays glDeleteVertexArraysOES

    // OES_mapbuffer
    #define glMapBuffer glMapBufferOES
    #define glUnmapBuffer glUnmapBufferOES
    #define GL_WRITE_ONLY GL_WRITE_ONLY_OES

    // EXT_map_buffer_range
    #define glMapBufferRange glMapBufferRangeEXT
    #define GL_MAP_WRITE_BIT GL_MAP_WRITE_BIT_EXT
    #define GL_MAP_INVALIDATE_BUFFER_BIT GL_MAP_INVALIDATE_BUFFER_BIT_EXT

    // EXT_texture_storage
    #define glTexStorage2D glTexStorage2DEXT
    #define GL_LUMINANCE8 GL_LUMINANCE8_EXT
    #define GL_LUMINANCE8_ALPHA8 GL_LUMINANCE8_ALPHA8_EXT

    // GL_OES_packed_depth_stencil
    #define GL_DEPTH_STENCIL GL_DEPTH_STENCIL_OES
    #define GL_UNSIGNED_INT_24_8 GL_UNSIGNED_INT_24_8_OES
    #define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES

    // OES_rgb8_rgba8
    #define GL_RGBA8 GL_RGBA8_OES

    // OES_texture_half_float
    #define GL_HALF_FLOAT GL_HALF_FLOAT_OES

    // EXT_color_buffer_half_float
    #define GL_RGBA16F GL_RGBA16F_EXT

    // EXT_texture_compression_s3tc
    #define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0
    #define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0
    #define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0

    // 3D textures: not supported
    #define GL_TEXTURE_3D 0
    #define glTexImage3D (void)sizeof
    #define glTexSubImage3D (void)sizeof
    #define glTexStorage3D (void)sizeof

    // RG format: not supported
    #define GL_RG 0
    #define GL_RG16 0
#else
// IMG_texture_compression_pvrtc
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG 0
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG 0
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG 0
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0
#endif

#endif //WEBWOWVIEWERCPP_HEADERSGL_H
