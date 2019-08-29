//
// Created by Deamon on 8/1/2019.
//

#include <iostream>
#include <cstring>
#include <set>
#include <vector>
#include <zconf.h>
#include <ctime>
#include "GDeviceVulkan.h"
#include "../../include/vulkancontext.h"

#include "buffers/GVertexBufferVLK.h"
#include "buffers/GIndexBufferVLK.h"

const int WIDTH = 800;
const int HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
    "VK_LAYER_LUNARG_assistant_layer",
    "VK_LAYER_LUNARG_api_dump",

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
//    createGraphicsPipeline(); //TODO:
    createFramebuffers();
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();
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

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

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

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
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

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 0;
    renderPassInfo.pDependencies = nullptr;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}
//
//void GDeviceVLK::createGraphicsPipeline() {
//    auto vertShaderCode = readFile("shaders/vert.spv");
//    auto fragShaderCode = readFile("shaders/frag.spv");
//
//    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
//    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
//
//    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
//    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
//    vertShaderStageInfo.module = vertShaderModule;
//    vertShaderStageInfo.pName = "main";
//
//    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
//    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//    fragShaderStageInfo.module = fragShaderModule;
//    fragShaderStageInfo.pName = "main";
//
//    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
//
//    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
//    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//    vertexInputInfo.vertexBindingDescriptionCount = 0;
//    vertexInputInfo.vertexAttributeDescriptionCount = 0;
//
//    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
//    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//    inputAssembly.primitiveRestartEnable = VK_FALSE;
//
//    VkViewport viewport = {};
//    viewport.x = 0.0f;
//    viewport.y = 0.0f;
//    viewport.width = (float) swapChainExtent.width;
//    viewport.height = (float) swapChainExtent.height;
//    viewport.minDepth = 0.0f;
//    viewport.maxDepth = 1.0f;
//
//    VkRect2D scissor = {};
//    scissor.offset = {0, 0};
//    scissor.extent = swapChainExtent;
//
//    VkPipelineViewportStateCreateInfo viewportState = {};
//    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//    viewportState.viewportCount = 1;
//    viewportState.pViewports = &viewport;
//    viewportState.scissorCount = 1;
//    viewportState.pScissors = &scissor;
//
//    VkPipelineRasterizationStateCreateInfo rasterizer = {};
//    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//    rasterizer.depthClampEnable = VK_FALSE;
//    rasterizer.rasterizerDiscardEnable = VK_FALSE;
//    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
//    rasterizer.lineWidth = 1.0f;
//    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
//    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
//    rasterizer.depthBiasEnable = VK_FALSE;
//
//    VkPipelineMultisampleStateCreateInfo multisampling = {};
//    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//    multisampling.sampleShadingEnable = VK_FALSE;
//    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
//
//    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
//    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//    colorBlendAttachment.blendEnable = VK_FALSE;
//
//    VkPipelineColorBlendStateCreateInfo colorBlending = {};
//    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//    colorBlending.logicOpEnable = VK_FALSE;
//    colorBlending.logicOp = VK_LOGIC_OP_COPY;
//    colorBlending.attachmentCount = 1;
//    colorBlending.pAttachments = &colorBlendAttachment;
//    colorBlending.blendConstants[0] = 0.0f;
//    colorBlending.blendConstants[1] = 0.0f;
//    colorBlending.blendConstants[2] = 0.0f;
//    colorBlending.blendConstants[3] = 0.0f;
//
//    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
//    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//    pipelineLayoutInfo.setLayoutCount = 0;
//    pipelineLayoutInfo.pushConstantRangeCount = 0;
//
//    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
//        throw std::runtime_error("failed to create pipeline layout!");
//    }
//
//    VkGraphicsPipelineCreateInfo pipelineInfo = {};
//    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//    pipelineInfo.stageCount = 2;
//    pipelineInfo.pStages = shaderStages;
//    pipelineInfo.pVertexInputState = &vertexInputInfo;
//    pipelineInfo.pInputAssemblyState = &inputAssembly;
//    pipelineInfo.pViewportState = &viewportState;
//    pipelineInfo.pRasterizationState = &rasterizer;
//    pipelineInfo.pMultisampleState = &multisampling;
//    pipelineInfo.pColorBlendState = &colorBlending;
//    pipelineInfo.layout = pipelineLayout;
//    pipelineInfo.renderPass = renderPass;
//    pipelineInfo.subpass = 0;
//    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
//
//    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
//        throw std::runtime_error("failed to create graphics pipeline!");
//    }
//
//    vkDestroyShaderModule(device, fragShaderModule, nullptr);
//    vkDestroyShaderModule(device, vertShaderModule, nullptr);
//}

