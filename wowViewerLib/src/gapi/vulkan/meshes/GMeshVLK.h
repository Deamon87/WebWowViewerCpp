//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GMESH_H
#define WEBWOWVIEWERCPP_GMESH_H

#include "../GVertexBufferBindingsVLK.h"
#include "../../interface/meshes/IMesh.h"
#include "../GDeviceVulkan.h"

class GMeshVLK : public IMesh {
    friend class GDeviceVLK;
protected:
    explicit GMeshVLK(IDevice &device,
                   const gMeshTemplate &meshTemplate
    );

public:
    ~GMeshVLK() override;
    HGUniformBuffer getVertexUniformBuffer(int slot) override;
    HGUniformBuffer getFragmentUniformBuffer(int slot) override;
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

    HGUniformBuffer m_vertexUniformBuffer[3] = {nullptr, nullptr, nullptr};
    HGUniformBuffer m_fragmentUniformBuffer[3] = {nullptr, nullptr, nullptr};

    int8_t m_depthWrite;
    int8_t m_depthCulling;
    int8_t m_backFaceCulling;
    int8_t m_triCCW = 1;
    EGxBlendEnum m_blendMode;
    bool m_isTransparent;


    uint8_t m_colorMask = 0;

    DrawElementMode m_element;


//Vulkan specific
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<bool> descriptorSetsUpdated;

    VkDescriptorPool m_descriptorPool;

    std::shared_ptr<GPipelineVLK> hgPipelineVLK;


private:
    GDeviceVLK &m_device;

    void createDescriptorSets(GShaderPermutationVLK *shaderVLK);

    void updateDescriptor();
};


#endif //WEBWOWVIEWERCPP_GMESH_H
