//
// Created by deamon on 22.06.17.
//

#ifndef WOWVIEWERLIB_M2OBJECT_H
#define WOWVIEWERLIB_M2OBJECT_H

class M2Object;
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


class M2Object {
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
    bool m_loading = false;
    bool m_loaded = false;
    bool m_geomLoaded = false;
    bool m_hasAABB = false;

    bool m_alwaysDraw = false;



    void load(std::string modelName, SMODoodadDef &doodadDef, mathfu::mat4 &wmoPlacementMat){
        createPlacementMatrix(doodadDef, wmoPlacementMat);
        calcWorldPosition();

        this->setLoadParams(0, {}, {});
        this->setModelFileName(modelName);
    }

    struct dynamicVaoMeshFrame {
        int batchIndex = -1;
        HGVertexBufferDynamic m_bufferVBO = nullptr;
        HGVertexBufferBindings m_bindings = nullptr;
        HGParticleMesh m_mesh = nullptr;
    };

private:
    mathfu::mat4 m_placementMatrix = mathfu::mat4::Identity();
    mathfu::mat4 m_placementInvertMatrix;
    mathfu::vec3 m_worldPosition;
    mathfu::vec3 m_localPosition;
    mathfu::vec3 m_localUpVector;
    mathfu::vec3 m_localRightVector;

    float m_currentDistance = 0;

    CAaBox aabb;
    CAaBox colissionAabb;

    HApiContainer m_api = nullptr;

    HM2Geom m_m2Geom = nullptr;
    HSkinGeom m_skinGeom = nullptr;
    HSkelGeom m_skelGeom = nullptr;
    HSkelGeom m_parentSkelGeom = nullptr;
    std::shared_ptr<CBoneMasterData> m_boneMasterData = nullptr;

    HGVertexBufferBindings bufferBindings = nullptr;
    HGUniformBufferChunk vertexModelWideUniformBuffer = nullptr;
    HGUniformBufferChunk fragmentModelWideUniformBuffer = nullptr;

    HGMesh boundingBoxMesh = nullptr;
    std::array<HGOcclusionQuery,4> occlusionQueries = {nullptr};

    mathfu::vec4 m_ambientColorOverride;
    bool m_setAmbientColor = false;

    float m_alpha = 1.0f;

    bool animationOverrideActive = false;
    float animationOverridePercent = 0;

    std::string m_modelName;
    std::string m_nameTemplate = "";

    bool useFileId = false;
    int m_modelFileId = 0;
    int m_skinFileId;

    std::vector<std::function<void()>> m_postLoadEvents;

    int m_skinNum = 0;
    CImVector m_localDiffuseColor = {0, 0, 0, 0};
    mathfu::vec4 m_ambientAddColor = mathfu::vec4(0, 0, 0, 0);
    mathfu::vec4 m_sunAddColor = mathfu::vec4(0, 0, 0, 0);
    mathfu::vec4 m_localDiffuseColorV = mathfu::vec4(0.0, 0.0, 0.0, 0.0);
    int m_useLocalDiffuseColor = -1;
    bool hasModf0x2Flag = false;
    std::vector<uint8_t> m_meshIds;
    std::vector<HBlpTexture> m_replaceTextures;
    bool particleColorReplacementIsSet = false;
    std::array<std::array<mathfu::vec4, 3>, 3> m_particleColorReplacement;


    std::vector<mathfu::mat4> bonesMatrices;
    std::vector<mathfu::mat4> textAnimMatrices;
    std::vector<mathfu::vec4> subMeshColors;
    std::vector<float> transparencies;
    std::vector<M2LightResult> lights;
    std::vector<ParticleEmitter*> particleEmitters;
    std::vector<CRibbonEmitter*> ribbonEmitters;

    std::unordered_map<int, HBlpTexture> loadedTextures;

    std::vector<HGM2Mesh> m_meshNaturalArray;
    std::vector<HGM2Mesh> m_meshForcedTranspArray;

    //TODO: think about if it's viable to do forced transp for dyn meshes
    std::vector<std::array<dynamicVaoMeshFrame, 4>> dynamicMeshes;
    std::vector<M2MaterialInst> m_materialArray;
    AnimationManager *m_animationManager;

    bool m_interiorAmbientWasSet = false; // For static only
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
    void initParticleEmitters();
    void initRibbonEmitters();

