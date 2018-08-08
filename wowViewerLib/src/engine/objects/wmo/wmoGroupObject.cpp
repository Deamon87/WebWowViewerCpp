//
// Created by deamon on 10.07.17.
//

#include "wmoGroupObject.h"
#include "../../algorithms/mathHelper.h"
#include "../../shader/ShaderDefinitions.h"
#include "../../../gapi/UniformBufferStructures.h"
#include <algorithm>

/*
//0
MapObjDiffuse {
    VertexShader(MapObjDiffuse_T1);
    PixelShader(MapObjDiffuse);
}

//1
MapObjSpecular {
    VertexShader(MapObjSpecular_T1);
    PixelShader(MapObjSpecular);
}

//2
MapObjMetal {
    VertexShader(MapObjSpecular_T1);
    PixelShader(MapObjMetal);
}

//3
MapObjEnv {
    VertexShader(MapObjDiffuse_T1_Refl);
    PixelShader(MapObjEnv);
}

//4
MapObjOpaque {
    VertexShader(MapObjDiffuse_T1);
    PixelShader(MapObjOpaque);
}

//5
Effect(MapObjEnvMetal {
    VertexShader(MapObjDiffuse_T1_Refl);
    PixelShader(MapObjEnvMetal);
}

//6
Effect(MapObjComposite) //aka MapObjTwoLayerDiffuse
{
    VertexShader(MapObjDiffuse_Comp);
    PixelShader(MapObjComposite); //aka MapObjTwoLayerDiffuse
}

//7
Effect(MapObjTwoLayerEnvMetal)
{
    VertexShader(MapObjDiffuse_Comp_Refl);
    PixelShader(MapObjTwoLayerEnvMetal);
}

//8
Effect(TwoLayerTerrain)
{
    VertexShader(MapObjDiffuse_Comp_Terrain);
    PixelShader(MapObjTwoLayerTerrain);
}

//9
Effect(MapObjDiffuseEmissive)
{
    VertexShader(MapObjDiffuse_Comp);
    PixelShader(MapObjDiffuseEmissive);
}

//10
Effect(waterWindow)
{
e    //unk
}

//11
Effect(MapObjMaskedEnvMetal)
{
    VertexShader(MapObjDiffuse_T1_Env_T2);
    PixelShader(MapObjMaskedEnvMetal);
}

//12
Effect(MapObjEnvMetalEmissive)
{
    VertexShader(MapObjDiffuse_T1_Env_T2);
    PixelShader(MapObjEnvMetalEmissive);
}

//13
Effect(TwoLayerDiffuseOpaque)
{
    VertexShader(MapObjDiffuse_Comp);
    PixelShader(MapObjTwoLayerDiffuseOpaque);
}

//14
Effect(submarineWindow)
{
    //unk
}

//15
Effect(TwoLayerDiffuseEmissive)
{
    VertexShader(MapObjDiffuse_Comp);
    PixelShader(MapObjTwoLayerDiffuseEmissive);
}

//16
Effect(MapObjDiffuseTerrain)
{
    VertexShader(MapObjDiffuse_T1);
    PixelShader(MapObjDiffuse);
}

*/

enum class WmoVertexShader : int {
    None = -1,
    MapObjDiffuse_T1 = 0,
    MapObjDiffuse_T1_Refl = 1,
    MapObjDiffuse_T1_Env_T2 = 2,
    MapObjSpecular_T1 = 3,
    MapObjDiffuse_Comp = 4,
    MapObjDiffuse_Comp_Refl = 5,
    MapObjDiffuse_Comp_Terrain = 6,
    MapObjDiffuse_CompAlpha = 7,
    MapObjParallax = 8,

};

enum class WmoPixelShader : int {
    None = -1,
    MapObjDiffuse = 0,
    MapObjSpecular = 1,
    MapObjMetal = 2,
    MapObjEnv = 3,
    MapObjOpaque = 4,
    MapObjEnvMetal = 5,
    MapObjTwoLayerDiffuse = 6, //MapObjComposite
    MapObjTwoLayerEnvMetal = 7,
    MapObjTwoLayerTerrain = 8,
    MapObjDiffuseEmissive = 9,
    MapObjMaskedEnvMetal = 10,
    MapObjEnvMetalEmissive = 11,
    MapObjTwoLayerDiffuseOpaque = 12,
    MapObjTwoLayerDiffuseEmissive = 13,
    MapObjAdditiveMaskedEnvMetal = 14,
    MapObjTwoLayerDiffuseMod2x = 15,
    MapObjTwoLayerDiffuseMod2xNA = 16,
    MapObjTwoLayerDiffuseAlpha = 17,
    MapObjLod = 18,
    MapObjParallax = 19
};

inline constexpr const int operator+ (WmoPixelShader const val) { return static_cast<const int>(val); };
inline constexpr const int operator+ (WmoVertexShader const val) { return static_cast<const int>(val); };

