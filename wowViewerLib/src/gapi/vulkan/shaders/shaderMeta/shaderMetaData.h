//
// Created by deamon on 18.10.19.
//

#ifndef AWEBWOWVIEWERCPP_SHADERMETADATA_H
#define AWEBWOWVIEWERCPP_SHADERMETADATA_H


#include <vector>
#include <unordered_map>

struct uboBindingData {
    int set;
    int binding;
    int size;
};

struct shaderMetaData {
    std::vector<uboBindingData> uboBindings;
};

//Per file
extern const std::unordered_map<std::string, shaderMetaData> shaderMetaInfo;


#endif //AWEBWOWVIEWERCPP_SHADERMETADATA_H
