//
// Created by deamon on 03.07.17.
//

#include "wmoObject.h"
#include "../../algorithms/mathHelper.h"
#include "../../shader/ShaderDefinitions.h"
#include "../../persistance/header/commonFileStructs.h"
#include "./../../../gapi/interface/IDevice.h"
#include <algorithm>

std::vector<mathfu::vec3> CreateOccluders(const HWmoGroupGeom groupGeom)
{
    std::vector<mathfu::vec3> points(0);

    if (groupGeom == nullptr || !groupGeom->isLoaded()) return points;
    for ( unsigned int mopy_index (0), movi_index (0)
            ; mopy_index < groupGeom->mopyLen
            ; ++mopy_index, ++movi_index
            )
    {
        points.push_back(mathfu::vec3(groupGeom->verticles[groupGeom->indicies[3*movi_index+0]]));
        points.push_back(mathfu::vec3(groupGeom->verticles[groupGeom->indicies[3*movi_index+1]]));
        points.push_back(mathfu::vec3(groupGeom->verticles[groupGeom->indicies[3*movi_index+2]]));
//        C3Vector points[3] =
//                {  groupGeom->verticles[groupGeom->indicies[movi_index]]
//                   groupGeom->verticles[groupGeom->indicies[movi_index]]
//                   groupGeom->verticles[groupGeom->indicies[movi_index]]
//                };
//
//        float avg ((points[0]->z + points[1]->z + points[2]->z) / 3.0);
//
//        unsigned int two_points[2];
//        unsigned int two_points_index (0);
//
//        for (unsigned int i (0); i < 3; ++i)
//        {
//            if (points[i]->z > avg)
//            {
//                two_points[two_points_index++] = i;
//            }
//        }
//
//        if (two_points_index > 1)
//        {
//            CMapObjOccluder* occluder (CMapObj::AllocOccluder());
//            occluder->p1 = points[two_points[0]];
//            occluder->p2 = points[two_points[1]];
//
//            append (this->occluders, occluder);
//        }

    }
    return points;
}


void WmoObject::startLoading() {
    if (!m_loading) {
        m_loading = true;


        Cache<WmoMainGeom> *wmoGeomCache = m_api->getWmoMainCache();
        if (!useFileId) {
            mainGeom = wmoGeomCache->get(m_modelName);
        } else {
            mainGeom = wmoGeomCache->getFileId(m_modelFileId);
        }

    }
}

M2Object *WmoObject::getDoodad(int index) {
    int doodadsSet = this->m_doodadSet;

    SMODoodadSet *doodadSetDef = &this->mainGeom->doodadSets[doodadsSet];
    if (index < doodadSetDef->firstinstanceindex
        || index > doodadSetDef->firstinstanceindex + doodadSetDef->numDoodads) return nullptr;

    int doodadIndex = index - doodadSetDef->firstinstanceindex;

    M2Object *doodadObject = this->m_doodadsArray[doodadIndex];
    if (doodadObject != nullptr) return doodadObject;


    SMODoodadDef *doodadDef = &this->mainGeom->doodadDefs[index];

    bool fileIdMode = false;
    int doodadfileDataId = 0;
    std::string fileName;
    if (this->mainGeom->doodadFileDataIds == nullptr && this->mainGeom->doodadFileDataIdsLen == 0) {
        assert(doodadDef->name_offset < this->mainGeom->doodadNamesFieldLen);
        fileName = std::string (&this->mainGeom->doodadNamesField[doodadDef->name_offset]);
    } else {
        doodadfileDataId = this->mainGeom->doodadFileDataIds[doodadDef->name_offset];
        fileIdMode = true;
    }

    M2Object *m2Object = new M2Object(m_api);
    m2Object->setDiffuseColor(doodadDef->color);
    m2Object->setLoadParams(0, {},{});
    if (fileIdMode) {
        m2Object->setModelFileId(doodadfileDataId);
    } else {
        m2Object->setModelFileName(fileName);
    }
    m2Object->createPlacementMatrix(*doodadDef, m_placementMatrix);
    m2Object->calcWorldPosition();

    this->m_doodadsArray[doodadIndex] = m2Object;

    return m2Object;
}
void WmoObject::createPlacementMatrix(SMMapObjDef &mapObjDef){
    mathfu::mat4 adtToWorldMat4 = MathHelper::getAdtToWorldMat4();

    mathfu::mat4 placementMatrix = mathfu::mat4::Identity();
    placementMatrix *= adtToWorldMat4;
    placementMatrix *= mathfu::mat4::FromTranslationVector(mathfu::vec3(mapObjDef.position));
    placementMatrix *= mathfu::mat4::FromScaleVector(mathfu::vec3(-1, 1, -1));

    placementMatrix *= MathHelper::RotationY(toRadian(mapObjDef.rotation.y-270));
    placementMatrix *= MathHelper::RotationZ(toRadian(-mapObjDef.rotation.x));
    placementMatrix *= MathHelper::RotationX(toRadian(mapObjDef.rotation.z-90));
    if (mapObjDef.unk != 0) {
        placementMatrix *= mathfu::mat4::FromScaleVector(mathfu::vec3(mapObjDef.unk / 1024.0f));
    }
    mathfu::mat4 placementInvertMatrix = placementMatrix.Inverse();

    m_placementInvertMatrix = placementInvertMatrix;
    m_placementMatrix = placementMatrix;

    //BBox is in ADT coordinates. We need to transform it first
    C3Vector &bb1 = mapObjDef.extents.min;
    C3Vector &bb2 = mapObjDef.extents.max;
    mathfu::vec4 bb1vec = mathfu::vec4(bb1.x, bb1.y, bb1.z, 1);
    mathfu::vec4 bb2vec = mathfu::vec4(bb2.x, bb2.y, bb2.z, 1);

    CAaBox worldAABB = MathHelper::transformAABBWithMat4(
            adtToWorldMat4, bb1vec, bb2vec);

    createBB(worldAABB);
}
void WmoObject::createPlacementMatrix(SMMapObjDefObj1 &mapObjDef){
    mathfu::mat4 adtToWorldMat4 = MathHelper::getAdtToWorldMat4();

    mathfu::mat4 placementMatrix = mathfu::mat4::Identity();
    placementMatrix *= adtToWorldMat4;
    placementMatrix *= mathfu::mat4::FromTranslationVector(mathfu::vec3(mapObjDef.position));
    placementMatrix *= mathfu::mat4::FromScaleVector(mathfu::vec3(-1, 1, -1));

    placementMatrix *= MathHelper::RotationY(toRadian(mapObjDef.rotation.y-270));
    placementMatrix *= MathHelper::RotationZ(toRadian(-mapObjDef.rotation.x));
    placementMatrix *= MathHelper::RotationX(toRadian(mapObjDef.rotation.z-90));

    if (mapObjDef.unk != 0) {
        placementMatrix *= mathfu::mat4::FromScaleVector(mathfu::vec3(mapObjDef.unk / 1024.0f));
    }

    mathfu::mat4 placementInvertMatrix = placementMatrix.Inverse();

    m_placementInvertMatrix = placementInvertMatrix;
    m_placementMatrix = placementMatrix;

    //BBox is in ADT coordinates. We need to transform it first
//    C3Vector &bb1 = mapObjDef.extents.min;
//    C3Vector &bb2 = mapObjDef.extents.max;
//    mathfu::vec4 bb1vec = mathfu::vec4(bb1.x, bb1.y, bb1.z, 1);
//    mathfu::vec4 bb2vec = mathfu::vec4(bb2.x, bb2.y, bb2.z, 1);
//
    mathfu::vec4 bb1vec = mathfu::vec4(-1000,-1000,-1000, 1);
    mathfu::vec4 bb2vec = mathfu::vec4(1000, 1000,1000, 1);

    CAaBox worldAABB = MathHelper::transformAABBWithMat4(
            adtToWorldMat4, bb1vec, bb2vec);

    createBB(worldAABB);
}

