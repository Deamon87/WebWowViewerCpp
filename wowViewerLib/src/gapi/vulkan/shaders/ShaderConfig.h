//
// Created by Deamon on 7/3/2023.
//

#ifndef AWEBWOWVIEWERCPP_SHADERCONFIG_H
#define AWEBWOWVIEWERCPP_SHADERCONFIG_H

#include <unordered_map>
#include "../context/vulkan_context.h"

//Per DescSet -> per binding point
typedef std::unordered_map<int,std::unordered_map<int, VkDescriptorType>> DescTypeOverride;

struct ShaderConfig {
    std::string shaderFolder;
    DescTypeOverride typeOverrides;
};

#endif //AWEBWOWVIEWERCPP_SHADERCONFIG_H
