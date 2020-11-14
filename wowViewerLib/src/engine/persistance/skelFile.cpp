//
// Created by Deamon on 8/6/2019.
//

#include "skelFile.h"

chunkDef<SkelFile> SkelFile::skelFileTable = {
    [](SkelFile &file, ChunkData &chunkData) {},
    {
        {
            '1LKS',
            {
                [](SkelFile &file, ChunkData &chunkData) {
                    file.m_skl1_len = chunkData.chunkLen;
                    chunkData.readValue(file.m_skl1);
                },
            }
        },
        {
            '1AKS',
            {
                [](SkelFile &file, ChunkData &chunkData) {
                    file.m_ska1_len = chunkData.chunkLen;
                    chunkData.readValue(file.m_ska1);
                },
            }
        },
        {
            '1BKS',
            {
                [](SkelFile &file, ChunkData &chunkData) {
                    file.m_skb1_len = chunkData.chunkLen;
                    chunkData.readValue(file.m_skb1);
                },
            },
        },
        {
            '1SKS',
            {
                [](SkelFile &file, ChunkData &chunkData) {
                    file.m_sks1_len = chunkData.chunkLen;
                    chunkData.readValue(file.m_sks1);
                },
            }
        },
        {
            'DPKS',
            {
                [](SkelFile &file, ChunkData &chunkData) {
                    debuglog("Entered SKPD");
                    file.m_skpd_len = chunkData.chunkLen;
                    chunkData.readValue(file.m_skpd);
                },
            }
        },
        {
            'DIFA',
            {
                [](SkelFile &file, ChunkData &chunkData) {
                    debuglog("Entered AFID");
                    file.animationFileDataIDs =
                        std::vector<M2_AFID>(
                            (unsigned long) (chunkData.chunkLen / sizeof(M2_AFID)));

                    for (int i = 0; i < file.animationFileDataIDs.size(); i++) {
                        chunkData.readValue(file.animationFileDataIDs[i]);
                    }
                }
            }
        },
    }
};

void SkelFile::process(HFileContent skelFile, const std::string &fileName) {
    m_skelFile = skelFile;
    CChunkFileReader reader(*m_skelFile.get());
    reader.processFile(*this, &SkelFile::skelFileTable);

    if (this->m_ska1 != nullptr) {
        this->m_ska1->attachments.initM2Array(this->m_ska1);
        this->m_ska1->attachment_lookup_table.initM2Array(this->m_ska1);
    }

    if (this->m_skb1 != nullptr) {
        this->m_skb1->bones.initM2Array(this->m_skb1);
        this->m_skb1->key_bone_lookup.initM2Array(this->m_skb1);
    }

    if (this->m_sks1 != nullptr) {
        this->m_sks1->global_loops.initM2Array(this->m_sks1);
        this->m_sks1->sequences.initM2Array(this->m_sks1);
        this->m_sks1->sequence_lookups.initM2Array(this->m_sks1);
    }

    fsStatus = FileStatus::FSLoaded;

    initTracks(nullptr);

    if (m_postLoadFunction != nullptr) {
        m_postLoadFunction();
    }
}

void SkelFile:: initTracks(CSkelSequenceLoad *cSkelSequenceLoad) {
    M2Array<M2Sequence> *sequences = nullptr;
    if (this->m_sks1 != nullptr) {
        sequences = &this->m_sks1->sequences;
    }

    if (this->m_ska1 != nullptr) {
        CM2SequenceLoad *cm2SequenceLoad = nullptr;
        if (cSkelSequenceLoad != nullptr) {
            CM2SequenceLoad l_cm2SequenceLoad;
            l_cm2SequenceLoad.animationIndex = cSkelSequenceLoad->animationIndex;
            if (cSkelSequenceLoad->animFileAttDataBlob != nullptr) {
                l_cm2SequenceLoad.animFileDataBlob = cSkelSequenceLoad->animFileAttDataBlob;
            } else {
                l_cm2SequenceLoad.animFileDataBlob = cSkelSequenceLoad->animFileDataBlob;
            }
            cm2SequenceLoad = &l_cm2SequenceLoad;
        }

        initM2Attachment(this->m_ska1, &this->m_ska1->attachments, sequences, cm2SequenceLoad);
    }
    if (this->m_skb1 != nullptr) {
        CM2SequenceLoad *cm2SequenceLoad = nullptr;
        if (cSkelSequenceLoad != nullptr) {
            CM2SequenceLoad l_cm2SequenceLoad;
            l_cm2SequenceLoad.animationIndex = cSkelSequenceLoad->animationIndex;
            if (cSkelSequenceLoad->animFileBoneDataBlob != nullptr) {
                l_cm2SequenceLoad.animFileDataBlob = cSkelSequenceLoad->animFileBoneDataBlob;
            } else {
                l_cm2SequenceLoad.animFileDataBlob = cSkelSequenceLoad->animFileDataBlob;
            }
            cm2SequenceLoad = &l_cm2SequenceLoad;
        }

        initCompBones(this->m_skb1, &this->m_skb1->bones, sequences, cm2SequenceLoad);
    }
}


void SkelFile::loadLowPriority(HApiContainer m_api, uint32_t animationId, uint32_t variationId) {
    int animationIndex = findAnimationIndex(animationId, &m_sks1->sequence_lookups, &m_sks1->sequences);
    if (animationIndex < 0) return;

    AnimCacheRecord animCacheRecord;
    animCacheRecord.animationId = animationId;
    animCacheRecord.variationId = variationId;
    auto it = loadedAnimationMap.find(animCacheRecord);
    if (it != loadedAnimationMap.end()) return;


    while (m_sks1->sequences[animationIndex]->variationIndex != variationId) {
        animationIndex = m_sks1->sequences[animationIndex]->variationNext;

        if (animationIndex <= 0) return;
    }

    if ((m_sks1->sequences[animationIndex]->flags & 0x20) > 0) return;

    int animationFileDataId = 0;
    if (animationFileDataIDs.size() > 0) {
        for (const auto &record : animationFileDataIDs) {
            if (record.anim_id == animationId && record.sub_anim_id == variationId) {
                animationFileDataId = record.file_id;
            }
        }
    }
    std::shared_ptr<AnimFile> animFile = nullptr;
    if (animationFileDataId != 0) {
        animFile = m_api->cacheStorage->getAnimCache()->getFileId(animationFileDataId);
    }
//    else if (!useFileId) {
//        char buffer[1024];
//        std::snprintf(buffer, 1024, "%s%04d-%02d.anim", m_nameTemplate.c_str(), animationId, variationId);
//
//        animFile = m_api->cacheStorage->getAnimCache()->get(buffer);
//    }
    if (animFile == nullptr) return;

    animFile->setPostLoad([this, animationIndex, animFile]() -> void {
        CSkelSequenceLoad cSkelSequenceLoad;
        cSkelSequenceLoad.animFileDataBlob = animFile->m_animFileDataBlob;
        cSkelSequenceLoad.animFileBoneDataBlob = animFile->m_animFileBoneAnimDataBlob;
        cSkelSequenceLoad.animFileAttDataBlob = animFile->m_animFileAttachAnimDataBlob;
        cSkelSequenceLoad.animationIndex = animationIndex;

        initTracks(&cSkelSequenceLoad);
        m_sks1->sequences[animationIndex]->flags |= 0x20;
    });
    loadedAnimationMap[animCacheRecord] = animFile;
}