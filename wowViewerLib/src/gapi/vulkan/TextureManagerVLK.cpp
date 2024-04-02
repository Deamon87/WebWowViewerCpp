//
// Created by Deamon on 2/20/2023.
//

#include "GDeviceVulkan.h"
#include "TextureManagerVLK.h"
#include "textures/GBlpTextureVLK.h"
#include "textures/GTextureSamplerVLK.h"

TextureManagerVLK::TextureManagerVLK(IDeviceVulkan &device) : mdevice(device) {
}

void TextureManagerVLK::initialize() {
    createUpdateCallback();

    m_textureSamplers[0] = std::make_shared<GTextureSamplerVLK>(mdevice, false, false, false);
    m_textureSamplers[1] = std::make_shared<GTextureSamplerVLK>(mdevice, false, true, false);
    m_textureSamplers[2] = std::make_shared<GTextureSamplerVLK>(mdevice, true, false, false);
    m_textureSamplers[3] = std::make_shared<GTextureSamplerVLK>(mdevice, true, true, false);
}

HGSamplableTexture TextureManagerVLK::createBlpTexture(HBlpTexture &texture, bool wrapX, bool wrapY) {
    auto textureVlk = createBlpTexture(texture);

    return createSampledTexture(wrapX, wrapY, textureVlk);
}

HGSamplableTexture TextureManagerVLK::createSampledTexture(bool wrapX, bool wrapY, const HGTexture &textureVlk) {
    SampledTextureCacheRecord sampledTextureCacheRecord = {
        .texture = std::weak_ptr<HGTexture::element_type>(textureVlk),
        .wrapX = wrapX,
        .wrapY = wrapY
    };

    std::lock_guard<std::mutex> lock(m_textureAllocation);

    auto i = sampledTextureCache.find(sampledTextureCacheRecord);
    if (i != sampledTextureCache.end()) {
        if (!i->second.expired()) {
            return i->second.lock();
        } else {
            sampledTextureCache.erase(i);
        }
    }

    auto sampler = m_textureSamplers[((wrapX ? 1 : 0) * 2) + (wrapY ? 1 : 0)];
    auto hgSamplableTexture = std::make_shared<ISamplableTexture>(textureVlk, sampler);

    std::weak_ptr<ISamplableTexture> weakPtr(hgSamplableTexture);

    sampledTextureCache[sampledTextureCacheRecord] = weakPtr;

    return hgSamplableTexture;
}

HGTexture TextureManagerVLK::createBlpTexture(HBlpTexture &texture) {
    BlpCacheRecord blpCacheRecord = {
        .texture = std::weak_ptr<HBlpTexture::element_type>(texture)
    };
    std::lock_guard<std::mutex> lock(m_textureAllocation);

    auto i = loadedTextureCache.find(blpCacheRecord);
    if (i != loadedTextureCache.end()) {
        if (!i->second.expired()) {
            return i->second.lock();
        } else {
            loadedTextureCache.erase(i);
        }
    }

    auto hgTexture = std::make_shared<GBlpTextureVLK>(static_cast<GDeviceVLK&>(mdevice), texture, onUpdateCallback);

    std::weak_ptr<GBlpTextureVLK> weakPtr(hgTexture);
    {
        std::lock_guard<std::mutex> lock(this->blpTextureLoadMutex);
        m_blpTextLoadQueue.push_back(weakPtr);
    }
    loadedTextureCache[blpCacheRecord] = weakPtr;

    return hgTexture;
}

HGSamplableTexture TextureManagerVLK::createTexture(bool wrapX, bool wrapY) {
    std::shared_ptr<GTextureVLK> h_texture = std::make_shared<GTextureVLK>(static_cast<GDeviceVLK&>(mdevice), onUpdateCallback);

    auto sampler = m_textureSamplers[((wrapX ? 1 : 0) * 2) + (wrapY ? 1 : 0)];
    return std::make_shared<HGSamplableTexture::element_type>(h_texture,sampler);
}

void TextureManagerVLK::createUpdateCallback() {
    auto weak_this = this->weak_from_this();
    onUpdateCallback = [weak_this](const std::weak_ptr<GTextureVLK>& texture) -> void {
        if (auto shared_this = weak_this.lock()) {
            std::lock_guard lock(shared_this->textUpdateMutex);

            shared_this->m_texturesReadyToBeUploadedNext.push_back(std::weak_ptr(texture));
        }
    };
}

void TextureManagerVLK::processBLPTextures() {
    std::lock_guard<std::mutex> lock(this->blpTextureLoadMutex);

    auto i = m_blpTextLoadQueue.begin();
    while (i != m_blpTextLoadQueue.end())
    {
        if (auto texture = i->lock()) {
            if (texture->postLoad() == TextureStatus::TSHasUpdates) {
                m_blpTextLoadQueue.erase(i++);
            } else {
                i++;
            }
        } else {
            m_blpTextLoadQueue.erase(i++);
        }
    }
}
