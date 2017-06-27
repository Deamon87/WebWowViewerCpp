//
// Created by deamon on 22.06.17.
//

#ifndef WOWVIEWERLIB_M2OBJECT_H
#define WOWVIEWERLIB_M2OBJECT_H

#include <cstdint>
#include <mathfu/glsl_mappings.h>
#include "../geometry/m2Geom.h"
#include "../geometry/skinGeom.h"
#include "../wowScene.h"
#include "m2Helpers/M2MaterialInst.h"
#include "../managers/animationManager.h"

class M2Object {
public:
    M2Object(IWoWInnerApi *api) : m_api(api), m_m2Geom(nullptr), m_skinGeom(nullptr) {}

private:
    void createAABB();
    bool m_loading = false;
    bool m_loaded = false;
private:
    mathfu::mat4 m_placementMatrix = mathfu::mat4::Identity();

    CAaBox aabb;

    IWoWInnerApi *m_api;

    M2Geom *m_m2Geom = nullptr;
    SkinGeom *m_skinGeom = nullptr;

    std::string m_modelName;
    std::string m_skinName;
    int m_skinNum;
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

public:

    void setLoadParams(std::string modelName, int skinNum, std::vector<uint8_t> meshIds,
                       std::vector<std::string> replaceTextures);


    void makeTextureArray();

    void update(int deltaTime, mathfu::vec3 cameraPos, mathfu::mat4 viewMat);
    void draw(bool drawTransparent, mathfu::mat4 placementMatrix, mathfu::vec4 diffuseColor);
};


#endif //WOWVIEWERLIB_M2OBJECT_H
