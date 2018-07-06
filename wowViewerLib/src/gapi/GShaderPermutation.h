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
    friend class GDevice;
    GShaderPermutation(std::string &shaderName, GDevice & device);

    void compileShader();

private:
    GDevice &m_device;

private:
    std::unordered_map<size_t, unsigned int> m_uniformMap;
    std::unordered_map<size_t, unsigned int> m_uboBindPoints;
    std::string m_shaderName;
    unsigned int m_programBuffer = 0;

    void setUnf(const std::string &name, GLuint index);
};


#endif //AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H
