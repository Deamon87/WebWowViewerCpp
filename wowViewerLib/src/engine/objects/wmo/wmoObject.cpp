//
// Created by deamon on 03.07.17.
//

#include "wmoObject.h"
#include "../../algorithms/mathHelper.h"
#include "../../persistance/header/commonFileStructs.h"
#include "./../../../gapi/interface/IDevice.h"
#include <algorithm>

std::vector<mathfu::vec3> createOccluders(const HWmoGroupGeom& groupGeom)
{
    std::vector<mathfu::vec3> points(0);

    if (groupGeom == nullptr || groupGeom->getStatus() != FileStatus::FSLoaded) return points;
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


        Cache<WmoMainGeom> *wmoGeomCache = m_api->cacheStorage->getWmoMainCache();
        if (!useFileId) {
            mainGeom = wmoGeomCache->get(m_modelName);
        } else {
            mainGeom = wmoGeomCache->getFileId(m_modelFileId);
        }

    }
}

std::shared_ptr<M2Object> WmoObject::getDoodad(int index) {
    int doodadsSet = this->m_doodadSet;

    if (doodadsSet >= this->mainGeom->doodadSetsLen) {
        doodadsSet = 0;
    }
    if (doodadsSet >= this->mainGeom->doodadSetsLen) {
        return nullptr;
    }

    SMODoodadSet *defaultDooodadSetDef = &this->mainGeom->doodadSets[0];
    SMODoodadSet *doodadSetDef = &this->mainGeom->doodadSets[doodadsSet];

    bool isInDefaultDoodadSetDef =
        (index >= defaultDooodadSetDef->firstinstanceindex) &&
        (index < defaultDooodadSetDef->firstinstanceindex + defaultDooodadSetDef->numDoodads);

    bool isInCurrentDoodadSetDef =
        (index >= doodadSetDef->firstinstanceindex) &&
        (index < doodadSetDef->firstinstanceindex + doodadSetDef->numDoodads);

    if (!isInCurrentDoodadSetDef && !isInDefaultDoodadSetDef)
        return nullptr;

    auto iterator = this->m_doodadsUnorderedMap.find(index);
    if (iterator != this->m_doodadsUnorderedMap.end())
        return iterator->second;

    SMODoodadDef *doodadDef = &this->mainGeom->doodadDefs[index];

    bool fileIdMode = false;
    int doodadfileDataId = 0;
    std::string fileName;
    if (this->mainGeom->doodadFileDataIds == nullptr && this->mainGeom->doodadFileDataIdsLen == 0) {
        if (this->mainGeom->doodadNamesFieldLen <= 0) {
            return nullptr;
        }
        assert(doodadDef->name_offset < this->mainGeom->doodadNamesFieldLen);
        fileName = std::string (&this->mainGeom->doodadNamesField[doodadDef->name_offset]);
    } else {
        doodadfileDataId = this->mainGeom->doodadFileDataIds[doodadDef->name_offset];
        fileIdMode = true;
    }

    auto m2Object = std::make_shared<M2Object>(m_api);

    m2Object->setLoadParams(0, {},{});
    if (fileIdMode) {
        m2Object->setModelFileId(doodadfileDataId);
    } else {
        m2Object->setModelFileName(fileName);
    }
    m2Object->createPlacementMatrix(*doodadDef, m_placementMatrix);
    m2Object->calcWorldPosition();

    if (doodadDef->color.a != 255 && doodadDef->color.a < this->mainGeom->lightsLen) {

        auto &light = this->mainGeom->lights[doodadDef->color.a];
        m2Object->setDiffuseColor(light.color);


        std::cout << "Found index into MOLT = " << (int)doodadDef->color.a << std::endl;
    } else {
        m2Object->setDiffuseColor(doodadDef->color);
    }

    this->m_doodadsUnorderedMap[index] = m2Object;

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
    groupObjects = std::vector<std::shared_ptr<WmoGroupObject>>(mainGeom->groupsLen, nullptr);
    groupObjectsLod1 = std::vector<std::shared_ptr<WmoGroupObject>>(mainGeom->groupsLen, nullptr);
    groupObjectsLod2 = std::vector<std::shared_ptr<WmoGroupObject>>(mainGeom->groupsLen, nullptr);
    drawGroupWMO = std::vector<bool>(mainGeom->groupsLen, false);
    lodGroupLevelWMO = std::vector<int>(mainGeom->groupsLen, 0);

    std::string nameTemplate = m_modelName.substr(0, m_modelName.find_last_of("."));
    for(int i = 0; i < mainGeom->groupsLen; i++) {

        groupObjects[i] = std::make_shared<WmoGroupObject>(this->m_placementMatrix, m_api, mainGeom->groups[i], i);
        groupObjects[i]->setWmoApi(this);

        if (mainGeom->gfids.size() > 0) {
            groupObjects[i]->setModelFileId(mainGeom->gfids[0][i]);
            if (mainGeom->gfids.size() > 1 && i < mainGeom->gfids[1].size() && mainGeom->gfids[1][i] > 0) {
                groupObjectsLod1[i] = std::make_shared<WmoGroupObject>(this->m_placementMatrix, m_api, mainGeom->groups[i], i);
                groupObjectsLod1[i]->setWmoApi(this);
                groupObjectsLod1[i]->setModelFileId(mainGeom->gfids[1][i]);
            }
            if (mainGeom->gfids.size() > 2 && i < mainGeom->gfids[2].size() && mainGeom->gfids[2][i] > 0) {
                groupObjectsLod2[i] = std::make_shared<WmoGroupObject>(this->m_placementMatrix, m_api, mainGeom->groups[i], i);
                groupObjectsLod2[i]->setWmoApi(this);
                groupObjectsLod2[i]->setModelFileId(mainGeom->gfids[2][i]);
            }
        } else if (!useFileId) {
            std::string numStr = std::to_string(i);
            for (int j = numStr.size(); j < 3; j++) numStr = '0' + numStr;

            std::string groupFilename = nameTemplate + "_" + numStr + ".wmo";
            std::string groupFilenameLod1 = nameTemplate + "_" + numStr + "_lod1.wmo";
            std::string groupFilenameLod2 = nameTemplate + "_" + numStr + "_lod2.wmo";
            groupObjects[i]->setModelFileName(groupFilename);

            groupObjectsLod1[i] = std::make_shared<WmoGroupObject>(this->m_placementMatrix, m_api, mainGeom->groups[i], i);
            groupObjectsLod1[i]->setWmoApi(this);
            groupObjectsLod1[i]->setModelFileName(groupFilenameLod1);

            groupObjectsLod2[i] = std::make_shared<WmoGroupObject>(this->m_placementMatrix, m_api, mainGeom->groups[i], i);
            groupObjectsLod2[i]->setWmoApi(this);
            groupObjectsLod2[i]->setModelFileName(groupFilenameLod2);
        }
    }
}

