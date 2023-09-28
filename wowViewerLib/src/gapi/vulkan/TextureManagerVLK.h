//
// Created by Deamon on 2/20/2023.
//

#ifndef AWEBWOWVIEWERCPP_TEXTUREMANAGERVLK_H
#define AWEBWOWVIEWERCPP_TEXTUREMANAGERVLK_H

#include <unordered_map>
#include <memory>
#include "utils/MutexLockedVector.h"
#include "../../engine/texture/BlpTexture.h"
#include "textures/GTextureVLK.h"
#include "../../include/custom_container_key.h"

class TextureManagerVLK : public std::enable_shared_from_this<TextureManagerVLK> {
public:
    TextureManagerVLK(IDeviceVulkan &device);
    void initialize();

    HGSamplableTexture createBlpTexture(HBlpTexture &texture, bool wrapX, bool wrapY);
    HGSamplableTexture createTexture(bool wrapX, bool wrapY);
    HGSamplableTexture createSampledTexture(bool wrapX, bool wrapY, const HGTexture &textureVlk);

    MutexLockedVector<std::weak_ptr<GTextureVLK>> getReadyToUploadTextures() {

        uint32_t uploadedThisFrame = std::min<uint32_t>(m_texturesReadyToBeUploadedNext.size(), 50);

        decltype(m_texturesReadyToBeUploaded)::const_iterator first = m_texturesReadyToBeUploadedNext.begin();
        decltype(m_texturesReadyToBeUploaded)::const_iterator last = m_texturesReadyToBeUploadedNext.begin() + uploadedThisFrame;

        m_texturesReadyToBeUploaded = decltype(m_texturesReadyToBeUploaded)(first, last);

        m_texturesReadyToBeUploadedNext.erase(m_texturesReadyToBeUploadedNext.begin(), m_texturesReadyToBeUploadedNext.begin() + uploadedThisFrame);

        return MutexLockedVector<std::weak_ptr<GTextureVLK>>(m_texturesReadyToBeUploaded, textUpdateMutex, true);
    }

    void processBLPTextures();

protected:
    struct BlpCacheRecord {
        wtf::KeyContainer<std::weak_ptr<HBlpTexture::element_type>> texture;

        bool operator==(const BlpCacheRecord &other) const {
            return
                (texture == other.texture);
        };
    };
    struct BlpCacheRecordHasher {
        std::size_t operator()(const BlpCacheRecord& k) const {
            using std::hash;
            return hash<decltype(k.texture)>{}(k.texture);
        };
    };
    std::unordered_map<BlpCacheRecord, std::weak_ptr<GTextureVLK>, BlpCacheRecordHasher> loadedTextureCache;


    struct SampledTextureCacheRecord {
        wtf::KeyContainer<std::weak_ptr<HGTexture::element_type>> texture;
        bool wrapX;
        bool wrapY;

        bool operator==(const SampledTextureCacheRecord &other) const {
            return
                (texture == other.texture) &&
                (wrapX == other.wrapX) &&
                (wrapY == other.wrapY);
        };
    };
    struct SampledTextureCacheRecordHasher {
        std::size_t operator()(const SampledTextureCacheRecord& k) const {
            using std::hash;
            return hash<decltype(k.texture)>{}(k.texture) ^
                (hash<bool>{}(k.wrapX) << 8) ^
                (hash<bool>{}(k.wrapY) << 16);
        };
    };
    std::unordered_map<SampledTextureCacheRecord, std::weak_ptr<HGSamplableTexture::element_type>, SampledTextureCacheRecordHasher> sampledTextureCache;

private:
    HGTexture createBlpTexture(HBlpTexture &texture);
private:
    IDeviceVulkan &mdevice;

    std::mutex m_textureAllocation;

    std::function<void(const std::weak_ptr<GTextureVLK>&)> onUpdateCallback = nullptr;
    void createUpdateCallback();

    std::mutex textUpdateMutex;
    std::vector<std::weak_ptr<GTextureVLK>> m_texturesReadyToBeUploaded;
    std::vector<std::weak_ptr<GTextureVLK>> m_texturesReadyToBeUploadedNext;

    std::mutex blpTextureLoadMutex;
    std::list<std::weak_ptr<GBlpTextureVLK>> m_blpTextLoadQueue;

    std::array<std::shared_ptr<ITextureSampler>, 4> m_textureSamplers;

};


#endif //AWEBWOWVIEWERCPP_TEXTUREMANAGERVLK_H
