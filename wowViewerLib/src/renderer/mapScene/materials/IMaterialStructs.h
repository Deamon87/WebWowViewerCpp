//
// Created by Deamon on 3/5/2023.
//

#ifndef AWEBWOWVIEWERCPP_IMATERIALSTRUCTS_H
#define AWEBWOWVIEWERCPP_IMATERIALSTRUCTS_H

#include <functional>

#include "../../../gapi/interface/materials/IMaterial.h"
#include "../../../gapi/UniformBufferStructures.h"
#include "../../../engine/objects/m2/m2Helpers/M2GpuAnimData.h"
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
    // When true, build the GPU-sim variant (pull-model vertex shader reading the
    // particle SSBO pools; mesh has no vertex buffer). Only the bindless Vulkan
    // renderer honors it; others ignore it.
    bool forGpuPath = false;
};
struct M2RibbonMaterialTemplate {
    std::array<HGSamplableTexture, 1> textures = {nullptr};
    // When true, build the GPU-sim variant (pull-model vertex shader reading the
    // ribbon SSBO pools; the strip index buffer is rewritten by the sim each frame).
    // Only the bindless Vulkan renderer honors it; others ignore it.
    bool forGpuPath = false;
};

struct WMOMaterialTemplate {
    std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> m_modelWide;

    std::array<HGSamplableTexture, 9> textures = {nullptr, nullptr, nullptr,
                                                  nullptr, nullptr, nullptr,
                                                  nullptr, nullptr, nullptr};
};

struct ADTMaterialTemplate {
    int maxLayerCount = 4;

    std::array<HGSamplableTexture, 8> layerTextures;
    std::array<HGSamplableTexture, 8> heightTextures;

    std::array<HGSamplableTexture, 2> blendTextures;
};

struct WaterMaterialTemplate {
    HGSamplableTexture texture = nullptr;
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

    // Dense M2ObjId, used for GPU object-id picking (ribbons/particles resolve to their owning M2 via this)
    uint32_t objectId = 0;
};

// Per-emitter GPU sim wiring info, filled by the renderer at GPU-anim-data creation
// and consumed by ParticleEmitter::setGpuBindFields (writes the per-emitter UBO).
struct GpuParticleEmitterBindInfo {
    int32_t stateIndex = -1;           // GpuParticleState slot (-1 = emitter stays on CPU)
    int32_t particleOffset = 0;
    int32_t capacity = 0;
    int32_t staticsIndex = 0;
    int32_t propsIndex = 0;
    int32_t colorReplOffset = -1;      // vec4 elements into the color-replacement pool
    int32_t quadsPerParticle = 1;
    int32_t valuesVec4Offset = 0;
    int32_t valuesFloatOffset = 0;
    int32_t partTimesOffset = 0;
};

// Per-ribbon GPU sim wiring info (CRibbonEmitter::setGpuBindFields).
struct GpuRibbonEmitterBindInfo {
    int32_t stateIndex = -1;           // GpuRibbonState slot
    int32_t propsIndex = 0;            // GpuM2RibbonFrameProps slot
    int32_t edgesOffset = 0;           // first GpuRibbonEdge in the edges pool
    int32_t edgeCount = 0;
    uint32_t objectId = 0;             // owning M2's dense M2ObjId (picking)
};

// GPU M2 animation: model-shared static track data (one per source model,
// shared by every M2Object instance of that model). Owns the renderer-pool
// chunks for bone statics, track headers/slices, timestamps, value pools,
// sequence durations, global loops, part times and particle/ribbon statics,
// plus the offsets of those chunks that get baked into GpuM2AnimState.
// Created once by the renderer's createM2GpuAnimData (keyed on the source
// geometry); later instances of the same model reuse it.
class IM2GpuTrackSet {
public:
    virtual ~IM2GpuTrackSet() = default;

    // Number of resident sequences (flag 0x20, both banks) baked into the
    // uploaded track data. Compared by the engine against
    // countGpuLoadedSequences to detect .anim streaming completion.
    virtual int32_t getLoadedSeqCount() const = 0;

    // Bumped on every track-data re-upload (.anim streaming). The engine
    // watches it to re-push per-object emitter bind fields, which embed
    // value-pool offsets that move with a re-upload.
    virtual uint32_t getDataGeneration() const = 0;

