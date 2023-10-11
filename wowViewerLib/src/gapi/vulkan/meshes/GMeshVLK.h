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

class GMeshVLK : public IM2Mesh {
    friend class GDeviceVLK;
public:
    explicit GMeshVLK(const gMeshTemplate &meshTemplate,
                      const HMaterialVLK &material, int layer, int priority);

public:
    ~GMeshVLK() override;

    bool getIsTransparent() override;
    MeshType getMeshType()  override;
    EGxBlendEnum getGxBlendMode() override {
        return material()->getBlendMode();
    }

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


private:
    HMaterialVLK m_material;
};


#endif //WEBWOWVIEWERCPP_GMESH_H
