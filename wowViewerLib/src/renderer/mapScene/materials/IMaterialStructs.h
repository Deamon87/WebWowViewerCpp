//
// Created by Deamon on 3/5/2023.
//

#ifndef AWEBWOWVIEWERCPP_IMATERIALSTRUCTS_H
#define AWEBWOWVIEWERCPP_IMATERIALSTRUCTS_H

#include "../../../gapi/interface/materials/IMaterial.h"
#include "../../../gapi/UniformBufferStructures.h"
#include "BindlessTexture.h"
//----------------------------
// Material Templates
//----------------------------

struct M2MaterialTemplate {
    int vertexShader = 0;
    int pixelShader = 0;
    int batchIndex;
    std::array<HGSamplableTexture, 4> textures = {nullptr, nullptr, nullptr, nullptr};
};

struct M2WaterfallMaterialTemplate {
    std::array<HGSamplableTexture, 5> textures = {nullptr, nullptr, nullptr, nullptr, nullptr};
};


struct M2ParticleMaterialTemplate {
    std::array<HGSamplableTexture, 3> textures = {nullptr, nullptr, nullptr};
};
struct M2RibbonMaterialTemplate {
    std::array<HGSamplableTexture, 1> textures = {nullptr};
};

struct WMOMaterialTemplate {
    std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> m_modelWide;

    std::array<HGSamplableTexture, 9> textures = {nullptr, nullptr, nullptr,
                                                  nullptr, nullptr, nullptr,
                                                  nullptr, nullptr, nullptr};
};

struct ADTMaterialTemplate {
    std::array<HGSamplableTexture, 9> textures = {nullptr, nullptr, nullptr,
                                                  nullptr, nullptr, nullptr,
                                                  nullptr, nullptr, nullptr};
};

struct WaterMaterialTemplate {
    HGSamplableTexture texture = nullptr;
    mathfu::vec3 color;
    int liquidFlags;
    int liquidMaterialId;
};

class IM2ModelData {
public:
    virtual ~IM2ModelData() = default;

    std::shared_ptr<IBufferChunk<M2::PlacementMatrix>> m_placementMatrix = nullptr;
    std::shared_ptr<IBufferChunk<M2::Bones>> m_bonesData = nullptr;
    std::shared_ptr<IBufferChunk<M2::M2Colors>> m_colors = nullptr;
    std::shared_ptr<IBufferChunk<M2::TextureWeights>> m_textureWeights = nullptr;

    std::shared_ptr<IBufferChunk<M2::modelWideBlockPS>> m_modelFragmentData = nullptr;
    std::shared_ptr<IBufferChunk<M2::TextureMatrices>> m_textureMatrices = nullptr;
    //Constants
    std::shared_ptr<IBufferChunk<M2::M2InstanceRecordBindless>> m_instanceBindless = nullptr;
};

class IM2Material : public IMaterial {
public:
    ~IM2Material() override = default;

    int vertexShader;
    int pixelShader;
    int batchIndex;
    EGxBlendEnum blendMode;
    bool depthWrite;
    bool depthCulling;
    bool backFaceCulling;
    std::shared_ptr<IBufferChunk<M2::meshWideBlockVSPS>> m_vertexFragmentData = nullptr;
};

class IM2MaterialBindless : public IM2Material {
public:
    ~IM2MaterialBindless() override = default;

    std::shared_ptr<IBufferChunk<M2::meshWideBlockVSPS_Bindless>> m_vertexFragmentDataBindless = nullptr;
    std::vector<std::shared_ptr<BindlessTexture>> m_bindlessText;
    int instanceIndex = 0;
};

class IM2ProjectiveMaterial : public IMaterial {
public:
    ~IM2ProjectiveMaterial() override = default;

    std::shared_ptr<IBufferChunk<M2::meshWideBlockVSPS>> m_vertexFragmentData = nullptr;
};
class IM2ProjectiveMaterialBindless : public IM2ProjectiveMaterial {
    ~IM2ProjectiveMaterialBindless() override = default;

