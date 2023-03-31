//
// Created by Deamon on 8/1/2019.
//

#include <iostream>
#include <cstring>
#include <set>
#include <vector>
#include <algorithm>
#include <array>
#include <thread>
#include "GDeviceVulkan.h"

#include "meshes/GMeshVLK.h"
#include "textures/GTextureVLK.h"
#include "textures/GBlpTextureVLK.h"
#include "GVertexBufferBindingsVLK.h"
#include "GPipelineVLK.h"
#include "../../engine/algorithms/hashString.h"
#include "GFrameBufferVLK.h"
#include "shaders/GShaderPermutationVLK.h"
#include "GRenderPassVLK.h"
#include "../../engine/algorithms/FrameCounter.h"
#include "buffers/GBufferVLK.h"
#include "synchronization/GFenceVLK.h"
#include "../../renderer/vulkan/IRenderFunctionVLK.h"
#include "commandBuffer/commandBufferRecorder/TextureUploadHelper.h"
#include <tbb/tbb.h>

const int WIDTH = 1900;
const int HEIGHT = 1000;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        std::cerr << "error ";

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
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;
}


GDeviceVLK::GDeviceVLK(vkCallInitCallback * callback) : m_textureManager(std::make_shared<TextureManagerVLK>(*this)){
    enableValidationLayers = false;

    m_textureManager->initialize();

    if (volkInitialize()) {
        std::cerr << "Failed to initialize volk loader" << std::endl;
        exit(1);
    }

    this->threadCount = std::max<int>((int)std::thread::hardware_concurrency() - 3, 1);

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
    std::vector<const char *> extensionsVec(extensions, extensions+extensionCnt);
    if (enableValidationLayers) {
        extensionsVec.push_back("VK_EXT_debug_report");
        extensionsVec.push_back("VK_EXT_debug_utils");
    }

    createInfo.enabledExtensionCount = extensionsVec.size();
    createInfo.ppEnabledExtensionNames = extensionsVec.data();


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

    volkLoadInstance(vkInstance);

    //Create surface
    vkSurface = callback->createSurface(vkInstance);
}

std::unordered_set<std::string> GDeviceVLK::get_enabled_extensions() {
    /*
     * From the link above:
     * If `pProperties` is NULL, then the number of extensions properties
     * available is returned in `pPropertyCount`.
     *
     * Basically, gets the number of extensions.
     */
    uint32_t count = 0;
    VkResult result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
    if (result != VK_SUCCESS) {
        // Throw an exception or log the error
    }

    std::vector<VkExtensionProperties> extensionProperties(count);

    // Get the extensions
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, extensionProperties.data());
    if (result != VK_SUCCESS) {
        // Throw an exception or log the error
    }

     std::unordered_set<std::string> extensions;

    std::cout << "enabled extensions : " << std::endl;
    for (auto & extension : extensionProperties) {
        extensions.insert(extension.extensionName);
        std::cout << "- " << extension.extensionName << std::endl;
    }
    std::cout << "enabled extensions end" << std::endl;

    return extensions;
}

void GDeviceVLK::initialize() {
    setupDebugMessenger();

    pickPhysicalDevice();
    createLogicalDevice();

    std::unordered_set<std::string> enabledExtensions = get_enabled_extensions();

    findQueueFamilies(physicalDevice);
//---------------
//Init AMD's VMA
    VmaVulkanFunctions vma_vulkan_func{};
    vma_vulkan_func.vkAllocateMemory                    = vkAllocateMemory;
    vma_vulkan_func.vkBindBufferMemory                  = vkBindBufferMemory;
    vma_vulkan_func.vkBindImageMemory                   = vkBindImageMemory;
    vma_vulkan_func.vkCreateBuffer                      = vkCreateBuffer;
    vma_vulkan_func.vkCreateImage                       = vkCreateImage;
    vma_vulkan_func.vkDestroyBuffer                     = vkDestroyBuffer;
    vma_vulkan_func.vkDestroyImage                      = vkDestroyImage;
    vma_vulkan_func.vkFlushMappedMemoryRanges           = vkFlushMappedMemoryRanges;
    vma_vulkan_func.vkFreeMemory                        = vkFreeMemory;
    vma_vulkan_func.vkGetBufferMemoryRequirements       = vkGetBufferMemoryRequirements;
    vma_vulkan_func.vkGetImageMemoryRequirements        = vkGetImageMemoryRequirements;
    vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    vma_vulkan_func.vkGetPhysicalDeviceProperties       = vkGetPhysicalDeviceProperties;
    vma_vulkan_func.vkInvalidateMappedMemoryRanges      = vkInvalidateMappedMemoryRanges;
    vma_vulkan_func.vkMapMemory                         = vkMapMemory;
    vma_vulkan_func.vkUnmapMemory                       = vkUnmapMemory;
    vma_vulkan_func.vkCmdCopyBuffer                     = vkCmdCopyBuffer;

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = vkInstance;
    allocatorInfo.pVulkanFunctions = &vma_vulkan_func;

    vmaCreateAllocator(&allocatorInfo, &vmaAllocator);
//---------------
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    uniformBufferOffsetAlign = deviceProperties.limits.minUniformBufferOffsetAlignment;
    maxUniformBufferSize = deviceProperties.limits.maxUniformBufferRange;
//---------------

    createSwapChainAndFramebuffer();

    createCommandPool();
    createCommandPoolForUpload();

    createCommandBuffers();
    createSyncObjects();

    std::cout << "uniformBufferOffsetAlign = " << uniformBufferOffsetAlign << std::endl;
    std::cout << "maxUniformBufferSize = " << maxUniformBufferSize << std::endl;

    m_blackPixelTexture = createTexture(false, false);
    unsigned int zero = 0;
    m_blackPixelTexture->loadData(1,1,&zero, ITextureFormat::itRGBA);

    m_whitePixelTexture = createTexture(false, false);
    unsigned int ff = 0xffffffff;
    m_whitePixelTexture->loadData(1,1,&ff, ITextureFormat::itRGBA);
}

void GDeviceVLK::setObjectName(uint64_t object, VkObjectType objectType, const char *name)
{
    // Check for valid function pointer (may not be present if not running in a debugging application)
    if (vkSetDebugUtilsObjectNameEXT != nullptr)
    {
        VkDebugUtilsObjectNameInfoEXT nameInfo = {};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.pNext = nullptr;
        nameInfo.objectType = objectType;
        nameInfo.objectHandle = object;
        nameInfo.pObjectName = name;
        vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
    }
}