    void sortMaterials(mathfu::Matrix<float, 4, 4> &modelViewMat);
    bool checkifBonesAreInRange(M2SkinProfile *skinProfile, M2SkinSection *mesh);


    void createMeshes();
    void createBoundingBoxMesh();

    static mathfu::vec4 getCombinedColor(M2SkinProfile *skinData, int batchIndex,  const std::vector<mathfu::vec4> &subMeshColors) ;
    static float getTextureWeight(M2SkinProfile *skinData, M2Data *m2data, int batchIndex, int textureIndex, const std::vector<float> &transparencies) ;
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

    bool getInteriorAmbientWasSet() {
        return m_interiorAmbientWasSet;
    }
    void setInteriorAmbientWasSet(bool value) {
        m_interiorAmbientWasSet = value;
    }

    void addPostLoadEvent(const std::function<void()> &value) {
        m_postLoadEvents.push_back(value);
    }

    const CAaBox &getAABB() { return aabb; };
    CAaBox getColissionAABB();;

    void setLoadParams(int skinNum, std::vector<uint8_t> meshIds,
                       std::vector<HBlpTexture> replaceTextures);

    void setReplaceTextures(std::vector<HBlpTexture> &replaceTextures);
    void setMeshIds(std::vector<uint8_t> &meshIds);
    void setReplaceParticleColors(std::array<std::array<mathfu::vec4, 3>, 3> &particleColorReplacement);
    void resetReplaceParticleColor();
    bool getReplaceParticleColors(std::array<std::array<mathfu::vec4, 3>, 3> &particleColorReplacement);
    void setModelFileName(std::string modelName);
    int getModelFileId();
    void setModelFileId(int fileId);

    void setAlpha(float alpha) {
        m_alpha = alpha;
    }
    void setOverrideAnimationPerc(float percent, bool active) {
        animationOverrideActive = active;
        animationOverridePercent = percent;
    }

    void setAnimationId(int animationId);
    int getCurrentAnimationIndex();
    void resetCurrentAnimation();
    void createPlacementMatrix(SMODoodadDef &def, mathfu::mat4 &wmoPlacementMat);
    void createPlacementMatrix(SMDoodadDef &def);
    void createPlacementMatrix(mathfu::vec3 pos, float f, mathfu::vec3 scaleVec,
                               mathfu::Matrix<float, 4, 4> *rotationMatrix);

    void updatePlacementMatrixFromParentAttachment(M2Object *parent, int attachment, float scale);

    void calcWorldPosition(){
        m_worldPosition = (m_placementMatrix * mathfu::vec4(0,0,0,1)).xyz();
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
    bool getGetIsLoaded() { return m_loaded; };
    mathfu::mat4 getModelMatrix() { return m_placementMatrix; };

    bool prepearMaterial(M2MaterialInst &materialData, int materialIndex);
    void collectMeshes(std::vector<HGMesh> &opaqueMeshes, std::vector<HGMesh> &transparentMeshes, int renderOrder);

    bool setUseLocalLighting(bool value) {
//        if (hasModf0x2Flag) {
//            m_useLocalDiffuseColor = 0;
//        }
        if (m_useLocalDiffuseColor == -1) {
            m_useLocalDiffuseColor = value ? 1 : 0;
        }
        m_useLocalDiffuseColor = ( m_useLocalDiffuseColor == 0 ? m_useLocalDiffuseColor : (value ? 1 : 0) );

        return m_useLocalDiffuseColor == 1;
    };
    bool getUseLocalLighting() { return m_useLocalDiffuseColor == 1; };
    const bool checkFrustumCulling(const mathfu::vec4 &cameraPos,
                                   const MathHelper::FrustumCullingData &frustumData);

    bool isMainDataLoaded();
    bool getHasBoundingBox() {return m_hasAABB;}

    void doLoadMainFile();
    void doLoadGeom();
    void update(double deltaTime, mathfu::vec3 &cameraPos, mathfu::mat4 &viewMat);
    void uploadGeneratorBuffers(mathfu::mat4 &viewMat);
    M2CameraResult updateCamera(double deltaTime, int cameraViewId);
    void drawDebugLight();


    void drawBBInternal(CAaBox &bb, mathfu::vec3 &color, mathfu::Matrix<float, 4, 4> &placementMatrix);

    void drawBB(mathfu::vec3 &color);

    void setDiffuseColor(CImVector& value);

    HBlpTexture getBlpTextureData(int textureInd);
    HGTexture getTexture(int textureInd);
    HBlpTexture getHardCodedTexture(int textureInd);

    mathfu::vec4 getM2SceneAmbientLight();
    void setAmbientColorOverride(mathfu::vec4 &ambientColor, bool override) {
        m_setAmbientColor = override;
        m_ambientColorOverride = ambientColor;
    }

    void drawParticles(std::vector<HGMesh> &opaqueMeshes, std::vector<HGMesh> &transparentMeshes,  int renderOrder);

    void createVertexBindings();

    int getCameraNum() {
        if (!getGetIsLoaded()) return 0;

        return m_m2Geom->m_m2Data->cameras.size;
    }

    HGM2Mesh createSingleMesh(const M2Data *m_m2Data, int i, int indexStartCorrection, HGVertexBufferBindings finalBufferBindings, const M2Batch *m2Batch,
                              const M2SkinSection *skinSection, M2MaterialInst &material, EGxBlendEnum &blendMode, bool overrideBlend);

    HGM2Mesh createWaterfallMesh();
    void updateDynamicMeshes();
};

#include "../../algorithms/mathHelper.h"

template<>
inline const CAaBox &retrieveAABB<>(const std::shared_ptr<M2Object> &object) {
    return object->getAABB();
}


struct M2ObjectHasher
{
    size_t operator()(const std::shared_ptr<M2Object>& val)const
    {
        return std::hash<std::shared_ptr<M2Object>>()(val);
    }
};

//typedef oneapi::tbb::concurrent_unordered_set<std::shared_ptr<M2Object>, M2ObjectHasher> M2ObjectListContainer;
//typedef std::unordered_set<std::shared_ptr<M2Object>> M2ObjectListContainer;

class M2ObjectListContainer {
private:
    std::vector<std::shared_ptr<M2Object>> candidates;
    std::vector<std::shared_ptr<M2Object>> drawn;
    std::vector<std::shared_ptr<M2Object>> toLoadMain;
    std::vector<std::shared_ptr<M2Object>> toLoadGeom;

