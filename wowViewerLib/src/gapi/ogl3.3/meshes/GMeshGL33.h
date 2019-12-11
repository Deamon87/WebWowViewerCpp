//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GMESH_H
#define WEBWOWVIEWERCPP_GMESH_H

#include "../GVertexBufferBindingsGL33.h"
#include "../textures/GBlpTextureGL33.h"
#include "../../interface/meshes/IMesh.h"
#include "../GDeviceGL33.h"

class GMeshGL33 : public IMesh {
    friend class GDeviceGL33;
protected:
    explicit GMeshGL33(IDevice &device,
                   const gMeshTemplate &meshTemplate
    );

public:
    ~GMeshGL33() override;
    HGUniformBufferChunk getUniformBuffer(int slot) override;

    EGxBlendEnum getGxBlendMode() override;
    bool getIsTransparent() override;
    MeshType getMeshType()  override;
    void setRenderOrder(int renderOrder) override;

    void setStart(int start) override;
    void setEnd(int end) override;
public:
    void setM2Object(void * m2Object) override { throw "Not Implemented";};
    void setLayer(int layer) override { throw "Not Implemented";};
    void setPriorityPlane(int priorityPlane) override { throw "Not Implemented";};
    void setQuery(const HGOcclusionQuery &query) override { throw "Not Implemented";};
    void setSortDistance(float distance) override { throw "Not Implemented";};
    float getSortDistance() override { throw "Not Implemented";};

protected:
    MeshType m_meshType;
private:

    HGShaderPermutation m_shader;

    HGUniformBufferChunk m_UniformBuffer[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

    int8_t m_depthWrite;
    int8_t m_depthCulling;
    int8_t m_backFaceCulling;
    int8_t m_triCCW = 1;
    EGxBlendEnum m_blendMode;
    bool m_isTransparent;

    uint8_t m_colorMask = 0;
    int m_element;

private:
    IDevice &m_device;
};


#endif //WEBWOWVIEWERCPP_GMESH_H
