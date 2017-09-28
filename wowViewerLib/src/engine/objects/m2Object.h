//
// Created by deamon on 22.06.17.
//

#ifndef WOWVIEWERLIB_M2OBJECT_H
#define WOWVIEWERLIB_M2OBJECT_H

#include <cstdint>
#include <mathfu/glsl_mappings.h>
#include "../wowInnerApi.h"
#include "../persistance/header/wmoFileHeader.h"
#include "../geometry/m2Geom.h"
#include "../geometry/skinGeom.h"
#include "m2Helpers/M2MaterialInst.h"
#include "../managers/animationManager.h"
#include "../../../3rdparty/mathfu/include/mathfu/matrix.h"
#include "../persistance/header/skinFileHeader.h"
#include "../../../3rdparty/mathfu/include/mathfu/internal/vector_4.h"

class M2Object {
public:
    M2Object(IWoWInnerApi *api) : m_api(api), m_m2Geom(nullptr), m_skinGeom(nullptr), m_animationManager(nullptr) {}

private:
    void createAABB();
    bool m_loading = false;
    bool m_loaded = false;

    void load(std::string modelName, SMODoodadDef &doodadDef, mathfu::mat4 &wmoPlacementMat){
        createPlacementMatrix(doodadDef, wmoPlacementMat);
        calcWorldPosition();

        this->setLoadParams(modelName, 0, {}, {});
    }

private:
    mathfu::mat4 m_placementMatrix = mathfu::mat4::Identity();
    mathfu::mat4 m_placementInvertMatrix;
    mathfu::vec3 m_worldPosition;

    float m_currentDistance = 0;

    CAaBox aabb;

    IWoWInnerApi *m_api;

    M2Geom *m_m2Geom = nullptr;
    SkinGeom *m_skinGeom = nullptr;

    std::string m_modelName;
    std::string m_skinName;
    int m_skinNum = 0;
    std::vector<uint8_t> m_meshIds;
    std::vector<std::string> m_replaceTextures;
    std::vector<mathfu::mat4> bonesMatrices;
    std::vector<mathfu::mat4> textAnimMatrices;
    std::vector<mathfu::vec4> subMeshColors;
    std::vector<float> transparencies;


    std::vector<M2MaterialInst> m_materialArray;
    AnimationManager *m_animationManager;

    void initAnimationManager();
    void initBoneAnimMatrices();

    void startLoading();

    void drawMeshes(bool drawTransparent, int instanceCount);

    void drawMaterial(M2MaterialInst &materialData, bool drawTransparent, int instanceCount);

    void initTextAnimMatrices();

    void initSubmeshColors();

    void initTransparencies();

    void sortMaterials(mathfu::Matrix<float, 4, 4> &lookAtMat4);

    mathfu::vec4 getCombinedColor(M2SkinProfile *skinData, M2MaterialInst &materialData,  std::vector<mathfu::vec4> subMeshColors);
    float getTransparency(M2SkinProfile *skinData,M2MaterialInst &materialData,std::vector<float> transparencies);
public:

    void setLoadParams(std::string modelName, int skinNum, std::vector<uint8_t> meshIds,
                       std::vector<std::string> replaceTextures);

    void createPlacementMatrix(SMODoodadDef &def, mathfu::mat4 &wmoPlacementMat);
    void createPlacementMatrix(SMDoodadDef &def);
    void createPlacementMatrix(mathfu::vec3 pos, float f, mathfu::vec3 scaleVec,
                               mathfu::Matrix<float, 4, 4> *rotationMatrix);

    void calcWorldPosition(){
        m_worldPosition = (m_placementMatrix * mathfu::vec4(0,0,0,1)).xyz();
    }
    void calcDistance(mathfu::vec3 cameraPos);
    float getCurrentDistance();

    void makeTextureArray();

    const bool checkFrustumCulling(const mathfu::vec4 &cameraPos,
                                   const std::vector<mathfu::vec4> &frustumPlanes,
                                   const std::vector<mathfu::vec3> &frustumPoints);

    void update(double deltaTime, mathfu::vec3 &cameraPos, mathfu::mat4 &viewMat);
    void draw(bool drawTransparent, mathfu::vec4 &diffuseColor);


    void drawBBInternal(CAaBox &bb, mathfu::vec3 &color, mathfu::Matrix<float, 4, 4> &placementMatrix);

    void drawBB(mathfu::vec3 &color);
};


#endif //WOWVIEWERLIB_M2OBJECT_H
