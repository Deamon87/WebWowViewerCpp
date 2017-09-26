//
// Created by deamon on 10.07.17.
//

#include "wmoGroupObject.h"
#include "../algorithms/mathHelper.h"
#include <algorithm>

void WmoGroupObject::update() {
    if (!this->m_loaded) {
        if (m_geom != nullptr && m_geom->isLoaded()) {
            this->postLoad();
            this->m_loaded = true;
            return;
        }

        this->startLoading();
        return;
    }
}

void WmoGroupObject::draw(SMOMaterial *materials, std::function<BlpTexture &(int materialId)> getTextureFunc) {
    if (!this->m_loaded) return;
    if (m_geom->batchesLen <= 0) return;
    m_geom->draw(m_api, materials, getTextureFunc);
}

void WmoGroupObject::startLoading() {
    if (!this->m_loading) {
        this->m_loading = true;

        m_geom = m_api->getWmoGroupGeomCache()->get(m_fileName);
    }
}

void WmoGroupObject::postLoad() {

    this->m_dontUseLocalLightingForM2 = ((m_geom->mogp->flags & 0x40) > 0) || ((m_geom->mogp->flags & 0x8) > 0);
    this->createWorldGroupBB(m_geom->mogp->boundingBox, *m_modelMatrix);
    this->loadDoodads();
}

