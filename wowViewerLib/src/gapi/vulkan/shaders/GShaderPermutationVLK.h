//
// Created by Deamon on 7/1/2018.
//

#ifndef AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H

#include <string>
#include <unordered_map>
#include "../GDeviceVulkan.h"
#include "../../interface/IShaderPermutation.h"
#include "../descriptorSets/GDescriptorSet.h"
#include "../../../engine/shader/ShaderDefinitions.h"

class GShaderPermutationVLK : public IShaderPermutation {
    friend class GDeviceVLK;

public:
    ~GShaderPermutationVLK() override {};

    VkShaderModule getVertexModule() {return vertShaderModule;}
    VkShaderModule getFragmentModule() {return fragShaderModule;}
    VkDescriptorSetLayout getImageDescriptorLayout() {return imageDescriptorSetLayout;}
    VkDescriptorSetLayout getUboDescriptorLayout() {return uboDescriptorSetLayout;}

    virtual int getTextureBindingStart() = 0;
    virtual int getTextureCount() = 0;

    const shaderMetaData *fragShaderMeta;
    const shaderMetaData *vertShaderMeta;

    std::string getShaderName() {
        return m_shaderName;
    }

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

    void createUBODescriptorLayout();

    void createImageDescriptorLayout();

    void createUboDescriptorSets();

    void updateDescriptorSet(int index);

    std::vector<bool> hasBondUBO = std::vector<bool>(7, false);
};


#endif //AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H