const int MAX_WMO_SHADERS = 23;
static const struct {
    int vertexShader;
    int pixelShader;
} wmoMaterialShader[MAX_WMO_SHADERS] = {
    //MapObjDiffuse = 0
    {
        +WmoVertexShader::MapObjDiffuse_T1,
        +WmoPixelShader::MapObjDiffuse,
    },
    //MapObjSpecular = 1
    {
        +WmoVertexShader::MapObjSpecular_T1,
        +WmoPixelShader::MapObjSpecular,
    },
    //MapObjMetal = 2
    {
        +WmoVertexShader::MapObjSpecular_T1,
        +WmoPixelShader::MapObjMetal,
    },
    //MapObjEnv = 3
    {
        +WmoVertexShader::MapObjDiffuse_T1_Refl,
        +WmoPixelShader::MapObjEnv,
    },
    //MapObjOpaque = 4
    {
        +WmoVertexShader::MapObjDiffuse_T1,
        +WmoPixelShader::MapObjOpaque,
    },
    //MapObjEnvMetal = 5
    {
        +WmoVertexShader::MapObjDiffuse_T1_Refl,
        +WmoPixelShader::MapObjEnvMetal,
    },
    //MapObjTwoLayerDiffuse = 6
    {
        +WmoVertexShader::MapObjDiffuse_Comp,
        +WmoPixelShader::MapObjTwoLayerDiffuse,
    },
    //MapObjTwoLayerEnvMetal = 7
    {
        +WmoVertexShader::MapObjDiffuse_T1,
        +WmoPixelShader::MapObjTwoLayerEnvMetal,
    },
    //TwoLayerTerrain = 8
    {
        +WmoVertexShader::MapObjDiffuse_Comp_Terrain,
        +WmoPixelShader::MapObjTwoLayerTerrain,
    },
    //MapObjDiffuseEmissive = 9
    {
        +WmoVertexShader::MapObjDiffuse_Comp,
        +WmoPixelShader::MapObjDiffuseEmissive,
    },
    //waterWindow = 10
    {
        +WmoVertexShader::None,
        +WmoPixelShader::None,
    },
    //MapObjMaskedEnvMetal = 11
    {
        +WmoVertexShader::MapObjDiffuse_T1_Env_T2,
        +WmoPixelShader::MapObjMaskedEnvMetal,
    },
    //MapObjEnvMetalEmissive = 12
    {
        +WmoVertexShader::MapObjDiffuse_T1_Env_T2,
        +WmoPixelShader::MapObjEnvMetalEmissive,
    },
    //TwoLayerDiffuseOpaque = 13
    {
        +WmoVertexShader::MapObjDiffuse_Comp,
        +WmoPixelShader::MapObjTwoLayerDiffuseOpaque,
    },
    //submarineWindow = 14
    {
        +WmoVertexShader::None,
        +WmoPixelShader::None,
    },
    //TwoLayerDiffuseEmissive = 15
    {
        +WmoVertexShader::MapObjDiffuse_Comp,
        +WmoPixelShader::MapObjTwoLayerDiffuseEmissive,
    },
    //MapObjDiffuseTerrain = 16
    {
        +WmoVertexShader::MapObjDiffuse_T1,
        +WmoPixelShader::MapObjDiffuse,
    },
    //17
    {
        +WmoVertexShader::MapObjDiffuse_T1_Env_T2,
        +WmoPixelShader::MapObjAdditiveMaskedEnvMetal,
    },
    //18
    {
        +WmoVertexShader::MapObjDiffuse_CompAlpha,
        +WmoPixelShader::MapObjTwoLayerDiffuseMod2x,
    },
    //19
    {
        +WmoVertexShader::MapObjDiffuse_Comp,
        +WmoPixelShader::MapObjTwoLayerDiffuseMod2xNA,
    },
    //20
    {
        +WmoVertexShader::MapObjDiffuse_CompAlpha,
        +WmoPixelShader::MapObjTwoLayerDiffuseAlpha,
    },
    //21
    {
        +WmoVertexShader::MapObjDiffuse_T1,
        +WmoPixelShader::MapObjLod,
    },
    //22
    {
        +WmoVertexShader::MapObjParallax,
        +WmoPixelShader::MapObjParallax,
    }
};


void WmoGroupObject::doPostLoad() {
    if (!this->m_loaded) {
        if (m_geom != nullptr && m_geom->isLoaded() && m_wmoApi->isLoaded()) {
            this->postLoad();
            this->m_loaded = true;
            this->m_loading = false;
            return;
        }

        this->startLoading();
    }
}

