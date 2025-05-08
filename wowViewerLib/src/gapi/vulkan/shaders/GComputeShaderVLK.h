//
// Created by Deamon on 3/10/2026.
//

#ifndef AWEBWOWVIEWERCPP_GCOMPUTESHADERVLK_H
#define AWEBWOWVIEWERCPP_GCOMPUTESHADERVLK_H

#include <string>
#include "GShaderPermutationVLKBase.h"


class GComputeShaderVLK : public GShaderPermutationVLKBase {
public:
    explicit GComputeShaderVLK(const std::string &shaderName,
                               const std::shared_ptr<GDeviceVLK> &device,
                               const ComputeShaderConfig &shaderConf,
                               const std::unordered_map<int, const std::shared_ptr<GDescriptorSetLayout>> &dsLayoutOverrides = {});
    ~GComputeShaderVLK() override = default;

    void compile();

    VkShaderModule getComputeModule() { return m_computeModule; }
    const shaderMetaData *getComputeMeta() const { return m_computeMeta; }

protected:
    std::vector<const shaderMetaData *> createMetaArray() override;

private:
    ComputeShaderConfig m_computeConfig;
    std::string m_shaderName;
    VkShaderModule m_computeModule = VK_NULL_HANDLE;
    const shaderMetaData *m_computeMeta = nullptr;
};


#endif //AWEBWOWVIEWERCPP_GCOMPUTESHADERVLK_H
