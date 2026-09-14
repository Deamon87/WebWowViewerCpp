//
// Created by deamon on 22.06.17.
//

#ifndef WOWVIEWERLIB_M2OBJECT_H
#define WOWVIEWERLIB_M2OBJECT_H

class M2Object;
class M2ObjectListContainer;
#define _USE_MATH_DEFINES

#include <cstdint>
#include <oneapi/tbb/concurrent_unordered_set.h>
#include <oneapi/tbb/parallel_sort.h>
#include <unordered_set>
#include "mathfu/glsl_mappings.h"
#include "../../managers/particles/particleEmitter.h"
#include "../../persistance/header/wmoFileHeader.h"
#include "../../geometry/m2Geom.h"
#include "../../geometry/skinGeom.h"
#include "m2Helpers/M2MaterialInst.h"
#include "../../managers/animationManager.h"
#include "../../persistance/header/skinFileHeader.h"
#include "../../persistance/skelFile.h"
#include "../../managers/CRibbonEmitter.h"
#include "../../ApiContainer.h"
#include "m2Helpers/CBoneMasterData.h"
#include "m2Helpers/M2GpuAnimData.h"
#include "../../../gapi/UniformBufferStructures.h"
#include "../scenes/EntityActorsFactory.h"


enum class M2ObjId : uintptr_t;

struct M2LoadedStatus {
    bool m_loading = false;
    bool m_loaded = false;
    bool m_geomLoaded = false;
    bool m_hasAABB = false;
};

extern std::shared_ptr<EntityFactory<10000, M2ObjId, M2Object, CAaBox, M2LoadedStatus>> m2Factory;

class M2Object : public ObjectWithId<M2ObjId> {
public:
    friend class IExporter;

    M2Object(HApiContainer &api, bool isSkybox = false, bool overrideSkyModelMat = true) : m_api(api), m_m2Geom(nullptr),
        m_skinGeom(nullptr), m_animationManager(nullptr), m_boolSkybox(isSkybox), m_overrideSkyModelMat(overrideSkyModelMat)
    {
//        std::cout << "M2Object constructed" << std::endl;
    }

    ~M2Object() {
//        std::cout << "M2Object destroyed" << std::endl;
    }

    friend class M2MeshBufferUpdater;
private:
    void createAABB();

    M2LoadedStatus *status;

    bool m_alwaysDraw = false;



//    void load(std::string modelName, SMODoodadDef &doodadDef, mathfu::mat4 &wmoPlacementMat){
//        createPlacementMatrix(doodadDef, wmoPlacementMat);
//        calcWorldPosition();
//
//        this->setLoadParams(0, {}, {});
//        this->setModelFileName(modelName);
//    }

    struct dynamicVaoMeshFrame {
        int batchIndex = -1;
        HGVertexBufferDynamic m_bufferVBO = nullptr;
        HGVertexBufferBindings m_bindings = nullptr;
        HGM2Mesh m_mesh = nullptr;
    };

private:
    mathfu::mat4 m_placementMatrix = mathfu::mat4::Identity();
    bool m_placementMatrixChanged = false;
    bool m_modelWideDataChanged = false;
    bool m_firstUpdate = true;
    mathfu::mat4 m_placementInvertMatrix;
    mathfu::vec3 m_worldPosition;
    mathfu::vec3 m_localPosition;
    mathfu::vec3 m_localUpVector;
    mathfu::vec3 m_localRightVector;
    float m_scale;

    float m_currentDistance = 0;

    CAaBox colissionAabb;

    HApiContainer m_api = nullptr;

    HM2Geom m_m2Geom = nullptr;
    HSkinGeom m_skinGeom = nullptr;
    HSkelGeom m_skelGeom = nullptr;
    HSkelGeom m_parentSkelGeom = nullptr;
    std::shared_ptr<CBoneMasterData> m_boneMasterData = nullptr;

    HGVertexBufferBindings bufferBindings = nullptr;
    std::shared_ptr<IM2ModelData> m_modelWideData = nullptr;