void WmoGroupObject::update() {
    if (!this->m_loaded) return;
    
    if (m_recalcBoundries) {
        this->updateWorldGroupBBWithM2();
        m_recalcBoundries = false;
    }

    int minBatch = m_api->getConfig()->getWmoMinBatch();
    int maxBatch = std::min(m_api->getConfig()->getWmoMaxBatch(), m_geom->batchesLen);
    MOGP *mogp = m_geom->mogp;

    for (int j = minBatch; j < maxBatch; j++) {
        SMOBatch &renderBatch = m_geom->batches[j];
        mathfu::vec4 ambientColor;
        bool isBatchA = (j >= 0 && j < (m_geom->mogp->transBatchCount));
        bool isBatchC = (j >= (mogp->transBatchCount+mogp->intBatchCount));

        if (isBatchC) {
            ambientColor = m_api->getGlobalAmbientColor();
        } else {
            ambientColor = this->getAmbientColor();
        }

        HGUniformBuffer buffer = this->m_meshArray[j]->getFragmentUniformBuffer(2);

        wmoMeshWideBlockPS &blockPS = buffer->getObject<wmoMeshWideBlockPS>();
        blockPS.uViewUp = mathfu::vec4_packed(mathfu::vec4(m_api->getViewUp(), 0.0));;
        blockPS.uSunDir_FogStart = mathfu::vec4_packed(mathfu::vec4(m_api->getGlobalSunDir(), m_api->getGlobalFogStart()));
        blockPS.uSunColor_uFogEnd = mathfu::vec4_packed(mathfu::vec4(m_api->getGlobalSunColor().xyz(), m_api->getGlobalFogEnd()));
        blockPS.uAmbientLight = ambientColor;
        if (isBatchA) {
            blockPS.uAmbientLight2AndIsBatchA = mathfu::vec4(m_api->getGlobalAmbientColor().xyz(), 1.0);
        } else {
            blockPS.uAmbientLight2AndIsBatchA = mathfu::vec4(0,0,0,0);
        }
        blockPS.FogColor_AlphaTest = mathfu::vec4_packed(mathfu::vec4(m_api->getGlobalFogColor().xyz(), blockPS.FogColor_AlphaTest.w));
        buffer->save();
    }
}

void WmoGroupObject::drawDebugLights() {
    /*
    if (!this->m_loaded) return;

    MOLP * lights = m_geom->molp;

    std::vector<float> points;

    for (int i = 0; i < this->m_geom->molpCnt; i++) {
        points.push_back(lights[i].vec1.x);
        points.push_back(lights[i].vec1.y);
        points.push_back(lights[i].vec1.z);
    }

    GLuint bufferVBO;
    glGenBuffers(1, &bufferVBO);
    glBindBuffer( GL_ARRAY_BUFFER, bufferVBO);
    if (points.size() > 0) {
        glBufferData(GL_ARRAY_BUFFER, points.size() * 4, &points[0], GL_STATIC_DRAW);
    }

    auto drawPointsShader = m_api->getDrawPointsShader();
    static float colorArr[4] = {0.058, 0.819607843, 0.058, 0.3};
    glUniform3fv(drawPointsShader->getUnf("uColor"), 1, &colorArr[0]);

#ifndef WITH_GLESv2
    glEnable( GL_PROGRAM_POINT_SIZE );
#endif
    glVertexAttribPointer(+drawPoints::Attribute::aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);  // position


    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    glDrawArrays(GL_POINTS, 0, points.size()/3);

#ifndef WITH_GLESv2
    glDisable( GL_PROGRAM_POINT_SIZE );
#endif
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, GL_ZERO);
    glBindBuffer( GL_ARRAY_BUFFER, GL_ZERO);

    glDepthMask(GL_TRUE);

    glDeleteBuffers(1, &bufferVBO);
     */
}


void WmoGroupObject::startLoading() {
    if (!this->m_loading) {
        this->m_loading = true;

        if (useFileId) {
            m_geom = m_api->getWmoGroupGeomCache()->getFileId(m_modelFileId);
        } else {
            m_geom = m_api->getWmoGroupGeomCache()->get(m_fileName);
        }
        m_geom->setMOHD(this->m_wmoApi->getWmoHeader());
        m_geom->setAttenuateFunction(this->m_wmoApi->getAttenFunction());
    }
}

void WmoGroupObject::postLoad() {

    this->m_dontUseLocalLightingForM2 = ((m_geom->mogp->flags.EXTERIOR_LIT) > 0) || ((m_geom->mogp->flags.EXTERIOR) > 0);
    m_localGroupBorder = m_geom->mogp->boundingBox;
    this->createWorldGroupBB(m_geom->mogp->boundingBox, *m_modelMatrix);
    this->loadDoodads();
    this->createMeshes();
}

