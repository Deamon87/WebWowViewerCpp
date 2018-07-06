//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GMESH_H
#define WEBWOWVIEWERCPP_GMESH_H


#include "GVertexBufferBindings.h"
#include "GTexture.h"

class GMesh {
    friend class GDevice;

    explicit GMesh(GDevice &device,
                   GVertexBufferBindings &bindings,
                   GShaderPermutation &shader,
                   bool m_depthWrite,
                   bool m_depthCulling,
                   int m_blendMode,

                   int m_start,
                   int m_end,
                   int m_element,
                   GTexture *m_texture1,
                   GTexture *m_texture2,
                   GTexture *m_texture3,
                   GTexture *m_texture4
    );
    ~GMesh();
private:
    GVertexBufferBindings &m_bindings;
    GShaderPermutation &m_shader;

    GUniformBuffer * m_uniformBuffer[3] = {nullptr, nullptr, nullptr};

    bool m_depthWrite;
    bool m_depthCulling;
    int m_blendMode;

    int m_start;
    int m_end;
    int m_element;

    GTexture *m_texture1;
    GTexture *m_texture2;
    GTexture *m_texture3;
    GTexture *m_texture4;

private:
    GDevice &m_device;
};


#endif //WEBWOWVIEWERCPP_GMESH_H
