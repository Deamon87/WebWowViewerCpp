//
// Created by deamon on 03.07.17.
//

#include "wmoObject.h"
#include "../../algorithms/mathHelper.h"
#include "../../shaderDefinitions.h"
#include "../../persistance/header/commonFileStructs.h"

std::vector<mathfu::vec3> CreateOccluders(const WmoGroupGeom * groupGeom)
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
    assert(doodadDef->name_offset < this->mainGeom->doodadNamesFieldLen);
    std::string fileName(&this->mainGeom->doodadNamesField[doodadDef->name_offset]);

    M2Object *m2Object = new M2Object(m_api);
    m2Object->setDiffuseColor(doodadDef->color);
    m2Object->setLoadParams(0, {},{});
    m2Object->setModelFileName(fileName);
    m2Object->createPlacementMatrix(*doodadDef, m_placementMatrix);
    m2Object->calcWorldPosition();

    this->m_doodadsArray[doodadIndex] = m2Object;

    return m2Object;
}

bool WmoObject::checkFrustumCulling (mathfu::vec4 &cameraPos, std::vector<mathfu::vec4> &frustumPlanes, std::vector<mathfu::vec3> &frustumPoints,
                                     std::unordered_set<M2Object*> &m2RenderedThisFrame) {
    if (!m_loaded) return true;

    bool result = false;
    CAaBox &aabb = this->m_bbox;

    //1. Check if camera position is inside Bounding Box
    if (
        cameraPos[0] > aabb.min.z && cameraPos[0] < aabb.max.x &&
        cameraPos[1] > aabb.min.y && cameraPos[1] < aabb.max.y &&
        cameraPos[2] > aabb.min.z && cameraPos[2] < aabb.max.z
        ) result = true;

    //2. Check aabb is inside camera frustum
    if (!result)
        result = MathHelper::checkFrustum(frustumPlanes, aabb, frustumPoints);

    std::set<M2Object*> wmoM2Candidates;
    if (result) {
        //1. Calculate visibility for groups
        for (int i = 0; i < this->groupObjects.size(); i++) {
            drawGroupWMO[i] = this->groupObjects[i]->checkGroupFrustum(cameraPos, frustumPlanes, frustumPoints, wmoM2Candidates);
        }

        //2. Check all m2 candidates
        for (auto it = wmoM2Candidates.begin(); it != wmoM2Candidates.end(); ++it) {
            M2Object *m2ObjectCandidate  = *it;
            bool frustumResult = m2ObjectCandidate->checkFrustumCulling(cameraPos, frustumPlanes, frustumPoints);
            if (frustumResult) {
                m2RenderedThisFrame.insert(m2ObjectCandidate);
            }
        }
    }

    return result;
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
    drawGroupWMO = std::vector<bool>(mainGeom->groupsLen, false);

    std::string nameTemplate = m_modelName.substr(0, m_modelName.find_last_of("."));
    for(int i = 0; i < mainGeom->groupsLen; i++) {

        groupObjects[i] = new WmoGroupObject(this->m_placementMatrix, m_api, mainGeom->groups[i], i);
        groupObjects[i]->setWmoApi(this);
        if (useFileId) {
            groupObjects[i]->setModelFileId(mainGeom->gfids[0][i]);
        } else {
            std::string numStr = std::to_string(i);
            for (int j = numStr.size(); j < 3; j++) numStr = '0' + numStr;

            std::string groupFilename = nameTemplate + "_" + numStr + ".wmo";
            std::string groupFilenameLod1 = nameTemplate + "_" + numStr + "_lod1.wmo";
            groupObjects[i]->setModelFileName(groupFilename);
        }



//        groupObjectsLod1[i] = new WmoGroupObject(this->m_placementMatrix, m_api, groupFilenameLod1, mainGeom->groups[i], i);
//        groupObjectsLod1[i]->setWmoApi(this);


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
        //.min = aaBox.min;
//        aaBoxCopyTo.max = aaBox.max;
    }
}

