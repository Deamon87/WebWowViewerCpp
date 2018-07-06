//
// Created by deamon on 05.06.18.
//

#include "GMesh.h"

GMesh::GMesh(GDevice &device,
            GVertexBufferBindings &bindings,
            GShaderPermutation &shader,
            bool depthWrite,
            bool depthCulling,
            int blendMode,

            int start,
            int end,
            int element,
            GTexture *texture1,
            GTexture *texture2,
            GTexture *texture3,
            GTexture *texture4
) : m_bindings(bindings), m_device(device), m_shader(shader) {

    m_depthWrite = depthWrite;
    m_depthCulling = depthCulling;
    m_blendMode = blendMode;

    m_start = start;
    m_end = end;
    m_element = element;
    m_texture1 = texture1;
    m_texture2 = texture2;
    m_texture3 = texture3;
    m_texture4 = texture4;


}
GMesh::~GMesh() {

}
