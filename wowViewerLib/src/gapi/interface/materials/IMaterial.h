//
// Created by Deamon on 28.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IMATERIAL_H
#define AWEBWOWVIEWERCPP_IMATERIAL_H

#include <memory>
#include "../IDevice.h"

class IMaterial {
public:
    HGShaderPermutation m_shader = nullptr;

    virtual ~IMaterial() = default;
};

typedef std::shared_ptr<IMaterial> HMaterial;

#endif //AWEBWOWVIEWERCPP_IMATERIAL_H