void WmoObject::createGroupObjects(){
    groupObjects = std::vector<WmoGroupObject*>(mainGeom->groupsLen, nullptr);
    groupObjectsLod1 = std::vector<WmoGroupObject*>(mainGeom->groupsLen, nullptr);
    groupObjectsLod2 = std::vector<WmoGroupObject*>(mainGeom->groupsLen, nullptr);
    drawGroupWMO = std::vector<bool>(mainGeom->groupsLen, false);
    lodGroupLevelWMO = std::vector<int>(mainGeom->groupsLen, 0);

    std::string nameTemplate = m_modelName.substr(0, m_modelName.find_last_of("."));
    for(int i = 0; i < mainGeom->groupsLen; i++) {

        groupObjects[i] = new WmoGroupObject(this->m_placementMatrix, m_api, mainGeom->groups[i], i);
        groupObjects[i]->setWmoApi(this);
        if (mainGeom->gfids.size() > 1) {
            groupObjectsLod1[i] = new WmoGroupObject(this->m_placementMatrix, m_api, mainGeom->groups[i], i);
            groupObjectsLod1[i]->setWmoApi(this);
        } else {
            groupObjectsLod1[i] = nullptr;
        }
        if (mainGeom->gfids.size() > 2) {
            groupObjectsLod2[i] = new WmoGroupObject(this->m_placementMatrix, m_api, mainGeom->groups[i], i);
            groupObjectsLod2[i]->setWmoApi(this);
        } else {
            groupObjectsLod2[i] = nullptr;
        };

        if (useFileId) {
            groupObjects[i]->setModelFileId(mainGeom->gfids[0][i]);
            if (mainGeom->gfids.size() > 1) {
                if (mainGeom->gfids[1][i] == 0) {
                    delete groupObjectsLod1[i];
                    groupObjectsLod1[i] = nullptr;
                } else if (groupObjectsLod1[i] != nullptr)
                    groupObjectsLod1[i]->setModelFileId(mainGeom->gfids[1][i]);
            }

            if (mainGeom->gfids.size() > 2) {
                if (mainGeom->gfids[2][i] == 0) {
                    delete groupObjectsLod2[i];
                    groupObjectsLod2[i] = nullptr;
                } else if (groupObjectsLod2[i] != nullptr)
                    groupObjectsLod2[i]->setModelFileId(mainGeom->gfids[2][i]);
            }
        } else {
            std::string numStr = std::to_string(i);
            for (int j = numStr.size(); j < 3; j++) numStr = '0' + numStr;

            std::string groupFilename = nameTemplate + "_" + numStr + ".wmo";
            std::string groupFilenameLod1 = nameTemplate + "_" + numStr + "_lod1.wmo";
            std::string groupFilenameLod2 = nameTemplate + "_" + numStr + "_lod2.wmo";
            groupObjects[i]->setModelFileName(groupFilename);
            if (groupObjectsLod1[i] != nullptr)
                groupObjectsLod1[i]->setModelFileName(groupFilenameLod1);
            if (groupObjectsLod2[i] != nullptr)
                groupObjectsLod2[i]->setModelFileName(groupFilenameLod2);
        }
    }
}

void WmoObject::createWorldPortals() {

    int portalCnt = mainGeom->portalsLen;
    SMOPortal *portals = mainGeom->portals;
    C3Vector *portalVerticles = mainGeom->portal_vertices;

    if (portalCnt <= 0) return;
    geometryPerPortal = std::vector<PortalInfo_t>(portalCnt);

    for (int j = 0; j < portalCnt; j++) {
        SMOPortal *portalInfo = &portals[j];


        int base_index = portalInfo->base_index;
        std::vector <mathfu::vec3> portalVecs;
        for (int k = 0; k < portalInfo->index_count; k++) {
            mathfu::vec3 verticle = mathfu::vec3(
                portalVerticles[base_index + k].x,
                portalVerticles[base_index + k].y,
                portalVerticles[base_index + k].z);

            portalVecs.push_back(verticle);
        }

        //Calculate center of the portal
        mathfu::vec3 center(0,0,0);
        for (int k = 0; k < portalVecs.size(); k++) {
            center += portalVecs[k];
        }
        center *= 1.0f / (float)portalVecs.size();

        //Calculate create projection and calc simplified, sorted polygon
        mathfu::vec3 lookAt = center + mathfu::vec3(portalInfo->plane.planeGeneral.normal);
        mathfu::vec3 upVector = portalVecs[0] - center;

        mathfu::mat4 projMat = mathfu::mat4::LookAt(
                lookAt,
                center,
                upVector
        );
        mathfu::mat4 projMatInv = projMat.Inverse();

        std::vector <mathfu::vec3> portalTransformed(portalVecs.size());
        for (int k = 0; k < portalVecs.size(); k++) {
            portalTransformed[k] = (projMat * mathfu::vec4(portalVecs[k], 1.0)).xyz();
        }

        std::vector<mathfu::vec3> hulled = MathHelper::getHullPoints(portalTransformed);

        portalVecs.clear();
        for (int k = 0; k < hulled.size(); k++) {
            portalVecs.push_back((projMatInv * mathfu::vec4(hulled[k], 1.0)).xyz());
        }
        geometryPerPortal[j].sortedVericles = portalVecs;

        //Calc CAAbox
        mathfu::vec3 min(99999,99999,99999), max(-99999,-99999,-99999);

        for (int k = 0; k < portalVecs.size(); k++) {

            min = mathfu::vec3::Min(portalVecs[k], min);
            max = mathfu::vec3::Max(portalVecs[k], max);
        }

        CAaBox &aaBoxCopyTo = geometryPerPortal[j].aaBox;

        aaBoxCopyTo = CAaBox(C3Vector(min), C3Vector(max));;
    }
}

