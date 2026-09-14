//
// Created by Deamon on 7/1/2018.
//

#ifndef AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H

#include <string>
#include <array>
#include <unordered_map>
#include "GShaderPermutationVLKBase.h"
#include "../../interface/IShaderPermutation.h"


class GShaderPermutationVLK : public GShaderPermutationVLKBase, public IShaderPermutation {
    friend class GDeviceVLK;
public:
    explicit GShaderPermutationVLK(const std::string &shaderVertName, const std::string &shaderFragName,
                                   const std::shared_ptr<GDeviceVLK> &device, const ShaderConfig &shaderConf,
                                   const std::unordered_map<int, const std::shared_ptr<GDescriptorSetLayout>> &dsLayoutOverrides);
    ~GShaderPermutationVLK() override {};

    VkShaderModule getVertexModule() {
        return vertShaderModule;
    }
    VkShaderModule getFragmentModule() {
        return fragShaderModule;
    }

    const shaderMetaData *fragShaderMeta;
    const shaderMetaData *vertShaderMeta;

protected:
    void compileShader(const std::string &vertExtraDefStrings, const std::string &fragExtraDefStrings) override;
    std::vector<const shaderMetaData *> createMetaArray() override;

    VkShaderModule vertShaderModule = VK_NULL_HANDLE;
    VkShaderModule fragShaderModule = VK_NULL_HANDLE;

private:
    ShaderConfig m_shaderConfig;
    std::string m_shaderNameVert;
    std::string m_shaderNameFrag;
};


#endif //AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H
