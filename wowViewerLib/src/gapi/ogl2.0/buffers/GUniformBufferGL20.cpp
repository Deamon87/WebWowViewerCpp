//
// Created by Deamon on 6/30/2018.
//
#include <memory.h>
#include <iostream>
#include "../../../engine/opengl/header.h"
#include "GUniformBufferGL20.h"
#include "../../interface/IDevice.h"

GUniformBufferGL20::GUniformBufferGL20(IDevice &device, size_t size) : m_device(device){

}

GUniformBufferGL20::~GUniformBufferGL20() {

}

void GUniformBufferGL20::createBuffer() {

}

void GUniformBufferGL20::destroyBuffer() {
}

void GUniformBufferGL20::unbind() {
}

void GUniformBufferGL20::uploadData(void * data, int length) {
        dataPtr = data;
}