    HGSortableMesh boundingBoxMesh = nullptr;
    
    mathfu::vec3 m_interiorAmbientColor;
    mathfu::vec3 m_interiorAmbientHorizontColor;
    mathfu::vec3 m_interiorAmbientGroundColor;

    mathfu::vec3 m_interiorSunDir = mathfu::vec3(0,0,0);
    bool m_setInteriorSunDir = false;

    float m_alpha = 1.0f;

    bool m_animationRequired = false;
    bool animationOverrideActive = false;
    float animationOverridePercent = 0;

    std::string m_modelName;
    std::string m_nameTemplate = "";

    bool useFileId = false;
    int m_modelFileId = 0;
    int m_skinFileId;

    std::vector<std::function<void(M2Object* m2Object)>> m_postLoadEvents;

    int m_skinNum = 0;
    mathfu::vec3 m_interiorDirectColor = mathfu::vec3(0.0, 0.0, 0.0);

    std::vector<uint8_t> m_meshIds;
    std::vector<HBlpTexture> m_replaceTextures;
    bool particleColorReplacementIsSet = false;
    std::array<std::array<mathfu::vec4, 3>, 3> m_particleColorReplacement;

    std::unique_ptr<AnimationManager> m_animationManager;
    std::vector<mathfu::mat4, tbb::cache_aligned_allocator<mathfu::mat4>> bonesMatrices;
    std::vector<mathfu::mat4, tbb::cache_aligned_allocator<mathfu::mat4>> textAnimMatrices;
    std::vector<mathfu::vec4, tbb::cache_aligned_allocator<mathfu::vec4>> subMeshColors;
    std::vector<float> transparencies;
    std::vector<M2LightResult> lights;
    std::vector<std::unique_ptr<ParticleEmitter>> particleEmitters;
    std::vector<std::unique_ptr<CRibbonEmitter>> ribbonEmitters;

    // ---- GPU animation path (Config::useGpuAnimation, bindless Vulkan only) ----
    // Lazily created by tryInitGpuAnimData() on the first frame the toggle is on.
    // When active, bone matrices are computed by the m2Animation compute pass and
    // written straight into the renderer's bone-matrix SSBO; the CPU only evaluates
    // the small m_cpuBoneSubset (sorting centers, light/emitter/ribbon attachments).
    // The static track data is NOT per-object: the renderer keeps one shared track
    // set per source model (m_m2Geom), referenced through m_gpuAnimData->getTrackSet().
    bool m_gpuAnimDataTried = false;
    std::shared_ptr<IM2GpuAnimData> m_gpuAnimData = nullptr;
    std::weak_ptr<IMapSceneBufferCreate> m_sceneRendererWeak;
    std::vector<int> m_cpuBoneSubset;
    // Frame (FrameContext processing number) when the GPU anim state was last written
    uint32_t m_gpuAnimStateFrame = 0;
    // Generation of the shared track data last seen by this object; when it bumps
    // (lazy .anim streaming re-upload, possibly triggered by another instance),
    // the emitter bind fields are re-pushed (they embed value-pool offsets)
    uint32_t m_gpuTrackDataGeneration = 0;
    // Streams newly resident (.anim) sequences into the shared GPU track set
    // (rebuild + re-upload happens at most once per model) and re-pushes this
    // object's emitter bind fields when the shared data moved
    void syncGpuAnimTrackData();
    // Forces one CPU bone-matrix upload when switching back from GPU to CPU animation
    bool m_forceBoneUploadAfterGpu = false;

    std::unordered_map<int, HBlpTexture> loadedTextures;

    std::vector<std::shared_ptr<IM2Material>> m_materialArray;
    std::vector<std::shared_ptr<IM2Material>> m_forcedTranspMaterialArray;

    std::vector<std::shared_ptr<IM2ProjectiveMaterial>> m_projectiveMaterialArray;


