//
// Created by Deamon on 7/23/2018.
//
#include "../GDevice.h"
#include "GTexture.h"
#include "../../../engine/opengl/header.h"
#include "../../interface/IDevice.h"

GTexture::GTexture(IDevice &device) : m_device(device) {
    pIdentifierBuffer = new GLuint;
    createBuffer();
}

GTexture::~GTexture() {
    destroyBuffer();
    delete (GLuint *) pIdentifierBuffer;
}

void GTexture::createBuffer() {
    glGenTextures(1, (GLuint *)pIdentifierBuffer);
}

void GTexture::destroyBuffer() {
    glDeleteTextures(1, (GLuint *)pIdentifierBuffer);
}

void GTexture::bind() {
    glBindTexture(GL_TEXTURE_2D, *(GLuint *)pIdentifierBuffer);

}

void GTexture::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool GTexture::getIsLoaded() {
    return true;
}

void GTexture::loadData(int width, int height, void *data) {
    m_device.bindTexture(this, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenerateMipmap(GL_TEXTURE_2D);

    m_device.bindTexture(nullptr, 0);
}