void WmoObject::createWorldPortals() {

    int portalCnt = mainGeom->portalsLen;
    auto &portals = mainGeom->portals;
    auto &portalVerticles = mainGeom->portal_vertices;

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

        mathfu::mat4 viewMat = mathfu::mat4::LookAt(
                lookAt,
                center,
                upVector
        );
        mathfu::mat4 projMatInv = viewMat.Inverse();

        std::vector <mathfu::vec3> portalTransformed(portalVecs.size());
        for (int k = 0; k < portalVecs.size(); k++) {
            portalTransformed[k] = (viewMat * mathfu::vec4(portalVecs[k], 1.0)).xyz();
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

bool WmoObject::doPostLoad() {
    if (!m_loaded) {
        if (mainGeom != nullptr && mainGeom->getStatus() == FileStatus::FSLoaded){
            this->createGroupObjects();
            this->createWorldPortals();
            this->createBB(mainGeom->header->bounding_box);
            this->createM2Array();

            if (mainGeom->skyBoxM2FileName != nullptr || mainGeom->skyboxM2FileId != 0) {
                skyBox = std::make_shared<M2Object>(m_api, true);
                skyBox->setLoadParams(0, {},{});

                if ( mainGeom->skyboxM2FileId != 0) {
                    skyBox->setModelFileId(mainGeom->skyboxM2FileId);
                } else {
                    skyBox->setModelFileName(&mainGeom->skyBoxM2FileName[0]);
                }
                skyBox->createPlacementMatrix(mathfu::vec3(0,0,0), 0, mathfu::vec3(1,1,1), nullptr);
                skyBox->calcWorldPosition();
            }

            m_loaded = true;
            m_loading = false;
            return true;
        } else {
            this->startLoading();
        }

        return false;
    }

//    for (auto &groupObject : groupObjects) {
//        if (groupsProcessedThisFrame > 3) return false;
//        groupObject->doPostLoad();
//    }
//    for (auto &groupObjectLod : groupObjectsLod1) {
//        if (groupsProcessedThisFrame > 3) return false;
//        if (groupObjectLod != nullptr) {
//            if (groupObjectLod->doPostLoad()) groupsProcessedThisFrame++;;
//        }
//    }
//    for (auto &groupObjectLod2 : groupObjectsLod2) {
//        if (groupsProcessedThisFrame > 3) return false;
//        if (groupObjectLod2 != nullptr) {
//            if (groupObjectLod2->doPostLoad()) groupsProcessedThisFrame++;;
//        }
//    }

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
void WmoObject::uploadGeneratorBuffers() {
    if (!m_loaded) return;

    for (int i= 0; i < groupObjects.size(); i++) {
        if(groupObjects[i] != nullptr) {
            groupObjects[i]->uploadGeneratorBuffers();
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

void WmoObject::createTransformedAntiPortalMesh() {

}

void WmoObject::updateTransformedAntiPortalPoints(){
#ifndef CULLED_NO_PORTAL_DRAWING
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

//    for (int i = 0; i < this->antiPortals.size(); i++) {
//        //if (portalInfo.index_count != 4) throw new Error("portalInfo.index_count != 4");
//
//        int verticlesCount = this->antiPortals[i].portalVertices.size();
////        if ((verticlesCount - 2) <= 0) {
////            stripOffsets.push_back(stripOffsets[i]);
////            continue;
////        };
////
////        for (int j =0; j < (((int)verticlesCount)-2); j++) {
////            indiciesArray.push_back(verticleOffset+0);
////            indiciesArray.push_back(verticleOffset+j+1);
////            indiciesArray.push_back(verticleOffset+j+2);
////        }
////        stripOffset += ((verticlesCount-2) * 3);
//
//        for (int j =0; j < verticlesCount; j++) {
//            verticles.push_back(this->antiPortals[i].portalVertices[j].x);
//            verticles.push_back(this->antiPortals[i].portalVertices[j].y);
//            verticles.push_back(this->antiPortals[i].portalVertices[j].z);
//        }
//
//        verticleOffset += verticlesCount;
//        stripOffsets.push_back(stripOffset);
//    }
//    GLuint indexVBO;
//    GLuint bufferVBO;
////    glGenBuffers(1, &indexVBO);
////    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexVBO);
////    if (indiciesArray.size() > 0) {
////        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indiciesArray.size() * 2, &indiciesArray[0], GL_STATIC_DRAW);
////    }
//    glGenBuffers(1, &bufferVBO);
//    glBindBuffer( GL_ARRAY_BUFFER, bufferVBO);
//    if (verticles.size() > 0) {
//        glBufferData(GL_ARRAY_BUFFER, verticles.size() * 4, &verticles[0], GL_STATIC_DRAW);
//    }
//
//    glDisable(GL_BLEND);
//
//    glVertexAttribPointer(+drawPortalShader::Attribute::aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);  // position
//
//    auto drawPortalShader = m_api->getPortalShader();
//    static float colorArr[4] = {0.819607843, 0.058, 0.058, 0.3};
//    glUniformMatrix4fv(drawPortalShader->getUnf("uPlacementMat"), 1, GL_FALSE, &this->m_placementMatrix[0]);
//    glUniform4fv(drawPortalShader->getUnf("uColor"), 1, &colorArr[0]);
//
//    glDisable(GL_CULL_FACE);
//    glDepthMask(GL_FALSE);
//
//    int offset = 0;
//    for (int i = 0; i < this->antiPortals.size(); i++) {
////        int indeciesLen = stripOffsets[i+1] - stripOffsets[i];
//
////        glDrawElements(GL_TRIANGLES, indeciesLen, GL_UNSIGNED_SHORT, (void *)(offset * 2));
//        glDrawArrays(GL_TRIANGLES, 0, verticles.size()*4);
//
////        offset += indeciesLen;
//    }
//    glDepthMask(GL_TRUE);
//    glDisable(GL_BLEND);
//
//    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, GL_ZERO);
//    glBindBuffer( GL_ARRAY_BUFFER, GL_ZERO);
//
//    glDeleteBuffers(1, &indexVBO);
//    glDeleteBuffers(1, &bufferVBO);

#endif

}

void WmoObject::setLoadingParam(SMMapObjDef &mapObjDef) {
    createPlacementMatrix(mapObjDef);

    this->m_doodadSet = mapObjDef.doodadSet;
    this->m_nameSet = mapObjDef.nameSet;
}
void WmoObject::setLoadingParam(SMMapObjDefObj1 &mapObjDef) {
    createPlacementMatrix(mapObjDef);

    this->m_doodadSet = mapObjDef.doodadSet;
    this->m_nameSet = mapObjDef.nameSet;
}

HGTexture WmoObject::getTexture(int textureId, bool isSpec) {
    if (textureId == 0) return nullptr; //Usual case

    //Non-usual case
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

        texture = m_api->cacheStorage->getTextureCache()->get(materialTexture);
    } else {
        texture = m_api->cacheStorage->getTextureCache()->getFileId(textureId);
    }

    HGTexture hgTexture = m_api->hDevice->createBlpTexture(texture, true, true);
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

    for (int j = 0; j < this->groupObjects.size(); j++) {
        std::shared_ptr<WmoGroupObject> wmoGroupObject= this->groupObjects[j];


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

CAaBox WmoObject::getAABB() {
    return this->m_bbox;
}

void WmoObject::createM2Array() {
}

void WmoObject::postWmoGroupObjectLoad(int groupId, int lod) {
    //1. Create portal verticles from geometry
}

void WmoObject::checkGroupDoodads(int groupId, mathfu::vec4 &cameraVec4,
                                  std::vector<mathfu::vec4> &frustumPlane,
                                  M2ObjectListContainer &m2Candidates) {
    std::shared_ptr<WmoGroupObject> groupWmoObject = groupObjects[groupId];
    if (groupWmoObject != nullptr && groupWmoObject->getIsLoaded()) {
        mathfu::vec4 ambientColor = groupWmoObject->getAmbientColor() ;

        for (auto &m2Object : groupWmoObject->getDoodads()) {

            if (!m2Object) continue;
            if (groupWmoObject->getDontUseLocalLightingForM2()) {
                m2Object->setUseLocalLighting(false);
            } else {
                m2Object->setUseLocalLighting(true);
                m2Object->setAmbientColorOverride(ambientColor, true);
            }

            m2Candidates.addCandidate(m2Object);
        }
    }
}

bool WmoObject::startTraversingWMOGroup(
    mathfu::vec4 &cameraVec4,
    const MathHelper::FrustumCullingData &frustumDataGlobal,
    int groupId,
    int globalLevel,
    int &renderOrder,
    bool traversingFromInterior,
    FrameViewsHolder &viewsHolder
) {
    if (!m_loaded)
        return true;

    std::vector<HInteriorView> ivPerWMOGroup = std::vector<HInteriorView>(mainGeom->groupsLen);

    uint32_t portalCount = (uint32_t) std::max(0, this->mainGeom->portalsLen);
    std::vector<bool> transverseVisitedPortals = std::vector<bool>(portalCount, false);

    //CurrentVisibleM2 and visibleWmo is array of global m2 objects, that are visible after frustum
    mathfu::vec4 cameraLocal = this->m_placementInvertMatrix * cameraVec4;

    mathfu::mat4 transposeModelMat = this->m_placementMatrix.Transpose();
    mathfu::mat4 transposeInverseModelMat = transposeModelMat.Inverse();

    //For interior cull
    mathfu::mat4 MVPMat = frustumDataGlobal.perspectiveMat*frustumDataGlobal.viewMat*this->m_placementMatrix;
    std::vector<mathfu::vec3> frustumPointsLocal = MathHelper::calculateFrustumPointsFromMat(MVPMat);
    std::vector<mathfu::vec4> frustumPlanesLocal = MathHelper::getFrustumClipsFromMatrix(MVPMat);
//    mathfu::vec4 headOfPyramidLocal = mathfu::vec4(MathHelper::getIntersection(
//        frustumPointsLocal[4], frustumPointsLocal[7],
//        frustumPointsLocal[3], frustumPointsLocal[0]
//    ), 1.0);

    auto globalPlane = frustumDataGlobal.frustums[0].planes[frustumDataGlobal.frustums[0].planes.size() - 2];
    auto altFarPlane = this->m_placementMatrix.Transpose() * globalPlane;
    PortalTraverseTempData traverseTempData = {
        viewsHolder,
        viewsHolder.getExterior() != nullptr,
        frustumPlanesLocal[frustumPlanesLocal.size() - 1], //farPlane is always last one,
        ivPerWMOGroup,
        cameraVec4,
        cameraLocal,
        transposeInverseModelMat,
        transverseVisitedPortals
    };

    if (traversingFromInterior && m_api->getConfig()->usePortalCulling) {
        std::shared_ptr<WmoGroupObject> nextGroupObject = groupObjects[groupId];
        if (nextGroupObject->getIsLoaded() && nextGroupObject->getWmoGroupGeom()->mogp->flags2.isSplitGroupChild) {
            groupId = nextGroupObject->getWmoGroupGeom()->mogp->parentSplitOrFirstChildGroupIndex;
            nextGroupObject = groupObjects[groupId];
        }

        auto interiorView = ivPerWMOGroup[groupId];
        //5.1 The portal is into interior wmo group. So go on.
        if (interiorView == nullptr) {
            interiorView = viewsHolder.createInterior(frustumDataGlobal);
            ivPerWMOGroup[groupId] = interiorView;
            interiorView->ownerGroupWMO = groupObjects[groupId];
            interiorView->wmoGroupArray.addToDraw(nextGroupObject);
            interiorView->wmoGroupArray.addToCheckM2(nextGroupObject);
        }

        if (nextGroupObject->getIsLoaded() && nextGroupObject->getWmoGroupGeom()->mogp->flags2.isSplitGroupParent) {
            this->addSplitChildWMOsToView(*interiorView, groupId);
        }

        if (globalLevel+1 >= interiorView->level) {
            interiorView->level = globalLevel + 1;
        } else {
            assert("BVH is not working. Something is wrong!");
        }

        this->traverseGroupWmo(
            groupId,
            traversingFromInterior,
            traverseTempData,
            frustumPlanesLocal,
            globalLevel,
            0);
    } else {
        auto exteriorView = viewsHolder.getOrCreateExterior(frustumDataGlobal);

        if (globalLevel+1 >= exteriorView->level) {
            exteriorView->level = globalLevel + 1;
        } else {
            assert("BVH is not working. Something is wrong!");
        }

        for (int i = 0; i< mainGeom->groupsLen; i++) {
            if ((mainGeom->groups[i].flags.EXTERIOR) > 0 || !m_api->getConfig()->usePortalCulling) { //exterior
                if (this->groupObjects[i] != nullptr) {
                    bool drawDoodads, drawGroup;
                    this->groupObjects[i]->checkGroupFrustum(drawDoodads, drawGroup, cameraVec4, frustumDataGlobal);
                    if (drawDoodads) {
                        exteriorView->wmoGroupArray.addToCheckM2(this->groupObjects[i]);
                    }
                    if (drawGroup) {
                        exteriorView->wmoGroupArray.addToDraw(this->groupObjects[i]);

                        this->traverseGroupWmo(
                            i,
                            false,
                            traverseTempData,
                            frustumPlanesLocal,
                            globalLevel,
                            0);
                    }
                }
            }
        }
    }

    //Add all ALWAYSRENDER to Exterior
    for (int i = 0; i< mainGeom->groupsLen; i++) {
        if ((mainGeom->groups[i].flags.ALWAYSDRAW) > 0) { //exterior
            auto exteriorView = viewsHolder.getOrCreateExterior(frustumDataGlobal);
            exteriorView->wmoGroupArray.addToDraw(this->groupObjects[i]);
        }
    }

    //Process results
    ivPerWMOGroup.erase(std::remove(ivPerWMOGroup.begin(), ivPerWMOGroup.end(), nullptr), ivPerWMOGroup.end());
    std::sort(ivPerWMOGroup.begin(), ivPerWMOGroup.end(), [](const HInteriorView &a, const HInteriorView &b) -> bool {
        if (a->level != b->level) {
            return a->level < b->level;
        }
        return false;
    });

    for (auto &createdInteriorView : ivPerWMOGroup) {
        createdInteriorView->renderOrder = renderOrder++;
    }

    bool result = ivPerWMOGroup.size() > 0;
    {
        auto exterior = viewsHolder.getExterior();

        if (exterior != nullptr) {
            exterior->renderOrder = renderOrder++;
            result = true;
        }
    }

    //M2s will be collected later from separate function call
    return true;
}
void WmoObject::addSplitChildWMOsToView(InteriorView &interiorView, int groupId) {
    if (!groupObjects[groupId]->getIsLoaded())
        return;

    auto &parentMogp = groupObjects[groupId]->getWmoGroupGeom()->mogp;
    if (!parentMogp->flags2.isSplitGroupParent)
        return;

    int nextChildGroupIndex = parentMogp->parentSplitOrFirstChildGroupIndex;
    while (nextChildGroupIndex != -1) {
        auto &groupWmo = groupObjects[nextChildGroupIndex];
        if (!groupWmo->getIsLoaded()) {
            interiorView.wmoGroupArray.addToLoad(groupWmo);
            return;
        }

        auto &mogp = groupWmo->getWmoGroupGeom()->mogp;
        if (!mogp->flags2.isSplitGroupChild)
            break;

        interiorView.wmoGroupArray.addToDraw(groupWmo);
        interiorView.wmoGroupArray.addToCheckM2(groupWmo);

        nextChildGroupIndex = mogp->nextSplitGroupChildIndex;
    }

}


static const float dotepsilon = pow(1.5f, 2.0f);
void WmoObject::traverseGroupWmo(
    int groupId,
    bool traversingStartedFromInterior,
    PortalTraverseTempData &traverseTempData,
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

    if (groupObjects[groupId]->getWmoGroupGeom()->mogp->flags.showSkyBox) {
        if (skyBox != nullptr) {
            traverseTempData.ivPerWMOGroup[groupId]->m2List.addToDraw(skyBox);
        }
    }

    for (int j = moprIndex; j < moprIndex+numItems; j++) {
        SMOPortalRef * relation = &mainGeom->portalReferences[j];
        SMOPortal * portalInfo = &mainGeom->portals[relation->portal_index];

        int nextGroup = relation->group_index;
        C4Plane &plane = portalInfo->plane;

        //Skip portals we already visited
        if (traverseTempData.transverseVisitedPortals[relation->portal_index]) continue;

        //Local coordinanes plane DOT local camera
        const mathfu::vec4 planeV4 = mathfu::vec4(plane.planeVector);
        float dotResult = mathfu::vec4::DotProduct(planeV4, traverseTempData.cameraLocal);
        //dotResult = dotResult + relation.side * 0.01;
        bool isInsidePortalThis = (relation->side < 0) ? (dotResult <= 0) : (dotResult >= 0);

        //This condition checks if camera is very close to the portal. In this case the math doesnt work very properly
        //So I need to make this hack exactly for this case.z

        bool hackCondition = (fabs(dotResult) > dotepsilon);

        /* Test code
        auto crossProd = mathfu::vec3::CrossProduct(
            geometryPerPortal[relation->portal_index].sortedVericles[0] - geometryPerPortal[relation->portal_index].sortedVericles[1],
            geometryPerPortal[relation->portal_index].sortedVericles[0] - geometryPerPortal[relation->portal_index].sortedVericles[2]
        );
        crossProd = crossProd.Normalized();
        float distantce = mathfu::vec3::DotProduct(crossProd,geometryPerPortal[relation->portal_index].sortedVericles[0]);
        mathfu::vec4 manualEquation = mathfu::vec4(crossProd.x, crossProd.y, crossProd.z, -distantce);
        */
        //Hacks to test!
        /*
        if (!isInsidePortalThis)
        {
            std::vector<mathfu::vec3> worldSpacePortalVertices;
            std::vector<mathfu::vec3> portalVerticesVec;
            std::transform(
                geometryPerPortal[relation->portal_index].sortedVericles.begin(),
                geometryPerPortal[relation->portal_index].sortedVericles.end(),
                std::back_inserter(portalVerticesVec),
                [](mathfu::vec3 &d) -> mathfu::vec3 { return d;}
            );
            std::transform(portalVerticesVec.begin(), portalVerticesVec.end(),
                           std::back_inserter(worldSpacePortalVertices),
                           [&](mathfu::vec3 &p) -> mathfu::vec3 {
                               return (this->m_placementMatrix * mathfu::vec4(p, 1.0f)).xyz();
                           }
            );
            HInteriorView &interiorView = traverseTempData.ivPerWMOGroup[groupId];
            if (interiorView == nullptr) {
                interiorView = traverseTempData.viewsHolder.createInterior({});
                traverseTempData.ivPerWMOGroup[nextGroup] = interiorView;

                interiorView->ownerGroupWMO = groupObjects[nextGroup];
                interiorView->drawnWmos.insert(groupObjects[nextGroup]);
                interiorView->wmosForM2.insert(groupObjects[nextGroup]);
                interiorView->portalIndexes.push_back(relation->portal_index);
            }
            interiorView->worldPortalVertices.push_back(worldSpacePortalVertices);
        }
         */

        if (!isInsidePortalThis && hackCondition) continue;

        //2.1 If portal has less than 4 vertices - skip it(invalid?)
        if (portalInfo->index_count < 4) continue;

        //2.2 Check if Portal BB made from portal vertexes intersects frustum
        std::vector<mathfu::vec3> portalVerticesVec;
        std::transform(
            geometryPerPortal[relation->portal_index].sortedVericles.begin(),
            geometryPerPortal[relation->portal_index].sortedVericles.end(),
            std::back_inserter(portalVerticesVec),
            [](mathfu::vec3 &d) -> mathfu::vec3 { return d;}
            );

        bool visible = MathHelper::planeCull(portalVerticesVec, localFrustumPlanes);

        if (!visible && hackCondition) continue;

        traverseTempData.transverseVisitedPortals[relation->portal_index] = true;

        int lastFrustumPlanesLen = localFrustumPlanes.size();

        //3. Construct frustum planes for this portal
        std::vector<mathfu::vec4> thisPortalPlanes;
        thisPortalPlanes.reserve(lastFrustumPlanesLen);

        if (hackCondition) {
            bool flip = (relation->side > 0);

            int portalCnt = portalVerticesVec.size();
            for (int i = 0; i < portalCnt; ++i) {
                int i2 = (i + 1) % portalCnt;

                mathfu::vec4 n = MathHelper::createPlaneFromEyeAndVertexes(traverseTempData.cameraLocal.xyz(),
                                                                           portalVerticesVec[i],
                                                                           portalVerticesVec[i2]);

                if (flip) {
                    n *= -1.0f;
                }

                thisPortalPlanes.push_back(n);
            }
            //The portalPlanes do not have far and near plane. So we need to add them
            //Near plane is this portal's plane, far plane is a global one
            auto nearPlane = mathfu::vec4(portalInfo->plane.planeVector);
            if (flip)
                nearPlane *= -1.0f;

            auto &farPlane = traverseTempData.farPlane;
            thisPortalPlanes.push_back(nearPlane);
            thisPortalPlanes.push_back(farPlane);
        } else {
            // If camera is too close - just use usual frustums
            thisPortalPlanes = localFrustumPlanes;
        }

        //Transform local planes into world planes to use with frustum culling of M2Objects
        MathHelper::PlanesUndPoints worldSpaceFrustum;
        worldSpaceFrustum.planes = std::vector<mathfu::vec4>(thisPortalPlanes.size());
        worldSpaceFrustum.points = std::vector<mathfu::vec3>();
        worldSpaceFrustum.points.reserve(thisPortalPlanes.size());

        for (int x = 0; x < thisPortalPlanes.size(); x++) {
            worldSpaceFrustum.planes[x] = traverseTempData.transposeInverseModelMat * thisPortalPlanes[x];
        }

        {
            worldSpaceFrustum.points = MathHelper::getIntersectionPointsFromPlanes(worldSpaceFrustum.planes);
            worldSpaceFrustum.hullLines = MathHelper::getHullLines(worldSpaceFrustum.points);
        }

        std::vector<mathfu::vec3> worldSpacePortalVertices;
        std::transform(portalVerticesVec.begin(), portalVerticesVec.end(),
                       std::back_inserter(worldSpacePortalVertices),
                       [&](mathfu::vec3 &p) -> mathfu::vec3 {
                           return (this->m_placementMatrix * mathfu::vec4(p, 1.0f)).xyz();
                       }
        );

        //5. Traverse next
        std::shared_ptr<WmoGroupObject> &nextGroupObject = groupObjects[nextGroup];
        SMOGroupInfo &nextGroupInfo = mainGeom->groups[nextGroup];
        if ((nextGroupInfo.flags.EXTERIOR) == 0) {
            auto &interiorView = traverseTempData.ivPerWMOGroup[nextGroup];
            //5.1 The portal is into interior wmo group. So go on.
            if (interiorView == nullptr) {
                interiorView = traverseTempData.viewsHolder.createInterior({});
                traverseTempData.ivPerWMOGroup[nextGroup] = interiorView;

                interiorView->ownerGroupWMO = nextGroupObject;
                interiorView->wmoGroupArray.addToDraw(nextGroupObject);
                interiorView->wmoGroupArray.addToCheckM2(nextGroupObject);
                interiorView->portalIndexes.push_back(relation->portal_index);
            }

            interiorView->worldPortalVertices.push_back(worldSpacePortalVertices);
            interiorView->frustumData.frustums.push_back(worldSpaceFrustum);
            if (globalLevel+1 >= interiorView->level) {
                interiorView->level = globalLevel + 1;
            } else {
                assert("BVH is not working. Something is wrong!");
            }

            if (nextGroupObject->getIsLoaded() && nextGroupObject->getWmoGroupGeom()->mogp->flags2.isSplitGroupParent) {
                this->addSplitChildWMOsToView(*interiorView, nextGroup);
            }

            traverseGroupWmo(
                nextGroup,
                traversingStartedFromInterior,
                traverseTempData,
                thisPortalPlanes,
                globalLevel + 1,
                localLevel + 1
            );

        } else if (((nextGroupInfo.flags.EXTERIOR) > 0) && traversingStartedFromInterior) {
            //5.2 The portal is from interior into exterior wmo group.
            //Makes sense to try to create or get only if exterior was not already created before traversing this entire WMO
            if (!traverseTempData.exteriorWasCreatedBeforeTraversing) {
                auto exteriorView = traverseTempData.viewsHolder.getOrCreateExterior({});
                exteriorView->level = globalLevel + 1;
                exteriorView->worldPortalVertices.push_back(worldSpacePortalVertices);
                exteriorView->frustumData.frustums.push_back(worldSpaceFrustum);
            }
        }
    }
}

bool WmoObject::isGroupWmoInterior(int groupId) {
    SMOGroupInfo *groupInfo = &this->mainGeom->groups[groupId];
    bool result = ((groupInfo->flags.EXTERIOR) == 0);
    return result;
}

bool WmoObject::isGroupWmoExteriorLit(int groupId) {
    SMOGroupInfo *groupInfo = &this->mainGeom->groups[groupId];
    bool result = ((groupInfo->flags.EXTERIOR_LIT) == 1);
    return result;
}

bool WmoObject::isGroupWmoExtSkybox(int groupId) {
    SMOGroupInfo *groupInfo = &this->mainGeom->groups[groupId];
    bool result = ((groupInfo->flags.SHOW_EXTERIOR_SKYBOX) == 1);
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

void WmoObject::checkFog(mathfu::vec3 &cameraPos, std::vector<LightResult> &fogResults) {
    mathfu::vec3 cameraLocal = (m_placementInvertMatrix * mathfu::vec4(cameraPos, 1.0)).xyz();
    for (int i = mainGeom->fogsLen-1; i >= 0; i--) {
        SMOFog &fogRecord = mainGeom->fogs[i];
        mathfu::vec3 fogPosVec = mathfu::vec3(fogRecord.pos);

        float distanceToFog = (fogPosVec - cameraLocal).Length();
        if ((distanceToFog < fogRecord.larger_radius) || fogRecord.flag_infinite_radius) {
            LightResult wmoFog;
            wmoFog.id = -1;
            wmoFog.FogScaler = fogRecord.fog.start_scalar;
            wmoFog.FogEnd = fogRecord.fog.end;
            wmoFog.FogDensity = 0.18f;

            wmoFog.FogHeightScaler = 0;
            wmoFog.FogHeightDensity = 0;
            wmoFog.SunFogAngle = 0;
            wmoFog.EndFogColorDistance = 0;
            wmoFog.SunFogStrength = 0;
            if (fogRecord.flag_infinite_radius) {
                wmoFog.blendCoef = 1.0;
                wmoFog.isDefault = true;
            } else {
                wmoFog.blendCoef = std::min(
                    (fogRecord.larger_radius - distanceToFog) / (fogRecord.larger_radius - fogRecord.smaller_radius),
                    1.0f);
                wmoFog.isDefault = false;
            }
            std::array<float, 3> fogColor = {fogRecord.fog.color.b/255.0f, fogRecord.fog.color.g/255.0f,  fogRecord.fog.color.r/255.0f};
            wmoFog.EndFogColor = fogColor;
            wmoFog.SunFogColor = fogColor;
            wmoFog.FogHeightColor = fogColor;

            fogResults.push_back(wmoFog);
        }
    }
}

bool WmoObject::hasPortals() {
    return mainGeom->header->nPortals != 0;
}

SMOHeader *WmoObject::getWmoHeader() {
    return mainGeom->header;
}

PointerChecker<SMOLight> &WmoObject::getLightArray() {
    return mainGeom->lights;
}

PointerChecker<SMOMaterial> &WmoObject::getMaterials() {
    return mainGeom->materials;
}

std::shared_ptr<M2Object> WmoObject::getSkyBoxForGroup(int groupNum) {
    if (!m_loaded) return nullptr;
    if (groupNum < 0 || groupNum >= this->groupObjects.size()) return nullptr;
    if (!this->groupObjects[groupNum]->getIsLoaded()) return nullptr;
    if (!this->groupObjects[groupNum]->getWmoGroupGeom()->mogp->flags.showSkyBox) return nullptr;

    return skyBox;
}

WmoObject::~WmoObject() {
}

int WmoObject::getWmoGroupId(int groupNum) {
    if (!m_loaded) return 0;
    if (!groupObjects[groupNum]->getIsLoaded()) return 0;

    return groupObjects[groupNum]->getWmoGroupGeom()->mogp->wmoGroupID;
}

mathfu::vec3 WmoObject::getAmbientColor() {
    CImVector ambientColor = mainGeom->header->ambColor;
    if (mainGeom->mavgs != nullptr) {
        int recordIndex = 0;
        for (int i = 0; i < mainGeom->mavgsLen; i++) {
            if (mainGeom->mavgs[i].doodadSetID == m_doodadSet) {
                recordIndex = i;
                break;
            }
        }
        auto &record = mainGeom->mavgs[recordIndex];
        if (record.flags & 1) {
            ambientColor = record.color3;
        } else {
            ambientColor = record.color1;
        }
    }
    mathfu::vec3 ambColor = mathfu::vec3(
        ((float) ambientColor.r / 255.0f),
        ((float) ambientColor.g / 255.0f),
        ((float) ambientColor.b / 255.0f)
    );

    return ambColor;
}
