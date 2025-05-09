//
// Created by deamon on 11.01.18.
//

#include "wdlObject.h"

bool WdlObject::checkFrustumCulling(const MathHelper::FrustumCullingData &frustumData,
                                    mathfu::vec4 &cameraPos,
                                    M2ObjectListContainer &m2ObjectsCandidates,
                                    WMOListContainer &wmoCandidates) {
    if (!this->m_loaded) {
        if (m_wdlFile->getStatus() == FileStatus::FSLoaded) {
            this->loadingFinished();
            m_loaded = true;
        } else {
            return false;
        }
    }

    for (const auto &m2Object : m2Objects) {
        m2ObjectsCandidates.addCandidate(m2Object);
    }
    for (const auto &wmoObject : wmoObjects) {
        wmoCandidates.addCand(wmoObject);
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
    if (m_wdlFile->m_mssn_len > 0) {
        for (int i = 0; i < m_wdlFile->m_mssn_len; i++) {
            auto &mssn_rec = m_wdlFile->m_mssn[i];
            SkyObjectScene skyObjectScene;

            for (int m = mssn_rec.msscIndex; m < mssn_rec.msscIndex+mssn_rec.msscRecordsNum; m++) {
                auto &mssc_rec = m_wdlFile->m_mssc[m];
                SkyObjectCondition condition;
                condition.conditionType = mssc_rec.conditionType;
                condition.conditionValue = mssc_rec.conditionValue;
                skyObjectScene.conditions.push_back(condition);
            }

            for (int l = mssn_rec.mssoIndex; l < mssn_rec.mssoIndex+mssn_rec.mssoRecordsNum; l++) {
                auto &msso_rec = m_wdlFile->m_msso[l];

                uint64_t mssf_val = 0;
                if (msso_rec.flags.unk_0x1) {
                    mssf_val = m_wdlFile->m_mssf[msso_rec.mssf_index];
                }

                if (m_wdlFile->m_msli) {
                    auto msliIndex = m_wdlFile->m_msli[l];
                    if (msliIndex != -1) {
                        auto &msld = m_wdlFile->m_msld[msliIndex];
                        //std::cout << msld.timeEnd0 << " " << mssf_val;
                    }
                }

                auto m2Object = m2Factory->createObject(m_api, false, false);
                m2Object->setLoadParams(0, {}, {});
                m2Object->setModelFileId(msso_rec.fileDataID);

                constexpr float degreeToRad = M_PI/180.0f;

                auto rotationMatrix = MathHelper::RotationZ(msso_rec.rotationInDegree.z*degreeToRad);
                rotationMatrix *= MathHelper::RotationY(msso_rec.rotationInDegree.y*degreeToRad);
                rotationMatrix *= MathHelper::RotationX(msso_rec.rotationInDegree.x*degreeToRad);

                m2Object->createPlacementMatrix(
                    mathfu::vec3(msso_rec.translateVec.x, msso_rec.translateVec.y, msso_rec.translateVec.z),
                    0,
                    mathfu::vec3(msso_rec.scale, msso_rec.scale, msso_rec.scale),
                    &rotationMatrix);
                m2Object->calcWorldPosition();
                m2Object->setAlwaysDraw(true);

                auto &skyModel = skyObjectScene.skyModels.emplace_back();
                skyModel.m_model = m2Object;
                skyModel.animateWithTimeOfDay = msso_rec.flags.animateWithTimeOfDay;
            }
            skyScenes.push_back(skyObjectScene);
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

WdlObject::WdlObject(HApiContainer api, std::string &wdlFileName) {
    m_api = api;
    m_wdlFile = m_api->cacheStorage->getWdlFileCache()->get(wdlFileName);
}

WdlObject::WdlObject(HApiContainer api, int wdlFileDataId) {
    m_api = api;
    m_wdlFile = m_api->cacheStorage->getWdlFileCache()->getFileId(wdlFileDataId);
}

bool hasId(const std::vector<IdAndBlend> &vec, int id) {
    return std::any_of(vec.begin(), vec.end(), [id](const IdAndBlend &val) {
        return val.id == id;
    });
}

bool hasId(const std::vector<IdAndBlendAndPriority> &vec, int id) {
    return std::any_of(vec.begin(), vec.end(), [id](const IdAndBlendAndPriority &val) {
        return val.id == id;
    });
}

void WdlObject::checkSkyScenes(const StateForConditions &state,
                               M2ObjectListContainer &m2ObjectsCandidates,
                               const mathfu::vec4 &cameraPos,
                               const MathHelper::FrustumCullingData &frustumData
                               ) {
    for (auto &skyScene : skyScenes) {
        bool conditionPassed = false;

        for (auto &condition : skyScene.conditions) {
            switch (condition.conditionType) {
                case 1 : {
                    if ((state.currentAreaId == condition.conditionValue) ||
                        (state.currentParentAreaId == condition.conditionValue))
                        conditionPassed = true;
                    break;
                }
                case 2 : {
                    conditionPassed = hasId(state.currentLightParams, condition.conditionValue);
                    break;
                }
                case 3 : {
                    conditionPassed = hasId(state.currentSkyboxIds, condition.conditionValue);
                    break;
                }
                case 5 : {
                    conditionPassed = hasId(state.currentZoneLights, condition.conditionValue);
                    break;
                }
                default:
                    std::cout << "Unk condition " << (int) condition.conditionType << std::endl;
            }

            if (conditionPassed)
                break;
        }

//        conditionPassed = true;

        if (conditionPassed) {
            auto const config = m_api->getConfig();
            for (const auto &skyModel : skyScene.skyModels) {
                auto const &m2Object = skyModel.m_model;

                if(skyModel.animateWithTimeOfDay) {
                    m2Object->setOverrideAnimationPerc(config->currentTime / 2880.0f, true);
                }
                m2ObjectsCandidates.addToDraw(m2Object);
            }
        }
    }
}
