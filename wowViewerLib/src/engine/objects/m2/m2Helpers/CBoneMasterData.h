//
// Created by Deamon on 9/16/2020.
//

#ifndef AWEBWOWVIEWERCPP_CBONEMASTERDATA_H
#define AWEBWOWVIEWERCPP_CBONEMASTERDATA_H


#include "../../../persistance/header/M2FileHeader.h"
#include "../../../wowCommonClasses.h"
#include "../../../ApiContainer.h"

struct SkelData {
    M2Array<M2CompBone> *m_m2CompBones = nullptr;
    M2Array<int16_t> *m_key_bone_lookup = nullptr;

    M2Array<M2Loop> *m_globalSequences = nullptr;
    M2Array<M2Sequence> *m_sequences = nullptr;
    M2Array<int16_t> *m_sequence_lookups = nullptr;

    M2Array<M2Attachment> *m_attachments = nullptr;
    M2Array<uint16_t> *m_attachment_lookup_table = nullptr;
};

class CBoneMasterData {
private:
    HM2Geom m_m2Geom = nullptr;
    HSkelGeom m_skelGeom = nullptr;
    HSkelGeom m_parentSkelGeom = nullptr;

    SkelData skelData;
    SkelData parentSkelData;
public:
    CBoneMasterData(HM2Geom m2Geom, HSkelGeom skelGeom, HSkelGeom parentSkelGeom);

    SkelData *getSkelData() {
        return &skelData;
    }
    SkelData *getParentSkelData() {
        if (m_parentSkelGeom != nullptr) {
            return &parentSkelData;
        } else {
            return nullptr;
        }
    }

    HM2Geom getM2Geom() {
        return m_m2Geom;
    }

    void loadLowPriority(ApiContainer *m_api, uint32_t animationId, uint32_t variationId, bool foundInParentAnim) {
        if (foundInParentAnim && m_parentSkelGeom != nullptr) {
            m_parentSkelGeom->loadLowPriority(m_api, animationId, variationId);
        } else if (m_skelGeom != nullptr) {
            m_skelGeom->loadLowPriority(m_api, animationId, variationId);
        } else {
            m_m2Geom->loadLowPriority(m_api, animationId, variationId);
        }
    }
};



#endif //AWEBWOWVIEWERCPP_CBONEMASTERDATA_H
