//
// Created by Deamon on 1/31/2021.
//

#ifndef AWEBWOWVIEWERCPP_GWATERFALLSHADERGL33_H
#define AWEBWOWVIEWERCPP_GWATERFALLSHADERGL33_H

#include "../GShaderPermutationGL33.h"

class GWaterfallShaderGL33 : public GShaderPermutationGL33 {
    friend class GDeviceGL33;
protected:
    explicit GWaterfallShaderGL33(std::string &shaderName, IDevice *device);

    void compileShader(const std::string &vertExtraDefStrings, const std::string &fragExtraDefStrings) override;
};


#endif //AWEBWOWVIEWERCPP_GWATERFALLSHADERGL33_H
