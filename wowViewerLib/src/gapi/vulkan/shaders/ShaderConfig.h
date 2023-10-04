//
// Created by Deamon on 7/3/2023.
//

#ifndef AWEBWOWVIEWERCPP_SHADERCONFIG_H
#define AWEBWOWVIEWERCPP_SHADERCONFIG_H

#include <unordered_map>
#include "../context/vulkan_context.h"

//Per DescSet -> per binding point
struct DescTypeConfig {
    VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    bool isBindless = false;
    int descriptorCount = 1;
    uint32_t stageMask;

    bool operator==(const DescTypeConfig &other) const {
        return
            (type == other.type) &&
            (isBindless == other.isBindless) &&
            (stageMask == other.stageMask) &&
            (descriptorCount == other.descriptorCount);
    };
};

typedef std::unordered_map<int,std::unordered_map<int, DescTypeConfig>> DescTypeOverride;

struct ShaderConfig {
    std::string shaderFolder;
    DescTypeOverride typeOverrides;
};

#endif //AWEBWOWVIEWERCPP_SHADERCONFIG_H