    //Tuple of Mesh and batch index
    std::vector<std::tuple<HGM2Mesh, int>> m_meshForcedTranspArray;
    std::vector<std::tuple<HGM2Mesh, int>> m_meshArray;
    std::vector<std::tuple<HGM2Mesh, int>> m_meshProjectiveArray;
    std::vector<float> m_finalTransparencies;

    //TODO: think about if it's viable to do forced transp for dyn meshes
    std::vector<std::array<dynamicVaoMeshFrame, IDevice::MAX_FRAMES_IN_FLIGHT>> dynamicMeshes;

    // Reused per-frame scratch for forEachVisibleMeshSorted (update thread only,
    // each M2 is collected once per frame)
    struct SortedTranspMeshEntry {
        HGSortableMesh mesh;
        bool hasDynamicDrawParams = false;
    };
    std::vector<SortedTranspMeshEntry> m_sortedTranspMeshScratch;

    //0.0 for full interior
    //1.0 for full exterior
    float m_interiorExteriorBlend = 1.0f; //exterior by default
    bool m_boolSkybox = false;
    bool m_overrideSkyModelMat = true;

    void debugDumpAnimationSequences();

    void initAnimationManager();
    void initBoneAnimMatrices();

    void startLoading();

    void initTextAnimMatrices();
    void initSubmeshColors();
    void initTransparencies();
    void initLights();
    void initParticleEmitters(const HMapSceneBufferCreate &sceneRenderer);
    void initRibbonEmitters(const HMapSceneBufferCreate &sceneRenderer);

    void sortMaterials(mathfu::Matrix<float, 4, 4> &modelViewMat);
    bool checkifBonesAreInRange(M2SkinProfile *skinProfile, M2SkinSection *mesh);

    // Builds the set of bones the CPU still evaluates when the GPU animation path is
    // active: transparent-sort centers, light/particle/ribbon attachment bones and
    // attachment points (their matrices are read by CPU-side code).
    void buildCpuBoneSubset();

    // Raw (unsorted) enumeration of visible static and dynamic-VAO meshes.
    void forEachVisibleMesh(const std::function<void(const HGM2Mesh &mesh)> &visitor);


    void createMeshes(const HMapSceneBufferCreate &sceneRenderer);
    void createBoundingBoxMesh(const HMapSceneBufferCreate &sceneRenderer);

    EGxBlendEnum getBlendMode(int batchIndex);

public:
    void setAABB(const CAaBox &aabb) {
        *m2Factory->getObjectById<1>(this->getObjectId()) = aabb;
    }

    void setAlwaysDraw(bool value) {
        m_alwaysDraw = value;
    }
    bool getAlwaysDraw() {
        return m_alwaysDraw;
    }
    bool getIsSkybox() {
        return m_boolSkybox;
    }

    void setInteriorExteriorBlend(float val) {
        m_interiorExteriorBlend = val;
    };

    void addPostLoadEvent(const std::function<void(M2Object * m2Object)> &value) {
        m_postLoadEvents.push_back(value);
    }

    const CAaBox &getAABB() { return *m2Factory->getObjectByIdConst<1>(this->getObjectId()); };
    CAaBox getColissionAABB();

    // Lets external code (e.g. world-object systems that already know a placement's
    // bounding box from DB data) seed the culling AABB before the M2 file is loaded.
    // Must be called after setLoadParams(...), which is what initializes `status`.
    // createAABB() will overwrite this with the accurate box once the model loads.
    void setInitialAABB(const CAaBox &aabb) {
        setAABB(aabb);
        status->m_hasAABB = true;
    }

    void setSize(float newSize);

    void setLoadParams(int skinNum, std::vector<uint8_t> meshIds,
                       std::vector<HBlpTexture> replaceTextures);

    void setReplaceTextures(const HMapSceneBufferCreate &sceneRenderer, const std::vector<HBlpTexture> &replaceTextures);
    void setMeshIds(const HMapSceneBufferCreate &sceneRenderer, const std::vector<uint8_t> &meshIds);
    void setReplaceParticleColors(std::array<std::array<mathfu::vec4, 3>, 3> &particleColorReplacement);
    void resetReplaceParticleColor();
    bool getReplaceParticleColors(std::array<std::array<mathfu::vec4, 3>, 3> &particleColorReplacement);
    void setModelFileName(std::string modelName);
    int getModelFileId();
    void setModelFileId(int fileId);

