//
// Created by Deamon on 8/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_IMESH_H
#define AWEBWOWVIEWERCPP_IMESH_H

#include <array>
#include <vector>
#include <iostream>

#include "../IDevice.h"

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
    HGShaderPermutation shader;
    MeshType meshType = MeshType::eGeneralMesh;

    int8_t triCCW = 1; //counter-clockwise
    bool depthWrite = true;
    bool depthCulling = true;
    bool backFaceCulling = true;
    EGxBlendEnum blendMode;
    bool skybox = false;

    uint8_t colorMask = 0xFF;

    int start;
    int end;
    DrawElementMode element;
    std::vector<HGTexture> texture = {};

    bool scissorEnabled = false;
    std::array<int, 2> scissorOffset = {0,0};
    std::array<int, 2> scissorSize = {0,0};
};


class IMesh {
    friend class IDevice;

public:
    auto start()       -> int& { return m_start; }
    auto end()       -> int& { return m_end; }
//    auto textureCount()       -> int { return m_texture.size(); }

    auto bindings() const -> const HGVertexBufferBindings& { return m_bindings; }


//    auto texture() const -> const HGVertexBufferBindings& { return m_bindings; }
protected:
    HGVertexBufferBindings m_bindings;
    int m_start;
    int m_end;


public:
    virtual ~IMesh() = default;

    virtual bool getIsTransparent() = 0;
    virtual MeshType getMeshType() = 0;

    void setStart(int start) {m_start = start; }
    void setEnd(int end) {m_end = end; }
};
#endif //AWEBWOWVIEWERCPP_IMESH_H
