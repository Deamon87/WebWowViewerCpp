//
// Created by deamon on 10.07.17.
//

#include "wmoGroupObject.h"
#include "../../algorithms/mathHelper.h"
#include "../../../gapi/interface/IDevice.h"
#include "../../../gapi/UniformBufferStructures.h"
#include "../../persistance/header/wmoFileHeader.h"
#include "../../../gapi/interface/materials/IMaterial.h"
#include <algorithm>

bool WmoGroupObject::doPostLoad(const HMapSceneBufferCreate &sceneRenderer) {
    if (this->m_loaded) return false;

    if (!this->m_loading) {
        this->startLoading();
        return false;
    }

    if ((m_geom == nullptr) || (m_geom->getStatus() != FileStatus::FSLoaded) || (!m_wmoApi->isLoaded())) return false;

    this->postLoad(sceneRenderer);
    this->m_loaded = true;
    this->m_loading = false;
    return true;
}

void WmoGroupObject::update() {
    if (!this->m_loaded) return;

    if (m_recalcBoundries) {
        this->updateWorldGroupBBWithM2();
        m_recalcBoundries = false;
    }


}

void WmoGroupObject::uploadGeneratorBuffers(const HFrameDependantData &frameDependantData, animTime_t mapCurrentTime)  {
    for(auto &liquidInstance : m_liquidInstances) {
        liquidInstance->updateLiquidMaterials(frameDependantData, mapCurrentTime);
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
            m_geom = m_api->cacheStorage->getWmoGroupGeomCache()->getFileId(m_modelFileId);
        } else {
            m_geom = m_api->cacheStorage->getWmoGroupGeomCache()->get(m_fileName);
        }

        m_geom->setAttenuateFunction(this->m_wmoApi->getAttenFunction());
    }
}

void WmoGroupObject::postLoad(const HMapSceneBufferCreate &sceneRenderer) {

    this->m_useLocalLightingForM2 =
        ((m_geom->mogp->flags.INTERIOR) > 0) && ((m_geom->mogp->flags.EXTERIOR_LIT) == 0);
    m_localGroupBorder = m_geom->mogp->boundingBox;
    this->createWorldGroupBB(m_geom->mogp->boundingBox, *m_modelMatrix);
    this->loadDoodads();
    this->createMeshes(sceneRenderer);
    this->createWaterMeshes(sceneRenderer);
}

void WmoGroupObject::createMeshes(const HMapSceneBufferCreate &sceneRenderer) {
    Config *config = m_api->getConfig();

    int minBatch = config->wmoMinBatch;
    int maxBatch = std::min(config->wmoMaxBatch, m_geom->batchesLen);

    //In Taanant Jungle in Draenor some WMO have no indicies and crash :D
    if (m_geom->indicesLen == 0) {
        return;
    }

    HGDevice device = m_api->hDevice;

    HGVertexBufferBindings binding = m_geom->getVertexBindings(sceneRenderer, this->m_wmoApi->getWmoHeader(),
                                                               mathfu::vec4(
        this->getAmbientColor().xyz(),
        ((this->m_geom->mogp->flags.INTERIOR > 0) && (!this->m_geom->mogp->flags.EXTERIOR_LIT)) ? 1.0f : 0.0f
    ));

    MOGP *mogp = m_geom->mogp;

    for (int j = minBatch; j < maxBatch; j++) {
        SMOBatch &renderBatch = m_geom->batches[j];

        int materialIndex;
        if (renderBatch.flag_use_material_id_large) {
            materialIndex = renderBatch.postLegion.material_id_large;
        } else {
            materialIndex = renderBatch.material_id;
        }

        auto materialInstance = m_wmoApi->getMaterialInstance(materialIndex, sceneRenderer);

        gMeshTemplate meshTemplate(binding);

        bool isBatchA = (j >= 0 && j < (m_geom->mogp->transBatchCount));
        bool isBatchC = (j >= (mogp->transBatchCount + mogp->intBatchCount));

        meshTemplate.meshType = MeshType::eWmoMesh;
        meshTemplate.start = renderBatch.first_index * 2;
        meshTemplate.end = renderBatch.num_indices;

        //Make mesh
        HGMesh hmesh = sceneRenderer->createMesh(meshTemplate, materialInstance);
        this->m_meshArray.push_back(hmesh);
    }
}


