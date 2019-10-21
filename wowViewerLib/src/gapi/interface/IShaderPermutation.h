//
// Created by Deamon on 8/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_ISHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_ISHADERPERMUTATION_H

#include <string>

class IShaderPermutation {
public:
    virtual ~IShaderPermutation() {};
    virtual void compileShader(const std::string &vertExtraDefStrings, const std::string &fragExtraDefStrings) = 0;
};

#endif //AWEBWOWVIEWERCPP_ISHADERPERMUTATION_H