bool WmoObject::doPostLoad(int &groupsProcessedThisFrame) {
    if (!m_loaded) {
        if (mainGeom != nullptr && mainGeom->getIsLoaded()){
            this->createGroupObjects();
            this->createWorldPortals();
            this->createBB(mainGeom->header->bounding_box);
            this->createM2Array();

            m_loaded = true;
            m_loading = false;
            return true;
        } else {
            this->startLoading();
        }

        return false;
    }

    for (auto &groupObject : groupObjects) {
        if (groupsProcessedThisFrame > 3) return false;
        groupObject->doPostLoad();
    }
    for (auto &groupObjectLod : groupObjectsLod1) {
        if (groupsProcessedThisFrame > 3) return false;
        if (groupObjectLod != nullptr) {
            if (groupObjectLod->doPostLoad()) groupsProcessedThisFrame++;;
        }
    }
    for (auto &groupObjectLod2 : groupObjectsLod2) {
        if (groupsProcessedThisFrame > 3) return false;
        if (groupObjectLod2 != nullptr) {
            if (groupObjectLod2->doPostLoad()) groupsProcessedThisFrame++;;
        }
    }

    return false;
}

void WmoObject::update() {
    if (!m_loaded) return;

    for (int i= 0; i < groupObjects.size(); i++) {
        if(groupObjects[i] != nullptr) {
            groupObjects[i]->update();
        }
    }
    for (int i= 0; i < groupObjectsLod1.size(); i++) {
        if(groupObjectsLod1[i] != nullptr) {
            groupObjectsLod1[i]->update();
        }
    }
    for (int i= 0; i < groupObjectsLod2.size(); i++) {
        if(groupObjectsLod2[i] != nullptr) {
            groupObjectsLod2[i]->update();
        }
    }

}

void WmoObject::collectMeshes(std::vector<HGMesh> &renderedThisFrame){
    if (!m_loaded) return;

//    for (int i= 0; i < groupObjects.size(); i++) {
//        if(drawGroupWMO[i]) {
//           if (groupObjects[i] != nullptr && lodGroupLevelWMO[i] == 0) {
//               groupObjects[i]->collectMeshes(renderedThisFrame);
//           } else if (groupObjectsLod1[i] != nullptr && lodGroupLevelWMO[i] == 1) {
//               groupObjectsLod1[i]->collectMeshes(renderedThisFrame);
//           } else if (groupObjectsLod2[i] != nullptr && lodGroupLevelWMO[i] == 2) {
//               groupObjectsLod2[i]->collectMeshes(renderedThisFrame);
//           } else if (groupObjects[i] != nullptr) {
//               groupObjects[i]->collectMeshes(renderedThisFrame);
//           }
//        }
//    }
}

void WmoObject::drawDebugLights(){
    if (!m_loaded) return;

    /*
    auto drawPointsShader = m_api->getDrawPointsShader();

    glUniformMatrix4fv(drawPointsShader->getUnf("uPlacementMat"), 1, GL_FALSE, &this->m_placementMatrix[0]);

    if (!this->m_loaded) return;

    SMOLight * lights = getLightArray();

    std::vector<float> points;

    for (int i = 0; i < mainGeom->lightsLen; i++) {
        points.push_back(lights[i].position.x);
        points.push_back(lights[i].position.y);
        points.push_back(lights[i].position.z);
    }

    GLuint bufferVBO;
    glGenBuffers(1, &bufferVBO);
    glBindBuffer( GL_ARRAY_BUFFER, bufferVBO);
    if (points.size() > 0) {
        glBufferData(GL_ARRAY_BUFFER, points.size() * 4, &points[0], GL_STATIC_DRAW);
    }

     static float colorArr[4] = {0.819607843, 0.058, 0.058, 0.3};
    glUniform3fv(drawPointsShader->getUnf("uColor"), 1, &colorArr[0]);

#ifndef WITH_GLESv2
    glEnable( GL_PROGRAM_POINT_SIZE );
#endif
    glVertexAttribPointer(+drawPoints::Attribute::aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);  // position


    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    glDrawArrays(GL_POINTS, 0, points.size()/3.0f);

#ifndef WITH_GLESv2
    glDisable( GL_PROGRAM_POINT_SIZE );
#endif
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, GL_ZERO);
    glBindBuffer( GL_ARRAY_BUFFER, GL_ZERO);

    glDepthMask(GL_TRUE);

    glDeleteBuffers(1, &bufferVBO);



    for (int i= 0; i < groupObjects.size(); i++) {
        if(groupObjects[i] != nullptr) {
            groupObjects[i]->drawDebugLights();
        }
    }
 */
}


