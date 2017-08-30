//
// Created by deamon on 03.07.17.
//

#include "wmoObject.h"
#include "../algorithms/mathHelper.h"

std::string WmoObject::getTextureName(int index) {
    return std::__cxx11::string();
}

//M2Object &WmoObject::getDoodad(int index) {
//    return M2Object;
//}

void WmoObject::startLoading() {
    if (!m_loading) {
        m_loading = true;

        Cache<WmoMainGeom> *wmoGeomCache = m_api->getWmoMainCache();

        mainGeom = wmoGeomCache->get(m_modelName);

    }
}

bool WmoObject::checkFrustumCulling (mathfu::vec4 &cameraPos, std::vector<mathfu::vec4> &frustumPlanes, std::vector<mathfu::vec3> &frustumPoints,
                                     std::set<M2Object*> &m2RenderedThisFrame) {
    if (m_loaded) {
        return true;
    }

    bool result = false;
    CAaBox &aabb = this->m_bbox;

    //1. Check if camera position is inside Bounding Box
    if (
        cameraPos[0] > aabb.min.z && cameraPos[0] < aabb.max.x &&
        cameraPos[1] > aabb.min.y && cameraPos[1] < aabb.max.y &&
        cameraPos[2] > aabb.min.z && cameraPos[2] < aabb.max.z
        ) return true;

    //2. Check aabb is inside camera frustum
    result = MathHelper::checkFrustum(frustumPlanes, aabb, frustumPoints);

    std::set<M2Object*> wmoM2Candidates;
    if (result) {
        //1. Calculate visibility for groups
        for (int i = 0; i < this->groupObjects.size(); i++) {
            this->groupObjects[i]->checkGroupFrustum(cameraPos, frustumPlanes, frustumPoints, wmoM2Candidates);
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
    float TILESIZE = 533.333333333;

    float posx = 32*TILESIZE - mapObjDef.position.x;
    float posy = mapObjDef.position.y;
    float posz = 32*TILESIZE - mapObjDef.position.z;


    mathfu::mat4 placementMatrix = mathfu::mat4::Identity();

    placementMatrix *= MathHelper::RotationX(toRadian(90));
    placementMatrix *= MathHelper::RotationY(toRadian(90));


    placementMatrix *= mathfu::mat4::FromTranslationVector(mathfu::vec3(posx, posy, posz));
//    placementMatrix *= mathfu::mat4::FromTranslationVector(mathfu::vec3(posz, -posx, -posy));

    placementMatrix *= MathHelper::RotationY(toRadian(mapObjDef.rotation.y-270));
    placementMatrix *= MathHelper::RotationZ(toRadian(-mapObjDef.rotation.x));
    placementMatrix *= MathHelper::RotationX(toRadian(mapObjDef.rotation.z-90));

    mathfu::mat4 placementInvertMatrix = placementMatrix.Inverse();

    m_placementInvertMatrix = placementInvertMatrix;
    m_placementMatrix = placementMatrix;

    createBB(mapObjDef.extents);
}

void WmoObject::createGroupObjects(){
    groupObjects = std::vector<WmoGroupObject*>(mainGeom->groupsLen, nullptr);

    std::string nameTemplate = m_modelName.substr(0, m_modelName.find_last_of("."));
    for(int i = 0; i < mainGeom->groupsLen; i++) {
        std::string numStr = std::to_string(i);
        for (int j = numStr.size(); j < 3; j++) numStr = '0'+numStr;

        std::string groupFilename = nameTemplate + "_" + numStr + ".wmo";


        groupObjects[i] = new WmoGroupObject(this->m_placementMatrix, m_api, groupFilename, mainGeom->groups[i]);
    }
}

void WmoObject::draw(){
    if (!m_loaded) {
        if (mainGeom != nullptr && mainGeom->getIsLoaded()){
            m_loaded = true;
            m_loading = false;

            this->createGroupObjects();
        } else {
            this->startLoading();
        }

        return;
    }
    auto wmoShader = m_api->getWmoShader();

    glUniformMatrix4fv(wmoShader->getUnf("uPlacementMat"), 1, GL_FALSE, &this->m_placementMatrix[0]);

    for (int i= 0; i < groupObjects.size(); i++) {
        if(groupObjects[i] != nullptr) {
            groupObjects[i]->draw(mainGeom->materials, m_getTextureFunc);
        }
    }
}

void WmoObject::setLoadingParam(std::string modelName, SMMapObjDef &mapObjDef) {
    m_modelName = modelName;

    //this->m_placementMatrix = mathfu::mat4::Identity();
    createPlacementMatrix(mapObjDef);
}

BlpTexture &WmoObject::getTexture(int textureId) {
    std::string materialTexture((char *)&mainGeom->textureNamesField[textureId]);

    //TODO: cache Textures used in WMO
    return *m_api->getTextureCache()->get(materialTexture);
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
