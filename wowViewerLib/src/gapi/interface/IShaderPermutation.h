//
// Created by Deamon on 8/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_ISHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_ISHADERPERMUTATION_H

class IShaderPermutation {
public:
    virtual ~IShaderPermutation() = 0;
    virtual void compileShader() = 0;
};

#endif //AWEBWOWVIEWERCPP_ISHADERPERMUTATION_H
