//
// Created by Deamon on 9/16/2020.
//

#include "CBoneMasterData.h"
#include "../../../geometry/m2Geom.h"

CBoneMasterData::CBoneMasterData(HM2Geom m2Geom, HSkelGeom skelGeom, HSkelGeom parentSkelGeom) {
    m_m2Geom = m2Geom;
    m_skelGeom = skelGeom;
    m_parentSkelGeom = parentSkelGeom;

    if (skelGeom == nullptr) {
        skelData.m_m2CompBones = &m_m2Geom->m_m2Data->bones;
        skelData.m_key_bone_lookup = &m_m2Geom->m_m2Data->key_bone_lookup;

        skelData.m_globalSequences = &m_m2Geom->m_m2Data->global_loops;
        skelData.m_sequences = &m_m2Geom->m_m2Data->sequences;
        skelData.m_sequence_lookups = &m_m2Geom->m_m2Data->sequence_lookups;

        skelData.m_attachments = &m_m2Geom->m_m2Data->attachments;
        skelData.m_attachment_lookup_table = &m_m2Geom->m_m2Data->attachment_lookup_table;
    } else {
        skelData.m_m2CompBones = &skelGeom->m_skb1->bones;
        skelData.m_key_bone_lookup = &skelGeom->m_skb1->key_bone_lookup;

        skelData.m_globalSequences = &skelGeom->m_sks1->global_loops;
        skelData.m_sequences = &skelGeom->m_sks1->sequences;
        skelData.m_sequence_lookups = &skelGeom->m_sks1->sequence_lookups;

        skelData.m_attachments = &skelGeom->m_ska1->attachments;
        skelData.m_attachment_lookup_table = &skelGeom->m_ska1->attachment_lookup_table;

        if (parentSkelGeom != nullptr) {
            parentSkelData.m_m2CompBones = &parentSkelGeom->m_skb1->bones;
            parentSkelData.m_key_bone_lookup = &parentSkelGeom->m_skb1->key_bone_lookup;

            parentSkelData.m_globalSequences = &parentSkelGeom->m_sks1->global_loops;
            parentSkelData.m_sequences = &parentSkelGeom->m_sks1->sequences;
            parentSkelData.m_sequence_lookups = &parentSkelGeom->m_sks1->sequence_lookups;

            parentSkelData.m_attachments = &parentSkelGeom->m_ska1->attachments;
            parentSkelData.m_attachment_lookup_table = &parentSkelGeom->m_ska1->attachment_lookup_table;
        }
    }
}
