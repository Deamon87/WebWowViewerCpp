//
// Created by Deamon on 4/29/2023.
//

#ifndef AWEBWOWVIEWERCPP_ITEXTURESAMPLER_H
#define AWEBWOWVIEWERCPP_ITEXTURESAMPLER_H

#include <memory>
class ITextureSampler {
public:
    virtual ~ITextureSampler() {};
};

typedef std::shared_ptr<ITextureSampler> HGSampler;

#endif //AWEBWOWVIEWERCPP_ITEXTURESAMPLER_H
