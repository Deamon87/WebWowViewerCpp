//
// Created by Deamon on 7/1/2018.
//

#ifndef AWEBWOWVIEWERCPP_GSHADERPERMUTATION20_H
#define AWEBWOWVIEWERCPP_GSHADERPERMUTATION20_H

class GDeviceGL20;

#include <string>
#include <unordered_map>
#include "GDeviceGL20.h"
#include "../../engine/opengl/header.h"
#include "../../engine/algorithms/hashString.h"
#include "../interface/IShaderPermutation.h"

class GShaderPermutationGL20 : public IShaderPermutation {
    friend class GDeviceGL20;

public:
    ~GShaderPermutationGL20() override {};

protected:
    explicit GShaderPermutationGL20(std::string &shaderName, IDevice *device);

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
    GUniformBufferGL20 * m_UniformBuffer[6] = {nullptr};

    void bindUniformBuffer(IUniformBuffer *buffer, int slot, int offset, int length);

    std::unordered_map<size_t, unsigned int> m_uniformMap;
    int m_uboVertexBlockIndex[3];
    int m_uboFragmentBlockIndex[3];
    std::string m_shaderName;


    void setUnf(const std::string &name, GLuint index);
};


#endif //AWEBWOWVIEWERCPP_GSHADERPERMUTATION_H