void WmoObject::drawTransformedPortalPoints(){
    return;
#ifndef CULLED_NO_PORTAL_DRAWING
    /*
    if (!m_loaded) return;

    std::vector<uint16_t> indiciesArray;
    std::vector<float> verticles;
    int k = 0;
    //int l = 0;
    std::vector<int> stripOffsets;
    stripOffsets.push_back(0);
    int verticleOffset = 0;
    int stripOffset = 0;
    for (int i = 0; i < this->interiorPortals.size(); i++) {
        //if (portalInfo.index_count != 4) throw new Error("portalInfo.index_count != 4");

        int verticlesCount = this->interiorPortals[i].portalVertices.size();
        if ((verticlesCount - 2) <= 0) {
            stripOffsets.push_back(stripOffsets[i]);
            continue;
        };

        for (int j =0; j < (((int)verticlesCount)-2); j++) {
            indiciesArray.push_back(verticleOffset+0);
            indiciesArray.push_back(verticleOffset+j+1);
            indiciesArray.push_back(verticleOffset+j+2);
        }
        stripOffset += ((verticlesCount-2) * 3);

        for (int j =0; j < verticlesCount; j++) {
            verticles.push_back(this->interiorPortals[i].portalVertices[j].x);
            verticles.push_back(this->interiorPortals[i].portalVertices[j].y);
            verticles.push_back(this->interiorPortals[i].portalVertices[j].z);
        }

        verticleOffset += verticlesCount;
        stripOffsets.push_back(stripOffset);
    }
    GLuint indexVBO;
    GLuint bufferVBO;
    glGenBuffers(1, &indexVBO);
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexVBO);
    if (indiciesArray.size() > 0) {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indiciesArray.size() * 2, &indiciesArray[0], GL_STATIC_DRAW);
    }
    glGenBuffers(1, &bufferVBO);
    glBindBuffer( GL_ARRAY_BUFFER, bufferVBO);
    if (verticles.size() > 0) {
        glBufferData(GL_ARRAY_BUFFER, verticles.size() * 4, &verticles[0], GL_STATIC_DRAW);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // default blend func

    glVertexAttribPointer(+drawPortalShader::Attribute::aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);  // position

    auto drawPortalShader = m_api->getPortalShader();
    static float colorArr[4] = {0.058, 0.058, 0.819607843, 0.3};
    glUniformMatrix4fv(drawPortalShader->getUnf("uPlacementMat"), 1, GL_FALSE, &this->m_placementMatrix[0]);
    glUniform4fv(drawPortalShader->getUnf("uColor"), 1, &colorArr[0]);

    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    int offset = 0;
    for (int i = 0; i < this->interiorPortals.size(); i++) {
        int indeciesLen = stripOffsets[i+1] - stripOffsets[i];

        glDrawElements(GL_TRIANGLES, indeciesLen, GL_UNSIGNED_SHORT, (void *)(offset * 2));

        offset += indeciesLen;
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, GL_ZERO);
    glBindBuffer( GL_ARRAY_BUFFER, GL_ZERO);

    glDeleteBuffers(1, &indexVBO);
    glDeleteBuffers(1, &bufferVBO);
     */
#endif
}

void WmoObject::drawTransformedAntiPortalPoints(){
    return;
#ifndef CULLED_NO_PORTAL_DRAWING
    /*
    if (!m_loaded) return;

    std::vector<uint16_t> indiciesArray;
    std::vector<float> verticles;
    int k = 0;
    //int l = 0;
    std::vector<int> stripOffsets;
    stripOffsets.push_back(0);
    int verticleOffset = 0;
    int stripOffset = 0;

    //

    for (int i = 0; i < this->antiPortals.size(); i++) {
        //if (portalInfo.index_count != 4) throw new Error("portalInfo.index_count != 4");

        int verticlesCount = this->antiPortals[i].portalVertices.size();
//        if ((verticlesCount - 2) <= 0) {
//            stripOffsets.push_back(stripOffsets[i]);
//            continue;
//        };
//
//        for (int j =0; j < (((int)verticlesCount)-2); j++) {
//            indiciesArray.push_back(verticleOffset+0);
//            indiciesArray.push_back(verticleOffset+j+1);
//            indiciesArray.push_back(verticleOffset+j+2);
//        }
//        stripOffset += ((verticlesCount-2) * 3);

        for (int j =0; j < verticlesCount; j++) {
            verticles.push_back(this->antiPortals[i].portalVertices[j].x);
            verticles.push_back(this->antiPortals[i].portalVertices[j].y);
            verticles.push_back(this->antiPortals[i].portalVertices[j].z);
        }

        verticleOffset += verticlesCount;
        stripOffsets.push_back(stripOffset);
    }
    GLuint indexVBO;
    GLuint bufferVBO;
//    glGenBuffers(1, &indexVBO);
//    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexVBO);
//    if (indiciesArray.size() > 0) {
//        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indiciesArray.size() * 2, &indiciesArray[0], GL_STATIC_DRAW);
//    }
    glGenBuffers(1, &bufferVBO);
    glBindBuffer( GL_ARRAY_BUFFER, bufferVBO);
    if (verticles.size() > 0) {
        glBufferData(GL_ARRAY_BUFFER, verticles.size() * 4, &verticles[0], GL_STATIC_DRAW);
    }

    glDisable(GL_BLEND);

    glVertexAttribPointer(+drawPortalShader::Attribute::aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);  // position

    auto drawPortalShader = m_api->getPortalShader();
    static float colorArr[4] = {0.819607843, 0.058, 0.058, 0.3};
    glUniformMatrix4fv(drawPortalShader->getUnf("uPlacementMat"), 1, GL_FALSE, &this->m_placementMatrix[0]);
    glUniform4fv(drawPortalShader->getUnf("uColor"), 1, &colorArr[0]);

    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    int offset = 0;
    for (int i = 0; i < this->antiPortals.size(); i++) {
//        int indeciesLen = stripOffsets[i+1] - stripOffsets[i];

//        glDrawElements(GL_TRIANGLES, indeciesLen, GL_UNSIGNED_SHORT, (void *)(offset * 2));
        glDrawArrays(GL_TRIANGLES, 0, verticles.size()*4);

//        offset += indeciesLen;
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, GL_ZERO);
    glBindBuffer( GL_ARRAY_BUFFER, GL_ZERO);

    glDeleteBuffers(1, &indexVBO);
    glDeleteBuffers(1, &bufferVBO);
          */
#endif

}

void WmoObject::setLoadingParam(SMMapObjDef &mapObjDef) {
    //this->m_placementMatrix = mathfu::mat4::Identity();
    createPlacementMatrix(mapObjDef);

    this->m_doodadSet = mapObjDef.doodadSet;
    this->m_nameSet = mapObjDef.nameSet;
}
void WmoObject::setLoadingParam(SMMapObjDefObj1 &mapObjDef) {
    //this->m_placementMatrix = mathfu::mat4::Identity();
    createPlacementMatrix(mapObjDef);

    this->m_doodadSet = mapObjDef.doodadSet;
    this->m_nameSet = mapObjDef.nameSet;
}

HGTexture WmoObject::getTexture(int textureId, bool isSpec) {
    if (textureId < 0 || (mainGeom->textureNamesField != nullptr && textureId >= mainGeom->textureNamesFieldLen)) {
        debuglog("Non valid textureindex for WMO")
        return nullptr;
    };

    std::unordered_map<int, HGTexture> &textureCache = diffuseTextures;
    if (isSpec) {
        textureCache = specularTextures;
    }

    auto i = textureCache.find(textureId);
    if (i != textureCache.end()) {
        return i->second;
    }
    HBlpTexture texture;
    if (mainGeom->textureNamesField != nullptr) {
        std::string materialTexture(&mainGeom->textureNamesField[textureId]);
        if (materialTexture == "") return nullptr;

        if (isSpec) {
            materialTexture = materialTexture.substr(0, materialTexture.length() - 4) + "_s.blp";
        }

        texture = m_api->getTextureCache()->get(materialTexture);
    } else {
        texture = m_api->getTextureCache()->getFileId(textureId);
    }

    HGTexture hgTexture = m_api->getDevice()->createBlpTexture(texture, true, true);
    textureCache[textureId] = hgTexture;

    return hgTexture;
}

