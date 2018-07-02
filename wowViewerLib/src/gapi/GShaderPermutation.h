//
// Created by Deamon on 7/1/2018.
//

#ifndef AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H

#include <string>
#include <unordered_map>
#include "GDevice.h"
#include "../engine/opengl/header.h"

class GShaderPermutation {
    GShaderPermutation(std::string &shaderName, GDevice & device);

private:
    GDevice &m_device;

private:
    std::unordered_map<size_t, GLuint> m_uniformMap;
    GLuint m_program;
};


#endif //AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H
