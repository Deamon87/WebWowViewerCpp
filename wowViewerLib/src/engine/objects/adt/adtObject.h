//
// Created by deamon on 10.07.17.
//

#ifndef WEBWOWVIEWERCPP_ADTOBJECT_H
#define WEBWOWVIEWERCPP_ADTOBJECT_H

class AdtObject;
class M2Object;

#include <array>
#include <vector>
#include <set>

#include "../../persistance/header/adtFileHeader.h"

#include "../../persistance/adtFile.h"
#include "../../persistance/wdtFile.h"
#include "../m2/m2Object.h"
#include "../wmo/wmoObject.h"
#include "../iMapApi.h"
#include "../ViewsObjects.h"


class AdtObject {
public:
    AdtObject(ApiContainer *api, std::string &adtFileTemplate, std::string mapname, int adt_x, int adt_y, HWdtFile wdtfile);
    AdtObject(ApiContainer *api, int adt_x, int adt_y, WdtFile::MapFileDataIDs &fileDataIDs, HWdtFile wdtfile);
    ~AdtObject() = default;

    void setMapApi(IMapApi *api) {
        m_mapApi = api;
        m_lastTimeOfUpdateOrRefCheck = m_mapApi->getCurrentSceneTime();
    }

    void collectMeshes(ADTObjRenderRes &adtRes, std::vector<HGMesh> &opaqueMeshes, std::vector<HGMesh> &transparentMeshes, int renderOrder);
    void collectMeshesLod(std::vector<HGMesh> &renderedThisFrame);

    void update(animTime_t deltaTime);
    void uploadGeneratorBuffers(ADTObjRenderRes &adtRes);
    void doPostLoad();

    int getAreaId(int mcnk_x, int mcnk_y);

    bool checkFrustumCulling(
            ADTObjRenderRes &adtFrustRes,
            mathfu::vec4 &cameraPos,
            int adt_glob_x,
            int adt_glob_y,
            std::vector<mathfu::vec4> &frustumPlanes,
            std::vector<mathfu::vec3> &frustumPoints,
            std::vector<mathfu::vec3> &hullLines,
            mathfu::mat4 &lookAtMat4,
            std::vector<std::shared_ptr<M2Object>> &m2ObjectsCandidates,
            std::vector<std::shared_ptr<WmoObject>> &wmoCandidates);

    bool
    checkReferences(ADTObjRenderRes &adtFrustRes, mathfu::vec4 &cameraPos, std::vector<mathfu::vec4> &frustumPlanes, std::vector<mathfu::vec3> &frustumPoints,
                    mathfu::mat4 &lookAtMat4,
                    int lodLevel,
                    std::vector<std::shared_ptr<M2Object>> &m2ObjectsCandidates, std::vector<std::shared_ptr<WmoObject>> &wmoCandidates,
                    int x, int y, int x_len, int y_len);

    animTime_t getLastTimeOfUpdate() {
        return m_lastTimeOfUpdateOrRefCheck;
    }
private:
    animTime_t m_lastTimeOfUpdateOrRefCheck = 0;
    animTime_t m_lastTimeOfUpdate = 0;
    animTime_t m_lastDeltaTime = 0;

    struct LodCommand {
        int index;
        int length;
    };

    void loadingFinished();
    void createVBO();
    void createMeshes();
    void loadAlphaTextures();

    ApiContainer *m_api;
    IMapApi *m_mapApi;
    HWdtFile m_wdtFile= nullptr;

    HAdtFile m_adtFile = nullptr;
    HAdtFile m_adtFileTex = nullptr;
    HAdtFile m_adtFileObj = nullptr;
    HAdtFile m_adtFileObjLod = nullptr;
    HAdtFile m_adtFileLod = nullptr;

    int alphaTexturesLoaded = 0;
    bool m_loaded = false;

    int mostDetailedLod = 0; // 0 = most detailed LOD, 5 = least detailed lod
    int leastDetiledLod = 0;

    std::unordered_map<int, HGTexture> m_requestedTextures;
    std::unordered_map<int, HGTexture> m_requestedTexturesHeight;
    std::unordered_map<int, HGTexture> m_requestedTexturesSpec;

    std::vector<LodCommand> lodCommands;

    HGVertexBuffer combinedVbo ;
    HGIndexBuffer stripIBO ;
    HGVertexBufferBindings adtVertexBindings;

    HGVertexBuffer heightVboLod;
    HGIndexBuffer stripVBOLod;

    HGVertexBufferBindings lodVertexBindings;

    HGUniformBufferChunk adtWideBlockPS;



private:
    std::vector<HGTexture> alphaTextures;
    HBlpTexture lodDiffuseTexture  = nullptr;
    HBlpTexture lodNormalTexture  = nullptr;


    std::array<HGMesh, 16*16> adtMeshes = {};
    //16x16, then layer
    std::array<std::vector<HGMesh>, 16*16> waterMeshes = {};
    std::vector<HGMesh> adtLodMeshes;

    std::vector<CAaBox> tileAabb;
    std::vector<CAaBox> waterTileAabb;
    std::vector<int> globIndexX;
    std::vector<int> globIndexY;

    int adt_x;
    int adt_y;


    std::string m_adtFileTemplate;


    struct lodLevels {
        std::vector<std::shared_ptr<M2Object>> m2Objects;
        std::vector<std::shared_ptr<WmoObject>> wmoObjects;
    };
    std::array<lodLevels, 2> objectLods;

    HGTexture getAdtTexture(int textureId);
    HGTexture getAdtHeightTexture(int textureId);
    HGTexture getAdtSpecularTexture(int textureId);

    struct AnimTextures {
        std::array<mathfu::mat4, 4> animTexture;
    };
    struct AnimTrans {
        std::array<mathfu::vec2, 4> transVectors;
    };
    std::vector<AnimTextures> texturesPerMCNK;

    std::vector<AnimTrans> animationTranslationPerMCNK;

    void calcBoundingBoxes();
    void loadM2s();
    void loadWmos();
    void loadWater();
    HGMesh createWaterMeshFromInstance(int x_chunk, int y_chunk, SMLiquidInstance &liquidInstance, mathfu::vec3 liquidBasePos);


    bool checkNonLodChunkCulling(ADTObjRenderRes &adtFrustRes, mathfu::vec4 &cameraPos, std::vector<mathfu::vec4> &frustumPlanes,
                                 std::vector<mathfu::vec3> &frustumPoints, std::vector<mathfu::vec3> &hullLines, int x,
                                 int y, int x_len, int y_len);

    bool
    iterateQuadTree(ADTObjRenderRes &adtFrustRes, mathfu::vec4 &camera, const mathfu::vec3 &pos, float x_offset, float y_offset, float cell_len,
                    int curentLod, int lastFoundLod,
                    const PointerChecker<MLND> &quadTree, int quadTreeInd, std::vector<mathfu::vec4> &frustumPlanes,
                    std::vector<mathfu::vec3> &frustumPoints, std::vector<mathfu::vec3> &hullLines,
                    mathfu::mat4 &lookAtMat4,
                    std::vector<std::shared_ptr<M2Object>> &m2ObjectsCandidates,
                    std::vector<std::shared_ptr<WmoObject>> &wmoCandidates);
};


#endif //WEBWOWVIEWERCPP_ADTOBJECT_H
