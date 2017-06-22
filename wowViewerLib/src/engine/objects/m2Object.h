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

class M2MaterialInst {
public:
    bool isRendered= false;
    bool isTransparent= false;
    bool isEnviromentMapping= false;
    int meshIndex= -1;


    int texUnit1TexIndex = 0;
    int mdxTextureIndex1 = 0;
    bool xWrapTex1 = false;
    bool yWrapTex1 = false;
    std::string textureUnit1TexName = "";
    void *texUnit1Texture = nullptr;

    int texUnit2TexIndex = 0;
    int mdxTextureIndex2 = 0;
    bool xWrapTex2 = false;
    bool yWrapTex2 = false;
    std::string textureUnit2TexName = "";
    void *texUnit2Texture = nullptr;

    int texUnit3TexIndex = 0;
    int mdxTextureIndex3 = 0;
    bool xWrapTex3 = false;
    bool yWrapTex3 = false;
    std::string textureUnit3TexName = "";
    void *texUnit3Texture = nullptr;

    int layer = 0;
    int renderFlagIndex = -1;
    int flags = 0;
};

class M2Object {
    M2Object(IWoWInnerApi *api) : m_api(api) {}

private:
    void createAABB();
    bool m_loading = false;
    bool m_loaded = false;
private:
    mathfu::mat4 m_placementMatrix = mathfu::mat4::Identity();

    CAaBox aabb;

    IWoWInnerApi *m_api;

    M2Geom *m_m2Geom;
    SkinGeom *m_skinGeom;

    std::string m_modelName;
    std::string m_skinName;
    int m_skinNum;
    std::vector<uint8_t> m_meshIds;
    std::vector<std::string> m_replaceTextures;

    std::vector<M2MaterialInst> m_materialArray;


    void setLoadParams(std::string modelName, int skinNum, std::vector<uint8_t> meshIds,
                       std::vector<std::string> replaceTextures);

    void startLoading();
    void update(int deltaTime, mathfu::vec3 cameraPos, mathfu::mat4 viewMat);

    void draw(bool drawTransparent, mathfu::mat4 placementMatrix, mathfu::vec4 diffuseColor);
    void drawMeshes(bool drawTransparent, int instanceCount);

    void drawMaterial(M2MaterialInst &materialData, bool drawTransparent, int instanceCount);
};


#endif //WOWVIEWERLIB_M2OBJECT_H
