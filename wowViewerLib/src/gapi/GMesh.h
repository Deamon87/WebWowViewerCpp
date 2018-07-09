//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GMESH_H
#define WEBWOWVIEWERCPP_GMESH_H


#include "GVertexBufferBindings.h"
#include "GTexture.h"

class gMeshTemplate {
public:
    gMeshTemplate(HGVertexBufferBindings &bindings, HGShaderPermutation &shader) : bindings(bindings), shader(shader) {}
    HGVertexBufferBindings &bindings;
    HGShaderPermutation &shader;
    bool depthWrite;
    bool depthCulling;
    bool backFaceCulling;
    int blendMode;

    int start;
    int end;
    int element;
    int textureCount;
    HGTexture texture[4];
    HGUniformBuffer buffers[3];
};

class GMesh {
    friend class GDevice;

    explicit GMesh(GDevice &device,
                   gMeshTemplate meshTemplate
    );
public:
    ~GMesh();
    HGUniformBuffer getUniformBuffer(int slot) {
        return m_uniformBuffer[slot];
    }
private:
    HGVertexBufferBindings &m_bindings;
    HGShaderPermutation &m_shader;

    HGUniformBuffer m_uniformBuffer[3] = {nullptr, nullptr, nullptr};
    HGTexture m_texture[4];

    bool m_depthWrite;
    bool m_depthCulling;
    bool m_backFaceCulling;
    int m_blendMode;

    int m_start;
    int m_end;
    int m_element;

    int m_textureCount;

private:
    GDevice &m_device;
};


#endif //WEBWOWVIEWERCPP_GMESH_H
