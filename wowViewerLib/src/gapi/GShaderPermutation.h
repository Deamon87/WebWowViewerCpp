//
// Created by Deamon on 7/1/2018.
//

#ifndef AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H

#include <string>
#include <unordered_map>
#include "GDevice.h"
#include "../engine/opengl/header.h"
#include "../engine/algorithms/hashString.h"

class GShaderPermutation {
    friend class GDevice;



protected:
    explicit GShaderPermutation(std::string &shaderName, GDevice & device);
    virtual void compileShader();
    GLuint getUnf(const HashedString name) const {
        return m_uniformMap.at(name.Hash());
    }

    unsigned int m_programBuffer = 0;
private:
    GDevice &m_device;
private:
    std::unordered_map<size_t, unsigned int> m_uniformMap;
    std::unordered_map<size_t, unsigned int> m_uboBindPoints;
    std::string m_shaderName;


    void setUnf(const std::string &name, GLuint index);
};


#endif //AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H
