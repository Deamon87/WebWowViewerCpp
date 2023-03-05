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
    TextureManagerVLK(IDevice &device);
    void initialize();


    HGTexture createBlpTexture(HBlpTexture &texture);
    HGTexture createTexture();

    MutexLockedVector<std::weak_ptr<GTextureVLK>> getReadyToUploadTextures() {
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

private:
    IDevice &mdevice;

    std::function<void(const std::weak_ptr<GTextureVLK>&)> onUpdateCallback = nullptr;
    void createUpdateCallback();

    std::mutex textUpdateMutex;
    std::vector<std::weak_ptr<GTextureVLK>> m_texturesReadyToBeUploaded;

    std::mutex blpTextureLoadMutex;
    std::list<std::weak_ptr<GBlpTextureVLK>> m_blpTextLoadQueue;
};


#endif //AWEBWOWVIEWERCPP_TEXTUREMANAGERVLK_H