void WmoGroupObject::createMeshes() {
    Config * config = m_api->getConfig();
    HGShaderPermutation shaderPermutation = m_api->getDevice()->getShader("wmoShader");

    int minBatch = config->getWmoMinBatch();
    int maxBatch = std::min(config->getWmoMaxBatch(), m_geom->batchesLen);

    SMOMaterial * materials = m_wmoApi->getMaterials();

    GDevice *device = m_api->getDevice();
    HGVertexBufferBindings binding = m_geom->getVertexBindings(*device);

    vertexModelWideUniformBuffer = device->createUniformBuffer(sizeof(wmoModelWideBlockVS));

    wmoModelWideBlockVS &blockVS = vertexModelWideUniformBuffer->getObject<wmoModelWideBlockVS>();
    blockVS.uPlacementMat = *m_modelMatrix;
    vertexModelWideUniformBuffer->save();

    MOGP *mogp = m_geom->mogp;

    for (int j = minBatch; j < maxBatch; j++) {
        SMOBatch &renderBatch = m_geom->batches[j];

        gMeshTemplate meshTemplate(binding, shaderPermutation);

        int texIndex;
        if (renderBatch.flag_use_material_id_large) {
            texIndex = renderBatch.postLegion.material_id_large;
        } else {
            texIndex = renderBatch.material_id;
        }

        const SMOMaterial &material = materials[texIndex];
        assert(material.shader < MAX_WMO_SHADERS && material.shader >= 0);
        auto shaderId = material.shader;
        if (shaderId >= MAX_WMO_SHADERS) {
            shaderId = 0;
        }
        int pixelShader = wmoMaterialShader[shaderId].pixelShader;
        int vertexShader = wmoMaterialShader[shaderId].vertexShader;

        bool isBatchA = (j >= 0 && j < (m_geom->mogp->transBatchCount));
        bool isBatchC = (j >= (mogp->transBatchCount+mogp->intBatchCount));

        mathfu::vec4 ambientColor;
        if (isBatchC) {
            ambientColor = m_api->getGlobalAmbientColor();
        } else {
            ambientColor = this->getAmbientColor();
        }

        auto blendMode = material.blendMode;
        float alphaTest = (blendMode > 0) ? 0.00392157f : -1.0f;
        meshTemplate.meshType = MeshType::eWmoMesh;
        meshTemplate.depthWrite = blendMode <= 1;
        meshTemplate.depthCulling = true;
        meshTemplate.backFaceCulling = !(material.flags.F_UNCULLED);

        meshTemplate.blendMode = static_cast<EGxBlendEnum>(blendMode);

        meshTemplate.start = renderBatch.first_index * 2;
        meshTemplate.end = renderBatch.num_indices;
        meshTemplate.element = GL_TRIANGLES;

        bool isSecondTextSpec = material.shader == 8;

        HGTexture texture1 = m_wmoApi->getTexture(material.diffuseNameIndex, false);
        HGTexture texture2 = m_wmoApi->getTexture(material.envNameIndex, isSecondTextSpec);
        HGTexture texture3 = m_wmoApi->getTexture(material.texture_2, false);

        meshTemplate.texture[0] = texture1;
        meshTemplate.texture[1] = texture2;
        meshTemplate.texture[2] = texture3;

        meshTemplate.textureCount = 3;

        meshTemplate.vertexBuffers[0] = m_api->getSceneWideUniformBuffer();
        meshTemplate.vertexBuffers[1] = vertexModelWideUniformBuffer;
        meshTemplate.vertexBuffers[2] = m_api->getDevice()->createUniformBuffer(sizeof(wmoMeshWideBlockVS));

        meshTemplate.fragmentBuffers[0] = m_api->getSceneWideUniformBuffer();
        meshTemplate.fragmentBuffers[1] = nullptr;
        meshTemplate.fragmentBuffers[2] = m_api->getDevice()->createUniformBuffer(sizeof(wmoMeshWideBlockPS));


        //Fill buffers
        wmoMeshWideBlockVS &blockVS = meshTemplate.vertexBuffers[2] ->getObject<wmoMeshWideBlockVS>();
        blockVS.UseLitColor = (material.flags.F_UNLIT > 0) ? 0 : 1;
        blockVS.VertexShader= vertexShader;
        meshTemplate.vertexBuffers[2] ->save();

        wmoMeshWideBlockPS &blockPS = meshTemplate.fragmentBuffers[2]->getObject<wmoMeshWideBlockPS>();
        blockPS.uViewUp = mathfu::vec4_packed(mathfu::vec4(m_api->getViewUp(), 0.0));;
        blockPS.uSunDir_FogStart = mathfu::vec4_packed(mathfu::vec4(m_api->getGlobalSunDir(), m_api->getGlobalFogStart()));
        blockPS.uSunColor_uFogEnd = mathfu::vec4_packed(mathfu::vec4(m_api->getGlobalSunColor().xyz(), m_api->getGlobalFogEnd()));
        blockPS.uAmbientLight = ambientColor;
        if (isBatchA) {
            blockPS.uAmbientLight2AndIsBatchA = mathfu::vec4(m_api->getGlobalAmbientColor().xyz(), 1.0);
        } else {
            blockPS.uAmbientLight2AndIsBatchA = mathfu::vec4(0,0,0,0);
        }

        blockPS.UseLitColor = (material.flags.F_UNLIT > 0) ? 0 : 1;
        blockPS.EnableAlpha = (blendMode > 0) ? 1 : 0;
        blockPS.PixelShader = pixelShader;
        blockPS.FogColor_AlphaTest = mathfu::vec4_packed(mathfu::vec4(m_api->getGlobalFogColor().xyz(), alphaTest));

        meshTemplate.fragmentBuffers[2]->save();

        //Make mesh
        HGMesh hmesh = m_api->getDevice()->createMesh(meshTemplate);
        this->m_meshArray.push_back(hmesh);
    }
}

void WmoGroupObject::loadDoodads() {
    if (this->m_geom->doodadRefsLen <= 0) return;
    if (m_wmoApi == nullptr) return;

    m_doodads = std::vector<M2Object *>(this->m_geom->doodadRefsLen, nullptr);

    //Load all doodad from MOBR
    for (int i = 0; i < this->m_geom->doodadRefsLen; i++) {
        m_doodads[i] = m_wmoApi->getDoodad(this->m_geom->doodadRefs[i]);
        std::function<void()> event = [&]() -> void {
            this->m_recalcBoundries = true;
        };
        if (m_doodads[i] != nullptr) {
            m_doodads[i]->addPostLoadEvent(event);
        }
    }
}