void WmoGroupObject::loadDoodads() {
    if (this->m_geom->doodadRefsLen <= 0) return;
    if (m_wmoApi == nullptr) return;

    m_doodads = std::vector<M2Object *>(this->m_geom->doodadRefsLen, nullptr);

    //Load all doodad from MOBR
    for (int i = 0; i < this->m_geom->doodadRefsLen; i++) {
        m_doodads[i] = m_wmoApi->getDoodad(this->m_geom->doodadRefs[i]);
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
}

void WmoGroupObject::updateWorldGroupBBWithM2() {
//    var doodadRefs = this.wmoGeom.wmoGroupFile.doodadRefs;
//    var mogp = this.wmoGeom.wmoGroupFile.mogp;
//    var groupAABB = this.worldGroupBorder;
//
//    var dontUseLocalLighting = ((mogp.flags & 0x40) > 0) || ((mogp.flags & 0x8) > 0);
//
//    for (var j = 0; j < this.wmoDoodads.length; j++) {
//        var mdxObject = this.wmoDoodads[j];
//        //1. Update the mdx
//        //If at least one exterior WMO group reference the doodad - do not use the diffuse lightning from modd chunk
//        if (dontUseLocalLighting) {
//            mdxObject.setUseLocalLighting(false);
//        }
//
//        if (!mdxObject.loaded) continue; //corrupted :(
//
//        //2. Update the world group BB
//        groupAABB[0] = vec3.fromValues(Math.min(mdxObject.aabb[0][0],groupAABB[0][0]),
//                                       Math.min(mdxObject.aabb[0][1],groupAABB[0][1]),
//                                       Math.min(mdxObject.aabb[0][2],groupAABB[0][2]));
//
//        groupAABB[1] = vec3.fromValues(Math.max(mdxObject.aabb[1][0],groupAABB[1][0]),
//                                       Math.max(mdxObject.aabb[1][1],groupAABB[1][1]),
//                                       Math.max(mdxObject.aabb[1][2],groupAABB[1][2]));
//    }
}

bool WmoGroupObject::checkGroupFrustum(mathfu::vec4 &cameraPos,
                                       std::vector<mathfu::vec4> &frustumPlanes,
                                       std::vector<mathfu::vec3> &points,
                                       std::set<M2Object *> &wmoM2Candidates) {
    CAaBox &bbArray = this->m_worldGroupBorder;

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

    if (drawDoodads) {
        this->checkDoodads(wmoM2Candidates);
    }
    return drawGroup;
}

bool WmoGroupObject::checkIfInsidePortals(mathfu::vec3 point,
                                          C3Vector *portalVerticles,
                                          SMOPortal *portalInfos,
                                          SMOPortalRef *portalRels) {
    int moprIndex = this->m_geom->mogp->moprIndex;
    int numItems = this->m_geom->mogp->moprCount;

    bool insidePortals = true;
    for (int j = moprIndex; j < moprIndex + numItems; j++) {
        SMOPortalRef *relation = &portalRels[j];
        SMOPortal *portalInfo = &portalInfos[relation->portal_index];

        int nextGroup = relation->group_index;
        C4Plane plane = portalInfo->plane;

        float minX = 99999;
        float minY = 99999;
        float minZ = 99999;
        float maxX = -99999;
        float maxY = -99999;
        float maxZ = -99999;


        int base_index = portalInfo->base_index;
        for (int k = 0; k < portalInfo->index_count; k++) {
            minX = std::min(minX, portalVerticles[base_index + k].x);
            minY = std::min(minY, portalVerticles[base_index + k].y);
            minZ = std::min(minZ, portalVerticles[base_index + k].z);

            maxX = std::max(maxX, portalVerticles[base_index + k].x);
            maxY = std::max(maxX, portalVerticles[base_index + k].y);
            maxZ = std::max(maxZ, portalVerticles[base_index + k].z);
        }

        CAaBox aaBox(C3Vector(mathfu::vec3(minX, minY, minZ)), C3Vector(mathfu::vec3(maxX, maxY, maxZ)));
        float distanceToBB = MathHelper::distanceFromAABBToPoint(aaBox, point);

        float dotResult = mathfu::vec3::DotProduct(mathfu::vec4(plane.planeVector).xyz(), point) + plane.planeVector.w;
        bool isInsidePortalThis = (relation->side < 0) ? (dotResult <= 0) : (dotResult >= 0);
        if (!isInsidePortalThis && (abs(dotResult) < 0.1) && (abs(distanceToBB) < 0.1)) {
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
        C3Vector *portalVerticles,
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
            if (!WmoGroupObject::checkIfInsidePortals(suggestedPoint, portalVerticles, portalInfos,
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

        for (int k = 0; k < portalInfo->index_count - 2; k++) {
            int portalIndex;
            portalIndex = base_index + 0;
            mathfu::vec3 point1 = mathfu::vec3(portalVerticles[portalIndex]);
            portalIndex = base_index + k + 1;
            mathfu::vec3 point2 = mathfu::vec3(portalVerticles[portalIndex]);
            portalIndex = base_index + k + 2;
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

    result = {min: bottomZ, max: topZ};

    return true;
}

bool WmoGroupObject::checkIfInsideGroup(mathfu::vec4 &cameraVec4,
                                        mathfu::vec4 &cameraLocal,
                                        C3Vector *portalVerticles,
                                        SMOPortal *portalInfos,
                                        SMOPortalRef *portalRels,
                                        std::vector<WmoGroupResult> &candidateGroups) {

    if (!this->m_loaded) return false;

    CAaBox &bbArray = this->m_volumeWorldGroupBorder;
    MOGP *groupInfo = this->m_geom->mogp;
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

    int moprIndex = groupInfo->moprIndex;
    int numItems = groupInfo->moprCount;

    bool insidePortals = this->checkIfInsidePortals(cameraLocal.xyz(), portalVerticles, portalInfos, portalRels);
    if (!insidePortals) return false;

    //3. Query bsp tree for leafs around the position of object(camera)


    float epsilon = 1;
    mathfu::vec3 cameraBBMin(cameraLocal[0] - epsilon, cameraLocal[1] - epsilon, groupInfo->boundingBox.min.z);
    mathfu::vec3 cameraBBMax(cameraLocal[0] + epsilon, cameraLocal[1] + epsilon, groupInfo->boundingBox.min.z);

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
                cameraLocal, portalVerticles, portalInfos, portalRels, bspLeafList, topBottom);
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

        candidateGroups.push_back({
            topBottom : topBottom,
            groupId : m_groupNumber,
            bspLeafList : bspLeafList,
            nodeId: nodeId
        });

        return true;
    }

    return false;
//candidateGroups.push({'topBottom' : {topZ : 0, bottomZ : 0}, groupId : this.groupId, bspList : [], nodeId: 0});
}


bool WmoGroupObject::checkDoodads(std::set<M2Object *> &wmoM2Candidates) {
    for (int i = 0; i < this->m_doodads.size(); i++) {
        if (this->m_doodads[i] != nullptr) {
            if (this->m_dontUseLocalLightingForM2) {
//                this->m_doodads[i]->setUseLocalLighting(false);
            }
            wmoM2Candidates.insert(this->m_doodads[i]);
        }
    }
}

void WmoGroupObject::setWmoApi(IWmoApi *api) {
    m_wmoApi = api;
}