    void setAlpha(float alpha) {
        m_alpha = alpha;
        m_modelWideDataChanged = true;
    }

    float getAlpha() const {
        return m_alpha;
    }
    void setOverrideAnimationPerc(float percent, bool active) {
        animationOverrideActive = active;
        animationOverridePercent = percent;
    }

    void setAnimationId(int animationId);
    int getCurrentAnimationIndex();
    void resetCurrentAnimation();
    void createPlacementMatrix(const SMODoodadDef &def, mathfu::mat4 &wmoPlacementMat);
    void createPlacementMatrix(const SMDoodadDef &def);
    void createPlacementMatrix(mathfu::vec3 pos, float f, mathfu::vec3 scaleVec,
                               mathfu::Matrix<float, 4, 4> *rotationMatrix);

    void updatePlacementMatrixFromParentAttachment(M2Object *parent, int attachment, float scale);

    void calcWorldPosition(){
        m_worldPosition = (m_placementMatrix * mathfu::vec4(0,0,0,1)).xyz();
    }
    mathfu::vec3 getWorldPosition(){
        return m_worldPosition;
    }
    void calcDistance(const mathfu::vec3 &cameraPos);
    float getCurrentDistance();
    mathfu::vec3 getLocalPosition() {
        return m_localPosition;
    };

    float getHeight();
    void getAvailableAnimation(std::vector<int> &allAnimationList);
    void getMeshIds(std::vector<int> &meshIdList);
    mathfu::mat4 getTextureTransformByLookup(int textureTrasformlookup);
    int32_t getTextureTransformIndexByLookup(int textureTrasformlookup);
    bool getGetIsLoaded() { return status->m_loaded; };
    mathfu::mat4 getModelMatrix() { return m_placementMatrix; };

    bool prepearMaterial(M2MaterialTemplate &materialTemplate, int batchIndex);
    void collectMeshes(COpaqueMeshCollector &opaqueMeshCollector, transp_vec<HGSortableMesh> &transparentMeshes);
    // Mesh enumeration for the GPU-indirect draw path (mesh pointers; ids are read off
    // the meshes themselves). Projective/decal meshes are not part of it and are still
    // collected separately via the CPU-side path below.
    //
    // Meshes are served in render order: opaque meshes first in arbitrary order (the
    // renderer re-sorts them for batching anyway), then transparent meshes pre-sorted
    // within this object with the shared SortMeshes comparator (sortLambda.h) — static,
    // dynamic-VAO, particle and ribbon meshes sorted together. The renderer orders the
    // M2 objects themselves by bounding-box distance; it must preserve the intra-object
    // mesh order served here, not re-sort it.
    // hasDynamicDrawParams marks particle/ribbon meshes whose start/end change per frame.
    void forEachVisibleMeshSorted(const std::function<void(const HGSortableMesh &mesh, bool hasDynamicDrawParams)> &visitor);
    void forEachParticleEmitter(const std::function<void(ParticleEmitter *emitter)> &visitor);
    void forEachRibbonEmitter(const std::function<void(CRibbonEmitter *emitter)> &visitor);
    void collectProjectiveMeshes(COpaqueMeshCollector &opaqueMeshCollector);

    const bool checkFrustumCulling(const mathfu::vec4 &cameraPos,
                                   const MathHelper::FrustumCullingData &frustumData);

    bool isMainDataLoaded() const;
    bool getHasBoundingBox() const {return status->m_hasAABB;}

    void doLoadMainFile();
    bool isFailedToLoadMainFile();
    void doLoadGeom(const HMapSceneBufferCreate &sceneRenderer);
    bool isFailedToLoadGeomFile();

    void dumpBoneAnimations();

