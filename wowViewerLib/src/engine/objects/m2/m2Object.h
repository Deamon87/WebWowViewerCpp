//
// Created by deamon on 22.06.17.
//

#ifndef WOWVIEWERLIB_M2OBJECT_H
#define WOWVIEWERLIB_M2OBJECT_H


#include <cstdint>
#include "mathfu/glsl_mappings.h"
#include "../../wowInnerApi.h"
#include "../../managers/particles/particleEmitter.h"
#include "../../persistance/header/wmoFileHeader.h"
#include "../../geometry/m2Geom.h"
#include "../../geometry/skinGeom.h"
#include "m2Helpers/M2MaterialInst.h"
#include "../../managers/animationManager.h"
#include "mathfu/matrix.h"
#include "../../persistance/header/skinFileHeader.h"

#include "mathfu/internal/vector_4.h"

class M2Object {
public:
    M2Object(IWoWInnerApi *api) : m_api(api), m_m2Geom(nullptr), m_skinGeom(nullptr), m_animationManager(nullptr) {}

    friend class M2InstancingObject;
    friend class M2MeshBufferUpdater;
private:
    void createAABB();
    bool m_loading = false;
    bool m_loaded = false;

    void load(std::string modelName, SMODoodadDef &doodadDef, mathfu::mat4 &wmoPlacementMat){
        createPlacementMatrix(doodadDef, wmoPlacementMat);
        calcWorldPosition();

        this->setLoadParams(0, {}, {});
        this->setModelFileName(modelName);
    }

private:
    mathfu::mat4 m_placementMatrix = mathfu::mat4::Identity();
    mathfu::mat4 m_placementInvertMatrix;
    mathfu::vec3 m_worldPosition;

    float m_currentDistance = 0;

    CAaBox aabb;

    IWoWInnerApi *m_api;

    HM2Geom m_m2Geom ;
    HSkinGeom m_skinGeom = nullptr;

    HGVertexBufferBindings bufferBindings = nullptr;
    HGUniformBuffer vertexModelWideUniformBuffer = nullptr;
    HGUniformBuffer fragmentModelWideUniformBuffer = nullptr;

    mathfu::vec4 m_ambientColorOverride;
    bool m_setAmbientColor = false;
    bool m_modelAsScene = false;


    bool m_hasBillboards = false;
    std::string m_modelName;
    std::string m_skinName;
    std::string m_modelIdent;

    bool useFileId = false;
    int m_modelFileId;
    int m_skinFileId;

    std::vector<std::function<void()>> m_postLoadEvents;

    int m_skinNum = 0;
    CImVector m_localDiffuseColor = {0, 0, 0, 0};
    mathfu::vec4 m_localDiffuseColorV = mathfu::vec4(0.0, 0.0, 0.0, 0.0);
    bool m_useLocalDiffuseColor = false;
    std::vector<uint8_t> m_meshIds;
    std::vector<HBlpTexture> m_replaceTextures;
    std::vector<mathfu::mat4> bonesMatrices;
    std::vector<mathfu::mat4> textAnimMatrices;
    std::vector<mathfu::vec4> subMeshColors;
    std::vector<float> transparencies;
    std::vector<M2LightResult> lights;
    std::vector<ParticleEmitter*> particleEmitters;

    std::unordered_map<int, HBlpTexture> loadedTextures;

    std::vector<HGM2Mesh> m_meshArray;
    std::vector<M2MaterialInst> m_materialArray;
    AnimationManager *m_animationManager;

    void debugDumpAnimationSequences();

    void initAnimationManager();
    void initBoneAnimMatrices();

    void startLoading();

    void initTextAnimMatrices();
    void initSubmeshColors();
    void initTransparencies();
    void initLights();
    void initParticleEmitters();

    void sortMaterials(mathfu::Matrix<float, 4, 4> &lookAtMat4);
    bool checkIfHasBillboarded();

    mathfu::vec4 getCombinedColor(M2SkinProfile *skinData, M2MaterialInst &materialData,  std::vector<mathfu::vec4> subMeshColors);
    float getTransparency(M2SkinProfile *skinData,M2MaterialInst &materialData, std::vector<float> &transparencies);
public:

    void addPostLoadEvent(std::function<void()> &value) {
        m_postLoadEvents.push_back(value);
    }

    CAaBox getAABB() { return aabb; };

    void setLoadParams(int skinNum, std::vector<uint8_t> meshIds,
                       std::vector<HBlpTexture> replaceTextures);

    void setModelFileName(std::string modelName);
    void setModelFileId(int fileId);
    void setModelAsScene(bool value) {
        m_modelAsScene = value;
    };

    void createPlacementMatrix(SMODoodadDef &def, mathfu::mat4 &wmoPlacementMat);
    void createPlacementMatrix(SMDoodadDef &def);
    void createPlacementMatrix(mathfu::vec3 pos, float f, mathfu::vec3 scaleVec,
                               mathfu::Matrix<float, 4, 4> *rotationMatrix);

    void calcWorldPosition(){
        m_worldPosition = (m_placementMatrix * mathfu::vec4(0,0,0,1)).xyz();
    }
    void calcDistance(mathfu::vec3 cameraPos);
    float getCurrentDistance();
    float getHeight();
    bool getGetIsLoaded() { return m_loaded; };
    bool getHasBillboarded() {
        return m_hasBillboards;
    }
    bool getIsInstancable();
    std::string getModelIdent() { return m_modelIdent; };

    bool prepearMatrial(M2MaterialInst &materialData, int materialIndex);
    void createMeshes();
    void collectMeshes(std::vector<HGMesh> &renderedThisFrame, int renderOrder);

    void setUseLocalLighting(bool value) { m_useLocalDiffuseColor = value; };
    const bool checkFrustumCulling(const mathfu::vec4 &cameraPos,
                                   const std::vector<mathfu::vec4> &frustumPlanes,
                                   const std::vector<mathfu::vec3> &frustumPoints);

    void doPostLoad();
    void update(double deltaTime, mathfu::vec3 &cameraPos, mathfu::mat4 &viewMat);
    M2CameraResult updateCamera(double deltaTime, int cameraViewId);
    void drawDebugLight();


    void drawBBInternal(CAaBox &bb, mathfu::vec3 &color, mathfu::Matrix<float, 4, 4> &placementMatrix);

    void drawBB(mathfu::vec3 &color);

    void setDiffuseColor(CImVector& value);

    HBlpTexture getBlpTextureData(int textureInd);
    HGTexture getTexture(int textureInd);
    HBlpTexture getHardCodedTexture(int textureInd);

    mathfu::vec4 getAmbientLight();
    void setAmbientColorOverride(mathfu::vec4 &ambientColor, bool override) {
        m_setAmbientColor = override;
        m_ambientColorOverride = ambientColor;
    }

    void drawParticles(std::vector<HGMesh> &meshes, int renderOrder);

    void createVertexBindings();
};


#endif //WOWVIEWERLIB_M2OBJECT_H
