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
#include "pipeline/GPipelineVLK.h"
#include "../../engine/algorithms/hashString.h"
#include "GFrameBufferVLK.h"
#include "shaders/GShaderPermutationVLK.h"
#include "GRenderPassVLK.h"
#include "../../engine/algorithms/FrameCounter.h"
#include "buffers/GBufferVLK.h"
#include "synchronization/GFenceVLK.h"
#include "../../renderer/vulkan/IRenderFunctionVLK.h"
#include "commandBuffer/commandBufferRecorder/TextureUploadHelper.h"
#include "../../renderer/frame/FrameProfile.h"
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


std::set<std::string> get_supported_extensions() {
    VkResult result = VK_NOT_READY;

    /*
     * From the link above:
     * If `pProperties` is NULL, then the number of extensions properties
     * available is returned in `pPropertyCount`.
     *
     * Basically, gets the number of extensions.
     */
    uint32_t count = 0;
    result = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    if (result != VK_SUCCESS) {
        return {};
    }

    std::vector<VkExtensionProperties> extensionProperties(count);

    // Get the extensions
    result = vkEnumerateInstanceExtensionProperties(nullptr, &count, extensionProperties.data());
    if (result != VK_SUCCESS) {
        // Throw an exception or log the error
        return {};
    }

    std::set<std::string> extensions;
    for (auto & extension : extensionProperties) {
        extensions.insert(extension.extensionName);
    }

    return extensions;
}

GDeviceVLK::GDeviceVLK(vkCallInitCallback * callback) : m_textureManager(std::make_shared<TextureManagerVLK>(*this)),
                                                        m_descriptorSetUpdater(std::make_shared<GDescriptorSetUpdater>()){
    enableValidationLayers = true;

    if (volkInitialize()) {
        std::cerr << "Failed to initialize volk loader" << std::endl;
        exit(1);
    }

    this->threadCount = std::max<int>((int)std::thread::hardware_concurrency() - 3, 1);

    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    uint32_t apiVersion = VK_API_VERSION_1_0;
    if (vkEnumerateInstanceVersion != nullptr)
        vkEnumerateInstanceVersion(&apiVersion);

    if (apiVersion > VK_API_VERSION_1_2)
        apiVersion = VK_API_VERSION_1_2;

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "WoW Map Viewer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "CustomMadeEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = apiVersion;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.pApplicationInfo = &appInfo;

    {
        auto supportedExtensions = get_supported_extensions();
        std::cout << "available instance extensions: " << std::endl;
        for (auto &ext: supportedExtensions) {
            std::cout << " " << ext << std::endl;
        }
        std::cout << std::endl;
    }

    char** extensions;
    int extensionCnt = 0;
    callback->getRequiredExtensions(extensions, extensionCnt);
    std::vector<const char *> extensionsVec(extensions, extensions+extensionCnt);
    if (enableValidationLayers) {
        extensionsVec.push_back("VK_EXT_debug_report");
    }

    //TODO: disable in general case
    extensionsVec.push_back("VK_EXT_debug_utils");

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
    uniformBufferOffsetAlign = deviceProperties.limits.minUniformBufferOffsetAlignment;
    maxUniformBufferSize = deviceProperties.limits.maxUniformBufferRange;
//---------------

    m_textureManager->initialize();
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
    m_blackPixelTexture->getTexture()->loadData(1,1,&zero, ITextureFormat::itRGBA);

    m_whitePixelTexture = createTexture(false, false);
    unsigned int ff = 0xffffffff;
    m_whitePixelTexture->getTexture()->loadData(1,1,&ff, ITextureFormat::itRGBA);

    m_ringBuffer = std::make_shared<GStagingRingBuffer>(shared_from_this());
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

    uint32_t imageCount = 0;
    if (swapChain != VK_NULL_HANDLE) {
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    }

    //Create swapchainImages and framebuffer
    std::vector<VkImage> swapChainImages = {};
    swapChainImages.resize(imageCount);

    if (swapChain != VK_NULL_HANDLE) {
        ERR_GUARD_VULKAN(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data()));
    }

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
        swapChainTextures[i] = std::make_shared<GTextureVLK>(*this, swapChainImages[i], swapChainImageViews[i],
                                                             false);
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
    if (swapChainSupport.capabilities.minImageCount < 2) {
        std::cerr << "Your GPU doesnt support 2 swapchain images inFlight, which is required by app" << std::endl << std::flush;
    }
    imageCount = swapChainSupport.capabilities.minImageCount;



