//
// Created by Deamon on 7/1/2018.
//

#ifndef AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H

class GDeviceGL33;

#include <string>
#include <unordered_map>
#include "../GDeviceVulkan.h"
#include "../../interface/IShaderPermutation.h"

class GShaderPermutationVLK : public IShaderPermutation {
    friend class GDeviceVulkan;

public:
    ~GShaderPermutationVLK() override {};

    VkShaderModule getVertexModule() {return vertShaderModule;}
    VkShaderModule getFragmentModule() {return fragShaderModule;}

protected:
    explicit GShaderPermutationVLK(std::string &shaderName, IDevice *device);

    VkShaderModule createShaderModule(const std::vector<char>& code);

    void compileShader(const std::string &vertExtraDefStrings, const std::string &fragExtraDefStrings) override;


    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;
private:
    GDeviceVLK *m_device;


private:
    std::unordered_map<size_t, unsigned int> m_uniformMap;
    int m_uboVertexBlockIndex[3];
    int m_uboFragmentBlockIndex[3];
    std::string m_shaderName;



};


#endif //AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H