void WmoGroupObject::createWorldGroupBB(CAaBox &bbox, mathfu::mat4 &placementMatrix) {
//            groupInfo = this.groupInfo;
//            bb1 = groupInfo.bb1;
//            bb2 = groupInfo.bb2;
//        } else {
//            groupInfo = this.wmoGeom.wmoGroupFile.mogp;
//            bb1 = groupInfo.BoundBoxCorner1;
//            bb2 = groupInfo.BoundBoxCorner2;
//        }
    C3Vector &bb1 = bbox.min;
    C3Vector &bb2 = bbox.max;

    mathfu::vec4 bb1vec = mathfu::vec4(bb1.x, bb1.y, bb1.z, 1);
    mathfu::vec4 bb2vec = mathfu::vec4(bb2.x, bb2.y, bb2.z, 1);

    CAaBox worldAABB = MathHelper::transformAABBWithMat4(placementMatrix, bb1vec, bb2vec);

    this->m_worldGroupBorder = worldAABB;
    this->m_volumeWorldGroupBorder = worldAABB;

//    std::cout << "Called WmoGroupObject::createWorldGroupBB " << std::endl;
}

void WmoGroupObject::updateWorldGroupBBWithM2() {
//    var doodadRefs = this.wmoGeom.wmoGroupFile.doodadRefs;
//    var mogp = this.wmoGeom.wmoGroupFile.mogp;
    CAaBox &groupAABB = this->m_worldGroupBorder;
//
//    var dontUseLocalLighting = ((mogp.flags & 0x40) > 0) || ((mogp.flags & 0x8) > 0);
//
    for (int j = 0; j < this->m_doodads.size(); j++) {
        M2Object* m2Object = this->m_doodads[j];
        if (m2Object == nullptr || !m2Object->getGetIsLoaded()) continue; //corrupted :(

        CAaBox m2AAbb = m2Object->getAABB(); 
        
        //2. Update the world group BB
        groupAABB.min = mathfu::vec3_packed(mathfu::vec3(std::min(m2AAbb.min.x,groupAABB.min.x),
                                            std::min(m2AAbb.min.y,groupAABB.min.y),
                                            std::min(m2AAbb.min.z,groupAABB.min.z)));

        groupAABB.max = mathfu::vec3_packed(mathfu::vec3(std::max(m2AAbb.max.x,groupAABB.max.x),
                                        std::max(m2AAbb.max.y,groupAABB.max.y),
                                        std::max(m2AAbb.max.z,groupAABB.max.z)));
    }

//    std::cout << "Called WmoGroupObject::updateWorldGroupBBWithM2 " << std::endl;
    this->m_worldGroupBorder = CAaBox(groupAABB.min, groupAABB.max);
    m_wmoApi->updateBB();
}

bool WmoGroupObject::checkGroupFrustum(mathfu::vec4 &cameraPos,
                                       std::vector<mathfu::vec4> &frustumPlanes,
                                       std::vector<mathfu::vec3> &points) {
    if (this == nullptr) return false;
    if (!m_loaded) return true;
    CAaBox bbArray = this->m_worldGroupBorder;

    bool isInsideM2Volume = (
            cameraPos[0] > bbArray.min.z && cameraPos[0] < bbArray.max.x &&
            cameraPos[1] > bbArray.min.y && cameraPos[1] < bbArray.max.y &&
            cameraPos[2] > bbArray.min.z && cameraPos[2] < bbArray.max.z
    );

    bool drawDoodads = isInsideM2Volume || MathHelper::checkFrustum(frustumPlanes, bbArray, points);

    bbArray = this->m_volumeWorldGroupBorder;
    bool isInsideGroup = (
            cameraPos[0] > bbArray.min.z && cameraPos[0] < bbArray.max.x &&
            cameraPos[1] > bbArray.min.y && cameraPos[1] < bbArray.max.y &&
            cameraPos[2] > bbArray.min.z && cameraPos[2] < bbArray.max.z
    );

    bool drawGroup = isInsideGroup || MathHelper::checkFrustum(frustumPlanes, bbArray, points);
    return drawGroup;
}

bool WmoGroupObject::checkIfInsidePortals(mathfu::vec3 point,
                                          SMOPortal *portalInfos,
                                          SMOPortalRef *portalRels) {
    int moprIndex = this->m_geom->mogp->moprIndex;
    int numItems = this->m_geom->mogp->moprCount;

    std::vector<PortalInfo_t> &portalGeoms = this->m_wmoApi->getPortalInfos();

    bool insidePortals = true;
    for (int j = moprIndex; j < moprIndex + numItems; j++) {
        SMOPortalRef *relation = &portalRels[j];
        SMOPortal *portalInfo = &portalInfos[relation->portal_index];

        int nextGroup = relation->group_index;
        C4Plane plane = portalInfo->plane;

        CAaBox &aaBox = portalGeoms[relation->portal_index].aaBox;
        float distanceToBB = MathHelper::distanceFromAABBToPoint(aaBox, point);

        float dotResult = mathfu::vec3::DotProduct(mathfu::vec4(plane.planeVector).xyz(), point) + plane.planeVector.w;
        bool isInsidePortalThis = (relation->side < 0) ? (dotResult <= 0) : (dotResult >= 0);
        if (!isInsidePortalThis && (abs(dotResult) < 0.01) && (abs(distanceToBB) < 0.01)) {
            insidePortals = false;
            break;
        }
    }

    return insidePortals;
}