void WmoObject::createBB(CAaBox bbox) {
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

    CAaBox worldAABB = MathHelper::transformAABBWithMat4(m_placementMatrix, bb1vec, bb2vec);

    this->m_bbox = worldAABB;
}

void WmoObject::updateBB() {
    CAaBox &AABB = this->m_bbox;
//
//    var dontUseLocalLighting = ((mogp.flags & 0x40) > 0) || ((mogp.flags & 0x8) > 0);
//
    for (int j = 0; j < this->groupObjects.size(); j++) {
        WmoGroupObject* wmoGroupObject= this->groupObjects[j];


        CAaBox groupAAbb = wmoGroupObject->getWorldAABB();

        //2. Update the world group BB
        AABB.min = mathfu::vec3_packed(mathfu::vec3(std::min(groupAAbb.min.x,AABB.min.x),
                                                         std::min(groupAAbb.min.y,AABB.min.y),
                                                         std::min(groupAAbb.min.z,AABB.min.z)));

        AABB.max = mathfu::vec3_packed(mathfu::vec3(std::max(groupAAbb.max.x,AABB.max.x),
                                                         std::max(groupAAbb.max.y,AABB.max.y),
                                                         std::max(groupAAbb.max.z,AABB.max.z)));
    }
}

void WmoObject::createM2Array() {
    this->m_doodadsArray = std::vector<M2Object*>(this->mainGeom->doodadDefsLen, nullptr);
}

void WmoObject::postWmoGroupObjectLoad(int groupId, int lod) {
    //1. Create portal verticles from geometry
}

void WmoObject::checkGroupDoodads(int groupId, mathfu::vec4 &cameraVec4,
                                  std::vector<mathfu::vec4> &frustumPlane,
                                  std::vector<M2Object *> &m2Candidates) {
    WmoGroupObject *groupWmoObject = groupObjects[groupId];
    if (groupWmoObject != nullptr && groupWmoObject->getIsLoaded()) {
        const std::vector <M2Object *> *doodads = groupWmoObject->getDoodads();

        mathfu::vec4 ambientColor = groupWmoObject->getAmbientColor() ;

        for (int j = 0; j < doodads->size(); j++) {
            M2Object *m2Object = doodads->at(j);
            if (!m2Object) continue;
            if (groupWmoObject->getDontUseLocalLightingForM2()) {
                m2Object->setUseLocalLighting(false);
            } else {
                m2Object->setUseLocalLighting(true);
                m2Object->setAmbientColorOverride(ambientColor, true);
            }

            m2Candidates.push_back(m2Object);
        }
    }
}


//Must be called only once per frame
void WmoObject::resetTraversedWmoGroups() {
    if (!m_loaded)
        return;

    // 1. Create array of visibility with all false
    uint32_t portalCount = (uint32_t) std::max(0, this->mainGeom->portalsLen);
    transverseVisitedPortals = std::vector<bool>(portalCount, false);

    uint32_t doodadsCount = (uint32_t) std::max(0, this->mainGeom->doodadDefsLen);
    collectedM2s = std::vector<int>(doodadsCount, false);
}

bool WmoObject::startTraversingWMOGroup(
    mathfu::vec4 &cameraVec4,
    mathfu::mat4 &viewPerspectiveMat,
    int groupId,
    int globalLevel,
    int &renderOrder,
    bool traversingFromInterior,
    std::vector<InteriorView> &interiorViews,
    ExteriorView &exteriorView
) {
    if (!m_loaded)
        return true;

    std::vector<InteriorView> createdInteriorViews = std::vector<InteriorView>(mainGeom->groupsLen);

    //CurrentVisibleM2 and visibleWmo is array of global m2 objects, that are visible after frustum
    mathfu::vec4 cameraLocal = this->m_placementInvertMatrix * cameraVec4;

    mathfu::mat4 transposeModelMat = this->m_placementMatrix.Transpose();
    mathfu::mat4 inverseTransposeModelMat = this->m_placementInvertMatrix.Transpose();

    //For exterior cull
    std::vector<mathfu::vec4> frustumPlanesExt = MathHelper::getFrustumClipsFromMatrix(viewPerspectiveMat);
    std::vector<mathfu::vec3> frustumPointsExt = MathHelper::calculateFrustumPointsFromMat(viewPerspectiveMat);

    //For interior cull
    mathfu::mat4 MVPMat = viewPerspectiveMat*this->m_placementMatrix;
    std::vector<mathfu::vec3> frustumPointsLocal = MathHelper::calculateFrustumPointsFromMat(MVPMat);
    std::vector<mathfu::vec4> frustumPlanes = MathHelper::getFrustumClipsFromMatrix(MVPMat);
    mathfu::vec4 headOfPyramidLocal = mathfu::vec4(MathHelper::getIntersection(
        frustumPointsLocal[4], frustumPointsLocal[7],
        frustumPointsLocal[3], frustumPointsLocal[0]
    ), 1.0);

    if (traversingFromInterior) {
        InteriorView &interiorView = createdInteriorViews[groupId];
        WmoGroupObject *nextGroupObject = groupObjects[groupId];
        //5.1 The portal is into interior wmo group. So go on.
        if (!interiorView.viewCreated) {
            interiorView.viewCreated = true;
            interiorView.drawnWmos.push_back(nextGroupObject);
            interiorView.portalIndex = -1;
            interiorView.frustumPlanes.push_back(frustumPlanesExt);
        }

        if (globalLevel+1 >= interiorView.level) {
            interiorView.level = globalLevel + 1;
        } else {
            assert("BVH is not working. Something is wrong!");
        }


        this->transverseGroupWMO(
            groupId,
            traversingFromInterior,
            createdInteriorViews,
            exteriorView,
            cameraVec4,
            cameraLocal,
            inverseTransposeModelMat,
            transverseVisitedPortals,
            frustumPlanes,
            globalLevel,
            0);
    } else {
        if (!exteriorView.viewCreated) {
            exteriorView.viewCreated = true;
        }

        if (globalLevel+1 >= exteriorView.level) {
            exteriorView.level = globalLevel + 1;
        } else {
            assert("BVH is not working. Something is wrong!");
        }


        for (int i = 0; i< mainGeom->groupsLen; i++) {
            if ((mainGeom->groups[i].flags.EXTERIOR) > 0) { //exterior
                if (this->groupObjects[i] != nullptr && this->groupObjects[i]->checkGroupFrustum(cameraVec4, frustumPlanesExt, frustumPointsExt)) {
                    exteriorView.drawnWmos.push_back(this->groupObjects[i]);

                    this->transverseGroupWMO(
                        i,
                        false,
                        createdInteriorViews,
                        exteriorView,
                        cameraVec4,
                        cameraLocal,
                        inverseTransposeModelMat,
                        transverseVisitedPortals,
                        frustumPlanes,
                        globalLevel,
                        0);
                }
            }
        }
    }

    //Process results
    std::sort(createdInteriorViews.begin(), createdInteriorViews.end(), [](const InteriorView &a, const InteriorView &b) -> bool {
        if (a.viewCreated != b.viewCreated) {
            return a.viewCreated > b.viewCreated;
        }
        return false;
    });
    for (int i = 0; i < createdInteriorViews.size(); i++) {
        if (!createdInteriorViews[i].viewCreated) {
            createdInteriorViews.resize(i);
            break;
        }
    }
    std::sort(createdInteriorViews.begin(), createdInteriorViews.end(), [](const InteriorView &a, const InteriorView &b) -> bool {
        if (a.level != b.level) {
            return a.level < b.level;
        }
        return false;
    });

    for (auto &createdInteriorView : createdInteriorViews) {
        createdInteriorView.renderOrder = renderOrder++;
    }

    bool result = createdInteriorViews.size() > 0;

    if (!exteriorView.viewCreated && exteriorView.level > 0) {
        exteriorView.viewCreated = true;
        exteriorView.renderOrder = renderOrder++;
        result = true;
    }

    //Copy all internal views to whole array
    std::copy(createdInteriorViews.begin(), createdInteriorViews.end(), std::back_inserter(interiorViews));

    //M2s will be collected later from separate function call
    return true;
}

