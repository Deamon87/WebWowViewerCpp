//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GMESH_H
#define WEBWOWVIEWERCPP_GMESH_H

#include "../GVertexBufferBindings.h"
#include "../textures/GBlpTexture.h"
#include "../../interface/meshes/IMesh.h"

class GMeshGL33 : public IMesh {
    friend class GDeviceGLL33;
protected:
    explicit GMeshGL33(IDevice &device,
                   const gMeshTemplate &meshTemplate
    );

public:
    ~GMeshGL33() override;
    HGUniformBuffer getVertexUniformBuffer(int slot) override;
    HGUniformBuffer getFragmentUniformBuffer(int slot) override;
    EGxBlendEnum getGxBlendMode() override;
    bool getIsTransparent() override;
    MeshType getMeshType()  override;
    void setRenderOrder(int renderOrder) override;

    void setEnd(int end) override;
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
    int8_t m_triCCW = 1;
    EGxBlendEnum m_blendMode;
    bool m_isTransparent;


    uint8_t m_colorMask = 0;
    int m_renderOrder = 0;
    int m_start;
    int m_end;
    int m_element;

    int m_textureCount;

private:
    IDevice &m_device;
};


#endif //WEBWOWVIEWERCPP_GMESH_H