void GDeviceVLK::createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {
            swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.pNext = NULL;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
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
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

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

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

void GDeviceVLK::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.pNext = NULL;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
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

    if (vkAllocateCommandBuffers(device, &allocInfo, &uploadCommandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate upload command buffers!");
    }


    for (size_t i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pNext = NULL;
        beginInfo.pInheritanceInfo = NULL;

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        std::array<VkClearValue, 2> clearValues = {};
        clearValues[0].color = {clearColor[0], clearColor[1], clearColor[2], 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.pNext = NULL;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[i % swapChainFramebuffers.size()];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


//        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
//
//        VkBuffer vertexBuffers[] = {vertexBuffer};
//        VkDeviceSize offsets[] = {0};
//        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
//
//        vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
//
//        vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);
//
//        vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);

        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}
void GDeviceVLK::createSyncObjects() {
    imageAvailableSemaphores.resize(commandBuffers.size());
    renderFinishedSemaphores.resize(commandBuffers.size());
    inFlightFences.resize(commandBuffers.size());

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
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }

    VkFenceCreateInfo uploadFenceInfo = {};
    uploadFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    uploadFenceInfo.pNext = NULL;
    uploadFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    uploadSemaphores.resize(4);
    for (size_t i = 0; i < 4; i++) {
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &uploadSemaphores[i]);
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

void GDeviceVLK::updateBuffers(std::vector<HGMesh> &meshes) {
    updateCommandBuffers();

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pNext = NULL;
    beginInfo.pInheritanceInfo = NULL;

    if (vkBeginCommandBuffer(uploadCommandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording uploadCommandBuffer command buffer!");
    }
}

void GDeviceVLK::uploadTextureForMeshes(std::vector<HGMesh> &meshes) {

    if (vkEndCommandBuffer(uploadCommandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record uploadCommandBuffer command buffer!");
    }

    int uploadFrame = getUpdateFrameNumber();


    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &uploadCommandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &uploadSemaphores[uploadFrame];

    if (vkQueueSubmit(uploadQueue, 1, &submitInfo, nullptr) != VK_SUCCESS) {
        std::cout << "failed to submit draw command buffer!" << std::endl << std::flush;
    }
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

void GDeviceVLK::beginFrame() {

}

void GDeviceVLK::commitFrame() {
    int currentDrawFrame = getDrawFrameNumber();


    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[0], VK_NULL_HANDLE, &imageIndex);

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

    vkWaitForFences(device, 1, &inFlightFences[imageIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(device, 1, &inFlightFences[imageIndex]);

//    vkWaitForFences(device, 1, &uploadFences[currentDrawFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
//    vkResetFences(device, 1, &uploadFences[currentDrawFrame]);
//    updateUniformBuffer(imageIndex);



    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[0], uploadSemaphores[currentDrawFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 2;
    submitInfo.pWaitSemaphores = &waitSemaphores[0];
    submitInfo.pWaitDstStageMask = &waitStages[0];

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentDrawFrame];

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphores[0];

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[imageIndex]) != VK_SUCCESS) {
        std::cout << "failed to submit draw command buffer!" << std::endl << std::flush;
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphores[0];

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

void GDeviceVLK::updateCommandBuffers() {
    int updateFrame = getUpdateFrameNumber();

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pNext = NULL;
    beginInfo.pInheritanceInfo = NULL;

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
    renderPassInfo.framebuffer = swapChainFramebuffers[updateFrame % swapChainFramebuffers.size()];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[updateFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


//        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
//
//        VkBuffer vertexBuffers[] = {vertexBuffer};
//        VkDeviceSize offsets[] = {0};
//        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
//
//        vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
//
//        vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);
//
//        vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffers[updateFrame]);

    if (vkEndCommandBuffer(commandBuffers[updateFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}
