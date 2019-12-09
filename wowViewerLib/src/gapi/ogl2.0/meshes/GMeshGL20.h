//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GMESH20_H
#define WEBWOWVIEWERCPP_GMESH20_H

#include "../GVertexBufferBindingsGL20.h"
#include "../textures/GBlpTextureGL20.h"
#include "../../interface/meshes/IMesh.h"
#include "../GDeviceGL20.h"

class GMeshGL20 : public IMesh {
    friend class GDeviceGL20;
protected:
    explicit GMeshGL20(IDevice &device,
                   const gMeshTemplate &meshTemplate
    );

public:
    ~GMeshGL20() override;
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

    HGUniformBufferChunk m_UniformBuffer[6] = {nullptr, nullptr, nullptr};

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