LiquidTypes WmoGroupObject::to_wmo_liquid (int x) {
    liquid_basic_types const basic (static_cast<liquid_basic_types>(x & liquid_basic_types_MASK));
    switch (basic)
    {
        case liquid_basic_types_water:
            return (m_geom->mogp->flags.is_not_water_but_ocean) ? LiquidTypes::LIQUID_WMO_Ocean : LiquidTypes::LIQUID_WMO_Water;
        case liquid_basic_types_ocean:
            return LiquidTypes::LIQUID_WMO_Ocean;
        case liquid_basic_types_magma:
            return LiquidTypes::LIQUID_WMO_Magma;
        case liquid_basic_types_slime:
            return LiquidTypes::LIQUID_WMO_Slime;
    }

    return LiquidTypes::LIQUID_NONE;
}

void WmoGroupObject::setLiquidType() {

    if ( getWmoApi()->getWmoHeader()->flags.flag_use_liquid_type_dbc_id)
    {
        if ( m_geom->mogp->liquidType < (int)LiquidTypes::LIQUID_FIRST_NONBASIC_LIQUID_TYPE )
        {
            this->liquid_type = to_wmo_liquid (m_geom->mogp->liquidType - 1);
        }
        else
        {
            this->liquid_type = static_cast<LiquidTypes>(m_geom->mogp->liquidType);
        }
    }
    else
    {
        if ( m_geom->mogp->liquidType == (int)LiquidTypes::LIQUID_Green_Lava )
        {
            this->liquid_type = static_cast<LiquidTypes>(0);
        }
        else
        {
            int const liquidType (m_geom->mogp->liquidType + 1);
            int const tmp (m_geom->mogp->liquidType);
            if ( m_geom->mogp->liquidType < (int)LiquidTypes::LIQUID_END_BASIC_LIQUIDS )
            {
                this->liquid_type = to_wmo_liquid (m_geom->mogp->liquidType);
            }
            else
            {
                this->liquid_type = static_cast<LiquidTypes>(m_geom->mogp->liquidType + 1);
            }
            assert (!liquidType || !(m_geom->mogp->flags.LIQUIDSURFACE));
        }
    }
}

void WmoGroupObject::createWaterMeshes(const HMapSceneBufferCreate &sceneRenderer) {
    HGDevice device = m_api->hDevice;

    //Get Liquid with new method
    setLiquidType();

    HGVertexBufferBindings binding = m_geom->getWaterVertexBindings(sceneRenderer, liquid_type, m_waterAaBB);
    if (binding == nullptr)
        return;

    auto l_liquidInstance = std::make_shared<LiquidInstance>(m_api, sceneRenderer, binding, (int)liquid_type, m_geom->waterIndexSize, m_wmoApi->getPlacementBuffer(), m_waterAaBB);

    m_liquidInstances.push_back(l_liquidInstance);
}

void WmoGroupObject::loadDoodads() {
    if (this->m_geom->doodadRefsLen <= 0) return;
    if (m_wmoApi == nullptr) return;

    m_doodads = {};
    m_doodads.reserve(this->m_geom->doodadRefsLen);

    //Load all doodad from MOBR
    for (int i = 0; i < this->m_geom->doodadRefsLen; i++) {
        auto newDoodad = m_wmoApi->getDoodad(this->m_geom->doodadRefs[i]);
        m_doodads.push_back(newDoodad);
        if (newDoodad != nullptr) {
            std::function<void()> event = [&]() -> void {
                this->m_recalcBoundries = true;
            };

            newDoodad->addPostLoadEvent(event);
        }
    }

    this->m_recalcBoundries = true;
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
    for (auto &m2Object : this->m_doodads) {
        if (m2Object == nullptr || !m2Object->isMainDataLoaded()) continue;

        CAaBox m2AAbb = m2Object->getAABB();

        //2. Update the world group BB
        groupAABB.min = mathfu::vec3_packed(mathfu::vec3(std::min(m2AAbb.min.x, groupAABB.min.x),
                                                         std::min(m2AAbb.min.y, groupAABB.min.y),
                                                         std::min(m2AAbb.min.z, groupAABB.min.z)));

        groupAABB.max = mathfu::vec3_packed(mathfu::vec3(std::max(m2AAbb.max.x, groupAABB.max.x),
                                                         std::max(m2AAbb.max.y, groupAABB.max.y),
                                                         std::max(m2AAbb.max.z, groupAABB.max.z)));
    }

//    std::cout << "Called WmoGroupObject::updateWorldGroupBBWithM2 " << std::endl;
    this->m_worldGroupBorder = CAaBox(groupAABB.min, groupAABB.max);
    m_wmoApi->updateBB();
}

