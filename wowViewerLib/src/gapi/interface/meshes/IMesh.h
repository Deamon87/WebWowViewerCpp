//
// Created by Deamon on 8/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_IMESH_H
#define AWEBWOWVIEWERCPP_IMESH_H
enum class MeshType {
    eGeneralMesh = 0,
    eAdtMesh = 1,
    eWmoMesh = 2,
    eOccludingQuery = 3,
    eM2Mesh = 4,
    eParticleMesh = 5,
};

class gMeshTemplate {
public:
    gMeshTemplate(HGVertexBufferBindings bindings, HGShaderPermutation shader) : bindings(bindings), shader(shader) {}
    HGVertexBufferBindings bindings;
    HGShaderPermutation shader;
    MeshType meshType = MeshType::eGeneralMesh;

    int8_t triCCW = 1; //counter-clockwise
    bool depthWrite;
    bool depthCulling;
    bool backFaceCulling;
    EGxBlendEnum blendMode;

    uint8_t colorMask = 0xFF;

    int start;
    int end;
    int element;
    unsigned int textureCount;
    std::vector<HGTexture> texture = std::vector<HGTexture>(4, nullptr);
    HGUniformBuffer vertexBuffers[3] = {nullptr,nullptr,nullptr};
    HGUniformBuffer fragmentBuffers[3] = {nullptr,nullptr,nullptr};
};



class IMesh {
public:
    virtual HGUniformBuffer getVertexUniformBuffer(int slot) = 0;
    virtual inline HGUniformBuffer getFragmentUniformBuffer(int slot) = 0;
    virtual inline EGxBlendEnum getGxBlendMode() = 0;
    virtual inline bool getIsTransparent() = 0;
    virtual inline MeshType getMeshType() = 0;
    virtual void setRenderOrder(int renderOrder) = 0;

    virtual void setEnd(int end);;
};
#endif //AWEBWOWVIEWERCPP_IMESH_H
