//
// Created by deamon on 10.07.17.
//

#ifndef WEBWOWVIEWERCPP_ADTOBJECT_H
#define WEBWOWVIEWERCPP_ADTOBJECT_H

class IWoWInnerApi;
class AdtObject;
class M2Object;

#include <vector>
#include <set>

#include "../../persistance/header/adtFileHeader.h"
#include "../../opengl/header.h"
#include "../../wowInnerApi.h"

#include "../../persistance/adtFile.h"
#include "../../persistance/wdtFile.h"
#include "../m2/m2Object.h"
#include "../wmo/wmoObject.h"
#include "../iMapApi.h"

class AdtObject {
public:
    AdtObject(IWoWInnerApi *api, std::string &adtFileTemplate, std::string mapname, int adt_x, int adt_y, HWdtFile wdtfile);
    void setMapApi(IMapApi *api) {
        m_mapApi = api;
    }


    void collectMeshes(std::vector<HGMesh> &renderedThisFrame, int renderOrder);
    void collectMeshesLod(std::vector<HGMesh> &renderedThisFrame);
    void update();
    void doPostLoad();

    bool checkFrustumCulling(
            mathfu::vec4 &cameraPos,
            int adt_glob_x,
            int adt_glob_y,
            std::vector<mathfu::vec4> &frustumPlanes,
            std::vector<mathfu::vec3> &frustumPoints,
            std::vector<mathfu::vec3> &hullLines,
            mathfu::mat4 &lookAtMat4,
            std::vector<M2Object*> &m2ObjectsCandidates,
            std::vector<WmoObject*> &wmoCandidates);

    bool
    checkReferences(mathfu::vec4 &cameraPos, std::vector<mathfu::vec4> &frustumPlanes, std::vector<mathfu::vec3> &frustumPoints,
                    mathfu::mat4 &lookAtMat4,
                    int lodLevel,
                    std::vector<M2Object *> &m2ObjectsCandidates, std::vector<WmoObject *> &wmoCandidates,
                    int x, int y, int x_len, int y_len);

private:
    struct LodCommand {
        int index;
        int length;
    };

    void loadingFinished();
    void createVBO();
    void createMeshes();
    void loadAlphaTextures();

    IWoWInnerApi *m_api;
    IMapApi *m_mapApi;
    HWdtFile m_wdtFile;

    HAdtFile m_adtFile;
    HAdtFile m_adtFileTex;
    HAdtFile m_adtFileObj;
    HAdtFile m_adtFileObjLod;
    HAdtFile m_adtFileLod;

    int alphaTexturesLoaded = 0;
    bool m_loaded = false;

    uint32_t indexOffset = 0;
	uint32_t heightOffset = 0;
	uint32_t normalOffset = 0;
	uint32_t colorOffset = 0;
	uint32_t lightingOffset = 0;

    int mostDetailedLod = 0; // 0 = most detailed LOD, 5 = least detailed lod
    int leastDetiledLod = 0;

    std::unordered_map<int, HGTexture> m_requestedTextures;
    std::unordered_map<int, HGTexture> m_requestedTexturesHeight;
    std::unordered_map<int, HGTexture> m_requestedTexturesSpec;

    std::vector<LodCommand> lodCommands;

    HGVertexBuffer combinedVbo ;
    HGIndexBuffer stripVBO ;
    HGVertexBufferBindings adtVertexBindings;

    HGVertexBuffer heightVboLod;
    HGIndexBuffer stripVBOLod;

    HGVertexBufferBindings lodVertexBindings;

    HGUniformBuffer adtWideBlockPS;



private:
    std::vector<HGTexture> alphaTextures;
    HBlpTexture lodDiffuseTexture  = nullptr;
    HBlpTexture lodNormalTexture  = nullptr;

    std::vector<HGMesh> adtMeshes;
    std::vector<HGMesh> adtLodMeshes;

    std::vector<CAaBox> tileAabb;
    std::vector<int> globIndexX;
    std::vector<int> globIndexY;

    int adt_x;
    int adt_y;


    std::string m_adtFileTemplate;

    bool drawChunk[256] = {};
    bool checkRefs[256] = {};

    struct {
        std::vector<M2Object *> m2Objects;
        std::vector<WmoObject *> wmoObjects;
    } objectLods[2];

    HGTexture getAdtTexture(int textureId);
    HGTexture getAdtHeightTexture(int textureId);
    HGTexture getAdtSpecularTexture(int textureId);

    void calcBoundingBoxes();
    void loadM2s();
    void loadWmos();

    bool checkNonLodChunkCulling(mathfu::vec4 &cameraPos, std::vector<mathfu::vec4> &frustumPlanes,
                                 std::vector<mathfu::vec3> &frustumPoints, std::vector<mathfu::vec3> &hullLines, int x,
                                 int y, int x_len, int y_len);

    bool
    iterateQuadTree(mathfu::vec4 &camera, const mathfu::vec3 &pos, float x_offset, float y_offset, float cell_len,
                    int curentLod, int lastFoundLod,
                    const PointerChecker<MLND> &quadTree, int quadTreeInd, std::vector<mathfu::vec4> &frustumPlanes,
                    std::vector<mathfu::vec3> &frustumPoints, std::vector<mathfu::vec3> &hullLines,
                    mathfu::mat4 &lookAtMat4,
                    std::vector<M2Object *> &m2ObjectsCandidates,
                    std::vector<WmoObject *> &wmoCandidates);
};


#endif //WEBWOWVIEWERCPP_ADTOBJECT_H
