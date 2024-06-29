//
// Created by Deamon on 8/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_IMESH_H
#define AWEBWOWVIEWERCPP_IMESH_H

#include <array>
#include <vector>
#include <iostream>

#include "../IDevice.h"
#include "../../../engine/objects/SceneObjectWithID.h"

enum class MeshType {
    eGeneralMesh = 0,
    eAdtMesh = 1,
    eWmoMesh = 2,
    eOccludingQuery = 3,
    eM2Mesh = 4,
    eParticleMesh = 5,
};

enum class EGxBlendEnum {
    GxBlend_UNDEFINED = -1,
    GxBlend_Opaque = 0,
    GxBlend_AlphaKey = 1,
    GxBlend_Alpha = 2,
    GxBlend_Add = 3,
    GxBlend_Mod = 4,
    GxBlend_Mod2x = 5,
    GxBlend_ModAdd = 6,
    GxBlend_InvSrcAlphaAdd = 7,
    GxBlend_InvSrcAlphaOpaque = 8,
    GxBlend_SrcAlphaOpaque = 9,
    GxBlend_NoAlphaAdd = 10,
    GxBlend_ConstantAlpha = 11,
    GxBlend_Screen = 12,
    GxBlend_BlendAdd = 13,
    GxBlend_MAX
};
struct BlendModeDesc {
    bool blendModeEnable;
    int SrcColor;
    int DestColor;
    int SrcAlpha;
    int DestAlpha;
};

enum class DrawElementMode {
    TRIANGLES,
    TRIANGLE_STRIP
};

class gMeshTemplate {
public:
    gMeshTemplate(HGVertexBufferBindings bindings) : bindings(bindings) {}
    HGVertexBufferBindings bindings;
    MeshType meshType = MeshType::eGeneralMesh;

    int start;
    int end;

    bool scissorEnabled = false;
    std::array<int, 2> scissorOffset = {0,0};
    std::array<uint32_t, 2> scissorSize = {0,0};
};

enum class GMeshId : uintptr_t;

class IMesh : public ObjectWithId<GMeshId> {
    friend class IDevice;

public:
    auto start()       -> uint32_t& { return m_start; }
    auto end()       -> uint32_t& { return m_end; }
//    auto textureCount()       -> int { return m_texture.size(); }

    auto bindings() const -> const HGVertexBufferBindings& { return m_bindings; }


    int vertexStart = 0;
    int instanceIndex = -1;
protected:
    HGVertexBufferBindings m_bindings;
    uint32_t m_start;
    uint32_t  m_end;



public:
    ~IMesh() override {
        1 + 1;
    };

    virtual bool getIsTransparent() = 0;
    virtual MeshType getMeshType() = 0;

    void setStart(int start) {m_start = start; }
    void setEnd(int end) {m_end = end; }
};


class COpaqueMeshCollector {
public:
    virtual ~COpaqueMeshCollector() {};
    virtual void addM2Mesh(const HGM2Mesh &mesh) = 0;
    virtual void addWMOMesh(const HGMesh &mesh) = 0;
    virtual void addWaterMesh(const HGMesh &mesh) = 0;
    virtual void addADTMesh(const HGMesh &mesh) = 0;

    virtual void addMesh(const HGMesh &mesh) = 0;

    virtual void merge(COpaqueMeshCollector & collector) = 0;
    virtual COpaqueMeshCollector * clone() = 0;
};
#endif //AWEBWOWVIEWERCPP_IMESH_H
