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
#include "shaders/GWMOWaterShaderVLK.h"
#include "shaders/GM2RibbonShaderPermutationVLK.h"

const int WIDTH = 1024;
const int HEIGHT = 768;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
    "VK_LAYER_LUNARG_assistant_layer",


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
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = {WIDTH, HEIGHT};

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
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
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
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
    extensionsVec.push_back("VK_EXT_debug_report");
    extensionsVec.push_back("VK_EXT_debug_utils");



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


    // Create pool
    VkBufferCreateInfo exampleBufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    exampleBufCreateInfo.size = 65536; // Whatever.
    exampleBufCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT; // Change if needed.

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY; // Change if needed.

    uint32_t memTypeIndex;
    vmaFindMemoryTypeIndexForBufferInfo(vmaAllocator, &exampleBufCreateInfo, &allocCreateInfo, &memTypeIndex);

    // Create a pool that can have at most 2 blocks, 128 MiB each.
    VmaPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.memoryTypeIndex = memTypeIndex;
    poolCreateInfo.blockSize = 128ull * 1024 * 1024;
    poolCreateInfo.maxBlockCount = 10;

    vmaCreatePool(vmaAllocator, &poolCreateInfo, &uboVmaPool);

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

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
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


void GDeviceVLK::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkImageLayout vkLaylout) {
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
    imageInfo.initialLayout = vkLaylout;
    imageInfo.usage = usage;
    imageInfo.samples = numSamples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

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
    createImage(swapChainExtent.width, swapChainExtent.height, 1, VK_SAMPLE_COUNT_1_BIT, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
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
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }

    VkCommandPoolCreateInfo uploadPoolInfo = {};
    uploadPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    uploadPoolInfo.pNext = NULL;
    uploadPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    uploadPoolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();

    if (vkCreateCommandPool(device, &uploadPoolInfo, nullptr, &uploadCommandPool) != VK_SUCCESS) {
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
    VkCommandBufferAllocateInfo texttrAllocInfoUpload = {};
    texttrAllocInfoUpload.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    texttrAllocInfoUpload.commandPool = commandPool;
    texttrAllocInfoUpload.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    texttrAllocInfoUpload.commandBufferCount = (uint32_t) commandBuffers.size();

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

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = NULL;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < commandBuffers.size(); i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &textureTransferFinishedSemaphores[i]) != VK_SUCCESS) {

            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }

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
    for (size_t i = 0; i < 4; i++) {
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &uploadSemaphores[i]);
        vkCreateFence(device, &uploadFenceInfo, nullptr, &uploadFences[i]);
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

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &uploadCommandBuffers[uploadFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &uploadSemaphores[uploadFrame];


    if (vkQueueSubmit(uploadQueue, 1, &submitInfo, uploadFences[uploadFrame]) != VK_SUCCESS) {
        std::cout << "failed to submit uploadCommandBuffer command buffer!" << std::endl << std::flush;
    }

    while ((!listOfDeallocators.empty())&&(listOfDeallocators.front().frameNumberToDoAt <= m_frameNumber)) {
        listOfDeallocators.front().callback();

        listOfDeallocators.pop_front();
    }
}

void GDeviceVLK::updateBuffers(std::vector<HGMesh> &iMeshes) {
//    aggregationBufferForUpload.resize(maxUniformBufferSize);
    if (!m_blackPixelTexture) {
        m_blackPixelTexture = createTexture();
        unsigned int ff = 0;
        m_blackPixelTexture->loadData(1,1,&ff);
    }

    for (const auto &mesh : iMeshes) {
        for (int i = 0; i < 3; i++ ) {
            GUniformBufferVLK *buffer = (GUniformBufferVLK *) mesh->getFragmentUniformBuffer(i).get();
            if (buffer != nullptr) {
                buffer->commitUpload();
            }
        }
        for (int i = 0; i < 3; i++ ) {
            GUniformBufferVLK * buffer = (GUniformBufferVLK *)mesh->getVertexUniformBuffer(i).get();
            if (buffer != nullptr) {
                buffer->commitUpload();
            }
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
        if (texturesLoaded > 4) return;
    }


    updateCommandBuffers(meshes);
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
        IShaderPermutation *iPremutation = new GWmoWaterShaderPermutationVLK(shaderName, this);
        sharedPtr.reset(iPremutation);
        sharedPtr->compileShader("","");
    } else if (shaderName == "adtShader"){
        IShaderPermutation *iPremutation = new GAdtShaderPermutationVLK(shaderName, this);
        sharedPtr.reset(iPremutation);
        sharedPtr->compileShader("","");
    }


    m_shaderPermutCache[hash] = sharedPtr;


    return sharedPtr;
}

