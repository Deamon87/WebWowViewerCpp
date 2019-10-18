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
#include "../descriptorSets/GDescriptorSet.h"

class GShaderPermutationVLK : public IShaderPermutation {
    friend class GDeviceVLK;

public:
    ~GShaderPermutationVLK() override {};

    VkShaderModule getVertexModule() {return vertShaderModule;}
    VkShaderModule getFragmentModule() {return fragShaderModule;}
    VkDescriptorSetLayout getImageDescriptorLayout() {return imageDescriptorSetLayout;}
    std::string getShaderName() {return m_shaderName; }
    virtual int getTextureBindingStart() = 0;
    virtual int getTextureCount() = 0;

protected:
    explicit GShaderPermutationVLK(std::string &shaderName, IDevice *device);

    VkShaderModule createShaderModule(const std::vector<char>& code);

    void compileShader(const std::string &vertExtraDefStrings, const std::string &fragExtraDefStrings) override;


    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;

    VkDescriptorSetLayout uboDescriptorSetLayout;
    VkDescriptorSetLayout imageDescriptorSetLayout;

    std::vector<std::shared_ptr<GDescriptorSets>> uboDescriptorSets;

    GDeviceVLK *m_device;


private:
    std::string m_shaderName;

    void createUBODescriptorLayout();

    void createImageDescriptorLayout();

    void createUboDescriptorSets();

    void updateDescriptorSet(int index);
};


#endif //AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H