void WmoGroupObject::queryBspTree(CAaBox &bbox, int nodeId, t_BSP_NODE *nodes, std::vector<int> &bspLeafIdList) {
    if (nodeId == -1) return;

    if ((nodes[nodeId].planeType & 0x4)) {
        bspLeafIdList.push_back(nodeId);
    } else if ((nodes[nodeId].planeType == 0)) {
        bool leftSide = MathHelper::checkFrustum({mathfu::vec4(-1, 0, 0, nodes[nodeId].fDist)}, bbox, {});
        bool rightSide = MathHelper::checkFrustum({mathfu::vec4(1, 0, 0, -nodes[nodeId].fDist)}, bbox, {});

        if (leftSide) {
            WmoGroupObject::queryBspTree(bbox, nodes[nodeId].children[0], nodes, bspLeafIdList);
        }
        if (rightSide) {
            WmoGroupObject::queryBspTree(bbox, nodes[nodeId].children[1], nodes, bspLeafIdList);
        }
    } else if ((nodes[nodeId].planeType == 1)) {
        bool leftSide = MathHelper::checkFrustum({mathfu::vec4(0, -1, 0, nodes[nodeId].fDist)}, bbox, {});
        bool rightSide = MathHelper::checkFrustum({mathfu::vec4(0, 1, 0, -nodes[nodeId].fDist)}, bbox, {});

        if (leftSide) {
            WmoGroupObject::queryBspTree(bbox, nodes[nodeId].children[0], nodes, bspLeafIdList);
        }
        if (rightSide) {
            WmoGroupObject::queryBspTree(bbox, nodes[nodeId].children[1], nodes, bspLeafIdList);
        }
    } else if ((nodes[nodeId].planeType == 2)) {
        bool leftSide = MathHelper::checkFrustum({mathfu::vec4(0, 0, -1, nodes[nodeId].fDist)}, bbox, {});
        bool rightSide = MathHelper::checkFrustum({mathfu::vec4(0, 0, 1, -nodes[nodeId].fDist)}, bbox, {});

        if (leftSide) {
            WmoGroupObject::queryBspTree(bbox, nodes[nodeId].children[0], nodes, bspLeafIdList);
        }
        if (rightSide) {
            WmoGroupObject::queryBspTree(bbox, nodes[nodeId].children[1], nodes, bspLeafIdList);
        }
    }
}

