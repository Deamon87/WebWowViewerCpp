//
// Created by Deamon on 2/20/2019.
//

#ifndef AWEBWOWVIEWERCPP_CRIBBONEMITTER_H
#define AWEBWOWVIEWERCPP_CRIBBONEMITTER_H
class CRibbonEmitter;

#include <cstdint>
#include <vector>
#include "../persistance/header/commonFileStructs.h"
#include "../ApiContainer.h"
#include "../objects/m2/m2Object.h"

struct CRibbonVertex
{
    C3Vector pos;
    CImVector diffuseColor;
    C2Vector texCoord;
};


class CRibbonEmitter {
    int textureTransformLookup = -1;
    int m_refCount;
    std::vector<float> m_edges;
    int m_writePos;
    int m_readPos;
    float m_startTime;
    mathfu::vec3 m_prevPos;
    mathfu::vec3 m_pos;
    std::vector<CRibbonVertex> m_gxVertices;
    std::vector<uint16_t> m_gxIndices;
    float m_ooLifeSpan;
    float m_tmpDU;
    float m_tmpDV;
    float m_ooTmpDU;
    float m_ooTmpDV;
    CRect m_texSlotBox;
    mathfu::vec3 m_prevVertical;
    mathfu::vec3 m_currVertical;
    mathfu::vec3 m_prevDir;
    mathfu::vec3 m_currDir;
    mathfu::vec3 m_prevDirScaled;
    mathfu::vec3 m_currDirScaled;
    mathfu::vec3 m_below0;
    mathfu::vec3 m_below1;
    mathfu::vec3 m_above0;
    mathfu::vec3 m_above1;
    mathfu::vec3 m_minWorldBounds;
    mathfu::vec3 m_maxWorldBound;
    float m_edgesPerSec;
    float m_edgeLifeSpan;
    //TSGrowableArray m_materials;
    //TSGrowableArray_CTexture m_textures;
    std::vector<uint32_t> m_replaces;
    CImVector m_diffuseClr;
    CRect m_texBox;
    int m_rows;
    int m_cols;
    struct {
        uint8_t m_posSet : 1;
        uint8_t m_initialized : 1;
        uint8_t m_dataEnabled : 1;
        uint8_t m_userEnabled : 1;
        uint8_t m_singletonUpdated : 1;
    } m_ribbonEmitterflags;
    mathfu::vec3 m_currPos;
    int m_texSlot;
    float m_above;
    float m_below;
    float m_gravity;
    int m_priority;

private:
    HApiContainer m_api;

    struct RibbonFrame {
        HGIndexBuffer m_indexVBO;
        HGVertexBuffer m_bufferVBO;

        HGVertexBufferBindings m_bindings;
        std::vector<HGParticleMesh> m_meshes = {};
        bool isDead = true;
    };

    std::array<RibbonFrame, 4> frame;

    void createMesh(M2Object *object, std::vector<M2Material> &materials, std::vector<int> &textureIndicies);

public:
    CRibbonEmitter(HApiContainer m_api, M2Object *object, std::vector<M2Material> &materials, std::vector<int> &textureIndicies, int textureTransformLookup);
    void SetDataEnabled(char a2);
    void SetUserEnabled(char a2);
    CRibbonEmitter *SetGravity(float a2);
    bool IsDead() const;
    void SetBelow(float a2);
    void SetAbove(float a2);
    void SetAlpha(float a2);
    void SetPriority(int a2) { m_priority = a2;};
    void InitInterpDeltas();
    int GetNumBatches();
    void SetPos(const mathfu::mat4 &modelViewMat, const mathfu::vec3 &a3, const mathfu::mat4 *frameOfReference);
    unsigned int GetNumPrimitives();
    void ChangeFrameOfReference(const mathfu::mat4 *frameOfReference);
    void SetColor(float a3, float a4, float a5);
    void SetTexSlot(unsigned int texSlot);
    void InterpEdge(float age, float t, unsigned int advance); // idb
    void Update(float deltaTime, int suppressNewEdges);

    void Advance(int &pos, unsigned int amount);
    void DecRef();
    void Initialize(float edgesPerSec, float edgeLifeSpanInSec, CImVector diffuseColor, CRect *a8, unsigned int rows, unsigned int cols); // idb
//    signed int Render(const mathfu::mat4 *a2);
    //CTexture **SetTexture(unsigned int a2, CTexture *a3);
    //int ReplaceTexture(unsigned int a2, CTexture *a3);

    void collectMeshes(std::vector<HGMesh> &opaqueMeshes, std::vector<HGMesh> &transparentMeshes, int renderOrder);

    void updateBuffers();
};


#endif //AWEBWOWVIEWERCPP_CRIBBONEMITTER_H
