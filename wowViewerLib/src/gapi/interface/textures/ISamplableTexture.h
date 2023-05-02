//
// Created by Deamon on 4/30/2023.
//

#ifndef AWEBWOWVIEWERCPP_ISAMPLABLETEXTURE_H
#define AWEBWOWVIEWERCPP_ISAMPLABLETEXTURE_H

#include <memory>
#include "ITexture.h"
#include "ITextureSampler.h"

class ISamplableTexture {
public:
    ISamplableTexture(const std::shared_ptr<ITexture> &texture,
                      const std::shared_ptr<ITextureSampler> &sampler) : m_texture(texture), m_sampler(sampler) {

    };

    auto getTexture() const {
        return m_texture;
    }

    auto getSampler() const {
        return m_sampler;
    }

private:
    std::shared_ptr<ITexture> m_texture;
    std::shared_ptr<ITextureSampler> m_sampler;
};

typedef std::shared_ptr<ISamplableTexture> HGSamplableTexture;

#endif //AWEBWOWVIEWERCPP_ISAMPLABLETEXTURE_H
