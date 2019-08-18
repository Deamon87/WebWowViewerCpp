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
            'SKPD',
            {
                [](SkelFile &file, ChunkData &chunkData) {
                    file.m_skpd_len = chunkData.chunkLen;
                    chunkData.readValue(file.m_skpd);
                },
            }
        }
    }
};

void SkelFile::process(const std::vector<unsigned char> &animFile, const std::string &fileName) {
    m_skelFile = std::vector<uint8_t>(animFile);
    CChunkFileReader reader(m_skelFile);
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

    m_loaded = true;

    for (int i =0; i < this->m_sks1->sequences.size; i++) {
        if(this->m_sks1->sequences.getElement(i)->id == 804) {
            debuglog("KU!")
        }
    }

    if (m_postLoadFunction != nullptr) {
        m_postLoadFunction();
    }
}
