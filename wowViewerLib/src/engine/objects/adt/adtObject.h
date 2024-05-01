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
#include <tbb/concurrent_unordered_set.h>

#include "../../persistance/header/adtFileHeader.h"

#include "../../persistance/adtFile.h"
#include "../../persistance/wdtFile.h"
#include "../m2/m2Object.h"
#include "../wmo/wmoObject.h"
#include "../iMapApi.h"
#include "../ViewsObjects.h"
#include "../liquid/LiquidInstance.h"

typedef std::function<bool(bool doCheck, bool doUpdate, animTime_t currentTime)> FreeStrategy;

class AdtObject;
enum class AdtObjectId : int;
extern EntityFactory<50, AdtObjectId, AdtObject> adtObjectFactory;
class AdtObject : public ObjectWithId<AdtObjectId> {
public:
    AdtObject(HApiContainer api, std::string &adtFileTemplate, std::string mapname, int adt_x, int adt_y, bool useWeightedBlend, HWdtFile wdtfile);
    AdtObject(HApiContainer api, int adt_x, int adt_y, WdtFile::MapFileDataIDs &fileDataIDs, bool useWeightedBlend, HWdtFile wdtfile);
    ~AdtObject() {
//        std::cout << "~AdtObject called" << std::endl;
    };

    void setFreeStrategy(FreeStrategy &freeStrat) {
        m_freeStrategy = freeStrat;
        m_freeStrategy(false, true, m_mapApi->getCurrentSceneTime());
    }

    void setMapApi(IMapApi *api) {
        m_mapApi = api;
    }

    FileStatus getLoadedStatus();

    void collectMeshes(ADTObjRenderRes &adtRes, std::vector<HGMesh> &opaqueMeshes, framebased::vector<HGSortableMesh> &transparentMeshes, int renderOrder);
    void collectMeshesLod(std::vector<HGMesh> &renderedThisFrame);

    void update(animTime_t deltaTime);
    void uploadGeneratorBuffers(const HFrameDependantData &frameDependantData);
    bool doPostLoad(const HMapSceneBufferCreate &sceneRenderer);

    int getAreaId(int mcnk_x, int mcnk_y);

    int getAdtX() {return adt_x;}
    int getAdtY() {return adt_y;}

    CAaBox calcAABB();
    bool getWaterColorFromDB(mathfu::vec4 cameraPos, mathfu::vec3 &closeRiverColor);

    bool checkFrustumCulling(
        ADTObjRenderRes &adtFrustRes,
        const mathfu::vec4 &cameraPos,
        const MathHelper::FrustumCullingData &frustumData,

        M2ObjectListContainer&m2ObjectsCandidates,
        WMOListContainer &wmoCandidates);

    bool
    checkReferences(ADTObjRenderRes &adtFrustRes,
                    const mathfu::vec4 &cameraPos,
                    const MathHelper::FrustumCullingData &frustumData,
                    int lodLevel,
                    M2ObjectListContainer &m2ObjectsCandidates,
                    WMOListContainer &wmoCandidates,
                    int x, int y, int x_len, int y_len);

    FreeStrategy &getFreeStrategy() {
        return m_freeStrategy;
    }
private:
    FreeStrategy m_freeStrategy = [](bool doCheck, bool doUpdate, animTime_t currentTime) -> bool {return false;};

    animTime_t m_lastTimeOfUpdate = 0;
    animTime_t m_lastDeltaTime = 0;

    struct LodCommand {
        int index;
        int length;
    };

    void loadingFinished(const HMapSceneBufferCreate &sceneRenderer);

    void createVBO(const HMapSceneBufferCreate &sceneRenderer);
    void createIBOAndBinding(const HMapSceneBufferCreate &sceneRenderer);

    void createMeshes(const HMapSceneBufferCreate &sceneRenderer);
    void loadAlphaTextures();

    HApiContainer m_api;
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

