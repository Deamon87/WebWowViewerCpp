//
// Created by Deamon on 29.12.22.
//

#include "ISimpleMaterialVLK.h"
#include "../shaders/GShaderPermutationVLK.h"
#include "../textures/GTextureVLK.h"

void ISimpleMaterialVLK::createImageDescriptorSet() {
    auto shaderVLK = std::dynamic_pointer_cast<GShaderPermutationVLK>(m_shader);
    auto descLayout = shaderVLK->getImageDescriptorLayout();

    imageDescriptorSet = m_device->createDescriptorSet(descLayout) ;

    int textureBegin = shaderVLK->getTextureBindingStart();

    auto blackTexture = m_device->getBlackTexturePixel();
    auto blackTextureVlk = std::dynamic_pointer_cast<GTextureVLK>(blackTexture);

    {
        auto updateHelper = imageDescriptorSet->beginUpdate();
        for (int i = 0; i < shaderVLK->getTextureCount(); i++) {
            updateHelper.texture(textureBegin + i, blackTextureVlk);
        }
    }
}


void ISimpleMaterialVLK::updateImageDescriptorSet() {
    bool allTexturesAreReady = true;

    auto shaderVLK = std::dynamic_pointer_cast<GShaderPermutationVLK>(m_shader);
    int textureBegin = shaderVLK->getTextureBindingStart();

    for (auto& texture : m_textures) {
        if (texture == nullptr) continue;
        allTexturesAreReady &= texture->getIsLoaded();
    }

    if (allTexturesAreReady) {
        auto updateHelper = imageDescriptorSet->beginUpdate();
        for (size_t i = 0; i < m_textures.size(); i++) {
            auto textureVlk = std::dynamic_pointer_cast<GTextureVLK>(m_textures[i]);
            if (textureVlk == nullptr) continue;

            updateHelper.texture(textureBegin + i, textureVlk);
        }
    }
}

void ISimpleMaterialVLK::createUBODescriptorSet() {
    auto shaderVLK = std::dynamic_pointer_cast<GShaderPermutationVLK>(m_shader);
    auto descLayout = shaderVLK->getUboDescriptorLayout();

    uboDescriptorSet = m_device->createDescriptorSet(descLayout) ;

    auto updateHelper = imageDescriptorSet->beginUpdate();
    for (size_t i = 0; i < m_ubos.size(); i++) {
        updateHelper.ubo(i, m_ubos[i]);
    }
}


ISimpleMaterialVLK::ISimpleMaterialVLK(const HGDeviceVLK &device,
                                       const std::string &vertexShader, const std::string &pixelShader,
                                       const std::vector<std::shared_ptr<IBufferVLK>> &ubos,
                                       const std::vector<HGTextureVLK> &textures) : m_device(device) {
    m_shader =  device->getShader(vertexShader, pixelShader, nullptr);

    auto shaderVLK = std::dynamic_pointer_cast<GShaderPermutationVLK>(m_shader);
    auto &shaderLayout = shaderVLK->getShaderLayout();

    auto uboSetLayout = shaderLayout.setLayouts[GShaderPermutationVLK::UBO_SET_INDEX];
    if (ubos.size() != uboSetLayout.uboBindings.length) {
        std::cerr << "not enough ubos for shaderName = " << shaderVLK->getShaderCombinedName() << std::endl;
    }
    for (unsigned int i = uboSetLayout.uboBindings.start; i <= uboSetLayout.uboBindings.end; i++) {
        auto it = uboSetLayout.uboSizesPerBinding.find(i);
        if (it != std::end(uboSetLayout.uboSizesPerBinding)) {
            auto uboIndex = i - uboSetLayout.uboBindings.start;
            if (ubos[uboIndex] == nullptr) {
                std::cerr << "UBO is not set for "
                          << "shader = " << shaderVLK->getShaderCombinedName()
                          << " set = " << 1
                          << " binding" << i
                          << std::endl;
            }

            if (it->second != ubos[uboIndex]->getSize()) {
                std::cout << "buffers missmatch! for"
                          << " shaderName = " << shaderVLK->getShaderCombinedName()
                          << " set = " << 1
                          << " binding = " << i
                          << " expected size " << (it->second)
                          << ", provided size = " << (ubos[uboIndex]->getSize())
                          << std::endl;
            }
        }
    }

    auto imageSetLayout = shaderLayout.setLayouts[GShaderPermutationVLK::IMAGE_SET_INDEX];
    if (imageSetLayout.imageBindings.length != textures.size()) {
        std::cout << "image count mismatch! for"
                  << " shaderName = " << shaderVLK->getShaderCombinedName()
                  << " set = " << 1
                  << " expected count " << uboSetLayout.imageBindings.length
                  << ", provided count = " << textures.size()
                  << std::endl;
    }

    m_textures = textures;
    m_ubos = ubos;

    createImageDescriptorSet();
    createUBODescriptorSet();

}
