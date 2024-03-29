//
// Created by Deamon on 7/23/2018.
//
#include <iostream>
#include "../GDeviceGL33.h"
#include "GTextureGL33.h"
#include "../../../engine/opengl/header.h"
#include "../../interface/IDevice.h"

int GTextureGL33::currentGLTexturesAllocated = 0;

GTextureGL33::GTextureGL33(const HGDevice &device, bool xWrapTex, bool yWrapTex) : m_device(device) {
    this->xWrapTex = xWrapTex;
    this->yWrapTex = yWrapTex;

    createBuffer();
}

GTextureGL33::~GTextureGL33() {
    destroyBuffer();
}

void GTextureGL33::createBuffer() {
    logGLError
    glGenTextures(1, &textureIdentifier);
    logGLError
    GTextureGL33::currentGLTexturesAllocated++;
}

void GTextureGL33::destroyBuffer() {
    const GLuint indent = textureIdentifier;
    m_device->addDeallocationRecord([indent]() -> void {
        glDeleteTextures(1, &indent);
        GTextureGL33::currentGLTexturesAllocated--;
    });
}

void GTextureGL33::bind() {
    logGLError
    glBindTexture(GL_TEXTURE_2D, textureIdentifier);
    logGLError
}

void GTextureGL33::unbind() {
    logGLError
    glBindTexture(GL_TEXTURE_2D, 0);
    logGLError
}

bool GTextureGL33::getIsLoaded() {
    return true;
}

static int pureTexturesUploaded = 0;
void GTextureGL33::loadData(int width, int height, void *data, ITextureFormat textureFormat) {
//    std::cout << "pureTexturesUploaded = " << pureTexturesUploaded++ << std::endl;
    this->width = width;
    this->height = height;


    m_device->bindTexture(this, 0);
    if (textureFormat == ITextureFormat::itRGBA) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    } else if (textureFormat == ITextureFormat::itRGBAFloat32)  {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data);
    }else if (textureFormat == ITextureFormat::itDepth32)  {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, data);
    }

    if (data != nullptr) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, (GLint) 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, (GLint) 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    if (xWrapTex) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    }
    if (yWrapTex) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    if (data != nullptr) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    m_device->bindTexture(nullptr, 0);

    m_loaded = true;
}

void GTextureGL33::readData(std::vector<uint8_t> &buff) {
#if !(defined(__EMSCRIPTEN__) || defined(WITH_GLESv2))
    if (buff.size() < width*height*4) {
    }

    m_device->bindTexture(this, 0);

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_BYTE, buff.data());


    m_device->bindTexture(nullptr, 0);
#else
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->textureIdentifier, 0);

    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buff.data());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
#endif
}

void GTextureGL33::bindToCurrentFrameBufferAsColor(uint8_t attachmentIndex) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+attachmentIndex, GL_TEXTURE_2D, this->textureIdentifier, 0);
}
void GTextureGL33::bindToCurrentFrameBufferAsDepth() {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, this->textureIdentifier, 0);
}