void WmoObject::transverseGroupWMO(
    int groupId,
    bool traversingStartedFromInterior,
    std::vector<InteriorView> &allInteriorViews, //GroupIndex as index
    ExteriorView &exteriorView,
    mathfu::vec4 &cameraVec4,
    mathfu::vec4 &cameraLocal,
    mathfu::mat4 &inverseTransposeModelMat,
    std::vector<bool> &transverseVisitedPortals,
    std::vector<mathfu::vec4> &localFrustumPlanes,
    int globalLevel,
    int localLevel
) {

    if (localLevel > 8) return;

    if (groupObjects[groupId] == nullptr || !groupObjects[groupId]->getIsLoaded()) {
        //The group has not been loaded yet
        return;
    }

    //2. Loop through portals of current group
    int moprIndex = groupObjects[groupId]->getWmoGroupGeom()->mogp->moprIndex;
    int numItems = groupObjects[groupId]->getWmoGroupGeom()->mogp->moprCount;

    for (int j = moprIndex; j < moprIndex+numItems; j++) {
        SMOPortalRef * relation = &mainGeom->portalReferences[j];
        SMOPortal * portalInfo = &mainGeom->portals[relation->portal_index];

        int nextGroup = relation->group_index;
        C4Plane &plane = portalInfo->plane;

        //Skip portals we already visited
        if (transverseVisitedPortals[relation->portal_index]) continue;

        //Local coordinanes plane DOT local camera
        const mathfu::vec4 planeV4 = mathfu::vec4(plane.planeVector);
        float dotResult = mathfu::vec4::DotProduct(planeV4, cameraLocal);
        //dotResult = dotResult + relation.side * 0.01;
        bool isInsidePortalThis = (relation->side < 0) ? (dotResult <= 0) : (dotResult >= 0);
        if (!isInsidePortalThis) continue;

        //2.1 If portal has less than 4 vertices - skip it(invalid?)
        if (portalInfo->index_count < 4) continue;

        //2.2 Check if Portal BB made from portal vertexes intersects frustum
        std::vector<mathfu::vec3> portalVerticesVec;
        std::transform(
            geometryPerPortal[relation->portal_index].sortedVericles.begin(),
            geometryPerPortal[relation->portal_index].sortedVericles.end(),
            std::back_inserter(portalVerticesVec),
            [](mathfu::vec3 d) -> mathfu::vec3 { return d;}
            );

        bool visible = MathHelper::planeCull(portalVerticesVec, localFrustumPlanes);

        const float dotepsilon = pow(1.5f, 2);
        bool hackCondition = (fabs(dotResult) > dotepsilon);
//        hackCondition = true;

        if (!visible && hackCondition) continue;

        transverseVisitedPortals[relation->portal_index] = true;

        int lastFrustumPlanesLen = localFrustumPlanes.size();

        //3. Construct frustum planes for this portal
        std::vector<mathfu::vec4> thisPortalPlanes;
        thisPortalPlanes.reserve(lastFrustumPlanesLen);

        if (hackCondition) {
            bool flip = (relation->side > 0);

            int portalCnt = portalVerticesVec.size();
            for (int i = 0; i < portalCnt; ++i) {
                int i2 = (i + 1) % portalCnt;

                mathfu::vec4 n = MathHelper::createPlaneFromEyeAndVertexes(cameraLocal.xyz(), portalVerticesVec[i],
                                                                           portalVerticesVec[i2]);

                if (flip) {
                    n *= -1.0f;
                }

                thisPortalPlanes.push_back(n);
            }
        } else {
            // If camera is too close - just use usual frustums
            thisPortalPlanes = localFrustumPlanes;
        }

        //Transform local planes into world planes to use with frustum culling of M2Objects
        std::vector<mathfu::vec4> worldSpaceFrustumPlanes;

        std::transform(thisPortalPlanes.begin(), thisPortalPlanes.end(),
                       std::back_inserter(worldSpaceFrustumPlanes),
                       [&](mathfu::vec4 p) -> mathfu::vec4 {
                           return inverseTransposeModelMat * p;
                       }
        );

        //5. Traverse next
        WmoGroupObject *nextGroupObject = groupObjects[nextGroup];
        SMOGroupInfo &nextGroupInfo = mainGeom->groups[nextGroup];
        if ((nextGroupInfo.flags.INTERIOR) > 0) {
            InteriorView &interiorView = allInteriorViews[nextGroup];
            //5.1 The portal is into interior wmo group. So go on.
            if (!interiorView.viewCreated) {
                interiorView.viewCreated = true;
                interiorView.drawnWmos.push_back(nextGroupObject);
                interiorView.portalIndex = relation->portal_index;
            }

            interiorView.portalVertices.push_back(portalVerticesVec);
            interiorView.frustumPlanes.push_back(worldSpaceFrustumPlanes);
            if (globalLevel+1 >= interiorView.level) {
                interiorView.level = globalLevel + 1;
            } else {
                assert("BVH is not working. Something is wrong!");
            }

            transverseGroupWMO(
                nextGroup,
                traversingStartedFromInterior,
                allInteriorViews,
                exteriorView,
                cameraVec4,
                cameraLocal,
                inverseTransposeModelMat,
                transverseVisitedPortals,
                thisPortalPlanes,
                globalLevel + 1,
                localLevel + 1
            );

        } else if (((nextGroupInfo.flags.EXTERIOR) > 0) && traversingStartedFromInterior) {
            //5.2 The portal is from interior into exterior wmo group.
            //Make sense to add only if whole traversing process started from interior
            if (!exteriorView.viewCreated) {
                exteriorView.level = globalLevel + 1;

                exteriorView.portalVertices.push_back(portalVerticesVec);
                exteriorView.frustumPlanes.push_back(worldSpaceFrustumPlanes);
            }
        }
    }
}

