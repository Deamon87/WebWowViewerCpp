//
// Created by Deamon on 7/1/2018.
//

#ifndef AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H
#define AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H

class GDeviceGL33;

#include <string>
#include <unordered_map>
#include "GDeviceGL33.h"
#include "../../engine/opengl/header.h"
#include "../../engine/algorithms/hashString.h"
#include "../interface/IShaderPermutation.h"

class GShaderPermutationGL33 : public IShaderPermutation {
    friend class GDeviceGL33;

public:
    ~GShaderPermutationGL33() override {};

protected:
    explicit GShaderPermutationGL33(std::string &shaderName, IDevice *device);
    explicit GShaderPermutationGL33(std::string &shaderNameVert, std::string &shaderNameFrag, IDevice *device);

    void compileShader(const std::string &vertExtraDefStrings, const std::string &fragExtraDefStrings) override;
    bool hasUnf(const HashedString name) {
        return m_uniformMap.find(name.Hash()) != m_uniformMap.end();
    }
    GLuint getUnf(const HashedString name) const {
        return m_uniformMap.at(name.Hash());
    }

    GLuint getUBOUnf(const HashedString name) const {
        return m_uniformMap.at(name.Hash());
    }

    void bindProgram();
    void unbindProgram();

    unsigned int m_programBuffer = 0;
private:
    IDevice *m_device;
private:
    std::unordered_map<size_t, unsigned int> m_uniformMap;
    std::array<int, 5> m_uboBlockIndex;
    std::string m_shaderNameVert;
    std::string m_shaderNameFrag;


    void setUnf(const std::string &name, GLuint index);
};


#endif //AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H
