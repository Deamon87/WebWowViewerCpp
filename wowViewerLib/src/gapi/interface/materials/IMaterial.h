//
// Created by Deamon on 28.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IMATERIAL_H
#define AWEBWOWVIEWERCPP_IMATERIAL_H

#include <memory>
#include "../IDevice.h"

struct PipelineTemplate {
    DrawElementMode element;

    int8_t triCCW = 1; //counter-clockwise
    bool depthWrite = true;
    bool depthCulling = true;
    bool backFaceCulling = true;
    EGxBlendEnum blendMode;

    uint8_t colorMask = 0xFF;
};


class IMaterial {
public:
    virtual ~IMaterial() = default;
};

typedef std::shared_ptr<IMaterial> HMaterial;

#endif //AWEBWOWVIEWERCPP_IMATERIAL_H
