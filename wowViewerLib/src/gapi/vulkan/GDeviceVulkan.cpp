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

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

GDeviceVLK::GDeviceVLK(ExtensionsRequired * er) {
    bool enableValidationLayers = false;

    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

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


    //Request validation layers
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        debugCreateInfo = {};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;

        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }


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
