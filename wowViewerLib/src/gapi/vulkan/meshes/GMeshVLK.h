//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GMESH_H
#define WEBWOWVIEWERCPP_GMESH_H

#include "../GVertexBufferBindingsVLK.h"
#include "../../interface/meshes/IMesh.h"
#include "../GDeviceVulkan.h"
#include "../descriptorSets/GDescriptorSet.h"

class GMeshVLK : public IMesh {
    friend class GDeviceVLK;
protected:
    explicit GMeshVLK(IDevice &device,
                   const gMeshTemplate &meshTemplate
    );

public:
    ~GMeshVLK() override;
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
    void setSortDistance(float distance) override { m_sortDistance = distance;};
    float getSortDistance() override { return m_sortDistance; };


    std::shared_ptr<GPipelineVLK> getPipeLineForRenderPass(VkRenderPass renderPass);

protected:
    MeshType m_meshType;
private:

    HGShaderPermutation m_shader;

    std::array<HGUniformBufferChunk, 6> m_UniformBuffer = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

    int8_t m_depthWrite;
    int8_t m_depthCulling;
    int8_t m_backFaceCulling;
    int8_t m_triCCW = 1;
    EGxBlendEnum m_blendMode;
    bool m_isTransparent;
    int8_t m_isScissorsEnabled = -1;

    std::array<int, 2> m_scissorOffset = {0,0};
    std::array<int, 2> m_scissorSize = {0,0};

    uint8_t m_colorMask = 0;

    DrawElementMode m_element;


//Vulkan specific
    std::vector<std::shared_ptr<GDescriptorSets>> imageDescriptorSets;
    std::vector<bool> descriptorSetsUpdated;

    VkDescriptorPool m_descriptorPool;

    VkRenderPass m_lastRenderPass = VK_NULL_HANDLE;
    std::shared_ptr<GPipelineVLK> m_lastPipelineForRenderPass;


private:
    GDeviceVLK &m_device;

    void createDescriptorSets(GShaderPermutationVLK *shaderVLK);

    void updateDescriptor();
};


#endif //WEBWOWVIEWERCPP_GMESH_H
