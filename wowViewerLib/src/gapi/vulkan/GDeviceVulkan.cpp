//
// Created by Deamon on 8/1/2019.
//

#include <iostream>
#include <cstring>
#include "GDeviceVulkan.h"
#include "../../include/vulkancontext.h"

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};


GDeviceVLK::GDeviceVLK(ExtensionsRequired * er) {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    createInfo.enabledExtensionCount = er->extensionCnt;
    createInfo.ppEnabledExtensionNames = er->extensions;

    createInfo.enabledLayerCount = 0;

    if (vkCreateInstance(&createInfo, nullptr, &vkInstance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

unsigned int GDeviceVLK::getDrawFrameNumber() {
    return 0;
}

unsigned int GDeviceVLK::getUpdateFrameNumber() {
    return 0;
}

unsigned int GDeviceVLK::getCullingFrameNumber() {
    return 0;
}

void GDeviceVLK::increaseFrameNumber() {

}

float GDeviceVLK::getAnisLevel() {
    return 0;
}

void GDeviceVLK::bindProgram(IShaderPermutation *program) {

}

void GDeviceVLK::bindIndexBuffer(IIndexBuffer *buffer) {

}

void GDeviceVLK::bindVertexBuffer(IVertexBuffer *buffer) {

}

void GDeviceVLK::bindVertexUniformBuffer(IUniformBuffer *buffer, int slot) {

}

void GDeviceVLK::bindFragmentUniformBuffer(IUniformBuffer *buffer, int slot) {

}

void GDeviceVLK::bindVertexBufferBindings(IVertexBufferBindings *buffer) {

}

void GDeviceVLK::bindTexture(ITexture *texture, int slot) {

}

void GDeviceVLK::updateBuffers(std::vector<HGMesh> &meshes) {

}

void GDeviceVLK::uploadTextureForMeshes(std::vector<HGMesh> &meshes) {

}

void GDeviceVLK::drawMeshes(std::vector<HGMesh> &meshes) {

}

std::shared_ptr<IShaderPermutation> GDeviceVLK::getShader(std::string shaderName, void *permutationDescriptor) {
    return std::shared_ptr<IShaderPermutation>();
}

HGUniformBuffer GDeviceVLK::createUniformBuffer(size_t size) {
    return HGUniformBuffer();
}

HGVertexBuffer GDeviceVLK::createVertexBuffer() {
    return HGVertexBuffer();
}

HGIndexBuffer GDeviceVLK::createIndexBuffer() {
    return HGIndexBuffer();
}

HGVertexBufferBindings GDeviceVLK::createVertexBufferBindings() {
    return HGVertexBufferBindings();
}

HGTexture GDeviceVLK::createBlpTexture(HBlpTexture &texture, bool xWrapTex, bool yWrapTex) {
    return HGTexture();
}

HGTexture GDeviceVLK::createTexture() {
    return HGTexture();
}

HGMesh GDeviceVLK::createMesh(gMeshTemplate &meshTemplate) {
    return HGMesh();
}

HGM2Mesh GDeviceVLK::createM2Mesh(gMeshTemplate &meshTemplate) {
    return HGM2Mesh();
}

HGParticleMesh GDeviceVLK::createParticleMesh(gMeshTemplate &meshTemplate) {
    return HGParticleMesh();
}

HGPUFence GDeviceVLK::createFence() {
    return HGPUFence();
}

HGOcclusionQuery GDeviceVLK::createQuery(HGMesh boundingBoxMesh) {
    return HGOcclusionQuery();
}

HGVertexBufferBindings GDeviceVLK::getBBVertexBinding() {
    return HGVertexBufferBindings();
}

HGVertexBufferBindings GDeviceVLK::getBBLinearBinding() {
    return HGVertexBufferBindings();
}

std::string GDeviceVLK::loadShader(std::string fileName, bool common) {
    return std::__cxx11::string();
}

void GDeviceVLK::drawMesh(HGMesh &hmesh) {

}

void GDeviceVLK::reset() {

}

VkInstance GDeviceVLK::getVkInstance() {
    return vkInstance;
}

void GDeviceVLK::clearScreen() {

}
