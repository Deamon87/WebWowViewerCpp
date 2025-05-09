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

    }

    ~M2Object();

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

    float m_currentDistance = 0;

    CAaBox *aabb;
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

    std::vector<std::function<void()>> m_postLoadEvents;

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


    void createMeshes(const HMapSceneBufferCreate &sceneRenderer);
    void createBoundingBoxMesh(const HMapSceneBufferCreate &sceneRenderer);

    EGxBlendEnum getBlendMode(int batchIndex);

public:
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

    void addPostLoadEvent(const std::function<void()> &value) {
        m_postLoadEvents.push_back(value);
    }

    const CAaBox &getAABB() { return *aabb; };
    CAaBox getColissionAABB();

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
    void calcDistance(mathfu::vec3 cameraPos);
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

    const bool checkFrustumCulling(const mathfu::vec4 &cameraPos,
                                   const MathHelper::FrustumCullingData &frustumData);

    bool isMainDataLoaded() const;
    bool getHasBoundingBox() const {return status->m_hasAABB;}
    CAaBox getBoundingBox() const {return *aabb;}

    void doLoadMainFile();
    bool isFailedToLoadMainFile();
    void doLoadGeom(const HMapSceneBufferCreate &sceneRenderer);
    bool isFailedToLoadGeomFile();
    void update(double deltaTime, mathfu::vec3 &cameraPos, mathfu::mat4 &viewMat);
    void collectLights(std::vector<LocalLight> &pointLights);
    void fitParticleAndRibbonBuffersToSize(const HMapSceneBufferCreate &sceneRenderer);
    void uploadBuffers(mathfu::mat4 &viewMat, const HFrameDependantData &frameDependantData);
    void uploadGeneratorBuffers(mathfu::mat4 &viewMat, const HFrameDependantData &frameDependantData);
    M2CameraResult updateCamera(double deltaTime, int cameraViewId);
    void drawDebugLight();


    void drawBBInternal(CAaBox &bb, mathfu::vec3 &color, mathfu::Matrix<float, 4, 4> &placementMatrix);

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

    void drawParticles(COpaqueMeshCollector &opaqueMeshCollector, transp_vec<HGSortableMesh> &transparentMeshes,  int renderOrder);

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
    auto * ptr = m2Factory->getObjectById<1>(objectId);
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
