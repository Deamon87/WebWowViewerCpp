//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GMESH_H
#define WEBWOWVIEWERCPP_GMESH_H

#include "../GVertexBufferBindingsVLK.h"
#include "../../interface/meshes/IMesh.h"
#include "../GDeviceVulkan.h"
#include "../descriptorSets/GDescriptorSet.h"
#include "../materials/ISimpleMaterialVLK.h"

class GMeshVLK : public IMesh {
    friend class GDeviceVLK;
public:
    explicit GMeshVLK(IDevice &device,
                   const gMeshTemplate &meshTemplate,
                   const HMaterialVLK &material
    );

public:
    ~GMeshVLK() override;

    bool getIsTransparent() override;
    MeshType getMeshType()  override;

public:
    auto material() const -> const HMaterialVLK& { return m_material; }
    auto scissorOffset() const -> const  std::array<int, 2>& { return m_scissorOffset; }
    auto scissorSize() const -> const std::array<uint32_t, 2>& { return m_scissorSize; }
    auto scissorEnabled() const -> const bool {return m_isScissorsEnabled;};
protected:
    MeshType m_meshType;
    bool m_isTransparent = false;

    bool m_isScissorsEnabled = false;

    std::array<int, 2> m_scissorOffset = {0,0};
    std::array<uint32_t, 2> m_scissorSize = {0,0};



//Vulkan specific
    std::vector<bool> descriptorSetsUpdated;

    std::shared_ptr<GRenderPassVLK> m_lastRenderPass = nullptr;
    std::shared_ptr<GPipelineVLK> m_lastPipelineForRenderPass;
private:
    GDeviceVLK &m_device;
    HMaterialVLK m_material;
};


#endif //WEBWOWVIEWERCPP_GMESH_H
