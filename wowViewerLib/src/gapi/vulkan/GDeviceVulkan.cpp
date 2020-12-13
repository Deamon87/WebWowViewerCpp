//
// Created by Deamon on 8/1/2019.
//

#include <iostream>
#include <cstring>
#include <set>
#include <vector>
#include <algorithm>
#include <ctime>
#include <array>
#include "GDeviceVulkan.h"
#include "../../include/vulkancontext.h"

#include "meshes/GM2MeshVLK.h"
#include "meshes/GMeshVLK.h"
#include "buffers/GUniformBufferVLK.h"
#include "buffers/GVertexBufferVLK.h"
#include "buffers/GVertexBufferDynamicVLK.h"
#include "buffers/GIndexBufferVLK.h"
#include "textures/GTextureVLK.h"
#include "textures/GBlpTextureVLK.h"
#include "GVertexBufferBindingsVLK.h"
#include "GPipelineVLK.h"
#include "shaders/GM2ShaderPermutationVLK.h"
#include "shaders/GM2ParticleShaderPermutationVLK.h"
#include "../../engine/algorithms/hashString.h"
#include "shaders/GAdtShaderPermutationVLK.h"
#include "shaders/GWMOShaderPermutationVLK.h"
#include "shaders/GWaterShaderPermutation.h"
#include "shaders/GImguiShaderPermutation.h"
#include "shaders/GM2RibbonShaderPermutationVLK.h"
#include "shaders/GSkyConusShaderVLK.h"
#include "GFrameBufferVLK.h"
#include "shaders/GFFXgauss4VLK.h"
#include "shaders/GFFXGlowVLK.h"
//#include "fastmemcp.h"

const int WIDTH = 1900;
const int HEIGHT = 1000;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl << std::flush;

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}


bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strncmp(layerName, layerProperties.layerName, 255) == 0) {
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

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    VkExtent2D actualExtent;
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        actualExtent = capabilities.currentExtent;
        std::cout << "capabilities.currentExtent.width = " << capabilities.currentExtent.width
            << " capabilities.currentExtent.height = " << capabilities.currentExtent.height << std::endl;
    } else {
        actualExtent = {WIDTH, HEIGHT};
    }

    std::cout << "capabilities.minImageExtent.width = " << capabilities.minImageExtent.width
              << " capabilities.minImageExtent.height = " << capabilities.minImageExtent.height << std::endl;
    std::cout << "capabilities.maxImageExtent.width = " << capabilities.maxImageExtent.width
              << " capabilities.maxImageExtent.height = " << capabilities.maxImageExtent.height << std::endl;

    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

    std::cout << "actualExtent.width = " << actualExtent.width
              << " actualExtent.height = " << actualExtent.height << std::endl;

    return actualExtent;
}

GDeviceVLK::SwapChainSupportDetails GDeviceVLK::querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vkSurface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkSurface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkSurface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkSurface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkSurface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.pNext = NULL;
    createInfo.flags = NULL;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = 0;
}


GDeviceVLK::GDeviceVLK(vkCallInitCallback * callback) {
    enableValidationLayers = false;

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
    createInfo.pNext = NULL;
    createInfo.pApplicationInfo = &appInfo;

    char** extensions;
    int extensionCnt = 0;
    callback->getRequiredExtensions(extensions, extensionCnt);
    std::vector<char *> extensionsVec(extensions, extensions+extensionCnt);
    if (enableValidationLayers) {
        extensionsVec.push_back("VK_EXT_debug_report");
        extensionsVec.push_back("VK_EXT_debug_utils");
    }

    createInfo.enabledExtensionCount = extensionsVec.size();
    createInfo.ppEnabledExtensionNames = &extensionsVec[0];


    //Request validation layers
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);

        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &vkInstance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

    //Create surface
    vkSurface = callback->createSurface(vkInstance);

    setupDebugMessenger();

    pickPhysicalDevice();
    createLogicalDevice();
//---------------
    //Init AMD's VMA
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = vkInstance;


    vmaCreateAllocator(&allocatorInfo, &vmaAllocator);
//---------------

    createSwapChain();
    createImageViews();

    createRenderPass();

    createDepthResources();
    createFramebuffers();
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();

    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    uniformBufferOffsetAlign = deviceProperties.limits.minUniformBufferOffsetAlignment;
    maxUniformBufferSize = deviceProperties.limits.maxUniformBufferRange;

    std::cout << "uniformBufferOffsetAlign = " << uniformBufferOffsetAlign << std::endl;
    std::cout << "maxUniformBufferSize = " << maxUniformBufferSize << std::endl;

    // Create pool
//    VkBufferCreateInfo exampleBufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
//    exampleBufCreateInfo.size = 65536; // Whatever.
//    exampleBufCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT; // Change if needed.
//
//    VmaAllocationCreateInfo allocCreateInfo = {};
//    allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY; // Change if needed.
//
//    uint32_t memTypeIndex;
//    vmaFindMemoryTypeIndexForBufferInfo(vmaAllocator, &exampleBufCreateInfo, &allocCreateInfo, &memTypeIndex);
//
//    // Create a pool that can have at most 2 blocks, 128 MiB each.
//    VmaPoolCreateInfo poolCreateInfo = {};
//    poolCreateInfo.memoryTypeIndex = memTypeIndex;
//    poolCreateInfo.blockSize = 128ull * 1024 * 1024;
//    poolCreateInfo.maxBlockCount = 10;
//
//    vmaCreatePool(vmaAllocator, &poolCreateInfo, &uboVmaPool);

}

void GDeviceVLK::recreateSwapChain() {
    createSwapChain();
    createImageViews();

    createDepthResources();
    createFramebuffers();
}