bool WmoObject::isGroupWmoInterior(int groupId) {
    SMOGroupInfo *groupInfo = &this->mainGeom->groups[groupId];
    bool result = ((groupInfo->flags.INTERIOR) != 0);
    return result;
}

bool WmoObject::getGroupWmoThatCameraIsInside (mathfu::vec4 cameraVec4, WmoGroupResult &groupResult) {
        if (this->groupObjects.size() ==0) return false;

        //Transform camera into local coordinates
        mathfu::vec4 cameraLocal = this->m_placementInvertMatrix * cameraVec4 ;

        //Check if camera inside wmo
        bool isInsideWMOBB = (
        cameraLocal[0] > this->mainGeom->header->bounding_box.min.x && cameraLocal[0] < this->mainGeom->header->bounding_box.max.x &&
        cameraLocal[1] > this->mainGeom->header->bounding_box.min.y && cameraLocal[1] < this->mainGeom->header->bounding_box.max.y &&
        cameraLocal[2] > this->mainGeom->header->bounding_box.min.z && cameraLocal[2] < this->mainGeom->header->bounding_box.max.z
        );
        if (!isInsideWMOBB) return false;

        //Loop
        int wmoGroupsInside = 0;
        int interiorGroups = 0;
        int lastWmoGroupInside = -1;
        std::vector<WmoGroupResult> candidateGroups;

        for (int i = 0; i < this->groupObjects.size(); i++) {
            if (this->groupObjects[i] == nullptr) continue;
            this->groupObjects[i]->checkIfInsideGroup(
                    cameraVec4,
                    cameraLocal,
                    this->mainGeom->portal_vertices,
                    this->mainGeom->portals,
                    this->mainGeom->portalReferences,
                    candidateGroups);
            /* Test: iterate through portals in 5 value radius and check if it fits there */
        }

        //6. Iterate through result group list and find the one with maximal bottom z coordinate for object position
        float minDist = 999999;
        bool result = false;
        for (int i = 0; i < candidateGroups.size(); i++) {
            WmoGroupResult *candidate = &candidateGroups[i];
            SMOGroupInfo *groupInfo = &this->mainGeom->groups[candidate->groupIndex];
            /*if ((candidate.topBottom.bottomZ < 99999) && (candidate.topBottom.topZ > -99999)){
                if ((cameraLocal[2] < candidateGroups[i].topBottom.bottomZ) || (cameraLocal[2] > candidateGroups[i].topBottom.topZ))
                    continue
            } */
            if (candidate->topBottom.min < 99999) {
                float dist = cameraLocal[2] - candidate->topBottom.min;
                if (dist > 0 && dist < minDist) {
                    result = true;
                    minDist = dist;
                    groupResult = *candidate;
                }
            }
        }


        return result;
}

void WmoObject::setModelFileName(std::string modelName) {
    m_modelName = modelName;
}

void WmoObject::setModelFileId(int fileId) {
    useFileId = true;
    m_modelFileId = fileId;
}

void WmoObject::fillLodGroup(mathfu::vec3 &cameraLocal) {
    for(int i = 0; i < mainGeom->groupsLen; i++) {
        if (drawGroupWMO[i]) {
            float distance = MathHelper::distanceFromAABBToPoint(groupObjects[i]->getLocalAABB(), cameraLocal);
            if (distance > 800) {
                lodGroupLevelWMO[i] = 2;
            } else if (distance > 500) {
                lodGroupLevelWMO[i] = 1;
            } else {
                lodGroupLevelWMO[i] = 0;
            }

            lodGroupLevelWMO[i] = std::min(lodGroupLevelWMO[i], getWmoHeader()->numLod-1);
        } else {
            lodGroupLevelWMO[i] = 0;
        }
    }

}
float distance(C4Plane &plane, C3Vector vertex) {
    return (float) sqrt(mathfu::vec4::DotProduct(mathfu::vec4(plane.planeVector), mathfu::vec4(vertex.x, vertex.y, vertex.z, 1.0)));
}