std::vector<std::string> VkFormatFeatureFlagBitsGetStrings(VkFormatFeatureFlags value) {
    std::vector<std::string> strings;
    if (value == 0) { strings.push_back("None"); return strings; }
    if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & value) strings.push_back("FORMAT_FEATURE_SAMPLED_IMAGE_BIT");
    if (VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT & value) strings.push_back("FORMAT_FEATURE_STORAGE_IMAGE_BIT");
    if (VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT & value) strings.push_back("FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT");
    if (VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT & value) strings.push_back("FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT");
    if (VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT & value) strings.push_back("FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT");
    if (VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT & value) strings.push_back("FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT");
    if (VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT & value) strings.push_back("FORMAT_FEATURE_VERTEX_BUFFER_BIT");
    if (VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT & value) strings.push_back("FORMAT_FEATURE_COLOR_ATTACHMENT_BIT");
    if (VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT & value) strings.push_back("FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT");
    if (VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT & value) strings.push_back("FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT");
    if (VK_FORMAT_FEATURE_BLIT_SRC_BIT & value) strings.push_back("FORMAT_FEATURE_BLIT_SRC_BIT");
    if (VK_FORMAT_FEATURE_BLIT_DST_BIT & value) strings.push_back("FORMAT_FEATURE_BLIT_DST_BIT");
    if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT & value) strings.push_back("FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT");
    if (VK_FORMAT_FEATURE_TRANSFER_SRC_BIT & value) strings.push_back("FORMAT_FEATURE_TRANSFER_SRC_BIT");
    if (VK_FORMAT_FEATURE_TRANSFER_DST_BIT & value) strings.push_back("FORMAT_FEATURE_TRANSFER_DST_BIT");
    if (VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT & value) strings.push_back("FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT");
    if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT & value) strings.push_back("FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT");
    if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT & value) strings.push_back("FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT");
    if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT & value) strings.push_back("FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT");
    if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT & value) strings.push_back("FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT");
    if (VK_FORMAT_FEATURE_DISJOINT_BIT & value) strings.push_back("FORMAT_FEATURE_DISJOINT_BIT");
    if (VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT & value) strings.push_back("FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT");
    if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT & value) strings.push_back("FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT");
    if (VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR & value) strings.push_back("FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR");
    if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT & value) strings.push_back("FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT");
    if (VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT & value) strings.push_back("FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT");
    if (VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR & value) strings.push_back("FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR");
    return strings;
}

void GDeviceVLK::createSwapChainAndFramebuffer() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    VkFormatProperties swapChainFormatProps;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, surfaceFormat.format, &swapChainFormatProps);

    //auto linearTilingFeatures = VkFormatFeatureFlagBitsGetStrings(swapChainFormatProps.linearTilingFeatures);
    //auto optimalTilingFeatures = VkFormatFeatureFlagBitsGetStrings(swapChainFormatProps.optimalTilingFeatures);

    //std::cout << "linear tiling features = ";
    //for (auto feature : linearTilingFeatures) std::cout << feature << " ";
    //std::cout << std::endl;

    //std::cout << "optimal tiling features = ";
    //for (auto feature : optimalTilingFeatures) std::cout << feature << " ";
    //std::cout << std::endl;

    //VkImageFormatProperties imageFormatProperties;
    //ERR_GUARD_VULKAN(vkGetPhysicalDeviceImageFormatProperties(physicalDevice,
    //                                         surfaceFormat.format,
    //                                         VK_IMAGE_TYPE_2D,
    //                                         VK_IMAGE_TILING_OPTIMAL,
    //                                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    //                                         0,
    //                                         &imageFormatProperties));


    createSwapChain(swapChainSupport, surfaceFormat, extent);

    uint32_t imageCount;
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);

    //Create swapchainImages and framebuffer
    std::vector<VkImage> swapChainImages = {};
    swapChainImages.resize(imageCount);

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainExtent = extent;

    //Create imageView
    std::vector<VkImageView> swapChainImageViews;
    createSwapChainImageViews(swapChainImages, swapChainImageViews, surfaceFormat.format);

    //Create swapchain renderPass
    createSwapChainRenderPass(surfaceFormat.format);

    //Create textures for automatic cleanup
    std::vector<HGTextureVLK> swapChainTextures;
    swapChainTextures.resize(swapChainImages.size());
    for (int i = 0; i < swapChainImages.size(); i++) {
        swapChainTextures[i] = std::make_shared<GTextureVLK>(*this, swapChainImages[i], swapChainImageViews[i], false);
    }

    createFramebuffers(swapChainTextures, extent);
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

void GDeviceVLK::createSwapChain(SwapChainSupportDetails &swapChainSupport, VkSurfaceFormatKHR &surfaceFormat, VkExtent2D &extent) {
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);

    uint32_t imageCount = MAX_FRAMES_IN_FLIGHT;
    if ((imageCount > swapChainSupport.capabilities.maxImageCount && (swapChainSupport.capabilities.maxImageCount != 0))
    || (imageCount < swapChainSupport.capabilities.minImageCount)) {
        std::cerr << "Your GPU doesnt support " << MAX_FRAMES_IN_FLIGHT << " images for swapchain, which is required by this application" << std::endl << std::flush;
        throw new std::runtime_error("Boo!");
    }
//    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
//        imageCount = swapChainSupport.capabilities.maxImageCount;
//    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.surface = vkSurface;
    createInfo.flags = 0;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

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
}

void GDeviceVLK::createSwapChainImageViews(std::vector<VkImage> &swapChainImages, std::vector<VkImageView> &swapChainImageViews, VkFormat swapChainImageFormat) {
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

void GDeviceVLK::createSwapChainRenderPass(VkFormat swapChainImageFormat) {
    swapchainRenderPass = std::make_shared<GRenderPassVLK>(this->device,
                                                  std::vector({swapChainImageFormat}),
                                                  findDepthFormat(),
                                                  VK_SAMPLE_COUNT_1_BIT,
                                                  false, true);
}

void GDeviceVLK::createFramebuffers(std::vector<HGTextureVLK> &swapChainTextures, VkExtent2D &extent) {
    swapChainFramebuffers.resize(swapChainTextures.size());

    for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
        swapChainFramebuffers[i] = std::make_shared<GFrameBufferVLK>(
            *this,
            swapChainTextures[i],
            extent.width,
            extent.height,
            swapchainRenderPass
            );
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

    std::vector<const char*> enabledDeviceExtensions = deviceExtensions;
    createInfo.enabledExtensionCount = enabledDeviceExtensions.size();
    createInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data();

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
    findQueueFamilies(device);
    return indices.isComplete();
}

void GDeviceVLK::findQueueFamilies(VkPhysicalDevice device) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    int dedicatedTransferQueue = -1;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
            indices.transferFamily = i;
        }

        if (queueFamily.queueCount > 0
            && (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
            && !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            && !(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            dedicatedTransferQueue = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vkSurface, &presentSupport);

        if (queueFamily.queueCount > 0 && presentSupport) {
            indices.presentFamily = i;
        }

        i++;
    }
    if (dedicatedTransferQueue > -1) {
        indices.transferFamily = dedicatedTransferQueue;
    }
}