void GDeviceVLK::setupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);
    auto result = CreateDebugUtilsMessengerEXT(vkInstance, &createInfo, nullptr, &debugMessenger);
    if ( result != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void GDeviceVLK::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = 4;
    if ((imageCount > swapChainSupport.capabilities.maxImageCount && (swapChainSupport.capabilities.maxImageCount != 0))
    || (imageCount < swapChainSupport.capabilities.minImageCount)) {
        std::cerr << "Your GPU doesnt support 4 images for swapchain, which is required by this application" << std::endl << std::flush;
        throw new std::runtime_error("Boo!");
    }
//    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
//        imageCount = swapChainSupport.capabilities.maxImageCount;
//    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.surface = vkSurface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

//    if (indices.graphicsFamily != indices.presentFamily) {
//        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
//        createInfo.queueFamilyIndexCount = 2;
//        createInfo.pQueueFamilyIndices = queueFamilyIndices;
//    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
//    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = swapChain;

    auto error = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);
    if ( error != VK_SUCCESS) {
        std::cout << "error = " << error << std::endl << std::flush;
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void GDeviceVLK::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

VkFormat GDeviceVLK::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat GDeviceVLK::findDepthFormat() {
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

void GDeviceVLK::createRenderPass() {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void GDeviceVLK::createColorResources() {
//    VkFormat colorFormat = swapChainImageFormat;
//
//    createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);
//    colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
//
//    transitionImageLayout(colorImage, colorFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);
}

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}


VkImageView GDeviceVLK::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}


void GDeviceVLK::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
    VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
    VkImage& image, VkDeviceMemory& imageMemory, VkImageLayout vkLaylout) {
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = numSamples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//    if (indices.graphicsFamily != indices.presentFamily) {
//        imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
//    } else {
//        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//    }

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, image, imageMemory, 0);
}

void GDeviceVLK::createDepthResources() {
    VkFormat depthFormat = findDepthFormat();
//
    createImage(swapChainExtent.width, swapChainExtent.height, 1, VK_SAMPLE_COUNT_1_BIT, depthFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        depthImage, depthImageMemory, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

//    transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

void GDeviceVLK::createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {
            swapChainImageViews[i],
            depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.pNext = NULL;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = &attachments[0];
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}


void GDeviceVLK::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}


void GDeviceVLK::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value(), indices.transferFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.pNext = NULL;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = true;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = NULL;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.transferFamily.value(), 0, &uploadQueue);
//    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

bool GDeviceVLK::isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);

    return indices.isComplete();
}

GDeviceVLK::QueueFamilyIndices GDeviceVLK::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
            indices.transferFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vkSurface, &presentSupport);

        if (queueFamily.queueCount > 0 && presentSupport) {
            indices.presentFamily = i;
        }

//        if (indices.isComplete()) {
//            break;
//        }

        i++;
    }

    return indices;
}