    // If the model's resident-sequence count has grown past the uploaded
    // baseline, invokes packBuilder (engine-side buildM2GpuTrackPack) exactly
    // once per growth across all instances and re-uploads the track pools:
    // the track-value pools (seqSlices/timestamps/valuesVec4/valuesFloat) are
    // replaced with fresh chunks; particle/ribbon statics are re-uploaded in
    // place. Live sim state (particles/ribbons) and state slots are preserved.
    // Thread-safe: concurrent calls from instances of the same model collapse
    // to a single rebuild.
    virtual void syncTrackData(int32_t loadedSeqCount,
                               const std::function<M2GpuTrackPack()> &packBuilder) = 0;
};

// GPU M2 animation: per-object animation state slot + per-object sim data.
// Created lazily by M2Object when Config::useGpuAnimation is on and the renderer
// supports it (bindless Vulkan only). Implemented by the renderer; engine code
// only writes the dynamic part of the state through getStateForWrite().
// The static track data lives in the shared IM2GpuTrackSet (getTrackSet()).
class IM2GpuAnimData {
public:
    virtual ~IM2GpuAnimData() = default;

    // Returns the per-object state struct for writing. The static fields (chunk
    // offsets) are pre-filled by the renderer from the shared track set; the
    // caller must fill the dynamic fields (animIndex/animTime/blend/global
    // sequence times/matrices) every frame.
    virtual GpuM2AnimState &getStateForWrite() = 0;

    // Element index of this object's state in the renderer's anim-state SSBO.
    virtual int32_t getStateIndex() const = 0;

    // The model-shared static track data this object references.
    virtual std::shared_ptr<IM2GpuTrackSet> getTrackSet() = 0;

    // Per-emitter GPU sim wiring (particle path). Returns false for emitters that
    // stay on the CPU path (spline/bone generators) or when the model has none.
    virtual bool getParticleEmitterBindInfo(int emitterLocalIndex, GpuParticleEmitterBindInfo &out) const = 0;

    // ParticleColor.dbc runtime replacement (M2Object::setReplaceParticleColors)
    virtual void updateParticleColorReplacements(const std::array<std::array<mathfu::vec4, 3>, 3> &colors, bool valid) = 0;

    // Per-ribbon GPU sim wiring. All ribbons of a GPU-animated model run on GPU.
    virtual bool getRibbonBindInfo(int ribbonLocalIndex, GpuRibbonEmitterBindInfo &out) const = 0;
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
    std::shared_ptr<IBufferChunk<M2::ProjectiveData>> m_projectiveTextData = nullptr;
};
class IM2ProjectiveMaterialBindless : public IM2ProjectiveMaterial {
public:
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

class IPlanetMaterial : public IMaterial {
public:
    ~IPlanetMaterial() override = default;

    std::shared_ptr<IBufferChunk<Planet::meshWideBlockVS>> m_planetData = nullptr;
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

class IADTMaterialBindless : public IADTMaterial {
public:
    std::shared_ptr<IBufferChunk<ADT::AdtInstanceData>> m_instanceData = nullptr;
    std::vector<std::shared_ptr<BindlessTexture>> m_bindlessText;
    int instanceIndex = 0;
};

constexpr uint8_t MAX_LIQUID_TYPE_TEXTURES = 6;

class ILiquidDataBase {
public:
    //Contains only base api for updating textures
    virtual ~ILiquidDataBase() = default;
    // Resolves which textures should be bound this frame (animation, frame pairs).
    // Default: only fills the material-specific extra texture (slot 7) when set.
    // Per-type specializations of LiquidData<T> override for custom logic
    // (e.g. LiquidData<Liquid::FelData> computes the normal-map frame pair).
    virtual void resolveAnimatedTextures(std::array<HGSamplableTexture, MAX_LIQUID_TYPE_TEXTURES + 1> &textures, float currentTime) {
        if (extraTexture != nullptr) {
            textures[MAX_LIQUID_TYPE_TEXTURES] = extraTexture;
        }
    };
    //The actual method of texture storage is defined inside the Renderer implementation.
    //Array has one spare slot (MAX+1) for material-specific extra textures (e.g. fel's second normal frame).
    virtual void updateAnimatedTextures(const std::array<HGSamplableTexture, MAX_LIQUID_TYPE_TEXTURES + 1> &textures, float currentTime) = 0;

public:
    //Storage to store the data related to this liquid
    int liquidMaterial;
    LiquidTypeAndMat liquidTypeAndMat;
    int matLVF;
    std::array<std::vector<HGSamplableTexture>, MAX_LIQUID_TYPE_TEXTURES> usedTextures = {};
    // Optional material-specific extra texture, bound at slot 7 (e.g. magma's noise volume)
    HGSamplableTexture extraTexture = nullptr;
    // Optional second extra texture, bound at binding 8 (e.g. azerithe's env map)
    HGSamplableTexture extraTexture2 = nullptr;
    // Optional third extra texture, bound at binding 9 (e.g. azerithe's foam)
    HGSamplableTexture extraTexture3 = nullptr;
};

class ILiquidMaterial : public IMaterial {
public:
    int materialId;
    int matLVF;
    bool generateTexCoordsFromPos = false;