void GDeviceVLK::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.pNext = NULL;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = indices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }

    if (!getIsAsynBuffUploadSupported()) {
        createCommandPoolForUpload();
    }
}
void GDeviceVLK::createCommandPoolForUpload(){
    VkCommandPoolCreateInfo renderPoolInfo = {};
    renderPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    renderPoolInfo.pNext = NULL;
    renderPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    renderPoolInfo.queueFamilyIndex = indices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &renderPoolInfo, nullptr, &renderCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render command pool!");
    }

    VkCommandPoolCreateInfo uploadPoolInfo = {};
    uploadPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    uploadPoolInfo.pNext = NULL;
    uploadPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    uploadPoolInfo.queueFamilyIndex = indices.transferFamily.value();

    if (vkCreateCommandPool(device, &uploadPoolInfo, nullptr, &uploadCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.pNext = NULL;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = indices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPoolForImageTransfer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

void GDeviceVLK::createCommandBuffers() {
    for (auto & commandBuffer : fbCommandBuffers) {
        commandBuffer = std::make_shared<GCommandBuffer>(*this, commandPool, true, indices.graphicsFamily.value());
    }
    for (auto & commandBuffer : swapChainCommandBuffers) {
        commandBuffer = std::make_shared<GCommandBuffer>(*this, commandPool, true, indices.graphicsFamily.value());
    }

    for (auto & commandBuffer : uploadCommandBuffers) {
        commandBuffer = std::make_shared<GCommandBuffer>(*this, uploadCommandPool, true, indices.transferFamily.value());
    }

}


void GDeviceVLK::createSyncObjects() {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        imageAvailableSemaphores[i] = std::make_shared<GSemaphoreVLK>(this->shared_from_this());
        renderFinishedSemaphores[i] = std::make_shared<GSemaphoreVLK>(this->shared_from_this());

        uploadSemaphores[i] = std::make_shared<GSemaphoreVLK>(this->shared_from_this());
        frameBufSemaphores[i] = std::make_shared<GSemaphoreVLK>(this->shared_from_this());
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        inFlightFences[i] = std::make_shared<GFenceVLK>(this->shared_from_this(), true);
        frameBufFences[i] = std::make_shared<GFenceVLK>(this->shared_from_this(), true);
        uploadFences[i] = std::make_shared<GFenceVLK>(this->shared_from_this(), true);
    }
}


unsigned int GDeviceVLK::getDrawFrameNumber() {
    return m_frameNumber % MAX_FRAMES_IN_FLIGHT;
}

unsigned int GDeviceVLK::getUpdateFrameNumber() {
    return (m_frameNumber + 1) % MAX_FRAMES_IN_FLIGHT;
}
unsigned int GDeviceVLK::getOcclusionFrameNumber() {
    return (m_frameNumber + 2) % MAX_FRAMES_IN_FLIGHT;
}
unsigned int GDeviceVLK::getCullingFrameNumber() {
    return (m_frameNumber + 3) % MAX_FRAMES_IN_FLIGHT;
}

void GDeviceVLK::increaseFrameNumber() {
    m_frameNumber++;
}

float GDeviceVLK::getAnisLevel() {
    return deviceProperties.limits.maxSamplerAnisotropy;
}

void GDeviceVLK::drawFrame(const std::vector<std::unique_ptr<IRenderFunction>> &renderFuncs) {
    this->waitInDrawStageAndDeps.beginMeasurement();
    int currentDrawFrame = getDrawFrameNumber();

    auto &uploadCmdBuf = uploadCommandBuffers[currentDrawFrame];
    auto &swapChainCmdBuf = swapChainCommandBuffers[currentDrawFrame];
    auto &frameBufCmdBuf = fbCommandBuffers[currentDrawFrame];

    uint32_t imageIndex = -1;
    {
        {

            //Wait for frameBuf CMD buffer to become available
            frameBufFences[currentDrawFrame]->wait(std::numeric_limits<uint64_t>::max());
            uploadFences[currentDrawFrame]->wait(std::numeric_limits<uint64_t>::max());
            frameBufFences[currentDrawFrame]->reset();
            uploadFences[currentDrawFrame]->reset();

        }

        auto uploadCmd = uploadCmdBuf->beginRecord(nullptr);
        auto frameBufCmd = frameBufCmdBuf->beginRecord(nullptr);

        //Do Texture update
        {
            m_textureManager->processBLPTextures();
            auto textureVector = m_textureManager->getReadyToUploadTextures();
            textureUploadStrategy(textureVector.get(), frameBufCmd, uploadCmd);
            //The next loop updates DescriptorSets
            for(auto &wtexture : textureVector.get()) {
                if( auto texture = wtexture.lock()) {
                    texture->executeOnChange();
                }
            }
        }

        //Wait for swapchain
        {
            inFlightFences[currentDrawFrame]->wait(std::numeric_limits<uint64_t>::max());
            inFlightFences[currentDrawFrame]->reset();
        }

        auto swapChainCmd = swapChainCmdBuf->beginRecord(nullptr);

        {
            //Begin render pass for Swap chain
            this->getNextSwapImageIndex(imageIndex);
            auto swapChainRenderPass = this->beginSwapChainRenderPass(imageIndex, swapChainCmd);

            for (int i = 0; i < renderFuncs.size(); i++) {
                dynamic_cast<IRenderFunctionVLK *>(renderFuncs[i].get())->execute(uploadCmd, frameBufCmd, swapChainCmd);
            }
        }
    }


    submitQueue(
        uploadQueue,
        {},
        {},
        {uploadCmdBuf->getNativeCmdBuffer()},
        {uploadSemaphores[currentDrawFrame]->getNativeSemaphore()},
        uploadFences[currentDrawFrame]->getNativeFence()
    );

    submitQueue(
        graphicsQueue,
        {
            uploadSemaphores[currentDrawFrame]->getNativeSemaphore()
        },
        {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
        {frameBufCmdBuf->getNativeCmdBuffer()},
        {frameBufSemaphores[currentDrawFrame]->getNativeSemaphore()},
        frameBufFences[currentDrawFrame]->getNativeFence()
    );

    submitQueue(
        graphicsQueue,
        {
            frameBufSemaphores[currentDrawFrame]->getNativeSemaphore(),
            imageAvailableSemaphores[currentDrawFrame]->getNativeSemaphore()
        },
        {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},

        {swapChainCmdBuf->getNativeCmdBuffer()},
        {renderFinishedSemaphores[currentDrawFrame]->getNativeSemaphore()},
        inFlightFences[currentDrawFrame]->getNativeFence()
    );

    presentQueue(
        {renderFinishedSemaphores[currentDrawFrame]->getNativeSemaphore()},
        {swapChain},
        {imageIndex}
    );

    executeDeallocators();
    this->waitInDrawStageAndDeps.endMeasurement();
}

RenderPassHelper GDeviceVLK::beginSwapChainRenderPass(uint32_t imageIndex, CmdBufRecorder &swapChainCmd) {
    int currentDrawFrame = getDrawFrameNumber();

    //Begin render pass for swap CMD buffer.
    //It used to execute secondary command buffer, but now this is altered

    return swapChainCmd.beginRenderPass(false,
                                        swapchainRenderPass,
                                        swapChainFramebuffers[imageIndex],
                                        {0,0},
                                        {swapChainExtent.width, swapChainExtent.height},
                                        {0.117647, 0.207843, 0.392157},
                                        0.0f
    );
}

void GDeviceVLK::getNextSwapImageIndex(uint32_t &imageIndex) {
    int currentDrawFrame = getDrawFrameNumber();

    VkResult result = vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentDrawFrame]->getNativeSemaphore(), VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        std::cout << "got VK_ERROR_OUT_OF_DATE_KHR" << std::endl << std::flush;
        createSwapChainAndFramebuffer();

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
}


void GDeviceVLK::updateBuffers(std::vector<HFrameDependantData> &frameDepedantData) {
//    aggregationBufferForUpload.resize(maxUniformBufferSize);
/*
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
        m_UBOFrames[getUpdateFrameNumber()].m_uniformBufferForUpload = bufferForUpload;
        m_shaderDescriptorUpdateNeeded = true;
    }

    auto bufferForUploadVLK = std::dynamic_pointer_cast<IBufferVLK>(bufferForUpload);
    size_t old_size = bufferForUploadVLK->getSize();


    if (old_size < fullSize) {
        //TODO:
        //bufferForUploadVLK->resize(fullSize);
        //Buffer identifier was changed, so we need to update shader UBO descriptor
        m_shaderDescriptorUpdateNeeded = true;
    }

    if (fullSize > 0) {
        char *pointerForUpload = static_cast<char *>(bufferForUploadVLK->getPointer());

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

            int gran = std::max<int>(bufferVec->size() / this->threadCount, 1);

            tbb::parallel_for(tbb::blocked_range<size_t>(0, bufferVec->size(), gran),
                              [&](tbb::blocked_range<size_t> r) {
                                  for (size_t i = r.begin(); i != r.end(); ++i) {
                                      auto& buffer = (*bufferVec)[i];
                                      buffer->update(frameDepData);
                                  }
                              }, tbb::simple_partitioner());

//            for (auto &buffer : *bufferVec) {
//                buffer->update(frameDepData);
//            }
        }
        if (currentSize > 0) {
            //TODO:
            //bufferForUploadVLK->uploadFromStaging(currentSize);
        }
    }
    */
}

void GDeviceVLK::uploadTextureForMeshes(std::vector<HGMesh> &meshes) {}

std::shared_ptr<IShaderPermutation> GDeviceVLK::getShader(std::string vertexName, std::string fragmentName, void *permutationDescriptor) {
    std::string combinedName = vertexName + " " + fragmentName;
    const char * cstr = combinedName.c_str();
    size_t hash = CalculateFNV(cstr);
    if (m_shaderPermuteCache.count(hash) > 0) {
        HGShaderPermutation ptr = m_shaderPermuteCache.at(hash);
        return ptr;
    }

    std::shared_ptr<GShaderPermutationVLK> sharedPtr = std::make_shared<GShaderPermutationVLK>(vertexName, fragmentName, this->shared_from_this());
    sharedPtr->compileShader("", "");

    m_shaderPermuteCache[hash] = sharedPtr;


    return sharedPtr;
}

HGBufferVLK GDeviceVLK::createUniformBuffer(size_t initialSize) {
    auto h_uniformBuffer = std::make_shared<GBufferVLK>(this->shared_from_this(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, initialSize, uniformBufferOffsetAlign);
    return h_uniformBuffer;
}

HGBufferVLK GDeviceVLK::createVertexBuffer(size_t initialSize) {
    auto h_vertexBuffer = std::make_shared<GBufferVLK>(this->shared_from_this(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, initialSize);

    return h_vertexBuffer;
}

HGBufferVLK GDeviceVLK::createIndexBuffer(size_t initialSize) {
    auto h_indexBuffer = std::make_shared<GBufferVLK>(this->shared_from_this(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, initialSize);
    return h_indexBuffer;
}

HGVertexBufferBindings GDeviceVLK::createVertexBufferBindings() {
    std::shared_ptr<GVertexBufferBindingsVLK> h_vertexBindings;
    h_vertexBindings.reset(new GVertexBufferBindingsVLK());

    return h_vertexBindings;
}

HGTexture GDeviceVLK::createBlpTexture(HBlpTexture &texture, bool xWrapTex, bool yWrapTex) {
    return m_textureManager->createBlpTexture(texture);
}

HGTexture GDeviceVLK::createTexture(bool xWrapTex, bool yWrapTex) {
    return m_textureManager->createTexture();
}

HGMesh GDeviceVLK::createMesh(gMeshTemplate &meshTemplate) {
//    std::shared_ptr<GMeshVLK> h_mesh;
//    h_mesh.reset(new GMeshVLK(*this, meshTemplate));
//
    return nullptr;
}

HGPUFence GDeviceVLK::createFence() {
    return HGPUFence();
}

void GDeviceVLK::submitDrawCommands() {
    if (m_firstFrame) {
        m_firstFrame = false;
        return;
    }

}

void GDeviceVLK::submitQueue( VkQueue queue,
                              const std::vector<VkSemaphore> &waitSemaphores,
                              const std::vector<VkPipelineStageFlags> &waitStages,
                              const std::vector<VkCommandBuffer> &commandBuffers,
                              const std::vector<VkSemaphore> &signalSemaphoresOnCompletion,
                              const VkFence signalFenceOnCompletion) {

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;

    submitInfo.waitSemaphoreCount = waitSemaphores.size();
    submitInfo.pWaitDstStageMask = waitSemaphores.empty() ? nullptr : waitStages.data();
    submitInfo.pWaitSemaphores = waitSemaphores.empty() ? nullptr : waitSemaphores.data();

    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();

    submitInfo.signalSemaphoreCount = signalSemaphoresOnCompletion.size();
    submitInfo.pSignalSemaphores = signalSemaphoresOnCompletion.empty() ? nullptr : signalSemaphoresOnCompletion.data();

    {
        auto result = vkQueueSubmit(queue, 1, &submitInfo, signalFenceOnCompletion);
        if (result != VK_SUCCESS) {
            std::cout << "failed to submit draw command buffer! result = " << result << std::endl << std::flush;
        }
    }
}

void GDeviceVLK::presentQueue(const std::vector<VkSemaphore> &waitSemaphores,
                              const std::vector<VkSwapchainKHR> &swapchains,
                              const std::vector<uint32_t> &imageIndexes) {
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;

    presentInfo.waitSemaphoreCount = waitSemaphores.size();
    presentInfo.pWaitSemaphores = waitSemaphores.data();

    presentInfo.swapchainCount = waitSemaphores.size();
    presentInfo.pSwapchains = swapchains.data();
    presentInfo.pImageIndices = imageIndexes.data();

    auto result = vkQueuePresentKHR(graphicsQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        createSwapChainAndFramebuffer();
        return;
    } else if (result != VK_SUCCESS) {
        std::cout << "failed to present swap chain image!" << std::endl << std::flush;
    }
}

void GDeviceVLK::executeDeallocators() {
    std::lock_guard<std::mutex> lock(m_listOfDeallocatorsAccessMtx);
    while ((!listOfDeallocators.empty()) && (listOfDeallocators.front().frameNumberToDoAt <= m_frameNumber)) {
        auto stuff = listOfDeallocators.front();
        if (stuff.callback != nullptr) {
            stuff.callback();
        }

        listOfDeallocators.pop_front();
    }
}

std::shared_ptr<GRenderPassVLK> GDeviceVLK::getRenderPass(
    std::vector<ITextureFormat> textureAttachments,
    ITextureFormat depthAttachment,
    VkSampleCountFlagBits sampleCountFlagBits,
    bool invertZ,
    bool isSwapChainPass
) {
    for (auto &renderPassAvalability : m_createdRenderPasses) {
        if (renderPassAvalability.attachments.size() == textureAttachments.size() &&
            renderPassAvalability.depthAttachment == depthAttachment &&
            renderPassAvalability.sampleCountFlagBits == sampleCountFlagBits &&
            renderPassAvalability.sampleCountFlagBits == sampleCountFlagBits &&
            renderPassAvalability.isSwapChainPass == isSwapChainPass &&
            renderPassAvalability.invertZ == invertZ)
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

    std::vector<VkFormat> attachmentFormats = {};

    GFrameBufferVLK::iterateOverAttachments(textureAttachments, [&](int i, VkFormat textureFormat) {
        attachmentFormats.push_back(textureFormat);
    });
    VkFormat fbDepthFormat = findDepthFormat();

    auto renderPass = std::make_shared<GRenderPassVLK>(this->device,
        attachmentFormats,
        findDepthFormat(),
        sampleCountFlagBits,
        invertZ,
        false
    );

    RenderPassAvalabilityStruct avalabilityStruct;
    avalabilityStruct.attachments = textureAttachments;
    avalabilityStruct.depthAttachment = depthAttachment;
    avalabilityStruct.renderPass = renderPass;
    avalabilityStruct.sampleCountFlagBits = sampleCountFlagBits;
    avalabilityStruct.isSwapChainPass = isSwapChainPass;

    m_createdRenderPasses.push_back(avalabilityStruct);

    return renderPass;
}

std::shared_ptr<GRenderPassVLK> GDeviceVLK::getSwapChainRenderPass() {
    return swapchainRenderPass;
}

HPipelineVLK GDeviceVLK::createPipeline(const HGVertexBufferBindings &m_bindings,
                                        const HGShaderPermutation &shader,
                                        const std::shared_ptr<GRenderPassVLK> &renderPass,
                                        DrawElementMode element,
                                        bool backFaceCulling,
                                        bool triCCW,
                                        EGxBlendEnum blendMode,
                                        bool depthCulling,
                                        bool depthWrite) {

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

    std::shared_ptr<GPipelineVLK> hgPipeline = std::make_shared<GPipelineVLK>(*this,
                                      m_bindings, renderPass,
                                      shader, element,
                                      backFaceCulling,
                                      triCCW,
                                      blendMode,
                                      depthCulling, depthWrite);

    std::weak_ptr<GPipelineVLK> weakPtr(hgPipeline);
    loadedPipeLines[pipelineCacheRecord] = weakPtr;

    return hgPipeline;

}

VkDescriptorSet
GDeviceVLK::allocateDescriptorSetPrimitive(const std::shared_ptr<GDescriptorSetLayout> &hDescriptorSetLayout, std::shared_ptr<GDescriptorPoolVLK> &desciptorPool) {
    //1. Try to allocate from existing sets
    for (size_t i = 0; i < m_descriptorPools.size(); i++) {
        desciptorPool = m_descriptorPools[i];
        auto result = desciptorPool->allocate(hDescriptorSetLayout);
        if (result != nullptr) {
            return result;
        }
    }
    //2. Create new descriptor set and allocate from it
    {
        auto newPool = std::make_shared<GDescriptorPoolVLK>(*this);
        desciptorPool = newPool;
        m_descriptorPools.push_back(newPool);

        return newPool->allocate(hDescriptorSetLayout);
    }
}

std::shared_ptr<GDescriptorSet>
GDeviceVLK::createDescriptorSet(std::shared_ptr<GDescriptorSetLayout> &hDescriptorSetLayout) {
    std::shared_ptr<GDescriptorSet> result = std::make_shared<GDescriptorSet>(this->shared_from_this(), hDescriptorSetLayout);

    return result;
}

//void GDeviceVLK::internalDrawStageAndDeps(HDrawStage drawStage) {
//    //Draw deps
//    for (int i = 0; i < drawStage->drawStageDependencies.size(); i++) {
//        this->internalDrawStageAndDeps(drawStage->drawStageDependencies[i]);
//    }
//
//    this->setInvertZ(drawStage->invertedZ);
//
//    if (drawStage->clearScreen) {
//        clearColor[0] = drawStage->clearColor[0];
//        clearColor[1] = drawStage->clearColor[1];
//        clearColor[2] = drawStage->clearColor[2];
//    }
//
//    int updateFrame = getUpdateFrameNumber();
//    auto commandBufferForFilling = renderCommandBuffers[updateFrame];
//    //Default renderPass for rendering to screen framebuffers
//    std::shared_ptr<GRenderPassVLK> renderPass = swapchainRenderPass;
//
//    if (drawStage->target != nullptr) {
//        commandBufferForFilling = renderCommandBuffersForFrameBuffers[updateFrame];
//
//        GFrameBufferVLK *frameBufferVlk = dynamic_cast<GFrameBufferVLK *>(drawStage->target.get());
//
//        renderPass = frameBufferVlk->m_renderPass;
//
//        std::vector<VkClearValue> clearValues = renderPass->produceClearColorVec(
//            { clearColor[0], clearColor[1], clearColor[2] },
//            drawStage->invertedZ ? 0.0f : 1.0f
//        );
//
//        VkRenderPassBeginInfo renderPassInfo = {};
//        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//        renderPassInfo.pNext = NULL;
//        renderPassInfo.renderPass = renderPass->getRenderPass();
//        renderPassInfo.framebuffer = frameBufferVlk->m_frameBuffer;
//        renderPassInfo.renderArea.offset = {drawStage->viewPortDimensions.mins[0], drawStage->viewPortDimensions.mins[1]};
//        renderPassInfo.renderArea.extent = {static_cast<uint32_t>(drawStage->viewPortDimensions.maxs[0]), static_cast<uint32_t>(drawStage->viewPortDimensions.maxs[1])};
//        renderPassInfo.clearValueCount = clearValues.size();
//        renderPassInfo.pClearValues = clearValues.data();
//
//        vkCmdBeginRenderPass(commandBufferForFilling, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
//    }
//
//    std::array<VkViewport, (int)ViewportType::vp_MAX> viewportsForThisStage;
//
//    VkViewport &usualViewport = viewportsForThisStage[(int)ViewportType::vp_usual];
//    usualViewport.width = drawStage->viewPortDimensions.maxs[0];
//    usualViewport.height =  drawStage->viewPortDimensions.maxs[1];
//    usualViewport.x = drawStage->viewPortDimensions.mins[0];
//    usualViewport.y =  drawStage->viewPortDimensions.mins[1];
//    if (!getInvertZ()) {
//        usualViewport.minDepth = 0;
//        usualViewport.maxDepth = 0.990f;
//    } else {
//        usualViewport.minDepth = 0.06f;
//        usualViewport.maxDepth = 1.0f;
//    }
//
//    VkViewport &mapAreaViewport = viewportsForThisStage[(int)ViewportType::vp_mapArea];
//    mapAreaViewport = usualViewport;
//    if (!getInvertZ()) {
//        mapAreaViewport.minDepth = 0.991f;
//        mapAreaViewport.maxDepth = 0.996f;
//    } else {
//        mapAreaViewport.minDepth = 0.04f;
//        mapAreaViewport.maxDepth = 0.05f;
//    }
//
//    VkViewport &skyBoxViewport = viewportsForThisStage[(int)ViewportType::vp_skyBox];
//    skyBoxViewport = usualViewport;
//    if (!getInvertZ()) {
//        skyBoxViewport.minDepth = 0.997f;
//        skyBoxViewport.maxDepth = 1.0f;
//    } else {
//        skyBoxViewport.minDepth = 0;
//        skyBoxViewport.maxDepth = 0.03f;
//    }
//
//
//    //Set scissors
//    VkRect2D defaultScissor = {};
//    defaultScissor.offset = {0, 0};
//    defaultScissor.extent = {
//        static_cast<uint32_t>(drawStage->viewPortDimensions.maxs[0]),
//        static_cast<uint32_t>(drawStage->viewPortDimensions.maxs[1])
//    };
//
//    vkCmdSetScissor(commandBufferForFilling, 0, 1, &defaultScissor);
//
//    //Set new viewport
//    vkCmdSetViewport(commandBufferForFilling, 0, 1, &usualViewport);
//
//    bool atLeastOneDrawCall = false;
//    if (drawStage->opaqueMeshes != nullptr)
//        atLeastOneDrawCall = drawMeshesInternal(drawStage, commandBufferForFilling, renderPass,
//                                                drawStage->opaqueMeshes, viewportsForThisStage,
//                                                defaultScissor) || atLeastOneDrawCall;
//    if (drawStage->transparentMeshes != nullptr)
//        atLeastOneDrawCall = drawMeshesInternal(drawStage, commandBufferForFilling, renderPass,
//                                                drawStage->transparentMeshes, viewportsForThisStage,
//                                                defaultScissor) || atLeastOneDrawCall;
//
//    if (drawStage->target != nullptr) {
//        vkCmdEndRenderPass(commandBufferForFilling);
//        renderCommandBuffersForFrameBuffersNotNull[updateFrame] = renderCommandBuffersForFrameBuffersNotNull[updateFrame] || atLeastOneDrawCall;
//    } else {
//        renderCommandBuffersNotNull[updateFrame] = renderCommandBuffersNotNull[updateFrame] || atLeastOneDrawCall;
//    }
//
//}

//Returns true if at least once command was written to buffer
//bool GDeviceVLK::drawMeshesInternal(
//    const HDrawStage &drawStage,
//    VkCommandBuffer commandBufferForFilling,
//    std::shared_ptr<GRenderPassVLK> renderPass,
//    const HMeshesToRender &meshes,
//    const std::array<VkViewport, (int) ViewportType::vp_MAX> &viewportsForThisStage,
//    VkRect2D &defaultScissor) {
//
//    int updateFrame = getUpdateFrameNumber();
//
//    ViewportType lastViewPort = ViewportType::vp_none;
//
//    VkBuffer lastIndexBuffer = VK_NULL_HANDLE;
//    VkBuffer lastVertexBuffer = VK_NULL_HANDLE;
//    int8_t lastIsScissorsEnabled = -1;
//    std::shared_ptr<GPipelineVLK> lastPipeline = nullptr;
////    uint32_t dynamicOffset[7] = {};
//
//    auto &iMeshes = meshes->meshes;
//
//    auto dynamicOffsetPerMesh = std::vector<std::array<uint32_t, 7>>(iMeshes.size());
//    auto uboIndPerMesh = std::vector<uint32_t>(iMeshes.size());
//
//    tbb::parallel_for(
//        tbb::blocked_range<size_t>(0, iMeshes.size(), 500),
//    [&](tbb::blocked_range<size_t> r) {
//      for (size_t i = r.begin(); i != r.end(); ++i) {
//          auto *meshVLK = ((GMeshVLK *)iMeshes[i].get());
//          auto *shaderVLK = ((GShaderPermutationVLK*)meshVLK->m_shader.get());
//          uint32_t uboInd = 0;
//          auto *uboB = drawStage->sceneWideBlockVSPSChunk.get();
//          if (uboB) {
//              dynamicOffsetPerMesh[i][uboInd++] = (uboB)->getOffset();
//          }
//
//          for (int k = 1; k < 6; k++) {
//              if (shaderVLK->hasBondUBO[k]) {
//                  auto *uboB = (meshVLK->getUniformBuffer(k).get());
//                  if (uboB) {
//                      dynamicOffsetPerMesh[i][uboInd++] = (uboB)->getOffset();
//                  }
//              }
//          }
//          uboIndPerMesh[i] = uboInd;
//      }
//    }, tbb::simple_partitioner());
//
//    bool atLeastOneDrawcall = false;
//    VkDeviceSize offsets[] = {0};
//
//    for (int i = 0; i < iMeshes.size(); i++) {
//        auto *meshVLK = ((GMeshVLK *)iMeshes[i].get());
////        auto *meshVLK = ((GMeshVLK *)mesh.get());
//        auto *binding = ((GVertexBufferBindingsVLK *)meshVLK->m_bindings.get());
//        auto *shaderVLK = ((GShaderPermutationVLK*)meshVLK->m_shader.get());
//
////        uint32_t uboInd = 0;
////        auto *uboB = drawStage->sceneWideBlockVSPSChunk.get();
////        if (uboB) {
////            dynamicOffset[uboInd++] = (uboB)->getOffset();
////        }
////
////        for (int k = 1; k < 6; k++) {
////            if (shaderVLK->hasBondUBO[k]) {
////                auto *uboB = (meshVLK->getUniformBuffer(k).get());
////                if (uboB) {
////                    dynamicOffset[uboInd++] = (uboB)->getOffset();
////                }
////            }
////        }
//
//        std::shared_ptr<GPipelineVLK> h_pipeLine = meshVLK->getPipeLineForRenderPass(renderPass, drawStage->invertedZ);
//
//        if (lastPipeline != h_pipeLine) {
//            vkCmdBindPipeline(commandBufferForFilling, VK_PIPELINE_BIND_POINT_GRAPHICS,
//                              h_pipeLine->graphicsPipeline);
//
//            lastPipeline = h_pipeLine;
//        }
//
//        ViewportType newViewPort = meshVLK->m_isSkyBox ? ViewportType::vp_skyBox : ViewportType::vp_usual;
//        if (lastViewPort != newViewPort) {
//            if (viewportsForThisStage[+newViewPort].height > 32768) {
//                std::cout << "newViewPort = " << +newViewPort << std::endl;
//            }
//            vkCmdSetViewport(commandBufferForFilling, 0, 1, &viewportsForThisStage[+newViewPort]);
//
//            lastViewPort = newViewPort;
//        }
//
//
//        if (meshVLK->m_isScissorsEnabled > 0) {
//            VkRect2D rect;
//            rect.offset.x = meshVLK->m_scissorOffset[0];
//            rect.offset.y = meshVLK->m_scissorOffset[1];
//            rect.extent.width = meshVLK->m_scissorSize[0];
//            rect.extent.height = meshVLK->m_scissorSize[1];
//
//            vkCmdSetScissor(commandBufferForFilling, 0, 1, &rect);
//        } else if (lastIsScissorsEnabled != meshVLK->m_isScissorsEnabled) {
//            vkCmdSetScissor(commandBufferForFilling, 0, 1, &defaultScissor);
//        }
//        lastIsScissorsEnabled = meshVLK->m_isScissorsEnabled;
//
//
//        auto indexBuffer = ((GIndexBufferVLK *)binding->m_indexBuffer.get())->g_hIndexBuffer;
//        auto vertexBuffer = ((GVertexBufferVLK *)binding->m_bindings[0].vertexBuffer.get())->g_hVertexBuffer;
//
//
//        if (lastIndexBuffer != indexBuffer) {
//            vkCmdBindIndexBuffer(commandBufferForFilling, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
//            lastIndexBuffer = indexBuffer;
//        }
//
//        if (lastVertexBuffer != vertexBuffer) {
//            uint32_t vboBind = 0;
//
//            vkCmdBindVertexBuffers(commandBufferForFilling, vboBind++, 1, &vertexBuffer, offsets);
//            lastVertexBuffer = vertexBuffer;
//        }
//
//        auto uboDescSet = shaderVLK->uboDescriptorSets[updateFrame]->getDescSet();
//        auto imageDescSet = meshVLK->imageDescriptorSets[updateFrame]->getDescSet();
//
//        atLeastOneDrawcall = true;
//
//        //UBO
//        vkCmdBindDescriptorSets(commandBufferForFilling, VK_PIPELINE_BIND_POINT_GRAPHICS,
//                                h_pipeLine->pipelineLayout, 0, 1, &uboDescSet, uboIndPerMesh[i], dynamicOffsetPerMesh[i].data());
//
//        //Image
//        vkCmdBindDescriptorSets(commandBufferForFilling, VK_PIPELINE_BIND_POINT_GRAPHICS,
//                                h_pipeLine->pipelineLayout, 1, 1, &imageDescSet, 0, nullptr);
//
//        vkCmdDrawIndexed(commandBufferForFilling, meshVLK->m_end, 1, meshVLK->m_start/2, 0, 0);
//    }
//
//    return atLeastOneDrawcall;
//}
//
//void GDeviceVLK::drawStageAndDeps(HDrawStage drawStage) {
//    int updateFrame = getUpdateFrameNumber();
//
////    std::cout << "drawStageAndDeps: updateFrame = " << updateFrame << std::endl;
//
//    if (drawStage->target == nullptr &&
//        (
//            (drawStage->viewPortDimensions.maxs[0] != swapChainExtent.width) ||
//            (drawStage->viewPortDimensions.maxs[1] != swapChainExtent.height)
//        )
//    ) {
//        recreateSwapChain();
//    }
//
//    vkWaitForFences(device, 1, &inFlightFences[updateFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
//
//    //Update
//    if (m_shaderDescriptorUpdateNeeded) {
//        for (auto shaderVLKRec : m_shaderPermutCache) {
//            ((GShaderPermutationVLK *) shaderVLKRec.second.get())->updateDescriptorSet(updateFrame);
//        }
//        m_shaderDescriptorUpdateNeeded = false;
//    }
//
//    auto commandBufferForFilling = renderCommandBuffers[updateFrame];
//    auto commandBufferForFillingFrameBuf = renderCommandBuffersForFrameBuffers[updateFrame];
//
//    renderCommandBuffersNotNull[updateFrame] = false;
//    renderCommandBuffersForFrameBuffersNotNull[updateFrame] = false;
//
//    {
//        VkCommandBufferInheritanceInfo bufferInheritanceInfo;
//        bufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
//        bufferInheritanceInfo.pNext = nullptr;
//        bufferInheritanceInfo.renderPass = swapchainRenderPass->getRenderPass();
//        bufferInheritanceInfo.subpass = 0;
//        bufferInheritanceInfo.framebuffer = VK_NULL_HANDLE;
//        bufferInheritanceInfo.occlusionQueryEnable = false;
//        bufferInheritanceInfo.queryFlags = 0;
//        bufferInheritanceInfo.pipelineStatistics = 0;
//
//        VkCommandBufferBeginInfo beginInfo = {};
//        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
//        beginInfo.pNext = NULL;
//        beginInfo.pInheritanceInfo = &bufferInheritanceInfo;
//
//        if (vkBeginCommandBuffer(commandBufferForFilling, &beginInfo) != VK_SUCCESS) {
//            throw std::runtime_error("failed to begin recording command buffer!");
//        }
//    }
//
//    {
//        VkCommandBufferInheritanceInfo bufferInheritanceInfo;
//        bufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
//        bufferInheritanceInfo.pNext = nullptr;
//        bufferInheritanceInfo.renderPass = VK_NULL_HANDLE;
//        bufferInheritanceInfo.subpass = 0;
//        bufferInheritanceInfo.framebuffer = VK_NULL_HANDLE;
//        bufferInheritanceInfo.occlusionQueryEnable = false;
//        bufferInheritanceInfo.queryFlags = 0;
//        bufferInheritanceInfo.pipelineStatistics = 0;
//
//        VkCommandBufferBeginInfo beginInfo = {};
//        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
//        beginInfo.pNext = NULL;
//        beginInfo.pInheritanceInfo = &bufferInheritanceInfo;
//
//        if (vkBeginCommandBuffer(commandBufferForFillingFrameBuf, &beginInfo) != VK_SUCCESS) {
//            throw std::runtime_error("failed to begin recording command buffer!");
//        }
//    }
//
//    internalDrawStageAndDeps(drawStage);
//
//    if (vkEndCommandBuffer(commandBufferForFilling) != VK_SUCCESS) {
//        throw std::runtime_error("failed to record command buffer!");
//    }
//    if (vkEndCommandBuffer(commandBufferForFillingFrameBuf) != VK_SUCCESS) {
//        throw std::runtime_error("failed to record command buffer!");
//    }
//}

void GDeviceVLK::initUploadThread() {
}



HFrameBuffer GDeviceVLK::createFrameBuffer(int width, int height, std::vector<ITextureFormat> attachments,
                                           ITextureFormat depthAttachment, int multiSampleCnt, int frameNumber) {

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

    HFrameBuffer h_frameBuffer = std::make_shared<GFrameBufferVLK>(*this, attachments, depthAttachment, multiSampleCnt, width, height);

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

void GDeviceVLK::singleExecuteAndWait(std::function<void(VkCommandBuffer)> callback) {
    //Allocate temporary command buffer
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer copyCmd;

    if (vkAllocateCommandBuffers(device, &allocInfo, &copyCmd) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pNext = NULL;
    beginInfo.pInheritanceInfo = NULL;

    ERR_GUARD_VULKAN(vkBeginCommandBuffer(copyCmd, &beginInfo));

    callback(copyCmd);


    if (copyCmd == VK_NULL_HANDLE)
    {
        return;
    }

    ERR_GUARD_VULKAN(vkEndCommandBuffer(copyCmd));

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &copyCmd;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    GFenceVLK fenceVlk(this->shared_from_this(), false);

        // Submit to the queue
    ERR_GUARD_VULKAN(vkQueueSubmit(graphicsQueue, 1, &submitInfo, fenceVlk.getNativeFence()));
    // Wait for the fence to signal that command buffer has finished executing
    fenceVlk.wait(std::numeric_limits<uint64_t>::max());

    vkFreeCommandBuffers(device, commandPool, 1, &copyCmd);
}
VkSampleCountFlagBits sampleCountToVkSampleCountFlagBits(uint8_t sampleCount) {
    switch (sampleCount) {
        case 1:
            return VK_SAMPLE_COUNT_1_BIT;
        case 2:
            return VK_SAMPLE_COUNT_2_BIT;
        case 4:
            return VK_SAMPLE_COUNT_4_BIT;
        case 8:
            return VK_SAMPLE_COUNT_8_BIT;
        case 16:
            return VK_SAMPLE_COUNT_16_BIT;
        case 32:
            return VK_SAMPLE_COUNT_32_BIT;
        case 64:
            return VK_SAMPLE_COUNT_64_BIT;
        default:
            return VK_SAMPLE_COUNT_1_BIT;
    }
    return VK_SAMPLE_COUNT_1_BIT;
};


static const constexpr uint8_t countFlagBitsToSampleCount(VkSampleCountFlagBits sampleCountBit) {
    switch (sampleCountBit) {
        case VK_SAMPLE_COUNT_1_BIT:
            return 1;
        case VK_SAMPLE_COUNT_2_BIT:
            return 2;
        case VK_SAMPLE_COUNT_4_BIT:
            return 4;
        case VK_SAMPLE_COUNT_8_BIT:
            return 8;
        case VK_SAMPLE_COUNT_16_BIT:
            return 16;
        case VK_SAMPLE_COUNT_32_BIT:
            return 32;
        case VK_SAMPLE_COUNT_64_BIT:
            return 64;
        default:
            return 1;
    }

    return 1;
}

int GDeviceVLK::getMaxSamplesCnt() {
    if (maxMultiSample < 0)  {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) {
            maxMultiSample = countFlagBitsToSampleCount(VK_SAMPLE_COUNT_64_BIT);
        } else if (counts & VK_SAMPLE_COUNT_32_BIT) {
            maxMultiSample = countFlagBitsToSampleCount(VK_SAMPLE_COUNT_32_BIT);
        } else if (counts & VK_SAMPLE_COUNT_16_BIT) {
            maxMultiSample = countFlagBitsToSampleCount(VK_SAMPLE_COUNT_16_BIT);
        } else if (counts & VK_SAMPLE_COUNT_8_BIT) {
            maxMultiSample = countFlagBitsToSampleCount(VK_SAMPLE_COUNT_8_BIT);
        } else if (counts & VK_SAMPLE_COUNT_4_BIT) {
            maxMultiSample = countFlagBitsToSampleCount(VK_SAMPLE_COUNT_4_BIT);
        } else if (counts & VK_SAMPLE_COUNT_2_BIT) {
            maxMultiSample = countFlagBitsToSampleCount(VK_SAMPLE_COUNT_2_BIT);
        } else if (counts & VK_SAMPLE_COUNT_1_BIT) {
            maxMultiSample = countFlagBitsToSampleCount(VK_SAMPLE_COUNT_1_BIT);
        } else {
            maxMultiSample = countFlagBitsToSampleCount(VK_SAMPLE_COUNT_1_BIT);
        }
    }

    return maxMultiSample;
}

VkSampleCountFlagBits GDeviceVLK::getMaxSamplesBit() {
    return sampleCountToVkSampleCountFlagBits(getMaxSamplesCnt());
}