void GDeviceVLK::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.pNext = NULL;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }

    if (!getIsAsynBuffUploadSupported()) {
        createCommandPoolForUpload();
    }
}
void GDeviceVLK::createCommandPoolForUpload(){
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo renderPoolInfo = {};
    renderPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    renderPoolInfo.pNext = NULL;
    renderPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    renderPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &renderPoolInfo, nullptr, &renderCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render command pool!");
    }

    VkCommandPoolCreateInfo uploadPoolInfo = {};
    uploadPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    uploadPoolInfo.pNext = NULL;
    uploadPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    uploadPoolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();

    if (vkCreateCommandPool(device, &uploadPoolInfo, nullptr, &uploadCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.pNext = NULL;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPoolForImageTransfer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

void GDeviceVLK::createCommandBuffers() {
    commandBuffers.resize(4);


    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    if (!getIsAsynBuffUploadSupported()) {
        createCommandBuffersForUpload();
    }
}
void GDeviceVLK::createCommandBuffersForUpload() {
    renderCommandBuffers.resize(4);
    renderCommandBuffersNotNull.resize(4);
    renderCommandBuffersForFrameBuffers.resize(4);
    renderCommandBuffersForFrameBuffersNotNull.resize(4);
    for (int i = 0; i < 4; i++) renderCommandBuffersNotNull[i] = false;
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = renderCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        allocInfo.commandBufferCount = (uint32_t) renderCommandBuffers.size();

        if (vkAllocateCommandBuffers(device, &allocInfo, renderCommandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = renderCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) renderCommandBuffersForFrameBuffers.size();

        if (vkAllocateCommandBuffers(device, &allocInfo, renderCommandBuffersForFrameBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    uploadCommandBuffers.resize(4);
    VkCommandBufferAllocateInfo allocInfoUpload = {};
    allocInfoUpload.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfoUpload.commandPool = uploadCommandPool;
    allocInfoUpload.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfoUpload.commandBufferCount = (uint32_t) commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfoUpload, uploadCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate upload command buffers!");
    }

    textureTransferCommandBuffers.resize(4);
    textureTransferCommandBufferNull.resize(4);
    for (int i = 0; i < 4; i++) textureTransferCommandBufferNull[i] = true;
    VkCommandBufferAllocateInfo texttrAllocInfoUpload = {};
    texttrAllocInfoUpload.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    texttrAllocInfoUpload.commandPool = commandPoolForImageTransfer;
    texttrAllocInfoUpload.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    texttrAllocInfoUpload.commandBufferCount = (uint32_t) textureTransferCommandBuffers.size();

    if (vkAllocateCommandBuffers(device, &texttrAllocInfoUpload, textureTransferCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate upload command buffers!");
    }
}

void GDeviceVLK::createSyncObjects() {
    imageAvailableSemaphores.resize(commandBuffers.size());
    renderFinishedSemaphores.resize(commandBuffers.size());
    textureTransferFinishedSemaphores.resize(commandBuffers.size());


    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = NULL;

    for (size_t i = 0; i < commandBuffers.size(); i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &textureTransferFinishedSemaphores[i]) != VK_SUCCESS) {

            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = NULL;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    inFlightFences.resize(4);
    for (size_t i = 0; i < 4; i++) {
        if (vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
    inFlightTextureTransferFences.resize(4);
    for (size_t i = 0; i < 4; i++) {
        if (vkCreateFence(device, &fenceInfo, nullptr, &inFlightTextureTransferFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a textureUpdate!");
        }
    }


    VkFenceCreateInfo uploadFenceInfo = {};
    uploadFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    uploadFenceInfo.pNext = NULL;
    uploadFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    uploadSemaphores.resize(4);
    uploadFences.resize(4);
    uploadSemaphoresSubmited.resize(4);
    for (size_t i = 0; i < 4; i++) {
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &uploadSemaphores[i]);
        vkCreateFence(device, &uploadFenceInfo, nullptr, &uploadFences[i]);
        uploadSemaphoresSubmited[i] = false;
    }

}


unsigned int GDeviceVLK::getDrawFrameNumber() {
    return m_frameNumber & 3;
}

unsigned int GDeviceVLK::getUpdateFrameNumber() {
    return (m_frameNumber + 1) & 3;
}

unsigned int GDeviceVLK::getCullingFrameNumber() {
    return (m_frameNumber + 3) & 3;
}

void GDeviceVLK::increaseFrameNumber() {
    m_frameNumber++;
}

float GDeviceVLK::getAnisLevel() {
    return deviceProperties.limits.maxSamplerAnisotropy;
}

void GDeviceVLK::bindProgram(IShaderPermutation *program) {

}

void GDeviceVLK::bindIndexBuffer(IIndexBuffer *buffer) {

}

void GDeviceVLK::bindVertexBuffer(IVertexBuffer *buffer) {

}


void GDeviceVLK::bindVertexBufferBindings(IVertexBufferBindings *buffer) {

}

void GDeviceVLK::bindTexture(ITexture *texture, int slot) {

}

void GDeviceVLK::startUpdateForNextFrame() {
    int uploadFrame = getUpdateFrameNumber();
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pNext = NULL;
    beginInfo.pInheritanceInfo = NULL;

//    std::cout << "updateBuffers: updateFrame = " << uploadFrame << std::endl;

    vkWaitForFences(device, 1, &uploadFences[uploadFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkWaitForFences(device, 1, &inFlightFences[uploadFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(device, 1, &uploadFences[uploadFrame]);

    if (vkBeginCommandBuffer(uploadCommandBuffers[uploadFrame], &beginInfo) != VK_SUCCESS) {
        std::cout << "failed to begin recording uploadCommandBuffer command buffer!" << std::endl;
    }

    textureTransferCommandBufferNull[uploadFrame] = true;
//    vkWaitForFences(device, 1, &inFlightTextureTransferFences[uploadFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    if (vkBeginCommandBuffer(textureTransferCommandBuffers[uploadFrame], &beginInfo) != VK_SUCCESS) {
        std::cout << "failed to begin recording textureTransferCommandBuffers command buffer!"  << std::endl;
    }
}
void GDeviceVLK::endUpdateForNextFrame() {
    int uploadFrame = getUpdateFrameNumber();
    if (vkEndCommandBuffer(uploadCommandBuffers[uploadFrame]) != VK_SUCCESS) {
        std::cout << "failed to record uploadCommandBuffer command buffer!" << std::endl;
    }
    if (vkEndCommandBuffer(textureTransferCommandBuffers[uploadFrame]) != VK_SUCCESS) {
        std::cout << "failed to record textureTransferCommandBuffers command buffer!" << std::endl;
    }

    if (this->canUploadInSeparateThread()) {
        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &uploadCommandBuffers[uploadFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &uploadSemaphores[uploadFrame];
        uploadSemaphoresSubmited[uploadFrame] = true;

        {
            auto result = vkQueueSubmit(uploadQueue, 1, &submitInfo, uploadFences[uploadFrame]);
            if ( result != VK_SUCCESS) {
                std::cout << "failed to submit uploadCommandBuffer command buffer! result = " << result << std::endl << std::flush;
            }
        }
    }

    while ((!listOfDeallocators.empty())&&(listOfDeallocators.front().frameNumberToDoAt <= m_frameNumber)) {
        auto stuff = listOfDeallocators.front();
        stuff.callback();

        listOfDeallocators.pop_front();
    }
}

typedef std::shared_ptr<GMeshVLK> HVKMesh;
void GDeviceVLK::updateBuffers(std::vector<std::vector<HGUniformBufferChunk>*> &bufferChunks, std::vector<HFrameDepedantData> &frameDepedantData) {
//    aggregationBufferForUpload.resize(maxUniformBufferSize);
    if (!m_blackPixelTexture) {
        m_blackPixelTexture = createTexture(false, false);
        unsigned int zero = 0;
        m_blackPixelTexture->loadData(1,1,&zero, ITextureFormat::itRGBA);
    }
    if (!m_whitePixelTexture) {
        m_whitePixelTexture = createTexture(false, false);
        unsigned int ff = 0xffffffff;
        m_whitePixelTexture->loadData(1,1,&ff, ITextureFormat::itRGBA);
    }

    int fullSize = 0;
    for (int i = 0; i < bufferChunks.size(); i++) {
        auto &bufferVec = bufferChunks[i];
        for (auto &buffer : *bufferVec) {
            fullSize += buffer->getSize();
            int offsetDiff = fullSize % uniformBufferOffsetAlign;
            if (offsetDiff != 0) {
                int bytesToAdd = uniformBufferOffsetAlign - offsetDiff;

                fullSize += bytesToAdd;
            }
        }
    }

    //2. Create buffers and update them
    int currentSize = 0;
    int buffersIndex = 0;

    HGUniformBuffer bufferForUpload = m_UBOFrames[getUpdateFrameNumber()].m_uniformBufferForUpload;

    if (bufferForUpload == nullptr) {
        bufferForUpload = createUniformBuffer(1000);
        bufferForUpload->createBuffer();
        m_UBOFrames[getUpdateFrameNumber()].m_uniformBufferForUpload = bufferForUpload;
        m_shaderDescriptorUpdateNeeded = true;
    }

    auto bufferForUploadVLK = ((GUniformBufferVLK *) bufferForUpload.get());
    size_t old_size = bufferForUploadVLK->m_size;


    if (old_size < fullSize) {
        bufferForUploadVLK->resize(fullSize);
        //Buffer identifier was changed, so we need to update shader UBO descriptor
        m_shaderDescriptorUpdateNeeded = true;
    }

    if (fullSize > 0) {
        char *pointerForUpload = static_cast<char *>(bufferForUploadVLK->stagingUBOBufferAllocInfo.pMappedData);

        for (int i = 0; i < bufferChunks.size(); i++) {
            auto &bufferVec = bufferChunks[i];
            for (auto &buffer : *bufferVec) {
                buffer->setOffset(currentSize);
                buffer->setPointer(&pointerForUpload[currentSize]);
                currentSize += buffer->getSize();

                int offsetDiff = currentSize % uniformBufferOffsetAlign;
                if (offsetDiff != 0) {
                    int bytesToAdd = uniformBufferOffsetAlign - offsetDiff;

                    currentSize += bytesToAdd;
                }
            }
        }
        assert(currentSize == fullSize);
        for (int i = 0; i < bufferChunks.size(); i++) {
            auto &bufferVec = bufferChunks[i];
            auto frameDepData = frameDepedantData[i];

            for (auto &buffer : *bufferVec) {
                buffer->update(frameDepData);
            }
        }
        if (currentSize > 0) {
            bufferForUploadVLK->uploadFromStaging(currentSize);
        }
    }
}

void GDeviceVLK::uploadTextureForMeshes(std::vector<HGMesh> &meshes) {
    std::vector<HGTexture> textures;
    textures.reserve(meshes.size() * 3);

    int texturesLoaded = 0;

    for (const auto &hmesh : meshes) {
        GMeshVLK * mesh = (GMeshVLK *) hmesh.get();
        mesh->updateDescriptor();


        for (int i = 0; i < mesh->m_textureCount; i++) {
            textures.push_back(mesh->m_texture[i]);
        }
    }

    std::sort(textures.begin(), textures.end());
    textures.erase( unique( textures.begin(), textures.end() ), textures.end() );

    for (const auto &texture : textures) {
        if (texture == nullptr) continue;
        if (texture->postLoad()) texturesLoaded++;
        if (texturesLoaded > 4) break;
    }
}

void GDeviceVLK::drawMeshes(std::vector<HGMesh> &meshes) {

}

std::shared_ptr<IShaderPermutation> GDeviceVLK::getShader(std::string shaderName, void *permutationDescriptor) {
    const char * cstr = shaderName.c_str();
    size_t hash = CalculateFNV(cstr);
    if (m_shaderPermutCache.count(hash) > 0) {
        HGShaderPermutation ptr = m_shaderPermutCache.at(hash);
        return ptr;
    }

    std::shared_ptr<IShaderPermutation> sharedPtr;

    if (shaderName == "m2Shader") {
        IShaderPermutation *iPremutation = new GM2ShaderPermutationVLK(shaderName, this);
        sharedPtr.reset(iPremutation);
        sharedPtr->compileShader("","");
    } else if (shaderName == "m2ParticleShader") {
        IShaderPermutation *iPremutation = new GM2ParticleShaderPermutationVLK(shaderName, this);
        sharedPtr.reset(iPremutation);
        sharedPtr->compileShader("","");
    } else if (shaderName == "ribbonShader") {
        IShaderPermutation *iPremutation = new GM2RibbonShaderPermutationVLK(shaderName, this);
        sharedPtr.reset(iPremutation);
        sharedPtr->compileShader("","");
    } else if (shaderName == "wmoShader"){
        IShaderPermutation *iPremutation = new GWMOShaderPermutationVLK(shaderName, this);
        sharedPtr.reset(iPremutation);
        sharedPtr->compileShader("","");
    } else if (shaderName == "waterShader"){
        IShaderPermutation *iPremutation = new GWaterShaderPermutation(shaderName, this);
        sharedPtr.reset(iPremutation);
        sharedPtr->compileShader("","");
    } else if (shaderName == "adtShader"){
        IShaderPermutation *iPremutation = new GAdtShaderPermutationVLK(shaderName, this);
        sharedPtr.reset(iPremutation);
        sharedPtr->compileShader("","");
    } else if (shaderName == "skyConus"){
        IShaderPermutation *iPremutation = new GSkyConusShaderVLK(shaderName, this);
        sharedPtr.reset(iPremutation);
        sharedPtr->compileShader("","");
    } else if (shaderName == "fullScreen_ffxgauss4") {
        IShaderPermutation *iPremutation = new GFFXgauss4VLK(shaderName, this);
        sharedPtr.reset(iPremutation);
        sharedPtr->compileShader("","");
        m_shaderPermutCache[hash] = sharedPtr;
    } else if (shaderName == "ffxGlowQuad") {
        IShaderPermutation *iPremutation = new GFFXGlowVLK(shaderName, this);
        sharedPtr.reset(iPremutation);
        sharedPtr->compileShader("","");
        m_shaderPermutCache[hash] = sharedPtr;
    } else if (shaderName == "imguiShader") {
        IShaderPermutation *iPremutation = new GImguiShaderPermutation(shaderName, this);
        sharedPtr.reset(iPremutation);
        sharedPtr->compileShader("","");
        m_shaderPermutCache[hash] = sharedPtr;
    }


    m_shaderPermutCache[hash] = sharedPtr;


    return sharedPtr;
}

HGUniformBuffer GDeviceVLK::createUniformBuffer(size_t size) {
    std::shared_ptr<GUniformBufferVLK> h_uniformBuffer;
    h_uniformBuffer.reset(new GUniformBufferVLK(*this, size));

    return h_uniformBuffer;
}

HGVertexBufferDynamic GDeviceVLK::createVertexBufferDynamic(size_t size) {
    std::shared_ptr<GVertexBufferDynamicVLK> h_vertexBuffer;
    h_vertexBuffer.reset(new GVertexBufferDynamicVLK(*this, size));

    return h_vertexBuffer;
}

HGVertexBuffer GDeviceVLK::createVertexBuffer() {
    std::shared_ptr<GVertexBufferVLK> h_vertexBuffer;
    h_vertexBuffer.reset(new GVertexBufferVLK(*this));

    return h_vertexBuffer;
}

HGIndexBuffer GDeviceVLK::createIndexBuffer() {
    std::shared_ptr<GIndexBufferVLK> h_indexBuffer;
    h_indexBuffer.reset(new GIndexBufferVLK(*this));

    return h_indexBuffer;
}

HGVertexBufferBindings GDeviceVLK::createVertexBufferBindings() {
    std::shared_ptr<GVertexBufferBindingsVLK> h_vertexBindings;
    h_vertexBindings.reset(new GVertexBufferBindingsVLK(*this));

    return h_vertexBindings;
}

HGTexture GDeviceVLK::createBlpTexture(HBlpTexture &texture, bool xWrapTex, bool yWrapTex) {
//    std::shared_ptr<GTextureVLK> h_texture;
//    h_texture.reset(new GBlpTextureVLK(*this, texture, xWrapTex, yWrapTex));
//
//    return h_texture;

    BlpCacheRecord blpCacheRecord;
    blpCacheRecord.texture = texture.get();
    blpCacheRecord.wrapX = xWrapTex;
    blpCacheRecord.wrapY = yWrapTex;

    auto i = loadedTextureCache.find(blpCacheRecord);
    if (i != loadedTextureCache.end()) {
        if (!i->second.expired()) {
            return i->second.lock();
        } else {
            loadedTextureCache.erase(i);
        }
    }

    std::shared_ptr<GBlpTextureVLK> hgTexture;
    hgTexture.reset(new GBlpTextureVLK(*this, texture, xWrapTex, yWrapTex));

    std::weak_ptr<GBlpTextureVLK> weakPtr(hgTexture);
    loadedTextureCache[blpCacheRecord] = weakPtr;

    return hgTexture;
}

HGTexture GDeviceVLK::createTexture(bool xWrapTex, bool yWrapTex) {
    std::shared_ptr<GTextureVLK> h_texture;
    h_texture.reset(new GTextureVLK(*this, xWrapTex, yWrapTex));

    return h_texture;
}

HGMesh GDeviceVLK::createMesh(gMeshTemplate &meshTemplate) {
    std::shared_ptr<GMeshVLK> h_mesh;
    h_mesh.reset(new GMeshVLK(*this, meshTemplate));

    return h_mesh;
}

HGM2Mesh GDeviceVLK::createM2Mesh(gMeshTemplate &meshTemplate) {
    std::shared_ptr<GM2MeshVLK> h_mesh;
    h_mesh.reset(new GM2MeshVLK(*this, meshTemplate));
    h_mesh->m_meshType = MeshType::eM2Mesh;

    return h_mesh;
}

HGParticleMesh GDeviceVLK::createParticleMesh(gMeshTemplate &meshTemplate) {
    std::shared_ptr<GM2MeshVLK> h_mesh;
    h_mesh.reset(new GM2MeshVLK(*this, meshTemplate));
    h_mesh->m_meshType = MeshType::eParticleMesh;

    return h_mesh;
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

std::string GDeviceVLK::loadShader(std::string fileName, IShaderType shaderType) {
    return std::string();
}

void GDeviceVLK::drawMesh(HGMesh &hmesh) {

}

void GDeviceVLK::reset() {

}

void GDeviceVLK::clearScreen() {

}

void GDeviceVLK::beginFrame() {

// Rebuild pipelines
//    for (auto &pipelineRec : loadedPipeLines) {
//        if (!pipelineRec.second.expired()) {
//            auto pipelineObj = pipelineRec.second.lock();
//            pipelineObj->createPipeline()
//        }
//    }

}

void GDeviceVLK::commitFrame() {
    if (m_firstFrame) {
        m_firstFrame = false;
        return;
    }

    int currentDrawFrame = getDrawFrameNumber();


    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentDrawFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        std::cout << "got VK_ERROR_OUT_OF_DATE_KHR" << std::endl << std::flush;
        recreateSwapChain();

        if (!this->canUploadInSeparateThread()) {
            VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &uploadCommandBuffers[currentDrawFrame];
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &uploadSemaphores[currentDrawFrame];
            uploadSemaphoresSubmited[currentDrawFrame] = true;



            if (vkQueueSubmit(uploadQueue, 1, &submitInfo, uploadFences[currentDrawFrame]) != VK_SUCCESS) {
                std::cout << "failed to submit uploadCommandBuffer command buffer!" << std::endl << std::flush;
            }
        }

        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        std::cout << "error happened " << result << std::endl << std::flush;
//        throw std::runtime_error("failed to acquire swap chain image!");
    }

    if (imageIndex >= inFlightFences.size()) {
        std::cout << "imageIndex >= inFlightFences.size()" << std::endl;
    }

//    std::cout << "imageIndex = " << imageIndex << " currentDrawFrame = " << currentDrawFrame << std::endl << std::flush;

    if (((imageIndex+1)&3) != currentDrawFrame) {
//        std::cout << "imageIndex != currentDrawFrame" << std::endl;
    }

    if (!this->canUploadInSeparateThread()) {
        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &uploadCommandBuffers[currentDrawFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &uploadSemaphores[currentDrawFrame];
        uploadSemaphoresSubmited[currentDrawFrame] = true;

        vkResetFences(device, 1, &uploadFences[currentDrawFrame]);
        {
            auto result = vkQueueSubmit(uploadQueue, 1, &submitInfo, uploadFences[currentDrawFrame]);
            if ( result != VK_SUCCESS) {
                std::cout << "failed to submit uploadCommandBuffer command buffer! result = " << result << std::endl << std::flush;
            }
        }
    }

    vkWaitForFences(device, 1, &inFlightFences[currentDrawFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(device, 1, &inFlightFences[currentDrawFrame]);



    //Fill command buffer
    auto commandBufferForFilling = commandBuffers[currentDrawFrame];

    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pNext = NULL;
        beginInfo.pInheritanceInfo = NULL;

        if (vkBeginCommandBuffer(commandBufferForFilling, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        std::array<VkClearValue, 2> clearValues = {};
        clearValues[0].color = {clearColor[0], clearColor[1], clearColor[2], 1.0f};
        clearValues[1].depthStencil = {getInvertZ() ? 0.0f : 1.0f, 0};

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.pNext = NULL;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBufferForFilling, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        if (renderCommandBuffersNotNull[currentDrawFrame]) {
            vkCmdExecuteCommands(commandBufferForFilling, 1, &renderCommandBuffers[currentDrawFrame]);
        }

        vkCmdEndRenderPass(commandBufferForFilling);

        if (vkEndCommandBuffer(commandBufferForFilling) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }


    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;

    VkSemaphore waitSemaphores[2];
    VkPipelineStageFlags waitStages[2];

    waitSemaphores[0] = imageAvailableSemaphores[currentDrawFrame];
    waitStages[0] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    if (uploadSemaphoresSubmited[currentDrawFrame]) {
        waitSemaphores[1] = uploadSemaphores[currentDrawFrame];
        waitStages[1] = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        submitInfo.waitSemaphoreCount = 2;
    } else {
        submitInfo.waitSemaphoreCount = 1;
    }

    submitInfo.pWaitSemaphores = &waitSemaphores[0];
    submitInfo.pWaitDstStageMask = &waitStages[0];

    std::vector<VkCommandBuffer> grCommandBuffers = {};
    if (!textureTransferCommandBufferNull[currentDrawFrame]) {
        grCommandBuffers.push_back(textureTransferCommandBuffers[currentDrawFrame]);
    }
    if (renderCommandBuffersForFrameBuffersNotNull[currentDrawFrame]) {
        grCommandBuffers.push_back(renderCommandBuffersForFrameBuffers[currentDrawFrame]);
    }
    grCommandBuffers.push_back(commandBufferForFilling);

    submitInfo.commandBufferCount = grCommandBuffers.size();
    submitInfo.pCommandBuffers = grCommandBuffers.data();

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentDrawFrame];

//    if (!renderCommandBuffersNull[currentDrawFrame]) {
    {
        auto result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentDrawFrame]);
        if (result != VK_SUCCESS) {
            std::cout << "failed to submit draw command buffer! result = " << result << std::endl << std::flush;
        }
    }
//    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
//    if (!renderCommandBuffersNull[currentDrawFrame]) {
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentDrawFrame];

//    } else {
//        presentInfo.waitSemaphoreCount = 0;
//    }
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain;
    presentInfo.pImageIndices = &imageIndex;
//    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(graphicsQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS) {
        std::cout << "failed to present swap chain image!" << std::endl << std::flush;
//        throw std::runtime_error("failed to present swap chain image!");
    }
}

void GDeviceVLK::setClearScreenColor(float r, float g, float b) {
    clearColor[0] = r;
    clearColor[1] = g;
    clearColor[2] = b;
}

void GDeviceVLK::setViewPortDimensions(float x, float y, float width, float height) {

}

VkRenderPass GDeviceVLK::getRenderPass(
    std::vector<ITextureFormat> textureAttachments,
    ITextureFormat depthAttachment
) {
    for (auto &renderPassAvalability : m_createdRenderPasses) {
        if (renderPassAvalability.attachments.size() == textureAttachments.size() &&
            renderPassAvalability.depthAttachment == depthAttachment)
        {
            //Check frame definition
            bool notEqual = false;
            for (int i = 0; i < textureAttachments.size(); i++) {
                if (textureAttachments[i] != renderPassAvalability.attachments[i]) {
                    notEqual = true;
                    break;
                }
            }
            if (!notEqual) {
                return renderPassAvalability.renderPass;
            }
        }
    }

    std::vector<VkAttachmentDescription> attachmentDescriptions = {};
    std::vector<VkAttachmentReference> colorReferences;
    uint32_t colorRefIndex = 0;

    GFrameBufferVLK::iterateOverAttachments(textureAttachments, [&](int i, VkFormat textureFormat) {
        VkAttachmentDescription colorAttachment;
        colorAttachment.format = textureFormat;
        colorAttachment.flags = 0;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        attachmentDescriptions.push_back(colorAttachment);
        colorReferences.push_back({ colorRefIndex++, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
    });
    VkFormat fbDepthFormat = findDepthFormat();

    // Depth attachment
    VkAttachmentDescription depthAttachmentDesc;
    depthAttachmentDesc.flags = 0;
    depthAttachmentDesc.format = fbDepthFormat;
    depthAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachmentDescriptions.push_back(depthAttachmentDesc);

    VkAttachmentReference depthReference = { static_cast<uint32_t>(colorReferences.size()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    // Create a separate render pass for the offscreen rendering as it may differ from the one used for scene rendering
    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = colorReferences.size();
    subpassDescription.pColorAttachments = colorReferences.data();
    subpassDescription.pDepthStencilAttachment = &depthReference;


    // Use subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Create the actual renderpass
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
    renderPassInfo.pAttachments = attachmentDescriptions.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    VkRenderPass renderPass;
    ERR_GUARD_VULKAN(vkCreateRenderPass(getVkDevice(), &renderPassInfo, nullptr, &renderPass));


    RenderPassAvalabilityStruct avalabilityStruct;
    avalabilityStruct.attachments = textureAttachments;
    avalabilityStruct.depthAttachment = depthAttachment;
    avalabilityStruct.renderPass = renderPass;

    m_createdRenderPasses.push_back(avalabilityStruct);

    return renderPass;
}

HPipelineVLK GDeviceVLK::createPipeline(HGVertexBufferBindings m_bindings,
                                        HGShaderPermutation shader,
                                        VkRenderPass renderPass,
                                        DrawElementMode element,
                                        int8_t backFaceCulling,
                                        int8_t triCCW,
                                        EGxBlendEnum blendMode,
                                        int8_t depthCulling,
                                        int8_t depthWrite) {

    PipelineCacheRecord pipelineCacheRecord;
    pipelineCacheRecord.shader = shader;
    pipelineCacheRecord.renderPass = renderPass;
    pipelineCacheRecord.element = element;
    pipelineCacheRecord.backFaceCulling = backFaceCulling;
    pipelineCacheRecord.triCCW = triCCW;
    pipelineCacheRecord.blendMode = blendMode;
    pipelineCacheRecord.depthCulling = depthCulling;
    pipelineCacheRecord.depthWrite = depthWrite;

    auto i = loadedPipeLines.find(pipelineCacheRecord);
    if (i != loadedPipeLines.end()) {
        if (!i->second.expired()) {
            return i->second.lock();
        } else {
            loadedPipeLines.erase(i);
        }
    }

    std::shared_ptr<GPipelineVLK> hgPipeline;
    hgPipeline.reset(new GPipelineVLK(*this, m_bindings, renderPass, shader, element, backFaceCulling, triCCW, blendMode, depthCulling, depthWrite));

    std::weak_ptr<GPipelineVLK> weakPtr(hgPipeline);
    loadedPipeLines[pipelineCacheRecord] = weakPtr;

    return hgPipeline;

}

std::shared_ptr<GDescriptorSets>
GDeviceVLK::createDescriptorSet(VkDescriptorSetLayout layout, int uniforms, int images) {
    //1. Try to allocate from existing sets
    std::shared_ptr<GDescriptorSets> descriptorSet;

    for (size_t i = 0; i < m_descriptorPools.size(); i++) {
        descriptorSet = m_descriptorPools[i]->allocate(layout, uniforms, images);
        if (descriptorSet != nullptr)
            return descriptorSet;
    }

    //2. Create new descriptor set and allocate from it
    GDescriptorPoolVLK * newPool = new GDescriptorPoolVLK(*this);
    m_descriptorPools.push_back(newPool);

    return newPool->allocate(layout, uniforms, images);
}

void GDeviceVLK::internalDrawStageAndDeps(HDrawStage drawStage) {
    //Draw deps
    for (int i = 0; i < drawStage->drawStageDependencies.size(); i++) {
        this->internalDrawStageAndDeps(drawStage->drawStageDependencies[i]);
    }

    this->setInvertZ(drawStage->invertedZ);

    if (drawStage->clearScreen) {
        clearColor[0] = drawStage->clearColor[0];
        clearColor[1] = drawStage->clearColor[1];
        clearColor[2] = drawStage->clearColor[2];
    }

    int updateFrame = getUpdateFrameNumber();
    auto commandBufferForFilling = renderCommandBuffers[updateFrame];
    //Default renderPass for rendering to screen framebuffers
    VkRenderPass renderPass = getRenderPass();

    if (drawStage->target != nullptr) {
        commandBufferForFilling = renderCommandBuffersForFrameBuffers[updateFrame];

        GFrameBufferVLK *frameBufferVlk = dynamic_cast<GFrameBufferVLK *>(drawStage->target.get());

        renderPass = frameBufferVlk->m_renderPass;

        std::array<VkClearValue,2> clearValues;
        clearValues[0].color = { clearColor[0], clearColor[1], clearColor[2] };
        clearValues[1].depthStencil = {getInvertZ() ? 0.0f : 1.0f, 0};

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.pNext = NULL;
        renderPassInfo.renderPass = frameBufferVlk->m_renderPass;
        renderPassInfo.framebuffer = frameBufferVlk->m_frameBuffer;
        renderPassInfo.renderArea.offset = {drawStage->viewPortDimensions.mins[0], drawStage->viewPortDimensions.mins[1]};
        renderPassInfo.renderArea.extent = {static_cast<uint32_t>(drawStage->viewPortDimensions.maxs[0]), static_cast<uint32_t>(drawStage->viewPortDimensions.maxs[1])};
        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBufferForFilling, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    std::array<VkViewport, (int)ViewportType::vp_MAX> viewportsForThisStage;

    VkViewport &usualViewport = viewportsForThisStage[(int)ViewportType::vp_usual];
    usualViewport.width = drawStage->viewPortDimensions.maxs[0];
    usualViewport.height =  drawStage->viewPortDimensions.maxs[1];
    usualViewport.x = drawStage->viewPortDimensions.mins[0];
    usualViewport.y =  drawStage->viewPortDimensions.mins[1];
    if (!getInvertZ()) {
        usualViewport.minDepth = 0;
        usualViewport.maxDepth = 0.990f;
    } else {
        usualViewport.minDepth = 0.06;
        usualViewport.maxDepth = 1.0f;
    }

    VkViewport &mapAreaViewport = viewportsForThisStage[(int)ViewportType::vp_mapArea];
    mapAreaViewport = usualViewport;
    if (!getInvertZ()) {
        mapAreaViewport.minDepth = 0.991f;
        mapAreaViewport.maxDepth = 0.996f;
    } else {
        mapAreaViewport.minDepth = 0.04f;
        mapAreaViewport.maxDepth = 0.05f;
    }

    VkViewport &skyBoxViewport = viewportsForThisStage[(int)ViewportType::vp_skyBox];
    skyBoxViewport = usualViewport;
    if (!getInvertZ()) {
        skyBoxViewport.minDepth = 0.997f;
        skyBoxViewport.maxDepth = 1.0f;
    } else {
        skyBoxViewport.minDepth = 0;
        skyBoxViewport.maxDepth = 0.03f;
    }


    //Set scissors
    VkRect2D defaultScissor = {};
    defaultScissor.offset = {0, 0};
    defaultScissor.extent = swapChainExtent;

    vkCmdSetScissor(commandBufferForFilling, 0, 1, &defaultScissor);

    //Set new viewport
    vkCmdSetViewport(commandBufferForFilling, 0, 1, &usualViewport);

    bool atLeastOneDrawCall = false;
    if (drawStage->opaqueMeshes != nullptr)
        atLeastOneDrawCall = drawMeshesInternal(drawStage, commandBufferForFilling, renderPass, drawStage->opaqueMeshes, viewportsForThisStage, defaultScissor) || atLeastOneDrawCall;
    if (drawStage->transparentMeshes != nullptr)
        atLeastOneDrawCall = drawMeshesInternal(drawStage, commandBufferForFilling, renderPass, drawStage->transparentMeshes, viewportsForThisStage, defaultScissor) || atLeastOneDrawCall;

    if (drawStage->target != nullptr) {
        vkCmdEndRenderPass(commandBufferForFilling);
        renderCommandBuffersForFrameBuffersNotNull[updateFrame] = renderCommandBuffersForFrameBuffersNotNull[updateFrame] || atLeastOneDrawCall;
    } else {
        renderCommandBuffersNotNull[updateFrame] = renderCommandBuffersNotNull[updateFrame] || atLeastOneDrawCall;
    }

}

//Returns true if at least once command was written to buffer
bool GDeviceVLK::drawMeshesInternal(
    const HDrawStage &drawStage,
    VkCommandBuffer commandBufferForFilling,
    VkRenderPass renderPass,
    const HMeshesToRender &meshes,
    const std::array<VkViewport, (int) ViewportType::vp_MAX> &viewportsForThisStage,
    VkRect2D &defaultScissor) {

    int updateFrame = getUpdateFrameNumber();

    ViewportType lastViewPort = ViewportType::vp_none;

    VkBuffer lastIndexBuffer = VK_NULL_HANDLE;
    VkBuffer lastVertexBuffer = VK_NULL_HANDLE;
    int8_t lastIsScissorsEnabled = -1;
    std::shared_ptr<GPipelineVLK> lastPipeline = nullptr;
    uint32_t dynamicOffset[7] = {};

    auto iMeshes = meshes->meshes;

    bool atLeastOneDrawcall = false;

    for (auto &mesh: iMeshes) {
        auto *meshVLK = ((GMeshVLK *)mesh.get());
        auto *binding = ((GVertexBufferBindingsVLK *)meshVLK->m_bindings.get());
        auto *shaderVLK = ((GShaderPermutationVLK*)meshVLK->m_shader.get());

        uint32_t uboInd = 0;
        auto *uboB = drawStage->sceneWideBlockVSPSChunk.get();
        if (uboB) {
            dynamicOffset[uboInd++] = (uboB)->getOffset();
        }

        for (int k = 1; k < 6; k++) {
            if (shaderVLK->hasBondUBO[k]) {
                auto *uboB = (meshVLK->getUniformBuffer(k).get());
                if (uboB) {
                    dynamicOffset[uboInd++] = (uboB)->getOffset();
                }
            }
        }

        std::shared_ptr<GPipelineVLK> h_pipeLine = meshVLK->getPipeLineForRenderPass(renderPass);

        if (lastPipeline != h_pipeLine) {
            vkCmdBindPipeline(commandBufferForFilling, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              h_pipeLine->graphicsPipeline);

            lastPipeline = h_pipeLine;
        }

        ViewportType newViewPort = meshVLK->m_isSkyBox ? ViewportType::vp_skyBox : ViewportType::vp_usual;
        if (lastViewPort != newViewPort) {
            if (viewportsForThisStage[+newViewPort].height > 32768) {
                std::cout << "newViewPort = " << +newViewPort << std::endl;
            }
            vkCmdSetViewport(commandBufferForFilling, 0, 1, &viewportsForThisStage[+newViewPort]);

            lastViewPort = newViewPort;
        }


        if (meshVLK->m_isScissorsEnabled > 0) {
            VkRect2D rect;
            rect.offset.x = meshVLK->m_scissorOffset[0];
            rect.offset.y = meshVLK->m_scissorOffset[1];
            rect.extent.width = meshVLK->m_scissorSize[0];
            rect.extent.height = meshVLK->m_scissorSize[1];

            vkCmdSetScissor(commandBufferForFilling, 0, 1, &rect);
        } else if (lastIsScissorsEnabled != meshVLK->m_isScissorsEnabled) {
            vkCmdSetScissor(commandBufferForFilling, 0, 1, &defaultScissor);
        }
        lastIsScissorsEnabled = meshVLK->m_isScissorsEnabled;


        auto indexBuffer = ((GIndexBufferVLK *)binding->m_indexBuffer.get())->g_hIndexBuffer;
        auto vertexBuffer = ((GVertexBufferVLK *)binding->m_bindings[0].vertexBuffer.get())->g_hVertexBuffer;
        VkDeviceSize offsets[] = {0};

        if (lastIndexBuffer != indexBuffer) {
            vkCmdBindIndexBuffer(commandBufferForFilling, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
            lastIndexBuffer = indexBuffer;
        }

        if (lastVertexBuffer != vertexBuffer) {
            uint32_t vboBind = 0;

            vkCmdBindVertexBuffers(commandBufferForFilling, vboBind++, 1, &vertexBuffer, offsets);
            lastVertexBuffer = vertexBuffer;
        }

        auto uboDescSet = shaderVLK->uboDescriptorSets[updateFrame]->getDescSet();
        auto imageDescSet = meshVLK->imageDescriptorSets[updateFrame]->getDescSet();

        atLeastOneDrawcall = true;

        //UBO
        vkCmdBindDescriptorSets(commandBufferForFilling, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                h_pipeLine->pipelineLayout, 0, 1, &uboDescSet, uboInd, &dynamicOffset[0]);

        //Image
        vkCmdBindDescriptorSets(commandBufferForFilling, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                h_pipeLine->pipelineLayout, 1, 1, &imageDescSet, 0, 0);

        vkCmdDrawIndexed(commandBufferForFilling, meshVLK->m_end, 1, meshVLK->m_start/2, 0, 0);
    }

    return atLeastOneDrawcall;
}

void GDeviceVLK::drawStageAndDeps(HDrawStage drawStage) {
    int updateFrame = getUpdateFrameNumber();

//    std::cout << "drawStageAndDeps: updateFrame = " << updateFrame << std::endl;
    vkWaitForFences(device, 1, &inFlightFences[updateFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

    //Update
    if (m_shaderDescriptorUpdateNeeded) {
        for (auto shaderVLKRec : m_shaderPermutCache) {
            ((GShaderPermutationVLK *) shaderVLKRec.second.get())->updateDescriptorSet(updateFrame);
        }
        m_shaderDescriptorUpdateNeeded = false;
    }

    auto commandBufferForFilling = renderCommandBuffers[updateFrame];
    auto commandBufferForFillingFrameBuf = renderCommandBuffersForFrameBuffers[updateFrame];

    renderCommandBuffersNotNull[updateFrame] = false;
    renderCommandBuffersForFrameBuffersNotNull[updateFrame] = false;

    {
        VkCommandBufferInheritanceInfo bufferInheritanceInfo;
        bufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        bufferInheritanceInfo.pNext = nullptr;
        bufferInheritanceInfo.renderPass = renderPass;
        bufferInheritanceInfo.subpass = 0;
        bufferInheritanceInfo.framebuffer = VK_NULL_HANDLE;
        bufferInheritanceInfo.occlusionQueryEnable = false;
        bufferInheritanceInfo.queryFlags = VK_NULL_HANDLE;
        bufferInheritanceInfo.pipelineStatistics = VK_NULL_HANDLE;

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        beginInfo.pNext = NULL;
        beginInfo.pInheritanceInfo = &bufferInheritanceInfo;

        if (vkBeginCommandBuffer(commandBufferForFilling, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
    }

    {
        VkCommandBufferInheritanceInfo bufferInheritanceInfo;
        bufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        bufferInheritanceInfo.pNext = nullptr;
        bufferInheritanceInfo.renderPass = VK_NULL_HANDLE;
        bufferInheritanceInfo.subpass = 0;
        bufferInheritanceInfo.framebuffer = VK_NULL_HANDLE;
        bufferInheritanceInfo.occlusionQueryEnable = false;
        bufferInheritanceInfo.queryFlags = VK_NULL_HANDLE;
        bufferInheritanceInfo.pipelineStatistics = VK_NULL_HANDLE;

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        beginInfo.pNext = NULL;
        beginInfo.pInheritanceInfo = &bufferInheritanceInfo;

        if (vkBeginCommandBuffer(commandBufferForFillingFrameBuf, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
    }

    internalDrawStageAndDeps(drawStage);

    if (vkEndCommandBuffer(commandBufferForFilling) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
    if (vkEndCommandBuffer(commandBufferForFillingFrameBuf) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void GDeviceVLK::initUploadThread() {
    if (getIsAsynBuffUploadSupported()) {
        createCommandPoolForUpload();
        createCommandBuffersForUpload();
    }
}

HFrameBuffer GDeviceVLK::createFrameBuffer(int width, int height, std::vector<ITextureFormat> attachments,
                                           ITextureFormat depthAttachment, int frameNumber) {

    if (frameNumber > -1) {
        for (auto &framebufAvalability : m_createdFrameBuffers) {
            if ((framebufAvalability.frame < m_frameNumber) &&
                framebufAvalability.attachments.size() == attachments.size() &&
                framebufAvalability.depthAttachment == depthAttachment &&
                framebufAvalability.width == width &&
                framebufAvalability.height == height
                ) {
                //Check frame definition
                bool notEqual = false;
                for (int i = 0; i < attachments.size(); i++) {
                    if (attachments[i] != framebufAvalability.attachments[i]) {
                        notEqual = true;
                        break;
                    }
                }
                if (!notEqual) {
                    framebufAvalability.frame = m_frameNumber + frameNumber+3;
                    return framebufAvalability.frameBuffer;
                }
            }
        }
    }

    HFrameBuffer h_frameBuffer = std::make_shared<GFrameBufferVLK>(*this, attachments, depthAttachment, width, height);

    if (frameNumber > -1) {
        FramebufAvalabilityStruct avalabilityStruct;
        avalabilityStruct.frameBuffer = h_frameBuffer;
        avalabilityStruct.height = height;
        avalabilityStruct.width = width;
        avalabilityStruct.frame = m_frameNumber + frameNumber+3;
        avalabilityStruct.attachments = attachments;
        avalabilityStruct.depthAttachment = depthAttachment;

        m_createdFrameBuffers.push_back(avalabilityStruct);
    }

    return h_frameBuffer;

}