    std::shared_ptr<IBufferChunk<Liquid::LiquidInstance>> m_instance = nullptr;
    std::shared_ptr<ILiquidDataBase> m_ldData = nullptr;
};

class IWaterMaterialBindless : public ILiquidMaterial {
public:
    std::vector<std::shared_ptr<BindlessTexture>> m_bindlessText;
    std::shared_ptr<IBufferChunk<Liquid::LiquidBindless>> m_instanceBindless = nullptr;
    int instanceIndex = 0;
};



template<typename T>
class LiquidData : public ILiquidDataBase {
public:
    std::shared_ptr<IBufferChunk<T>> m_liquidData = nullptr;

    virtual ~LiquidData() = default;
    void updateData();
    void resolveAnimatedTextures(std::array<HGSamplableTexture, MAX_LIQUID_TYPE_TEXTURES + 1> &textures, float currentTime) override {
        ILiquidDataBase::resolveAnimatedTextures(textures, currentTime);
    }
};

using WaterLiquidData = LiquidData<Liquid::WaterData>;
using MagmaLiquidData = LiquidData<Liquid::MagmaData>;
using MercuryLiquidData = LiquidData<Liquid::MercuryData>;
using FogLiquidData = LiquidData<Liquid::FogData>;
using LeyLineLiquidData = LiquidData<Liquid::LeyLineData>;
using FelLiquidData = LiquidData<Liquid::FelData>;
using SwampLiquidData = LiquidData<Liquid::SwampData>;
using AzeritheLiquidData = LiquidData<Liquid::AzeritheData>;

// Fel specialization: resolves the animated normal map (Texture[5]) frame pair
// (frame k -> slot 5, frame (k+1)%N -> slot 7) for the GPU crossfade (normalBlend.x).
// Defined in LiquidFel.cpp
template<>
void LiquidData<Liquid::FelData>::resolveAnimatedTextures(
    std::array<HGSamplableTexture, MAX_LIQUID_TYPE_TEXTURES + 1> &textures, float currentTime);

// Swamp specialization: resolves the animated normal map (Texture[4]) frame pair
// (frame k -> slot 4, frame (k+1)%N -> slot 7) for the GPU crossfade (animParams.x).
// Defined in LiquidSwamp.cpp
template<>
void LiquidData<Liquid::SwampData>::resolveAnimatedTextures(
    std::array<HGSamplableTexture, MAX_LIQUID_TYPE_TEXTURES + 1> &textures, float currentTime);

// Azerithe specialization: resolves the animated normal map (Texture[5]) frame pair
// (frame k -> slot 5, frame (k+1)%N -> slot 7) for the GPU crossfade (normalBlend.x).
// Defined in LiquidAzerithe.cpp
template<>
void LiquidData<Liquid::AzeritheData>::resolveAnimatedTextures(
    std::array<HGSamplableTexture, MAX_LIQUID_TYPE_TEXTURES + 1> &textures, float currentTime);

class IPortalMaterial : public IMaterial {
public:
    std::shared_ptr<IBufferChunk<DrawPortalShader::meshWideBlockPS>> m_materialPS = nullptr;
};

#endif //AWEBWOWVIEWERCPP_IMATERIALSTRUCTS_H
