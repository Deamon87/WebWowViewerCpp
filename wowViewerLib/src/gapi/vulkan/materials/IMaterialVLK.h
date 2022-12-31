//
// Created by Deamon on 29.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IMATERIALVLK_H
#define AWEBWOWVIEWERCPP_IMATERIALVLK_H


#include <vector>
#include <memory>
#include "../../interface/materials/IMaterial.h"
#include "../descriptorSets/GDescriptorSet.h"

class IMaterialVLK : public IMaterial {
public:
    IMaterialVLK(HGDeviceVLK device);
    ~IMaterialVLK() override = default;;

    HGShaderPermutation getShader() {
        return m_shader;
    }

    void createImageDescriptorSet();
    void updateImageDescriptorSet();

private:
    HGDeviceVLK m_device;

    std::vector<HGTexture> m_textures = {};

    std::vector<std::shared_ptr<GDescriptorSets>> imageDescriptorSets;
    std::vector<std::shared_ptr<GDescriptorSets>> uboDescriptorSets;
    std::vector<std::shared_ptr<GDescriptorSets>> ssboDescriptorSets;

    HGShaderPermutation m_shader;
};

typedef std::shared_ptr<IMaterialVLK> HMaterialVLK;

#endif //AWEBWOWVIEWERCPP_IMATERIALVLK_H