    void update(double deltaTime, mathfu::vec3 &cameraPos, mathfu::mat4 &viewMat);
    void collectLights(std::vector<LocalLight> &pointLights);
    void fitParticleAndRibbonBuffersToSize(const HMapSceneBufferCreate &sceneRenderer);
    void uploadBuffers(mathfu::mat4 &viewMat, const HFrameDependantData &frameDependantData);
    void uploadGeneratorBuffers(mathfu::mat4 &viewMat, const HFrameDependantData &frameDependantData);

    // GPU animation path: lazily packs track data and allocates renderer GPU slots.
    // No-op unless Config::useGpuAnimation is on and the renderer supports it.
    void tryInitGpuAnimData();
    // True when bones are being computed on GPU (checked by uploadBuffers)
    bool isGpuAnimActive() const { return m_gpuAnimData != nullptr && m_api->getConfig()->useGpuAnimation; }
    // Collects the GPU particle sim state indices of this object's GPU emitters
    // (only those inside the current [minParticle, maxParticle) range).
    void appendGpuParticleStateIndices(std::vector<uint32_t> &out) const;
    // Same for GPU ribbon sim state indices
    void appendGpuRibbonStateIndices(std::vector<uint32_t> &out) const;
private:
    // (Re)writes every GPU emitter's bind fields from the anim data (also called
    // when the ParticleColor replacement changes validity at runtime)
    void pushGpuParticleBindFields();
public:
    // For the renderer's per-frame animation dispatch list: state index if this
    // object's GPU anim state was written during the given processing frame, else -1.
    int32_t getGpuAnimStateIndexForFrame(uint32_t frameNumber) const {
        if (m_gpuAnimData != nullptr && m_gpuAnimStateFrame == frameNumber) {
            return m_gpuAnimData->getStateIndex();
        }
        return -1;
    }
    M2CameraResult updateCamera(double deltaTime, int cameraViewId);

    void drawBBInternal(const CAaBox &bb, mathfu::vec3 &color, mathfu::Matrix<float, 4, 4> &placementMatrix);

    void drawBB(mathfu::vec3 &color);

    HBlpTexture getBlpTextureData(int textureInd);
    HGSamplableTexture getTexture(int textureInd);
    HBlpTexture getHardCodedTexture(int textureInd);

    mathfu::vec4 getM2SceneAmbientLight();
    void setAmbientColorOverride(
        const mathfu::vec3 &ambientColor,
        const mathfu::vec3 &horizontAmbientColor,
        const mathfu::vec3 &groundAmbientColor
    ) {
        m_interiorAmbientColor = ambientColor;
        m_interiorAmbientHorizontColor = horizontAmbientColor;
        m_interiorAmbientGroundColor = groundAmbientColor;

        m_modelWideDataChanged = true;
    }
    void setInteriorDirectColor(const mathfu::vec3 &interiorDirectColor);
    void setSunDirOverride(const mathfu::vec3 &sunDir) {
        m_interiorSunDir = sunDir;
        m_setInteriorSunDir = true;
    }

    void drawParticles(COpaqueMeshCollector &opaqueMeshCollector, transp_vec<HGSortableMesh> &transparentMeshes);

    void createVertexBindings(const HMapSceneBufferCreate &sceneRenderer);

    int getCameraNum() {
        if (!getGetIsLoaded()) return 0;

        return m_m2Geom->m_m2Data->cameras.size;
    }

    std::shared_ptr<IM2Material> createM2Material(const HMapSceneBufferCreate &sceneRenderer, int batchIndex, const EGxBlendEnum blendMode, bool overrideBlend);
    std::shared_ptr<IM2ProjectiveMaterial> createM2ProjectiveMaterial(const HMapSceneBufferCreate &sceneRenderer, int batchIndex);

    HGM2Mesh createSingleMesh(const HMapSceneBufferCreate &sceneRenderer, int indexStartCorrection,
                              const HGVertexBufferBindings &finalBufferBindings,
                              const std::shared_ptr<IM2Material> &m2Material,
                              const M2SkinSection *skinSection,
                              const M2Batch *m2Batch);

