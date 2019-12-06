//
// Created by Deamon on 7/23/2018.
//
#include <iostream>
#include "../GDeviceGL20.h"
#include "GTextureGL20.h"
#include "../../../engine/opengl/header.h"
#include "../../interface/IDevice.h"

GTextureGL20::GTextureGL20(IDevice &device) : m_device(device) {
    createBuffer();
}

GTextureGL20::~GTextureGL20() {
    destroyBuffer();
}

void GTextureGL20::createBuffer() {
    glGenTextures(1, &textureIdentifier);
}

void GTextureGL20::destroyBuffer() {
    glDeleteTextures(1, &textureIdentifier);
}

void GTextureGL20::bind() {
    glBindTexture(GL_TEXTURE_2D, textureIdentifier);

}

void GTextureGL20::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool GTextureGL20::getIsLoaded() {
    return true;
}

static int pureTexturesUploaded = 0;
void GTextureGL20::loadData(int width, int height, void *data) {
//    std::cout << "pureTexturesUploaded = " << pureTexturesUploaded++ << std::endl;

    m_device.bindTexture(this, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenerateMipmap(GL_TEXTURE_2D);

    m_device.bindTexture(nullptr, 0);
}