    std::shared_ptr<IBufferChunk<M2::meshWideBlockVSPS_Bindless>> m_vertexFragmentDataBindless = nullptr;
    std::vector<std::shared_ptr<BindlessTexture>> m_bindlessText;
    int instanceIndex = 0;
};

class IM2WaterFallMaterial : public IMaterial {
public:
    int vertexShader;
    int pixelShader;
    EGxBlendEnum blendMode;

    std::shared_ptr<IBufferChunk<M2::WaterfallData::WaterfallCommon>> m_waterfallCommon = nullptr;
};
class IM2WaterFallMaterialBindless : public IM2WaterFallMaterial {
public:
    int instanceIndex = 0;
    std::shared_ptr<IBufferChunk<M2::WaterfallData::WaterfallBindless>> m_waterfallBindless = nullptr;
    std::vector<std::shared_ptr<BindlessTexture>> m_bindlessText;
};


class IM2ParticleMaterial : public IMaterial {
public:
    std::shared_ptr<IBufferChunk<Particle::meshParticleWideBlockPS>> m_fragmentData = nullptr;
};

class IM2RibbonMaterial : public IMaterial {
public:
    std::shared_ptr<IBufferChunk<Ribbon::meshRibbonWideBlockPS>> m_fragmentData = nullptr;
};

class ISkyMeshMaterial : public IMaterial {
public:
    ~ISkyMeshMaterial() override = default;

    std::shared_ptr<IBufferChunk<DnSky::meshWideBlockVS>> m_skyColors = nullptr;
};

class IWmoModelData {
public:
    virtual ~IWmoModelData() = default;

    std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> m_placementMatrix = nullptr;
    std::shared_ptr<IBufferChunk<WMO::GroupInteriorData>> m_groupInteriorData = nullptr;
};

class IWMOMaterial : public IMaterial {
public:
    std::shared_ptr<IBufferChunk<WMO::meshWideBlockVS>> m_materialVS= nullptr;
    std::shared_ptr<IBufferChunk<WMO::meshWideBlockPS>> m_materialPS = nullptr;
};
class IWMOMaterialBindless : public IWMOMaterial {
public:
    std::shared_ptr<IBufferChunk<WMO::meshWideBlockBindless>> m_meshBindless = nullptr;
    std::shared_ptr<IBufferChunk<WMO::perMeshData>> m_perMeshData = nullptr;
    std::vector<std::shared_ptr<BindlessTexture>> m_bindlessText;
    int meshWideBindlessIndex = 0;
    int interiorDataIndex = 0;
};

class IADTMaterial : public IMaterial {
public:
    std::shared_ptr<IBufferChunk<ADT::meshWideBlockVSPS>> m_materialVSPS = nullptr;
    std::shared_ptr<IBufferChunk<ADT::meshWideBlockPS>> m_materialPS = nullptr;
};

class IADTMaterialVis : public IADTMaterial {
public:
    std::shared_ptr<IBufferChunk<ADT::AdtInstanceData>> m_instanceData = nullptr;
    std::vector<std::shared_ptr<BindlessTexture>> m_bindlessText;
    int instanceIndex = 0;
};


class IWaterMaterial : public IMaterial {
public:
    mathfu::vec3 color;
    int liquidFlags;
    int materialId;
    std::shared_ptr<IBufferChunk<Water::meshWideBlockPS>> m_materialPS = nullptr;
    std::shared_ptr<IBufferChunk<Water::WaterBindless>> m_bindless = nullptr;
};

class IWaterMaterialBindless : public IWaterMaterial {
public:
    std::vector<std::shared_ptr<BindlessTexture>> m_bindlessText;
    int instanceIndex = 0;
};

class IPortalMaterial : public IMaterial {
public:
    std::shared_ptr<IBufferChunk<DrawPortalShader::meshWideBlockPS>> m_materialPS = nullptr;
};

#endif //AWEBWOWVIEWERCPP_IMATERIALSTRUCTS_H