bool WmoGroupObject::getTopAndBottomTriangleFromBsp(
        mathfu::vec4 &cameraLocal,
        SMOPortal *portalInfos,
        SMOPortalRef *portalRels,
        std::vector<int> &bspLeafList,
        M2Range &result) {

    t_BSP_NODE *nodes = this->m_geom->bsp_nodes;
    float topZ = -999999;
    float bottomZ = 999999;
    float minPositiveDistanceToCamera = 99999;

    //1. Loop through bsp results
    for (int i = 0; i < bspLeafList.size(); i++) {
        t_BSP_NODE *node = &nodes[bspLeafList[i]];

        for (int j = node->firstFace; j < node->firstFace + node->numFaces; j++) {
            int vertexInd1 = this->m_geom->indicies[3 * this->m_geom->bpsIndicies[j] + 0];
            int vertexInd2 = this->m_geom->indicies[3 * this->m_geom->bpsIndicies[j] + 1];
            int vertexInd3 = this->m_geom->indicies[3 * this->m_geom->bpsIndicies[j] + 2];

            mathfu::vec3 vert1 = mathfu::vec3(this->m_geom->verticles[vertexInd1]);
            mathfu::vec3 vert2 = mathfu::vec3(this->m_geom->verticles[vertexInd2]);
            mathfu::vec3 vert3 = mathfu::vec3(this->m_geom->verticles[vertexInd3]);

            //1. Get if camera position inside vertex
            float minX = std::min(std::min(vert1[0], vert2[0]), vert3[0]);
            float minY = std::min(std::min(vert1[1], vert2[1]), vert3[1]);
            float minZ = std::min(std::min(vert1[2], vert2[2]), vert3[2]);

            float maxX = std::max(std::max(vert1[0], vert2[0]), vert3[0]);
            float maxY = std::max(std::max(vert1[1], vert2[1]), vert3[1]);
            float maxZ = std::max(std::max(vert1[2], vert2[2]), vert3[2]);

            bool testPassed = (
                    (cameraLocal[0] > minX && cameraLocal[0] < maxX) &&
                    (cameraLocal[1] > minY && cameraLocal[1] < maxY)
            );
            if (!testPassed) continue;

            mathfu::vec4 plane = MathHelper::createPlaneFromEyeAndVertexes(vert1, vert2, vert3);
            //var z = MathHelper.calcZ(vert1,vert2,vert3,cameraLocal[0],cameraLocal[1]);
            if ((plane[2] < 0.0001) && (plane[2] > -0.0001)) continue;

            float z = (-plane[3] - cameraLocal[0] * plane[0] - cameraLocal[1] * plane[1]) / plane[2];

            //2. Get if vertex top or bottom
            mathfu::vec3 normal1 = mathfu::vec3(this->m_geom->normals[vertexInd1]);
            mathfu::vec3 normal2 = mathfu::vec3(this->m_geom->normals[vertexInd2]);
            mathfu::vec3 normal3 = mathfu::vec3(this->m_geom->normals[vertexInd3]);

            mathfu::vec3 suggestedPoint = mathfu::vec3(cameraLocal[0], cameraLocal[1], z);
            mathfu::vec3 bary = MathHelper::getBarycentric(
                    suggestedPoint,
                    vert1,
                    vert2,
                    vert3
            );

            if ((bary[0] < 0) || (bary[1] < 0) || (bary[2] < 0)) continue;
            if (!WmoGroupObject::checkIfInsidePortals(suggestedPoint, portalInfos,
                                                      portalRels))
                continue;

            float normal_avg = bary[0] * normal1[2] + bary[1] * normal2[2] + bary[2] * normal3[2];
            if (normal_avg > 0) {
                //Bottom
                float distanceToCamera = cameraLocal[2] - z;
                if ((distanceToCamera > 0) && (distanceToCamera < minPositiveDistanceToCamera))
                    bottomZ = z;
            } else {
                //Top
                topZ = std::max(z, topZ);
            }
        }
    }
    //2. Try to get top and bottom from portal planes
    MOGP *groupInfo = this->m_geom->mogp;
    int moprIndex = groupInfo->moprIndex;
    int numItems = groupInfo->moprCount;

    std::vector<PortalInfo_t> &portalGeoms = this->m_wmoApi->getPortalInfos();

    for (int j = moprIndex; j < moprIndex + numItems; j++) {
        SMOPortalRef *relation = &portalRels[j];
        SMOPortal *portalInfo = &portalInfos[relation->portal_index];

        int nextGroup = relation->group_index;
        C4Plane *plane = &portalInfo->plane;

        int base_index = portalInfo->base_index;


        float dotResult = mathfu::vec4::DotProduct(mathfu::vec4(plane->planeVector), cameraLocal);
        bool isInsidePortalThis = (relation->side < 0) ? (dotResult <= 0) : (dotResult >= 0);
        //If we are going to borrow z from this portal, we should be inside it
        if (!isInsidePortalThis) continue;

        if ((plane->planeVector.z < 0.0001) && (plane->planeVector.z > -0.0001)) continue;
        float z = (-plane->planeVector.w -
                   cameraLocal[0] * plane->planeVector.x -
                   cameraLocal[1] * plane->planeVector.y)
                  / plane->planeVector.z;

        std::vector<mathfu::vec3> &portalVerticles = portalGeoms[relation->portal_index].sortedVericles;
        for (int k = 0; k < portalVerticles.size() - 2; k++) {
            int portalIndex;
            portalIndex = 0;
            mathfu::vec3 point1 = mathfu::vec3(portalVerticles[portalIndex]);
            portalIndex = k + 1;
            mathfu::vec3 point2 = mathfu::vec3(portalVerticles[portalIndex]);
            portalIndex = k + 2;
            mathfu::vec3 point3 = mathfu::vec3(portalVerticles[portalIndex]);

            mathfu::vec3 pointToCheck = mathfu::vec3(cameraLocal[0], cameraLocal[1], z);
            mathfu::vec3 bary = MathHelper::getBarycentric(
                    pointToCheck,
                    point1,
                    point2,
                    point3
            );
            if ((bary[0] < 0) || (bary[1] < 0) || (bary[2] < 0)) continue;
            if (z > cameraLocal[2]) {
                if (topZ < -99999)
                    topZ = z;
            }
            if (z < cameraLocal[2]) {
                if (bottomZ > 99999)
                    bottomZ = z;
            }
        }
    }
    if ((bottomZ > 99999) && (topZ < -99999)) {
        return false;
    }

    result = {bottomZ, topZ};

    return true;
}