//    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
//        imageCount = swapChainSupport.capabilities.maxImageCount;
//    }

    auto oldSwapChain = swapChain;

    if (extent.width > 0 && extent.height > 0) {
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


        createInfo.oldSwapchain = oldSwapChain;

        auto error = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);
        if (error != VK_SUCCESS) {
            std::cout << "error = " << error << std::endl << std::flush;
            throw std::runtime_error("failed to create swap chain!");
        }
    } else {
        swapChain = VK_NULL_HANDLE;
    }

    if (oldSwapChain != VK_NULL_HANDLE) {
        auto vkDevice = this->getVkDevice();
        addDeallocationRecord([oldSwapChain, vkDevice]() {
            vkDestroySwapchainKHR(vkDevice, oldSwapChain, nullptr);
        });
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

std::string VkMemoryPropertyFlagBitsGetString(VkMemoryPropertyFlags value) {
    std::string strings = "";
    if (value == 0) { strings += "None"; return strings; }
    if (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT & value) strings += " MEMORY_PROPERTY_DEVICE_LOCAL_BIT";
    if (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT & value) strings += " MEMORY_PROPERTY_HOST_VISIBLE_BIT";
    if (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & value) strings += " MEMORY_PROPERTY_HOST_COHERENT_BIT";
    if (VK_MEMORY_PROPERTY_HOST_CACHED_BIT & value) strings += " MEMORY_PROPERTY_HOST_CACHED_BIT";
    if (VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT & value) strings += " MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT";
    if (VK_MEMORY_PROPERTY_PROTECTED_BIT & value) strings += " MEMORY_PROPERTY_PROTECTED_BIT";
    if (VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD & value) strings += " MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD";
    if (VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD & value) strings += " MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD";
    if (VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV & value) strings += " MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV";
    return strings;
}

std::string VkMemoryHeapFlagBitsGetStrings(VkMemoryHeapFlags value) {
    std::string string;
    if (value == 0) { string  += "None"; return string; }
    if (VK_MEMORY_HEAP_DEVICE_LOCAL_BIT & value) string  += " MEMORY_HEAP_DEVICE_LOCAL_BIT";
    if (VK_MEMORY_HEAP_MULTI_INSTANCE_BIT & value) string  += " MEMORY_HEAP_MULTI_INSTANCE_BIT";
    return string;
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

            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(device, &props);
            std::cout << "Selected " << props.deviceName << " -- " <<  (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? "suitable GPU" : "integrated GPU") << std::endl;

            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

    std::cout << "deviceMemoryProperties.memoryHeapCount = " << deviceMemoryProperties.memoryHeapCount << std::endl;
    for (int i = 0; i < deviceMemoryProperties.memoryHeapCount; i++) {
        std::cout << "memoryHeaps["<<i<<"] " << std::endl;
        std::cout << "  size = " << deviceMemoryProperties.memoryHeaps[i].size << std::endl;
        std::cout << "  flags = " << VkMemoryHeapFlagBitsGetStrings(deviceMemoryProperties.memoryHeaps[i].flags) << std::endl;
    }
    std::cout << std::endl;

    std::cout << "deviceMemoryProperties.memoryTypeCount = " << deviceMemoryProperties.memoryTypeCount << std::endl;
    for (int i = 0; i < deviceMemoryProperties.memoryTypeCount; i++) {
        auto memType = deviceMemoryProperties.memoryTypes[i];
        std::cout << "memoryTypes["<<i<<"] " << std::endl;
        std::cout << "  heapIndex = " << memType.heapIndex << " "
                   << VkMemoryPropertyFlagBitsGetString(memType.propertyFlags) << std::endl;
    }
    std::cout << std::endl;
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

    VkPhysicalDeviceDescriptorIndexingFeatures indexing_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT, nullptr };
    VkPhysicalDeviceShaderDrawParametersFeatures ext_feature = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES, nullptr};
    bool bindless_supported = false;
    bool hasDeviceFeatures2 = vkGetPhysicalDeviceFeatures2 != nullptr;
    if (hasDeviceFeatures2) {
        //Check Indexing features
        VkPhysicalDeviceFeatures2 physical_features2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &indexing_features};
        vkGetPhysicalDeviceFeatures2(physicalDevice, &physical_features2);

        //Check DrawParameters features

        physical_features2.pNext = &ext_feature;
        vkGetPhysicalDeviceFeatures2(physicalDevice, &physical_features2);
        if (ext_feature.shaderDrawParameters == VK_FALSE) {
            std::cout << "Draw parameters are not supported on this device" << std::endl;
        }


        bindless_supported = indexing_features.shaderSampledImageArrayNonUniformIndexing  &&
            indexing_features.runtimeDescriptorArray &&
            indexing_features.descriptorBindingVariableDescriptorCount &&
            indexing_features.descriptorBindingPartiallyBound &&
            ext_feature.shaderDrawParameters == VK_TRUE;

        if (!bindless_supported)
            std::cout << "Bindless is not supported on this device" << std::endl;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = supportedFeatures.samplerAnisotropy;
    deviceFeatures.textureCompressionBC = supportedFeatures.textureCompressionBC;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = NULL;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    std::vector<const char*> enabledDeviceExtensions = deviceExtensions;
    createInfo.enabledExtensionCount = enabledDeviceExtensions.size();
    createInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    VkPhysicalDeviceFeatures2 physical_features2 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
    if ( bindless_supported ) {
        // This should be already set to VK_TRUE, as we queried before.
        indexing_features.descriptorBindingPartiallyBound = VK_TRUE;
        indexing_features.runtimeDescriptorArray = VK_TRUE;

        ext_feature.shaderDrawParameters = VK_TRUE;
        indexing_features.pNext = &ext_feature;

        vkGetPhysicalDeviceFeatures2( physicalDevice, &physical_features2 );
        physical_features2.pNext = &indexing_features;
        physical_features2.features = deviceFeatures;

        createInfo.pEnabledFeatures = nullptr;
        createInfo.pNext = &physical_features2;

        m_supportsBindless = true;
   } else {
        createInfo.pEnabledFeatures = &deviceFeatures;
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

    indices.transferFamily = indices.graphicsFamily;
}