    HGM2Mesh createProjectiveMesh(const HMapSceneBufferCreate &sceneRenderer,
                                  const std::shared_ptr<IM2ProjectiveMaterial> &m2Material,
                                  const M2SkinSection *skinSection,
                                  const M2Batch *m2Batch);

    HGM2Mesh createWaterfallMesh(const HMapSceneBufferCreate &sceneRenderer, const HGVertexBufferBindings &finalBufferBindings);
    void updateDynamicMeshes();
};

#include "../../algorithms/mathHelper.h"
#include "../../../engine/custom_allocators/FrameBasedStackAllocator.h"



template<>
inline const CAaBox &retrieveAABB<>(const std::shared_ptr<M2Object> &object) {
    return object->getAABB();
}

//TODO: In retrieveAABB, AABB can be null
static const CAaBox nonexitsting = CAaBox(
    mathfu::vec3_packed(mathfu::vec3(999999, 999999, 999999)),
    mathfu::vec3_packed(mathfu::vec3(-999999, -999999, -999999))
);

template<>
inline const CAaBox &retrieveAABB<>(const M2ObjId &objectId) {
    auto * ptr = m2Factory->getObjectByIdConst<1>(objectId);
    return ptr ? *ptr : nonexitsting;
}



class M2ObjectListContainer {
using m2Container = framebased::vector<M2ObjId>;
//using m2Container = std::vector<std::shared_ptr<M2Object>>;
private:
    m2Container candidates;
    m2Container drawn;
    m2Container toLoadMain;
    m2Container toLoadGeom;

    bool m_locked = false;

    bool candCanHaveDuplicates = false;
    bool drawnCanHaveDuplicates = false;
    bool toLoadMainCanHaveDuplicates = false;
    bool toLoadGeomCanHaveDuplicates = false;

    void inline removeDuplicates(m2Container &array) {
        if (array.size() < 1000) {
            std::sort(array.begin(), array.end(), [](auto &a, auto &b) -> bool {
                return a < b;
            });
        } else {
            tbb::parallel_sort(array.begin(), array.end(), [](auto &a, auto &b) -> bool {
                return a < b;
            });
        }
        array.erase(std::unique(array.begin(), array.end()), array.end());
        return;
    }

public:
    M2ObjectListContainer() {
        candidates.reserve(100000);
        toLoadMain.reserve(10000);
        toLoadGeom.reserve(10000);
        drawn.reserve(10000);
    }
    inline void addCandidate(const std::shared_ptr<M2Object> &cand) {
//        if (m_locked) {
//            throw "oops";
//        }

        if (cand == nullptr) return;
        if (cand->getHasBoundingBox()) {
            candidates.push_back(cand->getObjectId());
            candCanHaveDuplicates = true;
        } else {
            if (cand->isFailedToLoadMainFile() || cand->isFailedToLoadGeomFile()) {
                //Do not accept such files. Maybe add those to separate vector?
                return;
            }

            toLoadMain.push_back(cand->getObjectId());
            toLoadMainCanHaveDuplicates = true;
        }
    }

    inline void addCandidate(const M2ObjId &cand) {
//        if (m_locked) {
//            throw "oops";
//        }
        auto status = m2Factory->getObjectById<2>(cand);
        if (status->m_hasAABB) {
            candidates.push_back(cand);
            candCanHaveDuplicates = true;
        } else {
            auto candObj = m2Factory->getObjectById<0>(cand);
            if (!candObj || candObj->isFailedToLoadMainFile() || candObj->isFailedToLoadGeomFile()) {
                //Do not accept such files. Maybe add those to separate vector?
                return;
            }

            toLoadMain.push_back(cand);
            toLoadMainCanHaveDuplicates = true;
        }
    }

