//
// Created by Deamon on 29.12.22.
//

#include "ISimpleMaterialVLK.h"
#include "../shaders/GShaderPermutationVLK.h"
#include "../textures/GTextureVLK.h"

//void ISimpleMaterialVLK::createImageDescriptorSet() {
//    auto shaderVLK = std::dynamic_pointer_cast<GShaderPermutationVLK>(m_shader);
//    auto descLayout = shaderVLK->getImageDescriptorLayout();
//
//    descriptors[GShaderPermutationVLK::IMAGE_SET_INDEX] = m_device->createDescriptorSet(descLayout) ;
//
//    int textureBegin = shaderVLK->getTextureBindingStart();
//
//    auto blackTexture = m_device->getBlackTexturePixel();
//    auto blackTextureVlk = std::dynamic_pointer_cast<GTextureVLK>(blackTexture);
//
//    {
//        auto updateHelper = descriptors[GShaderPermutationVLK::IMAGE_SET_INDEX]->beginUpdate();
//        for (int i = 0; i < shaderVLK->getTextureCount(); i++) {
//            updateHelper.texture(textureBegin + i, blackTextureVlk);
//        }
//    }
//}
//
//
//void ISimpleMaterialVLK::updateImageDescriptorSet() {
//    bool allTexturesAreReady = true;
//
//    auto shaderVLK = std::dynamic_pointer_cast<GShaderPermutationVLK>(m_shader);
//    int textureBegin = shaderVLK->getTextureBindingStart();
//
//    for (auto& texture : m_textures) {
//        if (texture == nullptr) continue;
//        allTexturesAreReady &= texture->getIsLoaded();
//    }
//
//    if (allTexturesAreReady) {
//        auto updateHelper = descriptors[GShaderPermutationVLK::IMAGE_SET_INDEX]->beginUpdate();
//        for (size_t i = 0; i < m_textures.size(); i++) {
//            auto textureVlk = std::dynamic_pointer_cast<GTextureVLK>(m_textures[i]);
//            if (textureVlk == nullptr) continue;
//
//            updateHelper.texture(textureBegin + i, textureVlk);
//        }
//    }
//}
//
//void ISimpleMaterialVLK::createAndUpdateUBODescriptorSet() {
//    auto shaderVLK = std::dynamic_pointer_cast<GShaderPermutationVLK>(m_shader);
//    auto descLayout = shaderVLK->getUboDescriptorLayout();
//
//    descriptors[GShaderPermutationVLK::UBO_SET_INDEX] = m_device->createDescriptorSet(descLayout) ;
//
//    auto updateHelper = descriptors[GShaderPermutationVLK::UBO_SET_INDEX]->beginUpdate();
//    for (size_t i = 0; i < m_ubos.size(); i++) {
//        updateHelper.ubo(i, m_ubos[i]);
//    }
//}

ISimpleMaterialVLK::ISimpleMaterialVLK(const HGShaderPermutation &shader,
                                       const std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> &descriptorSets) :
                                       m_shader(shader), descriptors(descriptorSets) {

}
