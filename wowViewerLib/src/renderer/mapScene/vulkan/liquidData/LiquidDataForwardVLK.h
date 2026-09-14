
#ifndef WOWSTUDIO_LIQUIDDATAFORWARDVLK_H
#define WOWSTUDIO_LIQUIDDATAFORWARDVLK_H

#include <memory>
#include <cmath>
#include "../../materials/IMaterialStructs.h"
#include "../../../../gapi/vulkan/descriptorSets/GDescriptorSet.h"
#include "../../../../gapi/vulkan/buffers/CBufferChunkVLK.h"

template<typename T>
class LiquidDataForwardVLK : public LiquidData<T> {
public:
    explicit LiquidDataForwardVLK(const std::shared_ptr<GDescriptorSet> &dsTemplate, const std::shared_ptr<IBufferChunk<T>> &liquidData);
    ~LiquidDataForwardVLK() override = default;

    void updateAnimatedTextures(const std::array<HGSamplableTexture, MAX_LIQUID_TYPE_TEXTURES + 1> &textures, float currentTime) override;

    std::shared_ptr<GDescriptorSet> getDescriptorSet() const {
        return m_descriptorSet;
    };
private:
    std::shared_ptr<GDescriptorSet> m_descriptorSet;
};

template<typename T>
inline LiquidDataForwardVLK<T>::LiquidDataForwardVLK(const std::shared_ptr<GDescriptorSet> &dsTemplate, const std::shared_ptr<IBufferChunk<T>> &liquidData) {
    m_descriptorSet = dsTemplate->clone();
    this->m_liquidData = liquidData;
    m_descriptorSet->beginUpdate().ubo(0, BufferChunkHelperVLK::cast(liquidData)).delayUpdate();
    LiquidDataForwardVLK::updateAnimatedTextures({nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}, 0.0f);
}

template<typename T>
inline void LiquidDataForwardVLK<T>::updateAnimatedTextures(const std::array<HGSamplableTexture, MAX_LIQUID_TYPE_TEXTURES + 1> &textures, float currentTime) {
    // Let the (GAPI-agnostic) LiquidData specialization resolve material-specific
    // animation (e.g. fel normal frame pair) and the slot-7 extra texture
    auto resolved = textures;
    this->resolveAnimatedTextures(resolved, currentTime);

    // Optional material-specific extra textures (azerithe: env map at 8, foam at 9)
    auto update = m_descriptorSet->beginUpdate();
    update.texture(1, resolved[0]);
    update.texture(2, resolved[1]);
    update.texture(3, resolved[2]);
    update.texture(4, resolved[3]);
    update.texture(5, resolved[4]);
    update.texture(6, resolved[5]);
    update.texture(7, resolved[6]);
    if (this->extraTexture2 != nullptr) {
        update.texture(8, this->extraTexture2);
    }
    if (this->extraTexture3 != nullptr) {
        update.texture(9, this->extraTexture3);
    }
    update.delayUpdate();
}

#endif //WOWSTUDIO_LIQUIDDATAFORWARDVLK_H
