//
// Created by Deamon on 7/1/2018.
//

#ifndef AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H

#include <string>
#include <array>
#include <unordered_map>
#include "../GDeviceVulkan.h"
#include "../../interface/IShaderPermutation.h"
#include "../descriptorSets/GDescriptorSet.h"
#include "../../../engine/shader/ShaderDefinitions.h"


struct ShaderSetLayout {
    std::unordered_map<unsigned int, unsigned int> uboSizesPerBinding;
    bindingAmountData uboBindings;
    bindingAmountData imageBindings;
};

struct CombinedShaderLayout {
    std::array<ShaderSetLayout, MAX_SHADER_DESC_SETS> setLayouts;
};

class GShaderPermutationVLK : public IShaderPermutation {
    friend class GDeviceVLK;
public:
    static const constexpr int UBO_SET_INDEX = 0;
    static const constexpr int IMAGE_SET_INDEX = 1;

    explicit GShaderPermutationVLK(std::string &shaderVertName, std::string &shaderFragName, const std::shared_ptr<GDeviceVLK> &device, const ShaderConfig &shaderConf);
    ~GShaderPermutationVLK() override {};

    VkShaderModule getVertexModule() {return vertShaderModule;}
    VkShaderModule getFragmentModule() {return fragShaderModule;}


    const std::shared_ptr<GDescriptorSetLayout> getDescriptorLayout(int bindPoint);

    const shaderMetaData *fragShaderMeta;
    const shaderMetaData *vertShaderMeta;

    std::string getShaderCombinedName() {
        return m_combinedName;
    }

    const CombinedShaderLayout &getShaderLayout() {
        return combinedShaderLayout;
    };

    VkPipelineLayout getPipelineLayout() {
        return pipelineLayout;
    }

protected:
    VkShaderModule createShaderModule(const std::vector<char>& code);

    void compileShader(const std::string &vertExtraDefStrings, const std::string &fragExtraDefStrings) override;


    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;

    VkPipelineLayout pipelineLayout;

    std::array<std::shared_ptr<GDescriptorSetLayout>, MAX_SHADER_DESC_SETS> descriptorSetLayouts;

    std::shared_ptr<GDeviceVLK> m_device;
private:
    //Used only for logging
    std::string m_combinedName;

    //Used for getting SPIRV
    std::string m_shaderNameVert;
    std::string m_shaderNameFrag;

    CombinedShaderLayout combinedShaderLayout;
    ShaderConfig m_shaderConf;


    void createSetDescriptorLayouts();
    std::vector<const shaderMetaData *> createMetaArray();
    void createShaderLayout();
    void createPipelineLayout();
};


#endif //AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H
