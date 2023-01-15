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
    ~GShaderPermutationVLK() override {};

    VkShaderModule getVertexModule() {return vertShaderModule;}
    VkShaderModule getFragmentModule() {return fragShaderModule;}
    VkDescriptorSetLayout getImageDescriptorLayout() {return imageDescriptorSetLayout;}
    VkDescriptorSetLayout getUboDescriptorLayout() {return uboDescriptorSetLayout;}

    virtual int getTextureBindingStart();
    virtual int getTextureCount();

    const shaderMetaData *fragShaderMeta;
    const shaderMetaData *vertShaderMeta;

    std::string getShaderName() {
        return m_shaderName;
    }

    const CombinedShaderLayout &getShaderLayout() {
        return shaderLayout;
    };

protected:
    explicit GShaderPermutationVLK(std::string &shaderName, IDevice *device);
    explicit GShaderPermutationVLK(std::string &shaderName, std::string &shaderVertName, std::string &shaderFragName, IDevice *device);

    VkShaderModule createShaderModule(const std::vector<char>& code);

    void compileShader(const std::string &vertExtraDefStrings, const std::string &fragExtraDefStrings) override;


    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;


    VkDescriptorSetLayout uboDescriptorSetLayout;
    VkDescriptorSetLayout imageDescriptorSetLayout;

    std::array<std::shared_ptr<GDescriptorSets>, 4> uboDescriptorSets = {nullptr};

    GDeviceVLK *m_device;


private:
    //Used only for logging
    std::string m_shaderName;

    //Used for getting SPIRV
    std::string m_shaderNameVert;
    std::string m_shaderNameFrag;

    CombinedShaderLayout shaderLayout;

    void createUBODescriptorLayout();

    void createImageDescriptorLayout();

    void createUboDescriptorSets();

    void updateDescriptorSet(int index);

    std::vector<bool> hasBondUBO = std::vector<bool>(7, false);

    void createShaderLayout();
};


#endif //AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H