void WmoObject::update() {
    if (!m_loaded) {
        if (mainGeom != nullptr && mainGeom->getIsLoaded()){
            m_loaded = true;
            m_loading = false;

            this->createGroupObjects();
            this->createWorldPortals();
            this->createBB(mainGeom->header->bounding_box);
            this->createM2Array();
        } else {
            this->startLoading();
        }

        return;
    } else {
        this->antiPortals = std::vector<PortalResults>(0);
        for (int i= 0; i < groupObjects.size(); i++) {
            if(groupObjects[i] != nullptr) {
                if ((mainGeom->groups[i].flags.ANTIPORTAL) > 0) {
                    //AntiPortal

                    std::vector<mathfu::vec3> oclluded = CreateOccluders(groupObjects[i]->getWmoGroupGeom());
                    this->antiPortals.push_back({
                                                        groupId: i,
                                                        portalIndex : -1,
#ifndef CULLED_NO_PORTAL_DRAWING
                                                        portalVertices: oclluded,
#endif
                                                        frustumPlanes: {},
                                                        level : 0});
                }
                groupObjects[i]->update();
            }
        }
        for (int i= 0; i < groupObjectsLod1.size(); i++) {
            if(groupObjectsLod1[i] != nullptr) {
                groupObjectsLod1[i]->update();
            }
        }
    }
}

void WmoObject::draw(){
    if (!m_loaded) return;

    auto wmoShader = m_api->getWmoShader();

    glUniformMatrix4fv(wmoShader->getUnf("uPlacementMat"), 1, GL_FALSE, &this->m_placementMatrix[0]);

    for (int i= 0; i < groupObjects.size(); i++) {
        if(groupObjects[i] != nullptr && drawGroupWMO[i]) {
            groupObjects[i]->draw(mainGeom->materials, m_getTextureFunc);
//            groupObjectsLod1[i]->draw(mainGeom->materials, m_getTextureFunc);
        }
    }
}

void WmoObject::drawDebugLights(){
    if (!m_loaded) return;

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
}


void WmoObject::drawTransformedPortalPoints(){
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
#endif
}