void WmoGroupObject::checkGroupFrustum(bool &drawDoodads, bool &drawGroup,
                                       mathfu::vec4 &cameraPos,
                                       const MathHelper::FrustumCullingData &frustumData) {
    drawDoodads = false;
    drawGroup = false;
    if (!m_loaded) {
        //Force load of group if it's exterior
        if (m_main_groupInfo->flags.EXTERIOR > 0 || !m_api->getConfig()->usePortalCulling) {
            drawGroup = true;
            drawDoodads = true;
        }
        return;
    }
    CAaBox bbArray = this->m_worldGroupBorder;

    bool isInsideM2Volume = (
        cameraPos[0] > bbArray.min.z && cameraPos[0] < bbArray.max.x &&
        cameraPos[1] > bbArray.min.y && cameraPos[1] < bbArray.max.y &&
        cameraPos[2] > bbArray.min.z && cameraPos[2] < bbArray.max.z
    );

    drawDoodads = isInsideM2Volume || MathHelper::checkFrustum(frustumData, bbArray);

    bbArray = this->m_volumeWorldGroupBorder;
    bool isInsideGroup = (
        cameraPos[0] > bbArray.min.z && cameraPos[0] < bbArray.max.x &&
        cameraPos[1] > bbArray.min.y && cameraPos[1] < bbArray.max.y &&
        cameraPos[2] > bbArray.min.z && cameraPos[2] < bbArray.max.z
    );

    drawGroup = isInsideGroup || MathHelper::checkFrustum(frustumData, bbArray);
}

bool WmoGroupObject::checkIfInsidePortals(mathfu::vec3 point,
                                          const PointerChecker<SMOPortal> &portalInfos,
                                          const PointerChecker<SMOPortalRef> &portalRels) {
    int moprIndex = this->m_geom->mogp->moprIndex;
    int numItems = this->m_geom->mogp->moprCount;

    std::vector<PortalInfo_t> &portalGeoms = this->m_wmoApi->getPortalInfos();

    bool insidePortals = true;
    for (int j = moprIndex; j < moprIndex + numItems; j++) {
        const SMOPortalRef &relation = portalRels[j];
        const SMOPortal &portalInfo = portalInfos[relation.portal_index];

        int nextGroup = relation.group_index;
        C4Plane plane = portalInfo.plane;

        CAaBox &aaBox = portalGeoms[relation.portal_index].aaBox;
        float distanceToBB = MathHelper::distanceFromAABBToPoint(aaBox, point);

        float dotResult = mathfu::vec3::DotProduct(mathfu::vec4(plane.planeVector).xyz(), point) + plane.planeVector.w;
        bool isInsidePortalThis = (relation.side < 0) ? (dotResult <= 0) : (dotResult >= 0);
        if (!isInsidePortalThis && (abs(dotResult) < 0.01) && (abs(distanceToBB) < 0.01)) {
            insidePortals = false;
            break;
        }
    }

    return insidePortals;
}