void GDeviceVLK::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.pNext = NULL;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
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
    renderPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
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
        commandBuffer = std::make_shared<GCommandBuffer>(*this, graphicsQueue, commandPool, true, indices.graphicsFamily.value());
    }
    for (auto & commandBuffer : swapChainCommandBuffers) {
        commandBuffer = std::make_shared<GCommandBuffer>(*this, graphicsQueue, commandPool, true, indices.graphicsFamily.value());
    }

    for (auto & commandBuffer : uploadCommandBuffers) {
        commandBuffer = std::make_shared<GCommandBuffer>(*this, uploadQueue, uploadCommandPool, true, indices.transferFamily.value());
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


unsigned int GDeviceVLK::getProcessingFrameNumber() {
    return m_frameNumber % MAX_FRAMES_IN_FLIGHT;
}

void GDeviceVLK::increaseFrameNumber() {
    m_frameNumber++;
}

bool GDeviceVLK::getIsAnisFiltrationSupported() {
    return supportedFeatures.samplerAnisotropy;
};
bool GDeviceVLK::getIsBCCompressedTexturesSupported() {
    return supportedFeatures.textureCompressionBC;
};

float GDeviceVLK::getAnisLevel() {
    return deviceProperties.limits.maxSamplerAnisotropy;
}
void GDeviceVLK::flushRingBuffer() {
    m_ringBuffer->flushBuffers();
}

void GDeviceVLK::drawFrame(const FrameRenderFuncs &frameRenderFuncs, bool windowSizeChanged) {
    ZoneScoped;

    int currentDrawFrame = getProcessingFrameNumber();

    auto &uploadCmdBuf = uploadCommandBuffers[currentDrawFrame];
    auto &swapChainCmdBuf = swapChainCommandBuffers[currentDrawFrame];
    auto &frameBufCmdBuf = fbCommandBuffers[currentDrawFrame];

    if (windowSizeChanged) {
        createSwapChainAndFramebuffer();
    }

    uint32_t imageIndex = -1;
    {
        {
            ZoneScopedN("frameBuf CMD wait");
            //Wait for frameBuf CMD buffer to become available
            frameBufFences[currentDrawFrame]->wait(std::numeric_limits<uint64_t>::max());
            uploadFences[currentDrawFrame]->wait(std::numeric_limits<uint64_t>::max());
            frameBufFences[currentDrawFrame]->reset();
            uploadFences[currentDrawFrame]->reset();

        }

        {
            auto uploadCmd = uploadCmdBuf->beginRecord(nullptr);
            {
                auto const &renderFuncs = frameRenderFuncs.renderFuncs;
                for (int i = 0; i < renderFuncs.size(); i++) {
                    dynamic_cast<IRenderFunctionVLK *>(renderFuncs[i].get())->executeUpload(*this, uploadCmd);
                }
            }

            //Do Texture update
            {
                ZoneScopedN("Texture update");
                m_textureManager->processBLPTextures();
                auto textureVector = m_textureManager->getReadyToUploadTextures();
                textureUploadStrategy(textureVector.get(), uploadCmd);
                //The next loop updates DescriptorSets
                for (auto &wtexture: textureVector.get()) {
                    if (auto texture = wtexture.lock()) {
                        texture->executeOnChange();
                    }
                }
            }
        }
        if (!frameRenderFuncs.renderFuncs.empty()) {
            flushRingBuffer();
        }
        {
            submitQueue(
                uploadQueue,
                {},
                {},
                {uploadCmdBuf->getNativeCmdBuffer()},
                {uploadSemaphores[currentDrawFrame]->getNativeSemaphore()},
                uploadFences[currentDrawFrame]->getNativeFence()
            );
        }


        {
            ZoneScopedN("DescriptorSet update");
            m_descriptorSetUpdater->updateDescriptorSets();
        }

        //Wait for swapchain
        {
            ZoneScopedN("Swapchain wait");
            inFlightFences[currentDrawFrame]->wait(std::numeric_limits<uint64_t>::max());
            inFlightFences[currentDrawFrame]->reset();
        }

        auto swapChainCmd = swapChainCmdBuf->beginRecord(nullptr);
        auto frameBufCmd = frameBufCmdBuf->beginRecord(nullptr);

        if (swapChain != VK_NULL_HANDLE)
        {
            //Begin render pass for Swap chain
            this->getNextSwapImageIndex(imageIndex);
            auto swapChainRenderPass = this->beginSwapChainRenderPass(imageIndex, swapChainCmd);

            auto const &renderFuncs = frameRenderFuncs.renderFuncs;
            for (int i = 0; i < renderFuncs.size(); i++) {
                dynamic_cast<IRenderFunctionVLK *>(renderFuncs[i].get())->executeRender(*this, frameBufCmd, swapChainCmd);
            }
        }
    }

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

    std::vector<VkSemaphore> waitSemaphores = { frameBufSemaphores[currentDrawFrame]->getNativeSemaphore() };
    if (swapChain != VK_NULL_HANDLE) waitSemaphores.push_back(imageAvailableSemaphores[currentDrawFrame]->getNativeSemaphore());

    std::vector<VkSemaphore> signalSemaphores = {};
    if (swapChain != VK_NULL_HANDLE) signalSemaphores = {renderFinishedSemaphores[currentDrawFrame]->getNativeSemaphore()};

    submitQueue(
        graphicsQueue,
        waitSemaphores,
        {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
        {swapChainCmdBuf->getNativeCmdBuffer()},
        signalSemaphores,
        inFlightFences[currentDrawFrame]->getNativeFence()
    );

    for (auto const &callback : frameRenderFuncs.onFinish) {
        inFlightFences[currentDrawFrame]->addOnFinish(callback);
    }

    if (swapChain != VK_NULL_HANDLE) {
        presentQueue(
            {renderFinishedSemaphores[currentDrawFrame]->getNativeSemaphore()},
            {swapChain},
            {imageIndex}
        );
    }

    executeDeallocators();
}

RenderPassHelper GDeviceVLK::beginSwapChainRenderPass(uint32_t imageIndex, CmdBufRecorder &swapChainCmd) {
    int currentDrawFrame = getProcessingFrameNumber();

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
    int currentDrawFrame = getProcessingFrameNumber();

    if (swapChain == VK_NULL_HANDLE) {
        imageIndex = 0xFFFFFFFF;
        return;
    }

    VkResult result = vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentDrawFrame]->getNativeSemaphore(), VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        std::cout << "got VK_ERROR_OUT_OF_DATE_KHR" << std::endl << std::flush;
        createSwapChainAndFramebuffer();

        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        std::cout << "error happened " << result << std::endl << std::flush;
//        throw std::runtime_error("failed to acquire swap chain image!");
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

std::shared_ptr<IShaderPermutation> GDeviceVLK::getShader(std::string vertexName, std::string fragmentName, const ShaderConfig &shaderConfig) {

    ShaderPermutationCacheRecord cacheRecord;
    cacheRecord.name = vertexName + " " + fragmentName;
    cacheRecord.shaderConfig = shaderConfig;

    if (m_shaderPermuteCache.find(cacheRecord) != m_shaderPermuteCache.end()) {
        HGShaderPermutation ptr = m_shaderPermuteCache.at(cacheRecord);
        return ptr;
    }

    std::shared_ptr<GShaderPermutationVLK> sharedPtr = std::make_shared<GShaderPermutationVLK>(vertexName, fragmentName, this->shared_from_this(), shaderConfig);
    sharedPtr->compileShader("", "");

    m_shaderPermuteCache[cacheRecord] = sharedPtr;

    return sharedPtr;
}

HGBufferVLK GDeviceVLK::createUniformBuffer(const char * objName, size_t initialSize) {
    auto h_uniformBuffer = std::make_shared<GBufferVLK>(this->shared_from_this(), objName, m_ringBuffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, initialSize, uniformBufferOffsetAlign);
    return h_uniformBuffer;
}

HGBufferVLK GDeviceVLK::createSSBOBuffer(const char * objName, size_t initialSize, int recordSize) {
    auto h_uniformBuffer = std::make_shared<GBufferVLK>(this->shared_from_this(), objName, m_ringBuffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, initialSize, recordSize);
    return h_uniformBuffer;
}

HGBufferVLK GDeviceVLK::createVertexBuffer(const char * objName, size_t initialSize, int recordSize) {
    auto h_vertexBuffer = std::make_shared<GBufferVLK>(this->shared_from_this(), objName, m_ringBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, initialSize, recordSize);

    return h_vertexBuffer;
}

HGBufferVLK GDeviceVLK::createIndexBuffer(const char * objName, size_t initialSize) {
    auto h_indexBuffer = std::make_shared<GBufferVLK>(this->shared_from_this(), objName, m_ringBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, initialSize, 2);
    return h_indexBuffer;
}

HGVertexBufferBindings GDeviceVLK::createVertexBufferBindings() {
    std::shared_ptr<GVertexBufferBindingsVLK> h_vertexBindings;
    h_vertexBindings.reset(new GVertexBufferBindingsVLK());

    return h_vertexBindings;
}

HGSamplableTexture GDeviceVLK::createBlpTexture(HBlpTexture &texture, bool xWrapTex, bool yWrapTex) {
    return m_textureManager->createBlpTexture(texture, xWrapTex, yWrapTex);
}

HGSamplableTexture GDeviceVLK::createTexture(bool xWrapTex, bool yWrapTex) {
    return m_textureManager->createTexture(xWrapTex, yWrapTex);
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
    presentInfo.pNext = nullptr;

    presentInfo.waitSemaphoreCount = waitSemaphores.size();
    presentInfo.pWaitSemaphores = waitSemaphores.data();

    presentInfo.swapchainCount = waitSemaphores.size();
    presentInfo.pSwapchains = swapchains.data(); 
    presentInfo.pImageIndices = imageIndexes.data();

    auto result = vkQueuePresentKHR(graphicsQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
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
    const std::vector<ITextureFormat> &textureAttachments,
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



    auto renderPass = std::make_shared<GRenderPassVLK>(*this,
        textureAttachments,
        depthAttachment,
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
                                        const std::shared_ptr<GPipelineLayoutVLK> &pipelineLayout,
                                        const std::shared_ptr<GRenderPassVLK> &renderPass,
                                        DrawElementMode element,
                                        bool backFaceCulling,
                                        bool triCCW,
                                        EGxBlendEnum blendMode,
                                        bool depthCulling,
                                        bool depthWrite) {

    PipelineCacheRecord pipelineCacheRecord = {
        .shader = shader,
        .renderPass = std::weak_ptr<GRenderPassVLK>(renderPass),
        .pipelineLayout = std::weak_ptr<GPipelineLayoutVLK>(pipelineLayout),
        .element = element,
        .backFaceCulling = backFaceCulling,
        .triCCW = triCCW,
        .blendMode = blendMode,
        .depthCulling = depthCulling,
        .depthWrite = depthWrite,
    };

    auto i = loadedPipeLines.find(pipelineCacheRecord);
    if (i != loadedPipeLines.end()) {
        if (!i->second.expired()) {
            return i->second.lock();
        } else {
            loadedPipeLines.erase(i);
        }
    }

    std::shared_ptr<GPipelineVLK> hgPipeline = std::make_shared<GPipelineVLK>(*this,
                                      m_bindings, renderPass, pipelineLayout,
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
    bool isBindlessDS = hDescriptorSetLayout->getIsBindless();
    auto &dsPools = isBindlessDS ? m_bindlessDescriptorPools : m_descriptorPools;

    //1. Try to allocate from existing sets
    for (int i = dsPools.size() - 1; i >= 0 ; i--) {
        desciptorPool = dsPools[i];
        auto result = desciptorPool->allocate(hDescriptorSetLayout);
        if (result != nullptr) {
            return result;
        }
    }
    //2. Create new descriptor set and allocate from it
    {
        auto newPool = std::make_shared<GDescriptorPoolVLK>(*this, isBindlessDS);
        desciptorPool = newPool;
        dsPools.push_back(newPool);

        return newPool->allocate(hDescriptorSetLayout);
    }
}

std::shared_ptr<GDescriptorSet>
GDeviceVLK::createDescriptorSet(std::shared_ptr<GDescriptorSetLayout> &hDescriptorSetLayout) {
    std::shared_ptr<GDescriptorSet> result = std::make_shared<GDescriptorSet>(this->shared_from_this(), hDescriptorSetLayout);

    return result;
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
    beginInfo.pNext = nullptr;
    beginInfo.pInheritanceInfo = nullptr;

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

HGSamplableTexture GDeviceVLK::createSampledTexture(HGTexture texture, bool xWrapTex, bool yWrapTex) {
    return this->m_textureManager->createSampledTexture(xWrapTex, yWrapTex, texture);
}