void attenuateTransVerts(HWmoMainGeom &mainGeom, WmoGroupGeom& wmoGroupGeom) {

    if (!wmoGroupGeom.mogp->transBatchCount)
    {
        return;
    }

//    for ( std::size_t vertex_index (0); vertex_index < wmoGroupGeom.batches[wmoGroupGeom.mogp->transBatchCount-1].last_vertex; ++vertex_index) {
//        float opacity_accum (0.0);
//
//        for ( std::size_t portal_ref_index (wmoGroupGeom.mogp->moprIndex);
//              portal_ref_index < (wmoGroupGeom.mogp->moprIndex + wmoGroupGeom.mogp->moprCount);
//              ++portal_ref_index)
//        {
//            SMOPortalRef const& portalRef (mainGeom->portalReferences[portal_ref_index]);
//            SMOPortal const& portal (mainGeom->portals[portalRef.portal_index]);
//            C3Vector const& vertex (wmoGroupGeom.verticles[vertex_index]);
//
//            float const portal_to_vertex (distance(portal.plane, vertex));
//
//            C3Vector vertex_to_use (vertex);
//
//            if (portal_to_vertex > 0.001 || portal_to_vertex < -0.001)
//            {
//                C3Ray ray ( C3Ray::FromStartEnd
//                                    ( vertex
//                                            , vertex
//                                              + (portal_to_vertex > 0 ? -1 : 1) * portal.plane.planeGeneral.normal
//                                            , 0
//                                    )
//                );
//                NTempest::Intersect
//                        (ray, &portal.plane, 0LL, &vertex_to_use, 0.0099999998);
//            }
//
//            float distance_to_use;
//
//            if ( NTempest::Intersect ( vertex_to_use
//                    , &mainGeom->portal_vertices[portal.base_index]
//                    , portal.index_count
//                    , C3Vector::MajorAxis (portal.plane.normal)
//            )
//                    )
//            {
//                distance_to_use = portalRef.side * distance (portal.plane, vertex);
//            }
//            else
//            {
//                distance_to_use = NTempest::DistanceFromPolygonEdge
//                        (vertex, &mainGeom->portal_vertices[portal.base_index], portal.index_count);
//            }
//
//            if (mainGeom->groups[portalRef.group_index].flags.EXTERIOR ||
//                mainGeom->groups[portalRef.group_index].flags.EXTERIOR_LIT)
//            {
//                float v25 (distance_to_use >= 0.0 ? distance_to_use / 6.0f : 0.0f);
//                if ((1.0 - v25) > 0.001)
//                {
//                    opacity_accum += 1.0 - v25;
//                }
//            }
//            else if (distance_to_use > -1.0)
//            {
//                opacity_accum = 0.0;
//                if (distance_to_use < 1.0)
//                {
//                    break;
//                }
//            }
//        }
//
//        float const opacity ( opacity_accum > 0.001
//                              ? std::min (1.0f, opacity_accum)
//                              : 0.0f
//        );
//
//        //! \note all assignments asserted to be > -0.5 && < 255.5f
//        CArgb& color (wmoGroupGeom.colorArray[vertex_index]);
//        color.r = (unsigned char) (((127.0f - color.r) * opacity) + color.r);
//        color.g = (unsigned char) (((127.0f - color.g) * opacity) + color.g);
//        color.b = (unsigned char) (((127.0f - color.b) * opacity) + color.b);
//        color.a = opacity * 255.0;
//    }
}

std::function<void(WmoGroupGeom &wmoGroupGeom)> WmoObject::getAttenFunction() {
    HWmoMainGeom &mainGeom = this->mainGeom;
    return [&mainGeom](  WmoGroupGeom &wmoGroupGeom ) -> void {
        attenuateTransVerts(mainGeom, wmoGroupGeom);
    } ;
}

bool WmoObject::checkFog(mathfu::vec3 &cameraPos, CImVector &fogColor) {
    mathfu::vec3 cameraLocal = (m_placementInvertMatrix * mathfu::vec4(cameraPos, 1.0)).xyz();
    for (int i = mainGeom->fogsLen-1; i >= 0; i--) {
        SMOFog &fogRecord = mainGeom->fogs[i];
        mathfu::vec3 fogPosVec = mathfu::vec3(fogRecord.pos);

        float distanceToFog = (fogPosVec - cameraLocal).Length();
        if ((distanceToFog < fogRecord.larger_radius) /*|| fogRecord.larger_radius == 0*/) {

            fogColor.r = fogRecord.fog.color.r;
            fogColor.g = fogRecord.fog.color.g;
            fogColor.b = fogRecord.fog.color.b;
//                this.sceneApi.setFogColor(fogRecord.fog_colorF);
            //this.sceneApi.setFogStart(wmoFile.mfog.fog_end);
//                this.sceneApi.setFogEnd(fogRecord.fog_end);
            return true;
        }
    }

    return false;
}

bool WmoObject::hasPortals() {
    return mainGeom->header->nPortals != 0;
}

SMOHeader *WmoObject::getWmoHeader() {
    return mainGeom->header;
}

SMOLight *WmoObject::getLightArray() {
    return mainGeom->lights;
}

SMOMaterial *WmoObject::getMaterials() {
    return mainGeom->materials;
}

void ExteriorView::collectMeshes(std::vector<HGMesh> &renderedThisFrame) {
    auto inserter = std::back_inserter(renderedThisFrame);
    std::copy(drawnChunks.begin(), drawnChunks.end(), inserter);

    GeneralView::collectMeshes(renderedThisFrame);
}


void GeneralView::collectMeshes(std::vector<HGMesh> &renderedThisFrame) {
    for (auto& wmoGroup : drawnWmos) {
        wmoGroup->collectMeshes(renderedThisFrame, renderOrder);
    }
//    for (auto& m2 : drawnM2s) {
//        m2->collectMeshes(renderedThisFrame, renderOrder);
//        m2->drawParticles(renderedThisFrame , renderOrder);
//    }
}

void GeneralView::addM2FromGroups(mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos) {
    std::vector<M2Object *> candidates;
    for (auto &wmoGroup : drawnWmos) {
        auto doodads = wmoGroup->getDoodads();
        std::copy(doodads->begin(), doodads->end(), std::back_inserter(candidates));
    }

    //Delete duplicates
    std::sort( candidates.begin(), candidates.end() );
    candidates.erase( unique( candidates.begin(), candidates.end() ), candidates.end() );

//    std::vector<bool> candidateResults = std::vector<bool>(candidates.size(), false);

    std::for_each(
        candidates.begin(),
        candidates.end(),
        [this, &cameraPos](M2Object * m2Candidate) {  // copies are safer, and the resulting code will be as quick.
            if (m2Candidate == nullptr) return;
            for (auto &frustumPlane : this->frustumPlanes) {
                if (m2Candidate->checkFrustumCulling(cameraPos, frustumPlane, {})) {
                    setM2Lights(m2Candidate);

                    break;
                }
            }
        });

//    drawnM2s = std::vector<M2Object *>();
    drawnM2s.reserve(drawnM2s.size() + candidates.size());
    for (auto &m2Candidate : candidates) {
        if (m2Candidate == nullptr) continue;
        if (m2Candidate->m_cullResult) {
            drawnM2s.push_back(m2Candidate);
        }
    }
}

void GeneralView::setM2Lights(M2Object *m2Object) {
    m2Object->setUseLocalLighting(false);
}

void InteriorView::setM2Lights(M2Object *m2Object) {
    if (!drawnWmos[0]->getIsLoaded()) return;

    if (drawnWmos[0]->getDontUseLocalLightingForM2()) {
        m2Object->setUseLocalLighting(false);
    } else {
        drawnWmos[0]->assignInteriorParams(m2Object);
    }
}
