//
// Created by Deamon on 7/3/2023.
//

#ifndef AWEBWOWVIEWERCPP_SHADERCONFIG_H
#define AWEBWOWVIEWERCPP_SHADERCONFIG_H

#include <unordered_map>
#include "../context/vulkan_context.h"

struct ShaderConfig {
    std::unordered_map<int, VkDescriptorType> typeOverrides;
};

#endif //AWEBWOWVIEWERCPP_SHADERCONFIG_H