template<typename Y>
void WmoGroupObject::queryBspTree(CAaBox &bbox, int nodeId, Y &nodes, std::vector<int> &bspLeafIdList) {
    if (nodeId == -1) return;

    if ((nodes[nodeId].planeType & 0x4)) {
        bspLeafIdList.push_back(nodeId);
    } else if (nodes[nodeId].planeType == 0) {
        std::vector<MathHelper::PlanesUndPoints> left = {{{mathfu::vec4(-1, 0, 0, nodes[nodeId].fDist)}}};
        std::vector<MathHelper::PlanesUndPoints> right = {{{mathfu::vec4(1, 0, 0, -nodes[nodeId].fDist)}}};

        bool leftSide = MathHelper::checkFrustum( left, bbox);
        bool rightSide = MathHelper::checkFrustum(right, bbox);

        if (leftSide) {
            WmoGroupObject::queryBspTree(bbox, nodes[nodeId].children[0], nodes, bspLeafIdList);
        }
        if (rightSide) {
            WmoGroupObject::queryBspTree(bbox, nodes[nodeId].children[1], nodes, bspLeafIdList);
        }
    } else if (nodes[nodeId].planeType == 1) {
        std::vector<MathHelper::PlanesUndPoints> left = {{{mathfu::vec4(0, -1, 0, nodes[nodeId].fDist)}}};
        std::vector<MathHelper::PlanesUndPoints> right = {{{mathfu::vec4(0, 1, 0, -nodes[nodeId].fDist)}}};

        bool leftSide = MathHelper::checkFrustum(left, bbox);
        bool rightSide = MathHelper::checkFrustum(right, bbox);

        if (leftSide) {
            WmoGroupObject::queryBspTree(bbox, nodes[nodeId].children[0], nodes, bspLeafIdList);
        }
        if (rightSide) {
            WmoGroupObject::queryBspTree(bbox, nodes[nodeId].children[1], nodes, bspLeafIdList);
        }
    } else if (nodes[nodeId].planeType == 2) {
        std::vector<MathHelper::PlanesUndPoints> left = {{{mathfu::vec4(0, 0, -1, nodes[nodeId].fDist)}}};
        std::vector<MathHelper::PlanesUndPoints> right = {{{mathfu::vec4(0, 0, 1, -nodes[nodeId].fDist)}}};

        bool leftSide = MathHelper::checkFrustum(left, bbox);
        bool rightSide = MathHelper::checkFrustum(right, bbox);

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
    PointerChecker<SMOPortal> &portalInfos,
    PointerChecker<SMOPortalRef> &portalRels,
    std::vector<int> &bspLeafList,
    M2Range &result) {

    float topZ;
    float bottomZ;
    mathfu::vec4 tempColor;
    getBottomVertexesFromBspResult(portalInfos, portalRels, bspLeafList, cameraLocal, topZ, bottomZ, tempColor);

    //2. Try to get top and bottom from portal planes
    // (That's for case when the portal plane is horizontal)
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

void WmoGroupObject::getBottomVertexesFromBspResult(
            const PointerChecker<SMOPortal> &portalInfos,
            const PointerChecker<SMOPortalRef> &portalRels,
            const std::vector<int> &bspLeafList, mathfu::vec4 &cameraLocal,
            float &topZ, float &bottomZ,
            mathfu::vec4 &colorUnderneath,
            bool checkPortals) {

    topZ = -999999;
    bottomZ = 999999;
    auto &nodes = m_geom->bsp_nodes;
    float minPositiveDistanceToCamera = 99999;

    //1. Loop through bsp results
    for (int i = 0; i < bspLeafList.size(); i++) {
        t_BSP_NODE *node = &nodes[bspLeafList[i]];

        for (int j = node->firstFace; j < node->firstFace + node->numFaces; j++) {
            int vertexInd1 = m_geom->indicies[3 * m_geom->bpsIndicies[j] + 0];
            int vertexInd2 = m_geom->indicies[3 * m_geom->bpsIndicies[j] + 1];
            int vertexInd3 = m_geom->indicies[3 * m_geom->bpsIndicies[j] + 2];

            mathfu::vec3 vert1 = mathfu::vec3(m_geom->verticles[vertexInd1]);
            mathfu::vec3 vert2 = mathfu::vec3(m_geom->verticles[vertexInd2]);
            mathfu::vec3 vert3 = mathfu::vec3(m_geom->verticles[vertexInd3]);

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
            if ((plane[2] < 0.0001) && (plane[2] > -0.0001)) continue;

            float z = (-plane[3] - cameraLocal[0] * plane[0] - cameraLocal[1] * plane[1]) / plane[2];

            //2. Get if vertex top or bottom
            mathfu::vec3 normal1 = mathfu::vec3(m_geom->normals[vertexInd1]);
            mathfu::vec3 normal2 = mathfu::vec3(m_geom->normals[vertexInd2]);
            mathfu::vec3 normal3 = mathfu::vec3(m_geom->normals[vertexInd3]);

            mathfu::vec3 suggestedPoint = mathfu::vec3(cameraLocal[0], cameraLocal[1], z);
            mathfu::vec3 bary = MathHelper::getBarycentric(
                suggestedPoint,
                vert1,
                vert2,
                vert3
            );

            if ((bary[0] < 0) || (bary[1] < 0) || (bary[2] < 0)) continue;
            if (checkPortals) {
                if (!this->checkIfInsidePortals(suggestedPoint, portalInfos, portalRels))
                    continue;
            }

            float normal_avg = bary[0] * normal1[2] + bary[1] * normal2[2] + bary[2] * normal3[2];
            if (normal_avg > 0) {
                //Bottom
                float distanceToCamera = cameraLocal[2] - z;
                if ((distanceToCamera > 0) && (distanceToCamera < minPositiveDistanceToCamera)) {
                    bottomZ = z;
                    if (m_geom->colorArray != nullptr) {
                        auto &colorArr = m_geom->colorArray;
                        colorUnderneath = mathfu::vec4(
                            bary[0] * colorArr[vertexInd1].r + bary[1] * colorArr[vertexInd2].r +
                            bary[2] * colorArr[vertexInd3].r,
                            bary[0] * colorArr[vertexInd1].g + bary[1] * colorArr[vertexInd2].g +
                            bary[2] * colorArr[vertexInd3].g,
                            bary[0] * colorArr[vertexInd1].b + bary[1] * colorArr[vertexInd2].b +
                            bary[2] * colorArr[vertexInd3].b,
                            0
                        ) * (1 / 255.0f);
                    }
                }

            } else {
                //Top
                topZ = std::max(z, topZ);
            }
        }
    }
}

bool WmoGroupObject::checkIfInsideGroup(mathfu::vec4 &cameraVec4,
                                        mathfu::vec4 &cameraLocal,
                                        PointerChecker<C3Vector> &portalVerticles,
                                        PointerChecker<SMOPortal> &portalInfos,
                                        PointerChecker<SMOPortalRef> &portalRels,
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
    auto &nodes = this->m_geom->bsp_nodes;
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
            if (nodes[nodeId].planeType == 0) {
                if (cameraLocal[0] < nodes[nodeId].fDist) {
                    nodeId = nodes[nodeId].children[0];
                } else {
                    nodeId = nodes[nodeId].children[1];
                }
            } else if (nodes[nodeId].planeType == 1) {
                if (cameraLocal[1] < nodes[nodeId].fDist) {
                    nodeId = nodes[nodeId].children[0];
                } else {
                    nodeId = nodes[nodeId].children[1];
                }
            } else if (nodes[nodeId].planeType == 2) {
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


void WmoGroupObject::checkDoodads(M2ObjectListContainer &wmoM2Candidates) {
    if (!m_loaded) return;

    mathfu::vec4 ambientColor = getAmbientColor();


    for (auto &doodad : this->m_doodads) {
        if (doodad != nullptr) {
            if (this->getDontUseLocalLightingForM2()) {
                doodad->setUseLocalLighting(false);
            } else {
                doodad->setUseLocalLighting(true);
                doodad->setAmbientColorOverride(ambientColor, true);
            }

            wmoM2Candidates.addCandidate(doodad);
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

void WmoGroupObject::collectMeshes(std::vector<HGMesh> &opaqueMeshes, std::vector<HGSortableMesh> &transparentMeshes, std::vector<HGSortableMesh> &liquidMeshes, int renderOrder) {
    if (!m_loaded) return;
    for (auto &i : this->m_meshArray) {
        opaqueMeshes.push_back(i);
    }
    for (auto &i : this->m_sortableMeshArray) {
        if (i->getIsTransparent()) {
            opaqueMeshes.push_back(i);
        } else {
            transparentMeshes.push_back(i);
        }
    }

    for (auto const &liquidInstance : m_liquidInstances) {
        liquidInstance->collectMeshes(liquidMeshes);
    }
}

mathfu::vec4 WmoGroupObject::getAmbientColor() {
    if (!m_geom->mogp->flags.EXTERIOR && !m_geom->mogp->flags.EXTERIOR_LIT) {
        mathfu::vec4 ambColor;

        ambColor = mathfu::vec4(m_wmoApi->getAmbientColor(), 1.0);
        if ((m_geom->use_replacement_for_header_color == 1) && (*(int *) &m_geom->replacement_for_header_color != -1)) {
            ambColor = mathfu::vec4(
                ((float) m_geom->replacement_for_header_color.r / 255.0f),
                ((float) m_geom->replacement_for_header_color.g / 255.0f),
                ((float) m_geom->replacement_for_header_color.b / 255.0f),
                ((float) m_geom->replacement_for_header_color.a / 255.0f)
            );
        }

        return ambColor;
    }
    return mathfu::vec4(0,0,0,0);
}

void AdjustLighting(const mathfu::vec3 color_in, mathfu::vec3 &color_out_0, uint8_t a4, mathfu::vec3 &color_out_1, uint8_t a6)
{
    float maxInputComponent = std::max<float>(color_in.x, std::max<float>(color_in.y, color_in.z));

    uint8_t v10 = 1;
    color_out_0 = color_in;
    if ( maxInputComponent > 0 )
    {
        v10 = std::floor(maxInputComponent * 255.0f);
    }
    if ( v10 < a4 )
    {
        auto hsv = MathHelper::rgb2hsv(color_in);
        hsv.v = (float)((float)(a4) / (float)(v10)) * hsv.v;
        color_out_0 = MathHelper::hsv2rgb(hsv);
    }
    if ( v10 <= a6 )
    {
        color_out_1 = color_in;
    }
    else
    {
        float v12 = (float)((float)a6) / (float)v10;
        color_out_1 = v12 * color_in;
    }
}

void WmoGroupObject::assignInteriorParams(std::shared_ptr<M2Object> m2Object) {
    mathfu::vec4 ambientColor = getAmbientColor();

    if (!m2Object->setUseLocalLighting(true)) return;

    if (!m2Object->getInteriorAmbientWasSet()) {
        if (m_geom->colorArray != nullptr) {
            int nodeId = 0;
            auto &nodes = this->m_geom->bsp_nodes;
            MOGP *groupInfo = this->m_geom->mogp;
            std::vector<int> bspLeafList;

            float epsilon = 1;
            mathfu::vec4 cameraLocal = mathfu::vec4(m2Object->getLocalPosition(), 0);
            mathfu::vec3 cameraBBMin(cameraLocal[0] - epsilon, cameraLocal[1] - epsilon, groupInfo->boundingBox.min.z);
            mathfu::vec3 cameraBBMax(cameraLocal[0] + epsilon, cameraLocal[1] + epsilon, groupInfo->boundingBox.max.z);

            CAaBox cameraBB;
            cameraBB.max = cameraBBMax;
            cameraBB.min = cameraBBMin;

            float topZ;
            float bottomZ;

            int initLen = -1;
            PointerChecker<SMOPortalRef> temp = PointerChecker<SMOPortalRef>(initLen);
            PointerChecker<SMOPortal> temp2 = initLen;

            mathfu::vec4 mocvColor(0, 0, 0, 0);
            WmoGroupObject::queryBspTree(cameraBB, nodeId, nodes, bspLeafList);
            WmoGroupObject::getBottomVertexesFromBspResult(
                temp2, temp, bspLeafList, cameraLocal, topZ, bottomZ, mocvColor, false);

            if (bottomZ < 99999) {
                mocvColor = mathfu::vec4(mocvColor.x, mocvColor.y, mocvColor.z, 0);

                mathfu::vec3 someColor;
                mathfu::vec3 colorCombined = (2.0f * mocvColor.xyz()) + ambientColor.xyz();
                colorCombined = mathfu::vec3::Min(mathfu::vec3(1.0f, 1.0f, 1.0f), colorCombined);
                mathfu::vec3 ambientOut = {0,0,0};
                AdjustLighting(colorCombined, someColor, 0xA8u, ambientOut, 0x60u);

                ambientColor = mathfu::vec4(ambientOut.x, ambientOut.y, ambientOut.z, ambientColor.w);

//                ambientColor += mocvColor;
            }
        }

        m2Object->setAmbientColorOverride(ambientColor, true);
        m2Object->setInteriorAmbientWasSet(true);
    }

//    mathfu::vec4 interiorSunDir = mathfu::vec4(-0.30822f, -0.30822f, -0.89999998f, 0);
//    mathfu::mat4 transformMatrix = m_api->getViewMat();
//    interiorSunDir = transformMatrix.Transpose().Inverse() * interiorSunDir;
//    interiorSunDir = mathfu::vec4(interiorSunDir.xyz() * (1.0f / interiorSunDir.xyz().Length()), 0.0f);
//    m2Object->setSunDirOverride(interiorSunDir, true);
}
