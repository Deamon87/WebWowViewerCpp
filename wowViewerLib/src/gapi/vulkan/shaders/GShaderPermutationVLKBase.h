//
// Created by Deamon on 3/10/2026.
//

#ifndef AWEBWOWVIEWERCPP_GSHADERPERMUTATIONVLKBASE_H
#define AWEBWOWVIEWERCPP_GSHADERPERMUTATIONVLKBASE_H

#include <string>
#include <array>
#include <unordered_map>
#include "../GDeviceVulkan.h"
#include "../descriptorSets/GDescriptorSet.h"
#include <ShaderDefinitions.h>
#include "ShaderConfig.h"


struct ShaderSetLayout {
    std::unordered_map<unsigned int, unsigned int> uboSizesPerBinding;
    std::unordered_map<unsigned int, unsigned int> ssboSizesPerBinding;
    bindingAmountData uboBindings;
    bindingAmountData ssboBindings;
    bindingAmountData imageBindings;
};

struct CombinedShaderLayout {
    std::array<ShaderSetLayout, MAX_SHADER_DESC_SETS> setLayouts;
};

class GShaderPermutationVLKBase {
public:
    explicit GShaderPermutationVLKBase(const std::string &combinedName,
                                       const std::shared_ptr<GDeviceVLK> &device,
                                       const DescTypeOverride &typeOverrides,
                                       const std::unordered_map<int, const std::shared_ptr<GDescriptorSetLayout>> &dsLayoutOverrides);
    virtual ~GShaderPermutationVLKBase() = default;

    const std::shared_ptr<GDescriptorSetLayout> getDescriptorLayout(int bindPoint);

    std::string getShaderCombinedName() {
        return m_combinedName;
    }

    const CombinedShaderLayout &getShaderLayout() {
        return m_combinedShaderLayout;
    };

    std::shared_ptr<GPipelineLayoutVLK> getPipelineLayout() {
        return m_pipelineLayout;
    }

    std::shared_ptr<GPipelineLayoutVLK> createPipelineLayoutOverrided(const std::unordered_map<int, const std::shared_ptr<GDescriptorSet>> &dses);

protected:
    static std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);

    // Subclasses must implement this to provide the list of metadata from their shader stages
    virtual std::vector<const shaderMetaData *> createMetaArray() = 0;

    // Shared initialization steps: merges metadata, creates descriptor set layouts and pipeline layout
    void buildLayoutsFromMeta();

    std::shared_ptr<GPipelineLayoutVLK> m_pipelineLayout;
    std::array<std::shared_ptr<GDescriptorSetLayout>, MAX_SHADER_DESC_SETS> m_descriptorSetLayouts = {};
    std::shared_ptr<GDeviceVLK> m_device;
    std::string m_combinedName;
    DescTypeOverride m_typeOverrides;

private:
    void createShaderLayout();
    void createSetDescriptorLayouts();
    void createPipelineLayout();

    CombinedShaderLayout m_combinedShaderLayout;
    const std::unordered_map<int, const std::shared_ptr<GDescriptorSetLayout>> m_dsLayoutOverrides;
};


#endif //AWEBWOWVIEWERCPP_GSHADERPERMUTATIONVLKBASE_H
