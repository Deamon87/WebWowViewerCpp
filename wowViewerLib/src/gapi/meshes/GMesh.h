//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GMESH_H
#define WEBWOWVIEWERCPP_GMESH_H

#include "../GVertexBufferBindings.h"
#include "../GBlpTexture.h"
enum class MeshType {
    eGeneralMesh = 0,
    eAdtMesh = 1,
    eWmoMesh = 2,
    eM2Mesh = 3,
    eParticleMesh = 4,
};

class gMeshTemplate {
public:
    gMeshTemplate(HGVertexBufferBindings bindings, HGShaderPermutation shader) : bindings(bindings), shader(shader) {}
    HGVertexBufferBindings bindings;
    HGShaderPermutation shader;
    MeshType meshType = MeshType::eGeneralMesh;

    bool depthWrite;
    bool depthCulling;
    bool backFaceCulling;
    EGxBlendEnum blendMode;

    int start;
    int end;
    int element;
    int textureCount;
    std::vector<HGTexture> texture = std::vector<HGTexture>(4, nullptr);
    HGUniformBuffer vertexBuffers[3] = {nullptr,nullptr,nullptr};
    HGUniformBuffer fragmentBuffers[3] = {nullptr,nullptr,nullptr};
};


class GMesh {
    friend class GDevice;

protected:
    explicit GMesh(GDevice &device,
                   const gMeshTemplate &meshTemplate
    );

public:
    ~GMesh();
    inline HGUniformBuffer getVertexUniformBuffer(int slot) {
        return m_vertexUniformBuffer[slot];
    }
    inline HGUniformBuffer getFragmentUniformBuffer(int slot) {
        return m_fragmentUniformBuffer[slot];
    }
    inline EGxBlendEnum getGxBlendMode() { return m_blendMode; }
    inline bool getIsTransparent() { return m_isTransparent; }
    inline MeshType getMeshType() {
        return m_meshType;
    }

    void setEnd(int end) {m_end = end; };
protected:
    MeshType m_meshType;
private:
    HGVertexBufferBindings m_bindings;
    HGShaderPermutation m_shader;

    HGUniformBuffer m_vertexUniformBuffer[3] = {nullptr, nullptr, nullptr};
    HGUniformBuffer m_fragmentUniformBuffer[3] = {nullptr, nullptr, nullptr};
    std::vector<HGTexture> m_texture;

    int8_t m_depthWrite;
    int8_t m_depthCulling;
    int8_t m_backFaceCulling;
    EGxBlendEnum m_blendMode;
    bool m_isTransparent;

    int m_start;
    int m_end;
    int m_element;

    int m_textureCount;

private:
    GDevice &m_device;
};


#endif //WEBWOWVIEWERCPP_GMESH_H
