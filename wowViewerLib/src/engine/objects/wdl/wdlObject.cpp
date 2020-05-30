//
// Created by deamon on 11.01.18.
//

#include "wdlObject.h"

bool WdlObject::checkFrustumCulling(mathfu::vec4 &cameraPos, std::vector<mathfu::vec4> &frustumPlanes,
                                    std::vector<mathfu::vec3> &frustumPoints, std::vector<mathfu::vec3> &hullLines,
                                    mathfu::mat4 &lookAtMat4, std::vector<std::shared_ptr<M2Object>> &m2ObjectsCandidates,
                                    std::vector<std::shared_ptr<WmoObject>> &wmoCandidates) {
    if (!this->m_loaded) {
        if (m_wdlFile->getStatus() == FileStatus::FSLoaded) {
            this->loadingFinished();
            m_loaded = true;
        } else {
            return false;
        }
    }

    for (auto m2Object : m2Objects) {
        m2ObjectsCandidates.push_back(m2Object);
    }

    for (auto wmoObject : wmoObjects) {
        wmoCandidates.push_back(wmoObject);
    }

    for (auto m2Object : skySceneObjects) {
        m2ObjectsCandidates.push_back(m2Object);
    }

    return false;
}

void WdlObject::loadM2s() {
    int offset = 0;
    int length = m_wdlFile->doodadDefObj_len;
    m2Objects = std::vector<std::shared_ptr<M2Object>>(m_wdlFile->doodadDefObj_len);
    for (int j = 0, i = offset; i < offset+length; i++, j++) {
        //1. Get filename
        SMDoodadDef &doodadDef = m_wdlFile->doodadDefObj[i];
        if (doodadDef.flags.mddf_entry_is_filedata_id) {
            //2. Get model
            int fileDataId = doodadDef.nameId;
            m2Objects[j] = m_mapApi->getM2Object(fileDataId, doodadDef);
        } else {
            std::cout << "WDL M2 do not have mddf_entry_is_filedata_id flag!!!" << std::endl;
        }
    }

    //LoadSkyObjects
    if (m_wdlFile->m_msso_len > 0) {
        for (int i = 0; i < m_wdlFile->m_msso_len; i++) {
            auto &msso_rec = m_wdlFile->m_msso[i];


            auto m2Object = std::make_shared<M2Object>(m_api, false, false);
            m2Object->setLoadParams(0, {}, {});
            m2Object->setModelFileId(msso_rec.fileDataID);
            std::cout << "fileDataID = " << msso_rec.fileDataID << std::endl;
            std::cout << "translateVec.x = " << msso_rec.translateVec.x << std::endl;
            std::cout << "translateVec.y = " << msso_rec.translateVec.y << std::endl;
            std::cout << "translateVec.z = " << msso_rec.translateVec.z << std::endl;
            std::cout << "rotationInRads.x = " << msso_rec.rotationInRads.x << std::endl;
            std::cout << "rotationInRads.y = " << msso_rec.rotationInRads.y << std::endl;
            std::cout << "rotationInRads.z = " << msso_rec.rotationInRads.z << std::endl;
            std::cout << "scale = " << msso_rec.scale << std::endl;

            auto rotationMatrix = MathHelper::RotationZ(msso_rec.rotationInRads.z - M_PI_2);
            rotationMatrix *= MathHelper::RotationY(msso_rec.rotationInRads.x);
            rotationMatrix *= MathHelper::RotationX(msso_rec.rotationInRads.y);

//            auto quat = mathfu::quat::FromMatrix(rotationMatrix);
//            auto rotationMatrix1 = quat.ToMatrix4();

            m2Object->createPlacementMatrix(
                mathfu::vec3(msso_rec.translateVec.x, msso_rec.translateVec.y, msso_rec.translateVec.z),
                0,
                mathfu::vec3(msso_rec.scale, msso_rec.scale, msso_rec.scale),
                &rotationMatrix);
            m2Object->calcWorldPosition();
            m2Object->setAlwaysDraw(true);

            skySceneObjects.push_back(m2Object);
        }
    }

}

void WdlObject::loadWmos() {
    uint32_t offset = 0;
    int32_t length = m_wdlFile->mapObjDefObj_len;

    wmoObjects = std::vector<std::shared_ptr<WmoObject>>(length);
    for (int j = 0, i = offset; i < offset + length; i++, j++) {
        //1. Get filename

        auto &mapDef = m_wdlFile->mapObjDefObj[i];

        int fileDataId = mapDef.nameId;
        wmoObjects[j] = m_mapApi->getWmoObject(fileDataId, mapDef);

//        std::cout << "wmo filename = "<< fileName << std::endl;

        //2. Get model

    }

}

void WdlObject::loadingFinished() {
    this->loadM2s();
    this->loadWmos();
}

WdlObject::WdlObject(ApiContainer *api, std::string &wdlFileName) {
    m_api = api;
    m_wdlFile = m_api->cacheStorage->getWdlFileCache()->get(wdlFileName);
}

WdlObject::WdlObject(ApiContainer *api, int wdlFileDataId) {
    m_api = api;
    m_wdlFile = m_api->cacheStorage->getWdlFileCache()->getFileId(wdlFileDataId);
}