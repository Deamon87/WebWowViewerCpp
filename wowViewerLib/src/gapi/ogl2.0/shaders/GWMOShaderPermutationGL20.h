//
// Created by Deamon on 7/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_GWMOSHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_GWMOSHADERPERMUTATION_H

#include "../GShaderPermutationGL20.h"

class GWMOShaderPermutationGL20 : public GShaderPermutationGL20 {
    friend class GDeviceGL20;
protected:
    explicit GWMOShaderPermutationGL20(std::string &shaderName, IDevice *device, WMOShaderCacheRecord &permutation);

    virtual void compileShader(const std::string &vertExtraDefStrings, const std::string &fragExtraDefStrings) override;
    WMOShaderCacheRecord &permutation;
};


#endif //AWEBWOWVIEWERCPP_GWMOSHADERPERMUTATION_H
