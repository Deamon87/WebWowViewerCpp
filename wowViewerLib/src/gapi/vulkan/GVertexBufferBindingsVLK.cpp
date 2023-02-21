//
// Created by deamon on 05.06.18.
//

#include <iostream>
#include "GVertexBufferBindingsVLK.h"
#include "../interface/IDevice.h"

GVertexBufferBindingsVLK::GVertexBufferBindingsVLK()  {

}

GVertexBufferBindingsVLK::~GVertexBufferBindingsVLK() {

}

VkFormat gBindingToVkFormat(GBindingType gType, uint32_t size, bool normalized ) {
    switch(gType) {
        case GBindingType::GFLOAT :
            switch (size) {
                case 1:
                    return VK_FORMAT_R32_SFLOAT;
                case 2:
                    return VK_FORMAT_R32G32_SFLOAT;
                case 3:
                    return VK_FORMAT_R32G32B32_SFLOAT;
                case 4:
                    return VK_FORMAT_R32G32B32A32_SFLOAT;
                default:
                    throw std::runtime_error("unknown gBindingFormat");
            }
        case GBindingType::GUNSIGNED_BYTE :
            if (normalized) {
                switch (size) {
                    case 1:
                        return VK_FORMAT_R8_UNORM;
                    case 2:
                        return VK_FORMAT_R8G8_UNORM;
                    case 3:
                        return VK_FORMAT_R8G8B8_UNORM;
                    case 4:
                        return VK_FORMAT_R8G8B8A8_UNORM;
                    default:
                        throw std::runtime_error("unknown gBindingFormat");
                }
            } else {
                switch (size) {
                    case 1:
                        return VK_FORMAT_R8_UINT;
                    case 2:
                        return VK_FORMAT_R8G8_UINT;
                    case 3:
                        return VK_FORMAT_R8G8B8_UINT;
                    case 4:
                        return VK_FORMAT_R8G8B8A8_UINT;
                    default:
                        throw std::runtime_error("unknown gBindingFormat");
                }
            }
    }
    throw std::runtime_error("unknown gBindingFormat");
    return VK_FORMAT_UNDEFINED;
}


void GVertexBufferBindingsVLK::setIndexBuffer(HGIndexBuffer indexBuffer) {
    m_indexBuffer = indexBuffer;
}

void GVertexBufferBindingsVLK::addVertexBufferBinding(const HGVertexBuffer &vertexBuffer, const std::vector<GBufferBinding> &bindings) {
    int bindingIdx = 0;
//    for (auto &gVertexBinding : m_bindings) {
        uint32_t stride = 0;
        if (bindings[0].stride != 0) {
            stride = bindings[0].stride;
        } else {
            stride = bindings[0].size * ((bindings[0].type == GBindingType::GFLOAT) ? 4 : 1);
        }

        vkBufferFormatHolder bufferFormatHolder;
        bufferFormatHolder.bindingDescription.binding = bindingIdx;
        bufferFormatHolder.bindingDescription.stride = stride;
        bufferFormatHolder.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        for (auto &gBinding : bindings) {
            VkVertexInputAttributeDescription attributeDescription = {};
            attributeDescription.binding = bindingIdx;
            attributeDescription.location = gBinding.position;
            attributeDescription.format = gBindingToVkFormat(gBinding.type, gBinding.size, gBinding.normalized);
            attributeDescription.offset = gBinding.offset;

            bufferFormatHolder.attributeDescription.push_back(attributeDescription);
        }
        bindingIdx++;

        m_BufferBindingsVLK.push_back(bufferFormatHolder);
        m_vertexBuffers.push_back(vertexBuffer);
//    }
}

void GVertexBufferBindingsVLK::save() {
    //Not used in VULKAN
}

std::vector<vkBufferFormatHolder> &GVertexBufferBindingsVLK::getVLKFormat() {
    return m_BufferBindingsVLK;
}