    void addToDraw(const std::shared_ptr<M2Object> &toDraw) {
        if (m_locked) {
            throw "oops";
        }

        if (toDraw->getGetIsLoaded()) {
            drawn.push_back(toDraw->getObjectId());
            drawnCanHaveDuplicates = true;
        } else if (!toDraw->isMainDataLoaded()) {
            toLoadMain.push_back(toDraw->getObjectId());
            toLoadMainCanHaveDuplicates = true;
        } else {
            toLoadGeom.push_back(toDraw->getObjectId());
            toLoadGeomCanHaveDuplicates = true;
        }
    }

    void addToDraw(const M2ObjId &toDrawId) {
        if (m_locked) {
            throw "oops";
        }

        auto status = m2Factory->getObjectById<2>(toDrawId);
        if (status->m_loaded) {
            drawn.push_back(toDrawId);
            drawnCanHaveDuplicates = true;
        } else {
            auto toDraw = m2Factory->getObjectById<0>(toDrawId);

            if (!toDraw || toDraw->isFailedToLoadMainFile() || toDraw->isFailedToLoadGeomFile()) {
                //Do not accept such files. Maybe add those to separate vector?
                return;
            }

            if (!toDraw->isMainDataLoaded()) {
                toLoadMain.push_back(toDrawId);
                toLoadMainCanHaveDuplicates = true;
            } else {
                toLoadGeom.push_back(toDrawId);
                toLoadGeomCanHaveDuplicates = true;
            }
        }
    }



    void addToDraw(M2Object * toDraw) {
        if (m_locked) {
            throw "oops";
        }

        if (toDraw->isFailedToLoadMainFile() || toDraw->isFailedToLoadGeomFile()) {
            //Do not accept such files. Maybe add those to separate vector?
            return;
        }

        if (toDraw->getGetIsLoaded()) {
            drawn.push_back(toDraw->getObjectId());
            drawnCanHaveDuplicates = true;
        } else if (!toDraw->isMainDataLoaded()) {
            toLoadMain.push_back(toDraw->getObjectId());
            toLoadMainCanHaveDuplicates = true;
        } else {
            toLoadGeom.push_back(toDraw->getObjectId());
            toLoadGeomCanHaveDuplicates = true;
        }
    }

    void addDrawnAndToLoad(M2ObjectListContainer &anotherList) {
        if (m_locked) {
            throw "oops";
        }

        auto &anotherDrawn = anotherList.getDrawn();
        this->drawn.insert(this->drawn.end(), anotherDrawn.begin(), anotherDrawn.end());

        auto &anotherToLoadMain = anotherList.getToLoadMain();
        this->toLoadMain.insert(this->toLoadMain.end(), anotherToLoadMain.begin(), anotherToLoadMain.end());

        auto &anotherToLoadGeom = anotherList.getToLoadGeom();
        this->toLoadGeom.insert(this->toLoadGeom.end(), anotherToLoadGeom.begin(), anotherToLoadGeom.end());

        toLoadMainCanHaveDuplicates = true;
        toLoadGeomCanHaveDuplicates = true;
        drawnCanHaveDuplicates = true;
    }


    const m2Container &getCandidates() {
        if (this->candCanHaveDuplicates) {
            removeDuplicates(candidates);
            candCanHaveDuplicates = false;
        }

        return candidates;
    }

    const m2Container &getDrawn() {
        if (this->drawnCanHaveDuplicates) {
            removeDuplicates(drawn);
            drawnCanHaveDuplicates = false;
        }

        return drawn;
    }

    const m2Container &getToLoadMain() {
        if (this->toLoadMainCanHaveDuplicates) {
            removeDuplicates(toLoadMain);
            toLoadMainCanHaveDuplicates = false;
        }

        return toLoadMain;
    }

    const m2Container &getToLoadGeom() {
        if (this->toLoadGeomCanHaveDuplicates) {
            removeDuplicates(toLoadGeom);
            toLoadGeomCanHaveDuplicates = false;
        }

        return toLoadGeom;
    }


    void lock() {
        drawnCanHaveDuplicates = true;

        getCandidates();
        getToLoadGeom();
        getToLoadMain();
        getDrawn();

        m_locked = true;
    }
};


#endif //WOWVIEWERLIB_M2OBJECT_H
