//
// Created by Deamon on 7/23/2018.
//
#include <iostream>
#include "GTextureVLK.h"
#include "../../interface/IDevice.h"

GTextureVLK::GTextureVLK(IDevice &device) : m_device(device) {
    createBuffer();
}

GTextureVLK::~GTextureVLK() {
    destroyBuffer();
}

void GTextureVLK::createBuffer() {
}

void GTextureVLK::destroyBuffer() {
}

void GTextureVLK::bind() {

}

void GTextureVLK::unbind() {
}

bool GTextureVLK::getIsLoaded() {
    return true;
}

static int pureTexturesUploaded = 0;
void GTextureVLK::loadData(int width, int height, void *data) {
//    std::cout << "pureTexturesUploaded = " << pureTexturesUploaded++ << std::endl;

    m_device.bindTexture(this, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D);

    m_device.bindTexture(nullptr, 0);
}