bool WmoGroupObject::checkIfInsideGroup(mathfu::vec4 &cameraVec4,
                                        mathfu::vec4 &cameraLocal,
                                        C3Vector *portalVerticles,
                                        SMOPortal *portalInfos,
                                        SMOPortalRef *portalRels,
                                        std::vector<WmoGroupResult> &candidateGroups) {

    CAaBox &bbArray = this->m_volumeWorldGroupBorder;

    //1. Check if group wmo is interior wmo
    //if ((groupInfo.flags & 0x2000) == 0) return null;
    //interiorGroups++;

    //2. Check if inside volume AABB
    bool isInsideAABB = (
            cameraVec4.x > bbArray.min.x && cameraVec4.x < bbArray.max.x &&
            cameraVec4.y > bbArray.min.y && cameraVec4.y < bbArray.max.y &&
            cameraVec4.z > bbArray.min.z && cameraVec4.z < bbArray.max.z
    );

    if (!isInsideAABB) return false;

    //wmoGroupsInside++;
    //lastWmoGroupInside = i;

    if (!m_loaded) {
        M2Range topBottom;
        topBottom.max = m_main_groupInfo->bounding_box.max.z;
        topBottom.min = m_main_groupInfo->bounding_box.min.z;


		WmoGroupResult result;
		
		result.topBottom = topBottom;
		result.groupIndex = m_groupNumber;
		result.WMOGroupID = -1;
		result.bspLeafList = {};
		result.nodeId = -1;
		
		candidateGroups.push_back(result);

        return true;
    }

    MOGP *groupInfo = this->m_geom->mogp;

    int moprIndex = groupInfo->moprIndex;
    int numItems = groupInfo->moprCount;

    bool insidePortals = this->checkIfInsidePortals(cameraLocal.xyz(), portalInfos, portalRels);
    if (!insidePortals) return false;

    //3. Query bsp tree for leafs around the position of object(camera)


    float epsilon = 1;
    mathfu::vec3 cameraBBMin(cameraLocal[0] - epsilon, cameraLocal[1] - epsilon, groupInfo->boundingBox.min.z);
    mathfu::vec3 cameraBBMax(cameraLocal[0] + epsilon, cameraLocal[1] + epsilon, groupInfo->boundingBox.max.z);

    int nodeId = 0;
    t_BSP_NODE *nodes = this->m_geom->bsp_nodes;
    std::vector<int> bspLeafList;

    M2Range topBottom;
    topBottom.max = groupInfo->boundingBox.max.z;
    topBottom.min = groupInfo->boundingBox.min.z;

    CAaBox cameraBB;
    cameraBB.max = cameraBBMax;
    cameraBB.min = cameraBBMin;

    if (nodes != nullptr) {
        WmoGroupObject::queryBspTree(cameraBB, nodeId, nodes, bspLeafList);
        bool result = WmoGroupObject::getTopAndBottomTriangleFromBsp(
                cameraLocal, portalInfos, portalRels, bspLeafList, topBottom);
        if (!result) return false;
        if (topBottom.min > 99999) return false;

        //5. The object(camera) is inside WMO group. Get the actual nodeId
        while (nodeId >= 0 && ((nodes[nodeId].planeType & 0x4) == 0)) {
            int prevNodeId = nodeId;
            if ((nodes[nodeId].planeType == 0)) {
                if (cameraLocal[0] < nodes[nodeId].fDist) {
                    nodeId = nodes[nodeId].children[0];
                } else {
                    nodeId = nodes[nodeId].children[1];
                }
            } else if ((nodes[nodeId].planeType == 1)) {
                if (cameraLocal[1] < nodes[nodeId].fDist) {
                    nodeId = nodes[nodeId].children[0];
                } else {
                    nodeId = nodes[nodeId].children[1];
                }
            } else if ((nodes[nodeId].planeType == 2)) {
                if (cameraLocal[2] < nodes[nodeId].fDist) {
                    nodeId = nodes[nodeId].children[0];
                } else {
                    nodeId = nodes[nodeId].children[1];
                }
            }
        }

		WmoGroupResult candidate;
		candidate.topBottom = topBottom;
		candidate.groupIndex = m_groupNumber;
		candidate.WMOGroupID = groupInfo->wmoGroupID;
		candidate.bspLeafList = bspLeafList;
		candidate.nodeId = nodeId;

        candidateGroups.push_back(candidate);

        return true;
    }

    return false;
}


bool WmoGroupObject::checkDoodads(std::vector<M2Object *> &wmoM2Candidates) {
    if (!m_loaded) return false;

    mathfu::vec4 ambientColor = getAmbientColor();


    for (int i = 0; i < this->m_doodads.size(); i++) {
        if (this->m_doodads[i] != nullptr) {
            if (this->m_dontUseLocalLightingForM2) {
                this->m_doodads[i]->setUseLocalLighting(false);
            } else {
                this->m_doodads[i]->setUseLocalLighting(true);
                this->m_doodads[i]->setAmbientColorOverride(ambientColor, true);
            }

            wmoM2Candidates.push_back(this->m_doodads[i]);
        }
    }
}

void WmoGroupObject::setWmoApi(IWmoApi *api) {
    m_wmoApi = api;
}

void WmoGroupObject::setModelFileName(std::string modelName) {
    m_fileName = modelName;
}

void WmoGroupObject::setModelFileId(int fileId) {
    useFileId = true;
    m_modelFileId = fileId;
}

void WmoGroupObject::collectMeshes(std::vector<HGMesh> &renderedThisFrame, int renderOrder) {
    for (int i = 0; i < this->m_meshArray.size(); i++) {
        this->m_meshArray[i]->setRenderOrder(renderOrder);
        renderedThisFrame.push_back(this->m_meshArray[i]);
    }
}

mathfu::vec4 WmoGroupObject::getAmbientColor() {

    if (!m_geom->mogp->flags.EXTERIOR && !m_geom->mogp->flags.EXTERIOR_LIT) {
        mathfu::vec4 ambColor;
        ambColor = mathfu::vec4(
            ((float) m_geom->mohd->ambColor.r / 255.0f),
            ((float) m_geom->mohd->ambColor.g / 255.0f),
            ((float) m_geom->mohd->ambColor.b / 255.0f),
            ((float) m_geom->mohd->ambColor.a / 255.0f)
        );

        if ((m_geom->use_replacement_for_header_color == 1) && (*(int *) &m_geom->replacement_for_header_color != -1)) {
            ambColor = mathfu::vec4(
                ((float) m_geom->replacement_for_header_color.r / 255.0f),
                ((float) m_geom->replacement_for_header_color.g / 255.0f),
                ((float) m_geom->replacement_for_header_color.b / 255.0f),
                ((float) m_geom->replacement_for_header_color.a / 255.0f)
            );
        }

        return ambColor ;
    } else {
        return m_api->getGlobalAmbientColor();
    }
}