    robin_hood::unordered_flat_map<int, HGSamplableTexture> m_requestedTextures;
    robin_hood::unordered_flat_map<int, HGSamplableTexture> m_requestedTexturesHeight;
    robin_hood::unordered_flat_map<int, HGSamplableTexture> m_requestedTexturesSpec;

    std::vector<LodCommand> lodCommands;

    HGVertexBuffer combinedVbo ;
    HGIndexBuffer stripIBO ;
    HGVertexBufferBindings adtVertexBindings;
    bool m_holesIgnored = false;

    HGVertexBuffer heightVboLod;
    HGIndexBuffer stripVBOLod;

    HGVertexBufferBindings lodVertexBindings;

private:
    HGSamplableTexture alphaTexture;
    HBlpTexture lodDiffuseTexture  = nullptr;
    HBlpTexture lodNormalTexture  = nullptr;


    std::array<HGMesh, 16*16> adtMeshes = {};
    std::array<std::shared_ptr<IADTMaterial>, 16*16> adtMaterials = {};
    //16x16, then layer
    std::array<std::vector<std::shared_ptr<LiquidInstance>>, 16*16> m_liquidInstancesPerChunk = {};
    std::vector<std::shared_ptr<LiquidInstance>> m_liquidInstances = {};
    std::vector<HGMesh> adtLodMeshes;

    std::vector<CAaBox> tileAabb;
    std::vector<CAaBox> waterTileAabb;
    std::vector<int> globIndexX;
    std::vector<int> globIndexY;

    std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> m_waterPlacementChunk = nullptr;
    std::vector<std::shared_ptr<IWaterMaterial>> m_waterMaterialArray = {};

    int adt_x;
    int adt_y;

    bool m_useWeightedBlend;

    std::string m_adtFileTemplate;


    struct lodLevels {
        std::vector<std::shared_ptr<M2Object>> m2Objects;
        std::vector<std::shared_ptr<WmoObject>> wmoObjects;

        std::vector<M2ObjId> m2ObjectIds;
        std::vector<WMOObjId> wmoObjectIds;
    };
    std::array<lodLevels, 2> objectLods;

    HGSamplableTexture getAdtTexture(int textureId);
    HGSamplableTexture getAdtHeightTexture(int textureId);
    HGSamplableTexture getAdtSpecularTexture(int textureId);

    void calcBoundingBoxes();
    void loadM2s();
    void loadWmos();
    void loadWater(const HMapSceneBufferCreate &sceneRenderer);

    bool checkNonLodChunkCulling(ADTObjRenderRes &adtFrustRes,
                                 const mathfu::vec4 &cameraPos,
                                 const MathHelper::FrustumCullingData &frustumData,
                                 int x, int y, int x_len, int y_len);

    bool
    iterateQuadTree(ADTObjRenderRes &adtFrustRes, const mathfu::vec4 &camera, const mathfu::vec3 &pos, float x_offset, float y_offset, float cell_len,
                    int currentLod, int lastFoundLod,
                    const PointerChecker<MLND> &quadTree,
                    int quadTreeInd,
                    const MathHelper::FrustumCullingData &frustumData,
                    M2ObjectListContainer &m2ObjectsCandidates,
                    WMOListContainer &wmoCandidates);

    void fillTextureForMCNK(HGDevice &device, int i, bool noLayers, ADTMaterialTemplate &adtMaterialTemplate);


    void
    checkWmoM2ByRef(int lodLevel, M2ObjectListContainer &m2ObjectsCandidates, WMOListContainer &wmoCandidates, int x,
                    int y,
                    int x_len, int y_len);

    void
    checkWmoGlobally(int lodLevel, M2ObjectListContainer &m2ObjectsCandidates, WMOListContainer &wmoCandidates, int x,
                    int y,
                    int x_len, int y_len);
};


#endif //WEBWOWVIEWERCPP_ADTOBJECT_H