    bool candCanHaveDuplicates = false;
    bool drawnCanHaveDuplicates = false;
    bool toLoadMainCanHaveDuplicates = false;
    bool toLoadGeomCanHaveDuplicates = false;

    void inline removeDuplicates(std::vector<std::shared_ptr<M2Object>> &array) {
        if (array.size() < 1000) {
            std::sort(array.begin(), array.end());
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
    void addCandidate(const std::shared_ptr<M2Object> &cand) {
        if (cand == nullptr) return;
        if (cand->getHasBoundingBox()) {
            candidates.push_back(cand);
            candCanHaveDuplicates = true;
        } else {
            toLoadMain.push_back(cand);
            toLoadMainCanHaveDuplicates = true;
        }
    }

    void addToDraw(const std::shared_ptr<M2Object> &toDraw) {
        if (toDraw->getGetIsLoaded()) {
            drawn.push_back(toDraw);
            drawnCanHaveDuplicates = true;
        } else if (!toDraw->isMainDataLoaded()) {
            toLoadMain.push_back(toDraw);
            toLoadMainCanHaveDuplicates = true;
        } else {
            toLoadGeom.push_back(toDraw);
            toLoadGeomCanHaveDuplicates = true;
        }
    }

    const std::vector<std::shared_ptr<M2Object>> &getCandidates() {
        if (this->candCanHaveDuplicates) {
            removeDuplicates(candidates);
            candCanHaveDuplicates = false;
        }

        return candidates;
    }

    const std::vector<std::shared_ptr<M2Object>> &getDrawn() {
        if (this->drawnCanHaveDuplicates) {
            removeDuplicates(drawn);
            drawnCanHaveDuplicates = false;
        }

        return drawn;
    }

    const std::vector<std::shared_ptr<M2Object>> &getToLoadMain() {
        if (this->toLoadMainCanHaveDuplicates) {
            removeDuplicates(toLoadMain);
            toLoadMainCanHaveDuplicates = false;
        }

        return toLoadMain;
    }

    const std::vector<std::shared_ptr<M2Object>> &getToLoadGeom() {
        if (this->toLoadGeomCanHaveDuplicates) {
            removeDuplicates(toLoadGeom);
            toLoadGeomCanHaveDuplicates = false;
        }

        return toLoadGeom;
    }

    void addDrawnAndToLoad(M2ObjectListContainer &anotherList) {
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
};


#endif //WOWVIEWERLIB_M2OBJECT_H