HGUniformBuffer GDeviceVLK::createUniformBuffer(size_t size) {
    std::shared_ptr<GUniformBufferVLK> h_uniformBuffer;
    h_uniformBuffer.reset(new GUniformBufferVLK(*this, size));

    return h_uniformBuffer;
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
    blpCacheRecord.texture = texture;
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

HGTexture GDeviceVLK::createTexture() {
    std::shared_ptr<GTextureVLK> h_texture;
    h_texture.reset(new GTextureVLK(*this));

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

std::string GDeviceVLK::loadShader(std::string fileName, bool common) {
    return std::string();
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

void GDeviceVLK::beginFrame() {

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
        //recreateSwapChain();
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
        std::cout << "imageIndex != currentDrawFrame" << std::endl;
    }

    vkWaitForFences(device, 1, &inFlightFences[currentDrawFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(device, 1, &inFlightFences[currentDrawFrame]);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;

    VkSemaphore waitSemaphores[2];
    VkPipelineStageFlags waitStages[2];

    waitSemaphores[0] = imageAvailableSemaphores[currentDrawFrame];
    waitStages[0] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    waitSemaphores[1] = uploadSemaphores[currentDrawFrame];
    waitStages[1] = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    submitInfo.waitSemaphoreCount = 2;
    submitInfo.pWaitSemaphores = &waitSemaphores[0];
    submitInfo.pWaitDstStageMask = &waitStages[0];

    std::array<VkCommandBuffer, 2> grCommandBuffers = {textureTransferCommandBuffers[currentDrawFrame], commandBuffers[currentDrawFrame]};

    submitInfo.commandBufferCount = 2;
    submitInfo.pCommandBuffers = &grCommandBuffers[0];

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentDrawFrame];

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentDrawFrame]) != VK_SUCCESS) {
        std::cout << "failed to submit draw command buffer!" << std::endl << std::flush;
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentDrawFrame];

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain;
    presentInfo.pImageIndices = &imageIndex;
//    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(graphicsQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
//        recreateSwapChain();
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

void GDeviceVLK::updateCommandBuffers(std::vector<HGMesh> &iMeshes) {
    int updateFrame = getUpdateFrameNumber();

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pNext = NULL;
    beginInfo.pInheritanceInfo = NULL;

//    std::cout << "updateCommandBuffers: updateFrame = " << updateFrame << std::endl;
    vkWaitForFences(device, 1, &inFlightFences[updateFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    if (vkBeginCommandBuffer(commandBuffers[updateFrame], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = {clearColor[0], clearColor[1], clearColor[2], 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.pNext = NULL;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[updateFrame];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[updateFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkBuffer lastIndexBuffer = VK_NULL_HANDLE;
    VkBuffer lastVertexBuffer = VK_NULL_HANDLE;

    for (auto &mesh: iMeshes) {
        auto *meshVLK = ((GMeshVLK *)mesh.get());
        auto *binding ((GVertexBufferBindingsVLK *)meshVLK->m_bindings.get());


        vkCmdBindPipeline(commandBuffers[updateFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, meshVLK->hgPipelineVLK->graphicsPipeline);


        auto indexBuffer = ((GIndexBufferVLK *)binding->m_indexBuffer.get())->g_hIndexBuffer;
        auto vertexBuffer = ((GVertexBufferVLK *)binding->m_bindings[0].vertexBuffer.get())->g_hVertexBuffer;
        VkDeviceSize offsets[] = {0};

        if (lastIndexBuffer != indexBuffer) {
            vkCmdBindIndexBuffer(commandBuffers[updateFrame], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
            lastIndexBuffer = indexBuffer;
        }

        if (lastVertexBuffer != vertexBuffer) {
            uint32_t vboBind = 0;

            vkCmdBindVertexBuffers(commandBuffers[updateFrame], vboBind++, 1, &vertexBuffer, offsets);
            lastVertexBuffer = vertexBuffer;
        }


        vkCmdBindDescriptorSets(commandBuffers[updateFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
            meshVLK->hgPipelineVLK->pipelineLayout, 0, 1, &meshVLK->descriptorSets[updateFrame], 0, 0);

        vkCmdDrawIndexed(commandBuffers[updateFrame], meshVLK->m_end, 1, meshVLK->m_start/2, 0, 0);
    }
    vkCmdEndRenderPass(commandBuffers[updateFrame]);


    if (vkEndCommandBuffer(commandBuffers[updateFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

HPipelineVLK GDeviceVLK::createPipeline(HGVertexBufferBindings m_bindings,
                                        HGShaderPermutation shader,
                                        DrawElementMode element,
                                        int8_t backFaceCulling,
                                        int8_t triCCW,
                                        EGxBlendEnum blendMode,
                                        int8_t depthCulling,
                                        int8_t depthWrite) {

    PipelineCacheRecord pipelineCacheRecord;
    pipelineCacheRecord.shader = shader;
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
    hgPipeline.reset(new GPipelineVLK(*this, m_bindings, shader, element, backFaceCulling, triCCW, blendMode, depthCulling, depthWrite));

    std::weak_ptr<GPipelineVLK> weakPtr(hgPipeline);
    loadedPipeLines[pipelineCacheRecord] = weakPtr;

    return hgPipeline;

}
