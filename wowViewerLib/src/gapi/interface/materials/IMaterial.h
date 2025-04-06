//
// Created by Deamon on 28.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IMATERIAL_H
#define AWEBWOWVIEWERCPP_IMATERIAL_H

#include <memory>
#include "../IDevice.h"

struct PipelineTemplate {
    DrawElementMode element;

    bool backFaceCulling = true;
    bool triCCW = true; //counter-clockwise
    EGxBlendEnum blendMode;
    bool depthCulling = true;
    bool depthWrite = true;

    uint8_t colorMask = 0xFF;

    bool operator==(const PipelineTemplate &other) const {
        return
            (element == other.element) &&
            (backFaceCulling == other.backFaceCulling) &&
            (triCCW == other.triCCW) &&
            (blendMode == other.blendMode) &&
            (depthCulling == other.depthCulling) &&
            (depthWrite == other.depthWrite) &&
            (colorMask == other.colorMask);
    };
};


class IMaterial {
public:
    virtual ~IMaterial() = default;
};

typedef std::shared_ptr<IMaterial> HMaterial;


#endif //AWEBWOWVIEWERCPP_IMATERIAL_H
