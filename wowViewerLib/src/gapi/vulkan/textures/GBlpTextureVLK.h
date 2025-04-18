//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GBLPTEXTUREVLK_H
#define WEBWOWVIEWERCPP_GBLPTEXTUREVLK_H

#include "GTextureVLK.h"
#include "../IDeviceVulkan.h"

class GBlpTextureVLK : public GTextureVLK {
public:
    explicit GBlpTextureVLK(IDeviceVulkan &device, const HBlpTexture &texture, const std::function<void(const std::weak_ptr<GTextureVLK>&)> &onUpdateCallback);
    ~GBlpTextureVLK() override;
    GBlpTextureVLK(const GBlpTextureVLK&) = delete;
    GBlpTextureVLK(const GBlpTextureVLK&&) = delete;

    void createTexture(TextureFormat textureFormat, const HMipmapsVector &hmipmaps) override;

    TextureStatus postLoad() override;
private:
    void decompressAndUpload(TextureFormat textureFormat, const HMipmapsVector &hmipmaps);
    void freeMipmaps() override {m_texture = nullptr;};
private:
    HBlpTexture m_texture;
};


#endif //WEBWOWVIEWERCPP_GBLPTEXTUREVLK_H