void WmoObject::drawTransformedAntiPortalPoints(){
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

BlpTexture *WmoObject::getTexture(int textureId, bool isSpec) {
    if (textureId < 0 || textureId >= mainGeom->textureNamesFieldLen) return nullptr;

    std::unordered_map<int, BlpTexture*> &textureCache = diffuseTextures;
    if (isSpec) {
        textureCache = specularTextures;
    }

    auto i = textureCache.find(textureId);
    if (i != textureCache.end()) {
        return i->second;
    }

    std::string materialTexture(&mainGeom->textureNamesField[textureId]);
    if (materialTexture == "") return nullptr;

    if (isSpec) {
        materialTexture = materialTexture.substr(0, materialTexture.length() - 4) + "_s.blp";
    }

    BlpTexture * texture = m_api->getTextureCache()->get(materialTexture);
    textureCache[textureId] =  texture;

    return texture;
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



// Portal culling
bool WmoObject::startTraversingFromInteriorWMO(std::vector<WmoGroupResult> &wmoGroupsResults,
                                               mathfu::vec4 &cameraVec4,
                                               mathfu::mat4 &viewPerspectiveMat,
                                               std::unordered_set<M2Object *> &m2RenderedThisFrame) {
    //CurrentVisibleM2 and visibleWmo is array of global m2 objects, that are visible after frustum
    mathfu::vec4 cameraLocal = this->m_placementInvertMatrix * cameraVec4;

    /* 1. Create array of visibility with all false */
    std::vector<bool> transverseVisitedGroups = std::vector<bool>(this->groupObjects.size());
    for (int i = 0; i < transverseVisitedGroups.size(); i++) {
        transverseVisitedGroups[i] = false;
    }

    std::vector<bool> transverseVisitedPortals = std::vector<bool>(this->mainGeom->portalsLen);
    for (int i = 0; i < transverseVisitedPortals.size(); i++) {
        transverseVisitedPortals[i] = false;
    }

    this->exteriorPortals = std::vector<PortalResults>();
    this->interiorPortals = std::vector<PortalResults>();

    mathfu::mat4 transposeModelMat = this->m_placementMatrix.Transpose();
    mathfu::mat4 inverseTransposeModelMat = this->m_placementInvertMatrix.Transpose();

    std::vector<mathfu::vec4> frustumPlanes = MathHelper::getFrustumClipsFromMatrix(viewPerspectiveMat);
    MathHelper::fixNearPlane(frustumPlanes, cameraVec4);
    std::vector<mathfu::vec3> frustumPoints = MathHelper::calculateFrustumPointsFromMat(viewPerspectiveMat);

    //Test code
    mathfu::mat4 MVPMat = viewPerspectiveMat*this->m_placementMatrix;
    std::vector<mathfu::vec3> frustumPointsLocal1 = MathHelper::calculateFrustumPointsFromMat(MVPMat);
    std::vector<mathfu::vec4> frustumPlanes1 = MathHelper::getFrustumClipsFromMatrix(MVPMat);
    mathfu::vec4 headOfPyramidLocal1 = mathfu::vec4(MathHelper::getIntersection(
            frustumPointsLocal1[4], frustumPointsLocal1[7],
            frustumPointsLocal1[3], frustumPointsLocal1[0]
    ), 1.0);

    std::vector<mathfu::vec4> localFrustumPlanes;
    std::transform(frustumPlanes.begin(), frustumPlanes.end(),
                   std::back_inserter(localFrustumPlanes),
                   [&](mathfu::vec4 p) -> mathfu::vec4 {
                       return transposeModelMat * p;
                   }
    );


    for (int i = 0; i < wmoGroupsResults.size(); i++) {
        this->interiorPortals.push_back({
                groupId: wmoGroupsResults[i].groupId,
                portalIndex: -1,
#ifndef CULLED_NO_PORTAL_DRAWING
                portalVertices: {},
#endif
                frustumPlanes: frustumPlanes1,
                level: 0});
        this->transverseGroupWMO(wmoGroupsResults[i].groupId, true, cameraVec4, headOfPyramidLocal1,
                                 inverseTransposeModelMat,
                                 transverseVisitedGroups,
                                 transverseVisitedPortals,
                                 frustumPlanes1, 0, m2RenderedThisFrame);
    }

    //If there are portals leading to exterior, we need to go through all exterior wmos.
    //Because it's not guaranteed that exterior wmo, that portals lead to, have portal connections to all visible interior wmo
    std::set<M2Object *> wmoM2Candidates ;
    if (this->exteriorPortals.size() > 0) {
        std::vector<std::vector<mathfu::vec4>> portalsToExt;
        for (auto &a : this->exteriorPortals) {
            portalsToExt.push_back(a.frustumPlanes);
        }

        for (int i = 0; i< mainGeom->groupsLen; i++) {
            if ((mainGeom->groups[i].flags.EXTERIOR) > 0) { //exterior
                if (this->groupObjects[i]->checkGroupFrustum(cameraVec4, frustumPlanes, frustumPoints, wmoM2Candidates)) {
//                    this->exteriorPortals.push_back({
//                        groupId: i,
//                        portalIndex : -1,
//#ifndef CULLED_NO_PORTAL_DRAWING
//                        portalVertices: {},
//#endif
//                        frustumPlanes: frustumPlanes,
//                        level : 0
//                    });
                    this->transverseGroupWMO(i, false, cameraVec4, headOfPyramidLocal1,  inverseTransposeModelMat,
                                             transverseVisitedGroups,
                                             transverseVisitedPortals, frustumPlanes1, 0, m2RenderedThisFrame);
                }
            }
        }
    }


    for (M2Object *m2Object : wmoM2Candidates) {
        if (m2Object == nullptr) continue;
        if (m2RenderedThisFrame.count(m2Object) > 0) continue;

        bool result = m2Object->checkFrustumCulling(cameraVec4, frustumPlanes, frustumPoints);
        if (result) m2RenderedThisFrame.insert(m2Object);
    };

    bool atLeastOneIsDrawn = false;
    for (int i = 0; i < groupObjects.size(); i++) {
        if (transverseVisitedGroups[i]) atLeastOneIsDrawn = true;
        drawGroupWMO[i] = transverseVisitedGroups[i];
    }

    return atLeastOneIsDrawn;
}


bool
WmoObject::startTraversingFromExterior(mathfu::vec4 &cameraVec4,
                                       mathfu::mat4 &viewPerspectiveMat,
                                       std::unordered_set<M2Object*> &m2RenderedThisFrame) {
    //CurrentVisibleM2 and visibleWmo is array of global m2 objects, that are visible after frustum
    mathfu::vec4 cameraLocal = this->m_placementInvertMatrix * cameraVec4;

    /* 1. Create array of visibility with all false */
    std::vector<bool> transverseVisitedGroups = std::vector<bool>(this->groupObjects.size());
    for (int i = 0; i < transverseVisitedGroups.size(); i++) {
        transverseVisitedGroups[i] = false;
    }

    std::vector<bool> transverseVisitedPortals = std::vector<bool>(this->mainGeom->portalsLen);
    for (int i = 0; i < transverseVisitedPortals.size(); i++) {
        transverseVisitedPortals[i] = false;
    }

    this->exteriorPortals = std::vector<PortalResults>();
    this->interiorPortals = std::vector<PortalResults>();

    mathfu::mat4 transposeModelMat = this->m_placementMatrix.Transpose();
    mathfu::mat4 inverseTransposeModelMat = this->m_placementInvertMatrix.Transpose();

    std::vector<mathfu::vec4> frustumPlanes = MathHelper::getFrustumClipsFromMatrix(viewPerspectiveMat);
    MathHelper::fixNearPlane(frustumPlanes, cameraVec4);
    std::vector<mathfu::vec3> frustumPoints = MathHelper::calculateFrustumPointsFromMat(viewPerspectiveMat);

    //Test code
    mathfu::mat4 MVPMat = viewPerspectiveMat*this->m_placementMatrix;
    std::vector<mathfu::vec3> frustumPointsLocal1 = MathHelper::calculateFrustumPointsFromMat(MVPMat);
    std::vector<mathfu::vec4> frustumPlanes1 = MathHelper::getFrustumClipsFromMatrix(MVPMat);
    mathfu::vec4 headOfPyramidLocal1 = mathfu::vec4(MathHelper::getIntersection(
            frustumPointsLocal1[4], frustumPointsLocal1[7],
            frustumPointsLocal1[3], frustumPointsLocal1[0]
    ), 1.0);



    std::vector<mathfu::vec4> localFrustumPlanes;
    std::transform(frustumPlanes.begin(), frustumPlanes.end(),
                   std::back_inserter(localFrustumPlanes),
                   [&](mathfu::vec4 p) -> mathfu::vec4 {
                       mathfu::vec4 plane = transposeModelMat * p;
                       float normalLength = plane.xyz().Length();
                       return plane * (1.0f / normalLength);
                   }
    );


    std::set<M2Object *> wmoM2Candidates ;
    for (int i = 0; i< mainGeom->groupsLen; i++) {
       if (mainGeom->groups[i].flags.ALWAYSDRAW) {
           this->transverseGroupWMO(i, false, cameraVec4, headOfPyramidLocal1, inverseTransposeModelMat,
                                    transverseVisitedGroups,
                                    transverseVisitedPortals, frustumPlanes1, 0, m2RenderedThisFrame);
       } else if ((mainGeom->groups[i].flags.EXTERIOR) > 0) { //exterior
            if (this->groupObjects[i]->checkGroupFrustum(cameraVec4, frustumPlanes, frustumPoints, wmoM2Candidates)) {
                this->exteriorPortals.push_back({
                                                    groupId: i,
                                                    portalIndex : -1,
#ifndef CULLED_NO_PORTAL_DRAWING
                                                    portalVertices: {},
#endif
                                                    frustumPlanes: frustumPlanes,
                                                    level : 0});
                this->transverseGroupWMO(i, false, cameraVec4, headOfPyramidLocal1, inverseTransposeModelMat,
                                         transverseVisitedGroups,
                                         transverseVisitedPortals, frustumPlanes1, 0, m2RenderedThisFrame);
            }
        }
    }

    for (M2Object *m2Object : wmoM2Candidates) {
        if (m2Object == nullptr) continue;
        if (m2RenderedThisFrame.count(m2Object) > 0) continue;

        bool result = m2Object->checkFrustumCulling(cameraVec4, frustumPlanes, frustumPoints);
        if (result) m2RenderedThisFrame.insert(m2Object);
    };

    bool atLeastOneIsDrawn = false;
    for (int i = 0; i < groupObjects.size(); i++) {
        if (transverseVisitedGroups[i]) atLeastOneIsDrawn = true;
        drawGroupWMO[i] = transverseVisitedGroups[i];
    }

    return atLeastOneIsDrawn;
}

void WmoObject::checkGroupDoodads(int groupId, mathfu::vec4 &cameraVec4,
                                  std::vector<mathfu::vec4> &frustumPlane, int level,
                                  std::unordered_set<M2Object *> &m2ObjectSet) {
    WmoGroupObject *groupWmoObject = groupObjects[groupId];
    if (groupWmoObject != nullptr && groupWmoObject->getIsLoaded()) {
        const std::vector <M2Object *> *doodads = groupWmoObject->getDoodads();

        for (int j = 0; j < doodads->size(); j++) {
            M2Object *mdxObject = doodads->at(j);
            if (!mdxObject) continue;
            if (m2ObjectSet.count(mdxObject) > 0) continue;

            mdxObject->setUseLocalLighting(!groupWmoObject->getDontUseLocalLightingForM2());

            bool inFrustum = true;
            inFrustum = inFrustum && mdxObject->checkFrustumCulling(cameraVec4, frustumPlane, {});

            if (inFrustum) {
                m2ObjectSet.insert(mdxObject);
            }
        }
    }
}

void WmoObject::transverseGroupWMO(
                                int groupId,
                                bool fromInterior,
                                mathfu::vec4 &cameraVec4,
                                mathfu::vec4 &cameraLocal,
                                mathfu::mat4 &inverseTransposeModelMat,
                                std::vector<bool> &transverseVisitedGroups,
                                std::vector<bool> &transverseVisitedPortals,
                                std::vector<mathfu::vec4> &localFrustumPlanes,
                                int level,
                                std::unordered_set<M2Object *> &m2ObjectSet) {

    transverseVisitedGroups[groupId] = true;

    if (level > 8) return;

    if (!groupObjects[groupId]->getIsLoaded()) {
        //The group has not been loaded yet
        return ;
    }

    //Transform local planes into world planes to use with frustum culling of M2Objects
    std::vector<mathfu::vec4> frustumPlanes;

    std::transform(localFrustumPlanes.begin(), localFrustumPlanes.end(),
        std::back_inserter(frustumPlanes),
        [&](mathfu::vec4 p) -> mathfu::vec4 {
            return inverseTransposeModelMat * p;
        }
    );

    this->checkGroupDoodads(groupId, cameraVec4, frustumPlanes, level, m2ObjectSet);

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
        //if (!fromInterior && (mainGeom->groups[nextGroup].flags & 0x2000) == 0) continue;


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

        if (!visible) continue;

        transverseVisitedPortals[relation->portal_index] = true;

        int lastFrustumPlanesLen = localFrustumPlanes.size();

        //3. Construct frustum planes for this portal

        std::vector<mathfu::vec4> thisPortalPlanes;
        bool flip = (relation->side > 0);

        int portalCnt = portalVerticesVec.size();
        for (int i = 0; i < portalCnt; ++i) {
            int i2 = (i + 1) % portalCnt;

            mathfu::vec4 n = MathHelper::createPlaneFromEyeAndVertexes(cameraLocal.xyz(), portalVerticesVec[i], portalVerticesVec[i2]);

            if (flip) {
                n *= -1.0f;
            }

            thisPortalPlanes.push_back(n);
        }
        //frustumPlanes.push(thisPortalPlanes);

        /*var nearPlane = vec4.fromValues(portalInfo.plane.x, portalInfo.plane.y, portalInfo.plane.z, portalInfo.plane.w)
        if (!flip) {
            vec4.scale(nearPlane, nearPlane, -1)
        }
        thisPortalPlanes.push(nearPlane);
          */
        //thisPortalPlanes.push(frustumPlanes[frustumPlanes.length-2]);
        //thisPortalPlanes.push(frustumPlanes[frustumPlanes.length-1]);

        //5. Traverse next
        if ((mainGeom->groups[nextGroup].flags.INTERIOR) > 0) {
            //5.1 The portal is into interior wmo group. So go on.
            this->interiorPortals.push_back({
                                                groupId: nextGroup,
                                                portalIndex : relation->portal_index,
#ifndef CULLED_NO_PORTAL_DRAWING
                                                portalVertices: portalVerticesVec,
#endif
                                                frustumPlanes: thisPortalPlanes,
                                                level : level+1});
            this->transverseGroupWMO(nextGroup, fromInterior, cameraVec4, cameraLocal, inverseTransposeModelMat, transverseVisitedGroups,
                                     transverseVisitedPortals, thisPortalPlanes, level+1, m2ObjectSet);
        } else if (fromInterior) {
            //5.2 The portal is from interior into exterior wmo group.
            //Make sense to add only if whole traversing process started from interior
            this->exteriorPortals.push_back({
                                                groupId: nextGroup,
                                                portalIndex : relation->portal_index,
#ifndef CULLED_NO_PORTAL_DRAWING
                                                portalVertices: portalVerticesVec,
#endif
                                                frustumPlanes: thisPortalPlanes,
                                                level : level+1});
            this->transverseGroupWMO(nextGroup, false, cameraVec4, cameraLocal, inverseTransposeModelMat,
                                     transverseVisitedGroups,
                                     transverseVisitedPortals, thisPortalPlanes, level+1, m2ObjectSet);
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
            SMOGroupInfo *groupInfo = &this->mainGeom->groups[candidate->groupId];
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

